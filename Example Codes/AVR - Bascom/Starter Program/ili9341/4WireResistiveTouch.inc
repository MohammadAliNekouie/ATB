' 4-Wire Resistive Touch Screen Library 1.1, Nov. 2014
' http://mat.midlight.eu/wiki/index.php?title=Touch_Screen_Library
' based on AVR341: Four and Five-wire touch screen controller

' Changelog:
'   1.1:  - Increased max. delta to 5000 pixels/sec to match handwriting speed
'         - fixed a little bug in calculating the rotated coordinates


' I/O states
'                    Xp          Xn          Yp          Yn
'              +-----------------------------------------------
' Standby      |  Pullup/INT     HiZ         HiZ         GND
' Pause        |     HiZ         HiZ         HiZ         HiZ
' X-Coordinate |     GND         VCC         HiZ       HiZ/ADC
' Y-Coordinate |     HiZ       HiZ/ADC       GND         VCC
' Z1           |     GND       HiZ/ADC       HiZ         VCC
' Z2           |     GND         HiZ       HiZ/ADC       VCC

$nocompile
#if Not Varexist( "False")
   Const False = 0
   Const True = 1
#endif


'### Settings #######################################################

' width of the underlying screen [pixels]
#if Not Varexist( "Touch_screen_width")
   Const Touch_screen_width = 240
#endif

' height of the underlying screen [pixels]
#if Not Varexist( "Touch_screen_height")
   Const Touch_screen_height = 320
#endif

#if Varexist( "Ili9341")
   Const Touch_enable_rotation = True
#endif

' if the display supports rotation, then the touch coordinates need to be rotated too
#if Not Varexist( "Touch_enable_rotation")
   Const Touch_enable_rotation = False
#endif

' measure the touch contact resistance [False: Off, 1: Method 1, 2: Method 2]
#if Not Varexist( "Touch_measure_resistance")
   Const Touch_measure_resistance = False
#endif

' if contact resistance measurement is enabled, the touch ITO layer resistances need to be known [ohms]
#if Touch_measure_resistance >= 1 And Not Varexist( "Touch_resistance_x")
   Const Touch_resistance_x = 200
#endif
#if Touch_measure_resistance = 1 And Not Varexist( "Touch_resistance_y")
   Const Touch_resistance_y = 600
#endif

' coordinates are valid if measured contact resistance is below
#if Touch_measure_resistance >= 1 And Not Varexist( "Touch_max_resistance")
   Const Touch_max_resistance = 1000
#endif

' control port for touch screen (ATXmega: 4 I/Os, ATmega: Port of X+ INT pin)
#if Not Varexist( "Touch_ctrl_port")
   Const Touch_ctrl_port = Varptr( "Portb")
#endif

' control port pin connections
#if Not Varexist( "Touch_pin_xp")
   Const Touch_pin_xp = 0
#endif
#if Not Varexist( "Touch_pin_xn")
   Const Touch_pin_xn = 1
#endif
#if Not Varexist( "Touch_pin_yp")
   Const Touch_pin_yp = 2
#endif
#if Not Varexist( "Touch_pin_yn")
   Const Touch_pin_yn = 3
#endif

' adc port pin connections (XMega only)
#if Not Varexist( "Touch_pin_xn_adc") And _xmega = True
   Const Touch_pin_xn_adc = 4
#endif
#if Not Varexist( "Touch_pin_yn_adc") And _xmega = True
   Const Touch_pin_yn_adc = 5
#endif
#if Not Varexist( "Touch_pin_yp_adc") And _xmega = True And Touch_measure_resistance >= 2
   Const Touch_pin_yp_adc = 6
#endif

' measurements per sample
#if Not Varexist( "Touch_samples")
   Const Touch_samples = 5
#endif

' new coordinate samples [coordinates/sec]
#if Not Varexist( "Touch_updaterate")
   Const Touch_updaterate = 100
#endif

' no. of samples to be sure the touch screen has been released
#if Not Varexist( "Touch_release_count")
   Const Touch_release_count = 5
#endif

' they bounce badly [sec]
#if Not Varexist( "Touch_time_debounce")
   Const Touch_time_debounce = 0.05
#endif

' depends on RC time constant of touch screen, 0.2ms should work for most [sec]
#if Not Varexist( "Touch_time_switch_measurement")
   Const Touch_time_switch_measurement = 0.0002
#endif

' click/drag detection delta [pixels]
#if Not Varexist( "Touch_click_delta")
   Const Touch_click_delta = 15
#endif

' max speed [pixels/sec]
#if Not Varexist( "Touch_max_delta")
   Const Touch_max_delta = 5000
#endif


'### Internal Constants #############################################

Const Touch_samples_median =(touch_samples + 1) / 2

' Timing calculations
Const Touch_time_pause_sample = 1 /(touch_updaterate * Touch_samples) -((2 + Touch_measure_resistance) * Touch_time_switch_measurement)
#if _xmega = True
   Const Touch_timer_register = 65536                       ' 16 bit timer/counter
#else
   Const Touch_timer_register = 256                         ' 8 bit timer/counter
#endif
#if Touch_time_pause_sample > Touch_time_switch_measurement ' determine longest timing
   Const Touch_time_max = Touch_time_pause_sample
#else
   Const Touch_time_max = Touch_time_switch_measurement
#endif
#if(touch_time_max * _xtal) / 1 < Touch_timer_register      ' get smallest possible prescaler for longest timing
   Const Touch_timer_prescale = 1
#elseif(touch_time_max * _xtal) / 2 < Touch_timer_register And _xmega = True
   Const Touch_timer_prescale = 2
#elseif(touch_time_max * _xtal) / 4 < Touch_timer_register And _xmega = True
   Const Touch_timer_prescale = 4
#elseif(touch_time_max * _xtal) / 8 < Touch_timer_register
   Const Touch_timer_prescale = 8
#elseif(touch_time_max * _xtal) / 64 < Touch_timer_register
   Const Touch_timer_prescale = 64
#elseif(touch_time_max * _xtal) / 256 < Touch_timer_register
   Const Touch_timer_prescale = 256
#elseif(touch_time_max * _xtal) / 1024 < Touch_timer_register
   Const Touch_timer_prescale = 1024
#endif
Const Touch_timer_period = 1 /(_xtal / Touch_timer_prescale)       ' prescaled clock rate of timer
#if _xmega = True
   Const Touch_per_switch_measurement = Touch_time_switch_measurement / Touch_timer_period       ' timer clock cycles
   Const Touch_per_pause_sample = Touch_time_pause_sample / Touch_timer_period
#else
   Const Touch_per_switch_measurement =(touch_timer_register -1) -(touch_time_switch_measurement / Touch_timer_period)       ' timer clock cycles
   Const Touch_per_pause_sample =(touch_timer_register -1) -(touch_time_pause_sample / Touch_timer_period)
#endif
Const Touch_per_debounce = Int((touch_time_debounce /(1 / Touch_updaterate)) + .5)       ' in measurement cycles

' click/drag thresholds and max speed calculations
Const Touch_adc_max_delta_x =(touch_max_delta / Touch_updaterate) / Touch_screen_width
Const Touch_adc_max_delta_y =(touch_max_delta / Touch_updaterate) / Touch_screen_height
Const Touch_adc_click_delta_x = Touch_click_delta / Touch_screen_width
Const Touch_adc_click_delta_y = Touch_click_delta / Touch_screen_height

' Calibration coordinates
Const Touch_cal_marker_radius = Touch_screen_width * 0.0625 + .5       ' X = 240: Radius = 15
Const Touch_cal1_x = Touch_screen_width * .9 + .5
Const Touch_cal1_y = Touch_screen_height * .5 + .5
Const Touch_cal2_x = Touch_screen_width * .5 + .5
Const Touch_cal2_y = Touch_screen_height * .1 + .5
Const Touch_cal3_x = Touch_screen_width * .1 + .5
Const Touch_cal3_y = Touch_screen_height * .9 + .5

' State machine
Const Touch_state_debounce = 0
Const Touch_state_sample_x = 1
Const Touch_state_sample_y = 2
#if Touch_measure_resistance >= 1
   Const Touch_state_sample_z1 = 3
#endif
#if Touch_measure_resistance >= 2
   Const Touch_state_sample_z2 = 4
#endif
Const Touch_state_pause_sample = 5


'### Portability ####################################################

#if _xmega = True                                           ' # XMEGA #
   Const Touch_unused_mask = Not(2 ^ Touch_pin_xp + 2 ^ Touch_pin_xn + 2 ^ Touch_pin_yp + 2 ^ Touch_pin_yn)

   ' Control port registers
   Const Touch_ddr_ptr = Touch_ctrl_port - 4
   Const Touch_port_ptr = Touch_ddr_ptr + 4
   Const Touch_pin_ptr = Touch_ddr_ptr + 8
   Const Touch_intctrl_ptr = Touch_ddr_ptr + 9
   Const Touch_int0mask_ptr = Touch_ddr_ptr + 10
   Const Touch_intflags_ptr = Touch_ddr_ptr + 12
   Const Touch_pinctrl_yn_ptr = Touch_ddr_ptr + 16 + Touch_pin_xp

   ' standby/measurement
   Const Touch_standby_ddr = 2 ^ Touch_pin_yn
   Const Touch_standby_port = 0                             ' internal pullup
   Const Touch_standby_pinctrl = &B00011010                 ' totem pole, pullup, falling edge
   Const Touch_standby_intctrl = &B0000_00_01               ' low level interrupt
   Const Touch_standby_intmask = 2 ^ Touch_pin_xp           ' int source
   Const Touch_standby_intflags = &B00000001

   Const Touch_sample_pinctrl = &B00_000_000
   Const Touch_sample_intctrl = &B0000_00_00

   ' high-z (pause)
   Const Touch_high_z_ddr = 0
   Const Touch_high_z_port = 0

   ' measure x
   Const Touch_sample_x_ddr = 2 ^ Touch_pin_xp + 2 ^ Touch_pin_xn
   Const Touch_sample_x_port = 2 ^ Touch_pin_xn
   Const Touch_sample_x_adcmux = Touch_pin_yn_adc * 8

   ' measure y
   Const Touch_sample_y_ddr = 2 ^ Touch_pin_yp + 2 ^ Touch_pin_yn
   Const Touch_sample_y_port = 2 ^ Touch_pin_yn
   Const Touch_sample_y_adcmux = Touch_pin_xn_adc * 8

   ' measure z1
   #if Touch_measure_resistance >= 1
      Const Touch_sample_z1_ddr = 2 ^ Touch_pin_xp + 2 ^ Touch_pin_yn
      Const Touch_sample_z1_port = 2 ^ Touch_pin_yn
      Const Touch_sample_z1_adcmux = Touch_pin_xn_adc * 8
   #endif

   ' measure z2
   #if Touch_measure_resistance >= 2
      Const Touch_sample_z2_ddr = 2 ^ Touch_pin_xp + 2 ^ Touch_pin_yn
      Const Touch_sample_z2_port = 2 ^ Touch_pin_yn
      Const Touch_sample_z2_adcmux = Touch_pin_yp_adc * 8
   #endif

   Macro Touch_set_high_z
      !lds R24, touch_ddr_ptr
      !andi R24, Touch_unused_mask
      !ori R24, Touch_high_z_ddr
      !sts Touch_ddr_ptr , R24

      !lds R24, touch_port_ptr
      !andi R24, Touch_unused_mask
      !ori R24, Touch_high_z_port
      !sts Touch_port_ptr , R24
   End Macro

   Macro Touch_set_sample_x
      !lds R24, touch_ddr_ptr
      !andi R24, Touch_unused_mask
      !ori R24, Touch_sample_x_ddr
      !sts Touch_ddr_ptr , R24

      !lds R24, touch_port_ptr
      !andi R24, Touch_unused_mask
      !ori R24, Touch_sample_x_port
      !sts Touch_port_ptr , R24
   End Macro

   Macro Touch_set_sample_y
      !lds R24, touch_ddr_ptr
      !andi R24, Touch_unused_mask
      !ori R24, Touch_sample_y_ddr
      !sts Touch_ddr_ptr , R24

      !lds R24, touch_port_ptr
      !andi R24, Touch_unused_mask
      !ori R24, Touch_sample_y_port
      !sts Touch_port_ptr , R24
   End Macro

   #if Touch_measure_resistance >= 1
      Macro Touch_set_sample_z1
         !lds R24, touch_ddr_ptr
         !andi R24, Touch_unused_mask
         !ori R24, Touch_sample_z1_ddr
         !sts Touch_ddr_ptr , R24

         !lds R24, touch_port_ptr
         !andi R24, Touch_unused_mask
         !ori R24, Touch_sample_z1_port
         !sts Touch_port_ptr , R24
      End Macro
   #endif

   #if Touch_measure_resistance >= 2
      Macro Touch_set_sample_z2
         !lds R24, touch_ddr_ptr
         !andi R24, Touch_unused_mask
         !ori R24, Touch_sample_z2_ddr
         !sts Touch_ddr_ptr , R24

         !lds R24, touch_port_ptr
         !andi R24, Touch_unused_mask
         !ori R24, Touch_sample_z2_port
         !sts Touch_port_ptr , R24
      End Macro
   #endif

   ' standby pin interrupt
   Macro Touch_init_standby
      !lds R24, touch_int0mask_ptr                          ' enable touch standby pin interrupt
      !andi R24, Touch_unused_mask
      !ori R24, Touch_standby_intmask
      !sts touch_int0mask_ptr , R24
   End Macro

   Macro Touch_enter_standby
      !lds R24, touch_ddr_ptr
      !andi R24, Touch_unused_mask
      !ori R24, Touch_standby_ddr
      !sts Touch_ddr_ptr , R24

      !lds R24, touch_port_ptr
      !andi R24, Touch_unused_mask
      !ori R24, Touch_standby_port
      !sts Touch_port_ptr , R24

      Out Touch_pinctrl_yn_ptr , Touch_standby_pinctrl
      Out Touch_intflags_ptr , Touch_standby_intflags
      Out Touch_intctrl_ptr , Touch_standby_intctrl
   End Macro

   Macro Touch_exit_standby
      Out Touch_intctrl_ptr , Touch_sample_intctrl
      Out Touch_pinctrl_yn_ptr , Touch_sample_pinctrl
   End Macro

   ' set interrupt for used port
   #if Varexist( "Porta")
      #if Varptr( "Touch_ctrl_port") = Varptr( "Porta")
         On Porta_int0 Touch_standby_isr
      #endif
   #endif
   #if Varexist( "Portb")
      #if Varptr( "Touch_ctrl_port") = Varptr( "Portb")
         On Portb_int0 Touch_standby_isr
      #endif
   #endif
   #if Varexist( "Portc")
      #if Varptr( "Touch_ctrl_port") = Varptr( "Portc")
         On Portc_int0 Touch_standby_isr
      #endif
   #endif
   #if Varexist( "Portd")
      #if Varptr( "Touch_ctrl_port") = Varptr( "Portd")
         On Portd_int0 Touch_standby_isr
      #endif
   #endif
   #if Varexist( "Porte")
      #if Varptr( "Touch_ctrl_port") = Varptr( "Porte")
         On Porte_int0 Touch_standby_isr
      #endif
   #endif
   #if Varexist( "Portf")
      #if Varptr( "Touch_ctrl_port") = Varptr( "Portf")
         On Portf_int0 Touch_standby_isr
      #endif
   #endif
   #if Varexist( "Porth")
      #if Varptr( "Touch_ctrl_port") = Varptr( "Porth")
         On Porth_int0 Touch_standby_isr
      #endif
   #endif
   #if Varexist( "Portj")
      #if Varptr( "Touch_ctrl_port") = Varptr( "Portj")
         On Portj_int0 Touch_standby_isr
      #endif
   #endif
   #if Varexist( "Portk")
      #if Varptr( "Touch_ctrl_port") = Varptr( "Portk")
         On Portk_int0 Touch_standby_isr
      #endif
   #endif
   #if Varexist( "Portq")
      #if Varptr( "Touch_ctrl_port") = Varptr( "Portq")
         On Portq_int0 Touch_standby_isr
      #endif
   #endif

   ' Timer
   Config Tcc1 = Normal , Prescale = Touch_timer_prescale
   Touch_tc_per Alias Tcc1_per

   Macro Touch_init_timer
      Config Tcc1 = Normal , Prescale = Touch_timer_prescale
      Enable Tcc1_ovf , Lo
      Tcc1_intflags = 1                                     ' clear ovf interrupt flag
   End Macro

   Macro Touch_start_timer
      Start Tcc1
   End Macro

   Macro Touch_stop_timer
      Stop Tcc1
   End Macro

   Touch_stop_timer
   On Tcc1_ovf Touch_timer_isr

   ' ADC
   #if(_xtal / 4) < 2000000                                 '  prescaler calculation
      Const Touch_adc_prescaler = 4
   #elseif(_xtal / 8) < 2000000
      Const Touch_adc_prescaler = 8
   #elseif(_xtal / 16) < 2000000
      Const Touch_adc_prescaler = 16
   #elseif(_xtal / 32) < 2000000
      Const Touch_adc_prescaler = 32
   #endif

   Const Touch_adc_ctrl_start = &B1_00_000_01

   Macro Touch_init_adc
      Config Adca = Single , Convmode = Unsigned , Resolution = 12bit , Dma = Off , Reference = Intvcc , _
              Event_mode = None , Prescaler = Touch_adc_prescaler , Ch0_gain = 1 , Ch0_inp = Single_ended
      Enable Adca_ch0 , Lo
   End Macro

   Touch_adc_mux Alias Adca_ch0_muxctrl

   Macro Touch_start_adc
      Adca_ch0_ctrl = Touch_adc_ctrl_start
   End Macro

   Macro Touch_get_adc
      Touch_adc_result_l = Adca_ch0resl
      Touch_adc_result_h = Adca_ch0resh
   End Macro

   Function Touch_getadc(byval Muxctrl As Byte) As Word
      Local Adcresult As Word
      Adcresult = Getadc(adca , 0 , Muxctrl)
      Touch_getadc = Adcresult
   End Function

   On Adca_ch0 Touch_adc_isr

   Config Eeprom = Mapped
#else                                                       ' # ATMEGA #
   Const Touch_unused_mask = Not(2 ^ Touch_pin_xn + 2 ^ Touch_pin_yp + 2 ^ Touch_pin_yn)
   #if Varexist( "Porta")
      Const Touch_port_ptr = Varptr( "Porta")
   #else
      Const Touch_port_ptr = Varptr( "Portc")
   #endif
   Const Touch_ddr_ptr = Touch_port_ptr + 1
   Const Touch_pin_ptr = Touch_port_ptr + 2

   Const Touch_int_unused_mask = Not(2 ^ Touch_pin_xp)
   Const Touch_int_port_ptr = Touch_ctrl_port
   Const Touch_int_ddr_ptr = Touch_int_port_ptr + 1
   Const Touch_int_pin_ptr = Touch_int_port_ptr + 2

   ' standby/measurement
   Const Touch_standby_ddr = 2 ^ Touch_pin_yn
   Const Touch_standby_port = 0                             ' internal pullup
   Const Touch_standby_int_ddr = 0
   Const Touch_standby_int_port = 2 ^ Touch_pin_xp

   ' high-z (pause)
   Const Touch_high_z_ddr = 0
   Const Touch_high_z_port = 0
   Const Touch_high_z_int_ddr = 0
   Const Touch_high_z_int_port = 0

   ' measure x
   Const Touch_sample_x_ddr = 2 ^ Touch_pin_xn
   Const Touch_sample_x_port = 2 ^ Touch_pin_xn
   Const Touch_sample_x_int_ddr = 2 ^ Touch_pin_xp
   Const Touch_sample_x_int_port = 0
   Const Touch_sample_x_adcmux = Touch_pin_yn

   ' measure y
   Const Touch_sample_y_ddr = 2 ^ Touch_pin_yp + 2 ^ Touch_pin_yn
   Const Touch_sample_y_port = 2 ^ Touch_pin_yn
   Const Touch_sample_y_int_ddr = 0
   Const Touch_sample_y_int_port = 0
   Const Touch_sample_y_adcmux = Touch_pin_xn

   ' measure z1
   #if Touch_measure_resistance >= 1
      Const Touch_sample_z1_ddr = 2 ^ Touch_pin_yn
      Const Touch_sample_z1_port = 2 ^ Touch_pin_yn
      Const Touch_sample_z1_int_ddr = 2 ^ Touch_pin_xp
      Const Touch_sample_z1_int_port = 0
      Const Touch_sample_z1_adcmux = Touch_pin_xn
   #endif

   ' measure z2
   #if Touch_measure_resistance >= 2
      Const Touch_sample_z2_ddr = 2 ^ Touch_pin_yn
      Const Touch_sample_z2_port = 2 ^ Touch_pin_yn
      Const Touch_sample_z2_int_ddr = 2 ^ Touch_pin_xp
      Const Touch_sample_z2_int_port = 0
      Const Touch_sample_z2_adcmux = Touch_pin_yp
   #endif

   Macro Touch_set_high_z
      !lds R24, touch_ddr_ptr
      !andi R24, Touch_unused_mask
      !ori R24, Touch_high_z_ddr
      !sts Touch_ddr_ptr , R24

      !lds R24, touch_port_ptr
      !andi R24, Touch_unused_mask
      !ori R24, Touch_high_z_port
      !sts Touch_port_ptr , R24

      !lds R24, touch_int_ddr_ptr
      !andi R24, Touch_int_unused_mask
      !ori R24, Touch_high_z_int_ddr
      !sts Touch_int_ddr_ptr , R24

      !lds R24, touch_int_port_ptr
      !andi R24, Touch_int_unused_mask
      !ori R24, Touch_high_z_int_port
      !sts Touch_int_port_ptr , R24
   End Macro

   Macro Touch_set_sample_x
      !lds R24, touch_ddr_ptr
      !andi R24, Touch_unused_mask
      !ori R24, Touch_sample_x_ddr
      !sts Touch_ddr_ptr , R24

      !lds R24, touch_port_ptr
      !andi R24, Touch_unused_mask
      !ori R24, Touch_sample_x_port
      !sts Touch_port_ptr , R24

      !lds R24, touch_int_ddr_ptr
      !andi R24, Touch_int_unused_mask
      !ori R24, Touch_sample_x_int_ddr
      !sts Touch_int_ddr_ptr , R24

      !lds R24, touch_int_port_ptr
      !andi R24, Touch_int_unused_mask
      !ori R24, Touch_sample_x_int_port
      !sts Touch_int_port_ptr , R24
   End Macro

   Macro Touch_set_sample_y
      !lds R24, touch_ddr_ptr
      !andi R24, Touch_unused_mask
      !ori R24, Touch_sample_y_ddr
      !sts Touch_ddr_ptr , R24

      !lds R24, touch_port_ptr
      !andi R24, Touch_unused_mask
      !ori R24, Touch_sample_y_port
      !sts Touch_port_ptr , R24

      !lds R24, touch_int_ddr_ptr
      !andi R24, Touch_int_unused_mask
      !ori R24, Touch_sample_y_int_ddr
      !sts Touch_int_ddr_ptr , R24

      !lds R24, touch_int_port_ptr
      !andi R24, Touch_int_unused_mask
      !ori R24, Touch_sample_y_int_port
      !sts Touch_int_port_ptr , R24
   End Macro

   #if Touch_measure_resistance >= 1
      Macro Touch_set_sample_z1
         !lds R24, touch_ddr_ptr
         !andi R24, Touch_unused_mask
         !ori R24, Touch_sample_z1_ddr
         !sts Touch_ddr_ptr , R24

         !lds R24, touch_port_ptr
         !andi R24, Touch_unused_mask
         !ori R24, Touch_sample_z1_port
         !sts Touch_port_ptr , R24

         !lds R24, touch_int_ddr_ptr
         !andi R24, Touch_int_unused_mask
         !ori R24, Touch_sample_z1_int_ddr
         !sts Touch_int_ddr_ptr , R24

         !lds R24, touch_int_port_ptr
         !andi R24, Touch_int_unused_mask
         !ori R24, Touch_sample_z1_int_port
         !sts Touch_int_port_ptr , R24
      End Macro
   #endif
   #if Touch_measure_resistance >= 2
      Macro Touch_set_sample_z2
         !lds R24, touch_ddr_ptr
         !andi R24, Touch_unused_mask
         !ori R24, Touch_sample_z2_ddr
         !sts Touch_ddr_ptr , R24

         !lds R24, touch_port_ptr
         !andi R24, Touch_unused_mask
         !ori R24, Touch_sample_z2_port
         !sts Touch_port_ptr , R24

         !lds R24, touch_int_ddr_ptr
         !andi R24, Touch_int_unused_mask
         !ori R24, Touch_sample_z2_int_ddr
         !sts Touch_int_ddr_ptr , R24

         !lds R24, touch_int_port_ptr
         !andi R24, Touch_int_unused_mask
         !ori R24, Touch_sample_z2_int_port
         !sts Touch_int_port_ptr , R24
      End Macro
   #endif

   ' standby interrupt
   Macro Touch_init_standby
      Config Int0 = Falling
   End Macro

   Macro Touch_enter_standby
      !lds R24, touch_ddr_ptr
      !andi R24, Touch_unused_mask
      !ori R24, Touch_standby_ddr
      !sts Touch_ddr_ptr , R24

      !lds R24, touch_port_ptr
      !andi R24, Touch_unused_mask
      !ori R24, Touch_standby_port
      !sts Touch_port_ptr , R24

      !lds R24, touch_int_ddr_ptr
      !andi R24, Touch_int_unused_mask
      !ori R24, Touch_standby_int_ddr
      !sts Touch_int_ddr_ptr , R24

      !lds R24, touch_int_port_ptr
      !andi R24, Touch_int_unused_mask
      !ori R24, Touch_standby_int_port
      !sts Touch_int_port_ptr , R24

      Enable Int0
   End Macro

   Macro Touch_exit_standby
      Disable Int0
   End Macro

   On Int0 Touch_standby_isr

   ' Timer
   Config Timer0 = Timer , Prescale = Touch_timer_prescale
   Touch_tc_per Alias Tcnt0

   Macro Touch_init_timer
      Config Timer0 = Timer , Prescale = Touch_timer_prescale
      Enable Ovf0
      #if Varexist( "Tifr0")
         Set Tifr0.0
      #else
         Set Tifr.0                                         ' clear interrupt flag
      #endif
   End Macro

   Macro Touch_start_timer
      Start Timer0
   End Macro

   Macro Touch_stop_timer
      Stop Timer0
   End Macro

   Touch_stop_timer
   On Ovf0 Touch_timer_isr

   ' ADC
   Macro Touch_init_adc
      Config Adc = Single , Prescaler = Auto , Reference = Avcc
      Enable Adcc
   End Macro

   Touch_adc_mux Alias Admux

   Macro Touch_start_adc
      Set Adcsra.adsc
   End Macro

   Macro Touch_get_adc
      Touch_adc_result_l = Adcl
      Touch_adc_result_h = Adch
   End Macro

   Function Touch_getadc(byval Muxctrl As Byte) As Word
      Local Adcresult As Word
      Adcresult = Getadc(muxctrl)
      Touch_getadc = Adcresult
   End Function

   On Adcc Touch_adc_isr
#endif


'### Variables ######################################################

' Status flags
Dim Touch_touched As Bit
Dim Touch_newsample As Byte
Dim Touch_sample_invalid As Bit
Dim Touch_clicked As Bit
Dim Touch_dragging As Bit
Dim Touch_calibrated As Bit

' Coordinates
Dim Touch_x As Word , Touch_y As Word
#if Touch_measure_resistance >= 1
   Dim Touch_r As Word
#endif

' Rotation = 0:   0°
'          = 1:  90°
'          = 2: 180°
'          = 3: 270°
#if Touch_enable_rotation = True And Not Varexist( "Touch_rotation")
   #if Not Varexist( "Ili9341")
      Dim Touch_rotation As Byte
   #else
      Dim Touch_rotation As Byte At Lcd_rotation Overlay
   #endif
#endif

' internal
Dim Touch_state As Byte
Dim Touch_debounce_counter As Byte
Dim Touch_release_counter As Byte

Dim Touch_adc_result As Word
Dim Touch_adc_result_l As Byte At Touch_adc_result Overlay
Dim Touch_adc_result_h As Byte At Touch_adc_result + 1 Overlay

Dim Touch_sample_index As Byte
Dim Touch_samples_x(touch_samples) As Word , Touch_samples_y(touch_samples) As Word
Dim Touch_sample_x As Word , Touch_sample_y As Word
#if Touch_measure_resistance >= 1
   Dim Touch_samples_z1(touch_samples) As Word , Touch_sample_z1 As Word
#endif
#if Touch_measure_resistance >= 2
   Dim Touch_samples_z2(touch_samples) As Word , Touch_sample_z2 As Word
#endif

Dim Touch_first_x As Word , Touch_first_y As Word
Dim Touch_previous_x As Word , Touch_previous_y As Word
Dim Touch_tmpw As Word

' calibration
Dim Touch_calibration(6) As Single , Touch_calibration_eep(6) As Eram Single
Dim Touch_threshold_x As Word , Touch_threshold_y As Word
Dim Touch_threshold_x_eep As Eram Word , Touch_threshold_y_eep As Eram Word
Dim Touch_max_delta_x As Word , Touch_max_delta_y As Word
Dim Touch_click_delta_x As Word , Touch_click_delta_y As Word
Dim Touch_adc_range_x_eep As Eram Word , Touch_adc_range_y_eep As Eram Word


'### ISRs ###########################################################

Goto Touch_runprog

Touch_standby_isr:
   Touch_exit_standby
   Touch_set_sample_x
   Touch_tc_per = Touch_per_switch_measurement
   Touch_state = Touch_state_sample_x
   Touch_start_timer

   Touch_touched = True
   Touch_newsample = False
   Touch_dragging = False
   Touch_clicked = False
   Touch_sample_invalid = False
   Touch_sample_index = 1
   Touch_debounce_counter = 0
   Touch_release_counter = 0
   Touch_first_x = &HFFFF
   Touch_previous_x = &HFFFF
Return

Touch_timer_isr:
   Select Case Touch_state
      Case Touch_state_sample_x :
         Touch_adc_mux = Touch_sample_x_adcmux
         Touch_start_adc
      Case Touch_state_sample_y:
         Touch_adc_mux = Touch_sample_y_adcmux
         Touch_start_adc
   #if Touch_measure_resistance >= 1
      Case Touch_state_sample_z1:
         Touch_adc_mux = Touch_sample_z1_adcmux
         Touch_start_adc
   #endif
   #if Touch_measure_resistance >= 2
      Case Touch_state_sample_z2:
         Touch_adc_mux = Touch_sample_z2_adcmux
         Touch_start_adc
   #endif
      Case Touch_state_pause_sample:
         Touch_set_sample_x
         Touch_tc_per = Touch_per_switch_measurement
         Touch_state = Touch_state_sample_x
   End Select
Return

Touch_adc_isr:
   Select Case Touch_state
      Case Touch_state_sample_x :
         Touch_get_adc
         Touch_samples_x(touch_sample_index) = Touch_adc_result
         Touch_set_sample_y
         Touch_state = Touch_state_sample_y

      Case Touch_state_sample_y:
         Touch_get_adc
         Touch_samples_y(touch_sample_index) = Touch_adc_result
   #if Touch_measure_resistance >= 1
         Touch_set_sample_z1
         Touch_state = Touch_state_sample_z1

      Case Touch_state_sample_z1:
         Touch_get_adc
         Touch_samples_z1(touch_sample_index) = Touch_adc_result
   #endif
   #if Touch_measure_resistance >= 2
         Touch_set_sample_z2
         Touch_state = Touch_state_sample_z2

      Case Touch_state_sample_z2:
         Touch_get_adc
         Touch_samples_z2(touch_sample_index) = Touch_adc_result
   #endif
         If Touch_samples_x(touch_sample_index) < 500 Or Touch_samples_y(touch_sample_index) < 500 Then Touch_sample_invalid = True
         If Touch_sample_index = Touch_samples Then
            ' new coordinates
            Touch_sample_index = 0
            If Touch_sample_invalid = True Then
               ' invalid sample(s)
               If Touch_debounce_counter = Touch_per_debounce Then Touch_release_counter = Touch_release_counter + 1       ' outside debounce time, increase releasecounter
               If Touch_release_counter = Touch_release_count Then
                  ' touchscreen has been released
                  Touch_stop_timer
                  Touch_touched = False
                  Touch_enter_standby
                  ' check if clicked
                  If Touch_dragging = False Then Touch_clicked = True
               End If
               Touch_sample_invalid = False
            Else
               Touch_release_counter = 0
               ' valid samples, test if valid coordinates
               Sort Touch_samples_x
               Sort Touch_samples_y
               If Touch_previous_x = &HFFFF Then
                     Touch_previous_x = Touch_samples_x(touch_samples_median)
                     Touch_previous_y = Touch_samples_y(touch_samples_median)
               End If
               ' check if delta to previous sample is in a defined window
               If Touch_previous_x < Touch_samples_x(touch_samples_median) Then
                  Touch_tmpw = Touch_samples_x(touch_samples_median) - Touch_previous_x
               Else
                  Touch_tmpw = Touch_previous_x - Touch_samples_x(touch_samples_median)
               End If
               If Touch_tmpw < Touch_max_delta_x Then
                  If Touch_previous_y < Touch_samples_y(touch_samples_median) Then
                     Touch_tmpw = Touch_samples_y(touch_samples_median) - Touch_previous_y
                  Else
                     Touch_tmpw = Touch_previous_y - Touch_samples_y(touch_samples_median)
                  End If
                  If Touch_tmpw < Touch_max_delta_y Then
                     ' remember first sample for click/drag detection
                     If Touch_first_x = &HFFFF Then
                        Touch_first_x = Touch_samples_x(touch_samples_median)
                        Touch_first_y = Touch_samples_y(touch_samples_median)
                     End If

                     ' check if dragging
                     If Touch_dragging = False Then
                        If Touch_first_x < Touch_samples_x(touch_samples_median) Then
                           Touch_tmpw = Touch_samples_x(touch_samples_median) - Touch_first_x
                        Else
                           Touch_tmpw = Touch_first_x - Touch_samples_x(touch_samples_median)
                        End If
                        If Touch_click_delta_x < Touch_tmpw Then Touch_dragging = True
                        If Touch_first_y < Touch_samples_y(touch_samples_median) Then
                           Touch_tmpw = Touch_samples_y(touch_samples_median) - Touch_first_y
                        Else
                           Touch_tmpw = Touch_first_y - Touch_samples_y(touch_samples_median)
                        End If
                        If Touch_click_delta_y < Touch_tmpw Then Touch_dragging = True
                     End If

                     ' new coordinate pair
                     Touch_sample_x = Touch_samples_x(touch_samples_median)
                     Touch_sample_y = Touch_samples_y(touch_samples_median)
                     #if Touch_measure_resistance >= 1
                        Sort Touch_samples_z1
                        Touch_sample_z1 = Touch_samples_z1(touch_samples_median)
                     #endif
                     #if Touch_measure_resistance >= 2
                        Sort Touch_samples_z2
                        Touch_sample_z2 = Touch_samples_z2(touch_samples_median)
                     #endif
                     Touch_previous_x = Touch_sample_x
                     Touch_previous_y = Touch_sample_y

                     Touch_newsample = True
                  End If
               End If
            End If
         End If
         If Touch_touched = True Then
            ' still touched, delay next measurement a bit
            Touch_set_high_z
            Touch_sample_index = Touch_sample_index + 1
            Touch_state = Touch_state_pause_sample
            Touch_tc_per = Touch_per_pause_sample
         End If
         If Touch_debounce_counter < Touch_per_debounce Then Touch_debounce_counter = Touch_debounce_counter + 1
   End Select
Return

Touch_runprog:


'### Functions ######################################################

' initialize the touch screen
Sub Touch_init()
   Local Temp As Byte
   Temp = Sreg
   Disable Interrupts

   Touch_init_adc
   Touch_init_timer
   Touch_init_standby

   #if _xmega = True
      Pmic_ctrl.0 = True                                    ' enable low level interrupts
   #endif

   Touch_load_calibration
   Touch_enter_standby
   Sreg = Temp
End Sub

' calibrates the touch coordinate to screen coordinate translation
' and stores the calculated values in EEPROM
Sub Touch_calibrate()
   Local X1 As Single , Y1 As Single
   Local X2 As Single , Y2 As Single
   Local X3 As Single , Y3 As Single
   Local Tmps As Single

   #if Touch_enable_rotation = True
      Local Rotation As Byte
      Rotation = Touch_rotation                             ' remember current rotation
      ' set 0° rotation
      #if Varexist( "Ili9341")
         Lcd_set_rotation 0
      #else
         !nop                                               ' # DISPLAY ROTATION #
      #endif
   #endif
   #if Varexist( "Ili9341")
      Lcd_clear &H0000
   #else
      Cls                                                   ' # DISPLAY CLEAR #
   #endif

   Touch_max_delta_x = 100
   Touch_max_delta_y = 100
   Touch_calibrated = False
   While Touch_calibrated = False
      ' wait until touchscreen is not pressed
      While Touch_touched = True : Wend
      Disable Interrupts
      Touch_exit_standby
      ' measure x offset
      Touch_set_sample_x
      Waitms 50
      For Touch_debounce_counter = 1 To Touch_samples
         Touch_samples_x(touch_debounce_counter) = Touch_getadc(touch_sample_x_adcmux)
         Waitms 10
      Next
      ' measure y offset
      Touch_set_sample_y
      Waitms 10
      For Touch_debounce_counter = 1 To Touch_samples
         Touch_samples_y(touch_debounce_counter) = Touch_getadc(touch_sample_y_adcmux)
         Waitms 10
      Next
      Enable Interrupts
      Sort Touch_samples_x
      Sort Touch_samples_y
      ' threshold value to sort out invalid measurements
      Touch_threshold_x = Touch_samples_x(touch_samples_median)
      Touch_threshold_y = Touch_samples_y(touch_samples_median)
      Touch_enter_standby

      ' sample touch coordinates of calibration points
      #if Varexist( "Ili9341")                              ' calibration point 1
         Lcd_circle Touch_cal1_x , Touch_cal1_y , Touch_cal_marker_radius , &HFFFF
         Lcd_circle Touch_cal1_x , Touch_cal1_y , 2 , &HFFFF
      #else
         Circle(touch_cal1_x , Touch_cal1_y) , Touch_cal_marker_radius , &HFF       ' # DISPLAY CIRCLE #
         Circle(touch_cal1_x , Touch_cal1_y) , 2 , &HFF
      #endif
      While Touch_touched = False : Wend
      While Touch_newsample = False : Wend
      X1 = Touch_sample_x : Y1 = Touch_sample_y
      While Touch_touched = True : Wend
      #if Varexist( "Ili9341")                              ' calibration point 2
         Lcd_circle Touch_cal1_x , Touch_cal1_y , Touch_cal_marker_radius , &H0000
         Lcd_circle Touch_cal1_x , Touch_cal1_y , 2 , &H0000
         Lcd_circle Touch_cal2_x , Touch_cal2_y , Touch_cal_marker_radius , &HFFFF
         Lcd_circle Touch_cal2_x , Touch_cal2_y , 2 , &HFFFF
      #else
         Circle(touch_cal1_x , Touch_cal1_y) , Touch_cal_marker_radius , 0       ' # DISPLAY CIRCLE #
         Circle(touch_cal1_x , Touch_cal1_y) , 2 , 0
         Circle(touch_cal2_x , Touch_cal2_y) , Touch_cal_marker_radius , &HFF
         Circle(touch_cal2_x , Touch_cal2_y) , 2 , &HFF
      #endif
      While Touch_touched = False : Wend
      While Touch_newsample = False : Wend
      X2 = Touch_sample_x : Y2 = Touch_sample_y
      Touch_calibration(2) = Touch_sample_y
      While Touch_touched = True : Wend
      #if Varexist( "Ili9341")                              ' calibration point 3
         Lcd_circle Touch_cal2_x , Touch_cal2_y , Touch_cal_marker_radius , &H0000
         Lcd_circle Touch_cal2_x , Touch_cal2_y , 2 , &H0000
         Lcd_circle Touch_cal3_x , Touch_cal3_y , Touch_cal_marker_radius , &HFFFF
         Lcd_circle Touch_cal3_x , Touch_cal3_y , 2 , &HFFFF
      #else
         Circle(touch_cal2_x , Touch_cal2_y) , Touch_cal_marker_radius , 0       ' # DISPLAY CIRCLE #
         Circle(touch_cal2_x , Touch_cal2_y) , 2 , 0
         Circle(touch_cal3_x , Touch_cal3_y) , Touch_cal_marker_radius , &HFF
         Circle(touch_cal3_x , Touch_cal3_y) , 2 , &HFF
      #endif
      While Touch_touched = False : Wend
      While Touch_newsample = False : Wend
      X3 = Touch_sample_x : Y3 = Touch_sample_y
      Touch_calibration(1) = Touch_sample_x
      While Touch_touched = True : Wend
      Touch_newsample = False
      #if Varexist( "Ili9341")
         Lcd_circle Touch_cal3_x , Touch_cal3_y , Touch_cal_marker_radius , &H0000
         Lcd_circle Touch_cal3_x , Touch_cal3_y , 2 , &H0000
      #else
         Circle(touch_cal3_x , Touch_cal3_y) , Touch_cal_marker_radius , 0       ' # DISPLAY CIRCLE #
         Circle(touch_cal3_x , Touch_cal3_y) , 2 , 0
      #endif

      ' calculate ADC range
      Touch_calibration(1) = Touch_calibration(1) * 1.12
      Touch_calibration(2) = Touch_calibration(2) * 1.12
      If 4096 <= Touch_calibration(1) Then Touch_calibration(1) = 4095
      If 4096 <= Touch_calibration(2) Then Touch_calibration(2) = 4095
      Touch_max_delta_x = Touch_calibration(1)
      Touch_max_delta_y = Touch_calibration(2)

      ' calculate coefficients
      Touch_calibration(1) = Y2 - Y3                        ' A
      Touch_calibration(1) = Touch_calibration(1) * Touch_cal1_x
      Touch_calibration(2) = Y3 - Y1
      Touch_calibration(2) = Touch_calibration(2) * Touch_cal2_x
      Touch_calibration(1) = Touch_calibration(1) + Touch_calibration(2)
      Touch_calibration(2) = Y1 - Y2
      Touch_calibration(2) = Touch_calibration(2) * Touch_cal3_x
      Touch_calibration(1) = Touch_calibration(1) + Touch_calibration(2)
      Touch_calibration(2) = Y2 - Y3
      Tmps = Touch_calibration(2) * X1
      Touch_calibration(3) = Y3 - Y1
      Touch_calibration(3) = Touch_calibration(3) * X2
      Tmps = Tmps + Touch_calibration(3)
      Touch_calibration(3) = Y1 - Y2
      Touch_calibration(3) = Touch_calibration(3) * X3
      Tmps = Tmps + Touch_calibration(3)
      Touch_calibration(1) = Touch_calibration(1) / Tmps

      Touch_calibration(4) = Y2 - Y3                        ' D
      Touch_calibration(4) = Touch_calibration(4) * Touch_cal1_y
      Touch_calibration(5) = Y3 - Y1
      Touch_calibration(5) = Touch_calibration(5) * Touch_cal2_y
      Touch_calibration(4) = Touch_calibration(4) + Touch_calibration(5)
      Touch_calibration(5) = Y1 - Y2
      Touch_calibration(5) = Touch_calibration(5) * Touch_cal3_y
      Touch_calibration(4) = Touch_calibration(4) + Touch_calibration(5)
      Touch_calibration(4) = Touch_calibration(4) / Tmps

      Touch_calibration(2) = X3 - X2                        ' B
      Touch_calibration(2) = Touch_calibration(1) * Touch_calibration(2)
      Touch_calibration(2) = Touch_calibration(2) + Touch_cal2_x
      Touch_calibration(2) = Touch_calibration(2) - Touch_cal3_x
      Touch_calibration(3) = Y2 - Y3
      Touch_calibration(2) = Touch_calibration(2) / Touch_calibration(3)

      Touch_calibration(5) = X3 - X2                        ' E
      Touch_calibration(5) = Touch_calibration(4) * Touch_calibration(5)
      Touch_calibration(5) = Touch_calibration(5) + Touch_cal2_y
      Touch_calibration(5) = Touch_calibration(5) - Touch_cal3_y
      Touch_calibration(6) = Y2 - Y3
      Touch_calibration(5) = Touch_calibration(5) / Touch_calibration(6)

      Touch_calibration(3) = Touch_calibration(1) * X3      ' C
      Tmps = Touch_calibration(2) * Y3
      Touch_calibration(3) = Touch_calibration(3) + Tmps
      Touch_calibration(3) = Touch_cal3_x - Touch_calibration(3)

      Touch_calibration(6) = Touch_calibration(4) * X3      ' F
      Tmps = Touch_calibration(5) * Y3
      Touch_calibration(6) = Touch_calibration(6) + Tmps
      Touch_calibration(6) = Touch_cal3_y - Touch_calibration(6)

      Touch_calibrated = True

      ' sanity check
      If X2 < X1 Or X3 < X2 Or Y1 < Y3 Or Y2 < Y1 Then Touch_calibrated = False
   Wend

   ' write calibration values to EEPROM and calc rest
   Touch_threshold_x_eep = Touch_threshold_x
   Touch_threshold_y_eep = Touch_threshold_y
   Touch_adc_range_x_eep = Touch_max_delta_x
   Touch_adc_range_y_eep = Touch_max_delta_y
   For Touch_debounce_counter = 1 To 6
      Touch_calibration_eep(touch_debounce_counter) = Touch_calibration(touch_debounce_counter)
   Next
   Touch_calculate_calibration

   ' rotate back in previous rotation
   #if Touch_enable_rotation = True
      #if Varexist( "Ili9341")
         Lcd_set_rotation Rotation
      #else
         ' other display types
      #endif
   #endif
End Sub

' loads calibration values from EEPROM, if not available,
' starts the calibration
Sub Touch_load_calibration()
   Local Tempb As Byte
   ' load first calibration values from eeprom
   Touch_threshold_x = Touch_threshold_x_eep
   Touch_threshold_y = Touch_threshold_y_eep
   ' check if calibration is needed
   If Touch_threshold_x = 0 And Touch_threshold_y = 0 Then
      Touch_calibrate
   Else
      For Tempb = 1 To 6                                    ' load rest
         Touch_calibration(tempb) = Touch_calibration_eep(tempb)
      Next
      Touch_max_delta_x = Touch_adc_range_x_eep
      Touch_max_delta_y = Touch_adc_range_y_eep
      Touch_calculate_calibration
   End If
End Sub

' calculates additional calibration data after loading from EEPROM
Sub Touch_calculate_calibration()
   Local Temps As Single
   ' click/drag detection and max speed delta (applied on raw measurement values, not 100% pixel-accurate)
   Touch_max_delta_x = Touch_max_delta_x - Touch_threshold_x       ' adc range
   Touch_max_delta_y = Touch_max_delta_y - Touch_threshold_y
   Temps = Touch_max_delta_x
   Temps = Temps * Touch_adc_click_delta_x
   Touch_click_delta_x = Temps
   Temps = Touch_max_delta_y
   Temps = Temps * Touch_adc_click_delta_y
   Touch_click_delta_y = Temps
   Temps = Touch_max_delta_x
   Temps = Temps * Touch_adc_max_delta_x
   Touch_max_delta_x = Temps
   Temps = Touch_max_delta_y
   Temps = Temps * Touch_adc_max_delta_y
   Touch_max_delta_y = Temps

   ' min. ADC values to consider samples as valid
   Touch_threshold_x = Touch_threshold_x * 50
   Touch_threshold_x = Touch_threshold_x / 40
   Touch_threshold_y = Touch_threshold_y * 50
   Touch_threshold_y = Touch_threshold_y / 40

   Touch_calibrated = True
End Sub

' deletes stored calibration data
Sub Touch_reset_calibration()
   Touch_threshold_x_eep = 0
   Touch_threshold_y_eep = 0
   Touch_calibrated = False
End Sub

' returns true if a new coordinate pair has been generated and
' transforms the measurements into screen coordinates
Function Touch_get_sample() As Byte
   Local Returnvalue As Byte
   Local Tmps1 As Single , Tmps2 As Single
   #if Touch_measure_resistance = 1
      Local Tmps3 As Single
   #endif
   Returnvalue = Touch_newsample
   If Returnvalue = True Then
      Touch_newsample = False
      If Touch_calibrated = True Then
         ' calc screen coordinate x
         Tmps1 = Touch_calibration(1) * Touch_sample_x
         Tmps2 = Touch_calibration(2) * Touch_sample_y
         Tmps1 = Tmps1 + Tmps2
         Tmps1 = Tmps1 + Touch_calibration(3)
         Touch_x = Tmps1

         ' calc screen coordinate y
         Tmps1 = Touch_calibration(4) * Touch_sample_x
         Tmps2 = Touch_calibration(5) * Touch_sample_y
         Tmps1 = Tmps1 + Tmps2
         Tmps1 = Tmps1 + Touch_calibration(6)
         Touch_y = Tmps1

         If Touch_screen_width <= Touch_x Or Touch_screen_height <= Touch_y Then
            Returnvalue = False                             ' invalid coordinates
         #if Touch_enable_rotation = True
         Else
            Select Case Touch_rotation                      ' valid, rotate coordinates
               'Case 0:                                                          ' 0°
               Case 1:                                      ' 90°
                  Touch_x = Touch_screen_width - Touch_x
                  Swap Touch_x , Touch_y
               Case 2:                                      ' 180°
                  Touch_x = Touch_screen_width - Touch_x
                  Touch_y = Touch_screen_height - Touch_y
               Case 3:                                      ' 270°
                  Touch_y = Touch_screen_height - Touch_y
                  Swap Touch_x , Touch_y
            End Select
         #endif
         End If
      Else
         ' no calibration values available, deliver raw ADC values
         Touch_x = Touch_sample_x
         Touch_y = Touch_sample_y
      End If

      ' calculate touch contact resistance
      #if Touch_measure_resistance >= 1
         If Returnvalue = True Then
            #if Touch_measure_resistance = 1                ' method 1 (X, Z1, Rx, Ry)
               Tmps1 = 4096 / Touch_sample_z1
               Tmps1 = Tmps1 - 1
               Tmps2 = Touch_sample_x
               Tmps1 = Tmps1 * Tmps2
               Tmps2 = Touch_resistance_x
               Tmps1 = Tmps1 * Tmps2
               Tmps1 = Tmps1 / 4096
               Tmps2 = Touch_sample_y
               Tmps2 = Touch_sample_y / 4096
               Tmps2 = 1 - Tmps2
               Tmps3 = Touch_resistance_y
               Tmps2 = Tmps2 * Tmps3
               Tmps1 = Tmps1 - Tmps2
            #elseif Touch_measure_resistance = 2            ' method 2 (X, Z1, Z2, Rx)
               Tmps1 = Touch_sample_z2
               Tmps2 = Touch_sample_z1
               Tmps1 = Tmps1 / Tmps2
               Tmps1 = Tmps1 - 1
               Tmps1 = Tmps1 * Touch_sample_x
               Tmps1 = Tmps1 * Touch_resistance_x
               Tmps1 = Tmps1 / 4096
            #endif
            Touch_r = Tmps1
         End If
         ' sanity check
         If Touch_max_resistance < Touch_r Then Returnvalue = False
      #endif
   End If
   Touch_get_sample = Returnvalue
End Function

' returns true if a click event has been detected and
' transforms the measurements into screen coordinates
Function Touch_get_clicked() As Byte
   Local Returnvalue As Byte
   If Touch_clicked = True Then
      Touch_clicked = False
      Returnvalue = Touch_getsample()
   Else
      Returnvalue = False
   End If
   Touch_get_clicked = Returnvalue
End Function