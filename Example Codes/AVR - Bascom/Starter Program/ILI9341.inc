' ILI9341 TFT Library 1.1, Aug. 2017
' http://mat.midlight.eu/wiki/index.php?title=ILI9341_Library

$nocompile

Const Ili9341 = True                                        ' Library identification

#if Not Varexist( "False")
   Const False = 0 : Const True = 1
#endif

' Enables the SPI- instead of the 8/16-Bit parallel interface
#if Not Varexist( "Lcd_enable_spi")
   Const Lcd_enable_spi = False
#endif

' Use software SPI
#if Not Varexist( "Lcd_use_soft_spi") And Lcd_enable_spi = True
   Const Lcd_use_soft_spi = False
#endif

' Enables the 16-Bit parallel interface
#if Not Varexist( "Lcd_enable_16bit")
   Const Lcd_enable_16bit = False
#endif

' Enable backlight control
#if Not Varexist( "Lcd_enable_backlight")
   Const Lcd_enable_backlight = False
#endif

' Use a timer to generate a PWM signal for the backlight instead of switching it on or off
#if Not Varexist( "Lcd_enable_backlight_pwm")
   Const Lcd_enable_backlight_pwm = True
#endif

' ATMega only: select the timer to use for the PWM signal (XMega: automatic)
#if Not Varexist( "Lcd_backlight_timer") And Lcd_enable_backlight_pwm = True And _xmega = False
   Lcd_backlight_timer Alias Timer2
#endif

' invert backlight logic (1: off, 0: on)
#if Not Varexist( "Lcd_invert_backlight") And(lcd_enable_backlight = True Or Lcd_enable_backlight_pwm = True)
   Const Lcd_invert_backlight = False
#endif

' Select the port of the display control pins (XMega: PortC-PortF, ATMega: for HW SPI)
#if Not Varexist( "Lcd_ctrl_port")
   #if _xmega = True
      Const Lcd_ctrl_port = Portc
   #else
      Const Lcd_ctrl_port = Portb
   #endif
#endif

' 8 Bit mode data port
#if Not Varexist( "Lcd_data_port_1") And Lcd_enable_spi = False
   #if _xmega = True
      Const Lcd_data_port_1 = Portf
   #else
      Const Lcd_data_port_1 = Portc
   #endif
#endif

' 16 Bit mode data port #2
#if Not Varexist( "Lcd_data_port_2") And Lcd_enable_16bit = True
   Const Lcd_data_port_2 = Portd
#endif

' Port where the display reset is connected to
#if Not Varexist( "Lcd_reset_port")
   Const Lcd_reset_port = Lcd_ctrl_port
#endif

' Pin of the display reset signal
#if Not Varexist( "Lcd_reset_pin")
   Const Lcd_reset_pin = 0
#endif

#if Lcd_enable_spi = True
   ' SPI mode pin assignments
   #if Not Varexist( "Lcd_pin_dc")
      #if _xmega = True
         Const Lcd_pin_dc = 1
      #else
         Const Lcd_pin_dc = 4
      #endif
   #endif
   ' (HW: fixed, SW: select any)
   #if Not Varexist( "Lcd_pin_cs")
      #if _xmega = True
         Const Lcd_pin_cs = 4
      #else
         Const Lcd_pin_cs = 0
      #endif
   #endif
   #if Not Varexist( "Lcd_pin_sdo")
      #if _xmega = True
         Const Lcd_pin_sdo = 5
      #else
         Const Lcd_pin_sdo = 2
      #endif
   #endif
   #if Not Varexist( "Lcd_pin_clk")
      #if _xmega = True
         Const Lcd_pin_clk = 7
      #else
         Const Lcd_pin_clk = 1
      #endif
   #endif
'   Const Lcd_pin_sdi = 6
#else
   ' parallel (8/16 Bit) mode pin assignments
   #if Not Varexist( "Lcd_pin_wr")
      Const Lcd_pin_wr = 1
   #endif
   #if Not Varexist( "Lcd_pin_rd")
      Const Lcd_pin_rd = 5
   #endif
   #if Not Varexist( "Lcd_pin_dc")
      Const Lcd_pin_dc = 7
   #endif
   #if Not Varexist( "Lcd_pin_cs")
      Const Lcd_pin_cs = 4
   #endif
#endif

' Port of the backlight control signal
#if Not Varexist( "Lcd_backlight_port") And(lcd_enable_backlight = True Or Lcd_enable_backlight_pwm = True)
   Const Lcd_backlight_port = Lcd_ctrl_port
#endif

' Pin of the backlight control signal
#if Not Varexist( "Lcd_pin_backlight" ) And(lcd_enable_backlight = True Or Lcd_enable_backlight_pwm = True)
   #if _xmega = True
      Const Lcd_pin_backlight = 0
   #else
      Const Lcd_pin_backlight = 6
   #endif
#endif

' If the display looks somewhat flawed (interference lines), try another init sequence (source: Hkipnik) for the lcd controller
#if Not Varexist( "Lcd_alternative_init")
   Const Lcd_alternative_init = False
#endif

'==============================================================================

Const Lcd_width = 240
Const Lcd_height = 320

#if _xmega = True
   Const Lcd_ctrl_ddr = Lcd_ctrl_port - 4
   Const Lcd_reset_ddr = Lcd_reset_port - 4
#else
   #if _chip = 60                                           ' m328
      Const Lcd_ctrl_ddr = Lcd_ctrl_port - 1
      Const Lcd_reset_ddr = Lcd_reset_port - 1
   #else
      Const Lcd_ctrl_ddr = Lcd_ctrl_port + 1
      Const Lcd_reset_ddr = Lcd_reset_port + 1
   #endif
#endif

Const Lcd_reset_high = 2 ^ Lcd_reset_pin
Const Lcd_reset_low = Not(lcd_reset_high)

#if Lcd_enable_backlight = True Or Lcd_enable_backlight_pwm = True
   #if _xmega = True
      Const Lcd_backlight_ddr = Lcd_backlight_port - 4
   #else
      #if _chip = 60                                        ' m328
          Const Lcd_backlight_ddr = Lcd_backlight_port - 1
      #else
          Const Lcd_backlight_ddr = Lcd_backlight_port + 1
      #endif
   #endif
   Const Lcd_backlight_high = 2 ^ Lcd_pin_backlight
   Const Lcd_backlight_low = Not(lcd_backlight_high)
#endif

$include "Colors565.inc"

' For reduced Digit Font
Dim Lcd_digit_font As Bit

' Toggle normal font/color font format
Dim Lcd_color_font As Bit

' the actual max X and Y coordinates (after rotation, starting with 0)
Dim Lcd_screen_width As Word
Dim Lcd_screen_height As Word

'Dim Lcd_databyte As Byte
Lcd_databyte Alias ___r17
#if Lcd_enable_16bit = True
   Lcd_databyte2 Alias ___r18
#endif

'==============================================================================
#if Lcd_enable_spi = True
   ' SPI subroutines
   Const Lcd_set_cs = 2 ^ Lcd_pin_cs
   Const Lcd_clear_cs = Not Lcd_set_cs
   Const Lcd_set_dc = 2 ^ Lcd_pin_dc
   Const Lcd_clear_dc = Not Lcd_set_dc
   Const Lcd_set_cs_dc = Lcd_set_cs + Lcd_set_dc
   Const Lcd_clear_cs_dc = Not(lcd_set_cs_dc)
   Const Lcd_write_idle_mask = 2 ^ Lcd_pin_cs + 2 ^ Lcd_pin_dc

   Const Lcd_set_sdo = 2 ^ Lcd_pin_sdo
   Const Lcd_clear_sdo = Not Lcd_set_sdo
   Const Lcd_set_clk = 2 ^ Lcd_pin_clk
   Const Lcd_clear_clk = Not Lcd_set_clk
   Const Lcd_ctrl_output_mask = Lcd_write_idle_mask + Lcd_set_sdo + Lcd_set_clk

   #if Lcd_use_soft_spi = True
      ' Software SPI
      Sub Lcd_spi_out_bitbang()
         !ldi R22, &H80
         Lcd_bitbang_loop:
            !mov R23, R17                                   ' rest of byte is in R17
            !and R23, R22                                   ' get MSB
            !breq lcd_Bitbang_zero                          ' 0 or 1?
               !ori R16, Lcd_set_sdo                        ' set 1
               !rjmp lcd_Bitbang_continue
            Lcd_bitbang_zero:
               !andi R16, Lcd_clear_sdo                     ' set 0
            Lcd_bitbang_continue:
            !andi R16, Lcd_clear_clk
            !out Lcd_ctrl_port, R16                         ' output value
            !ori R16, Lcd_set_clk
            !out Lcd_ctrl_port, R16                         ' advance bit position
            !lsr R22                                        ' decr bits to transfer
         !brne lcd_Bitbang_loop                             ' continue loop if bits left to transfer
      End Sub

      Macro Lcd_ctrl_init
         !in R16, Lcd_ctrl_ddr                              ' configure SPI output pins
         !ori R16, Lcd_ctrl_output_mask
         !out Lcd_ctrl_ddr, R16
      End Macro
   #else
      ' Hardware SPI
      #if _xmega = True
         #if Varptr( "Lcd_ctrl_port") = Varptr( "Portc")
            Const Lcd_spi_ctrl = Spic_ctrl
            Lcd_spi_status Alias Spic_status
            Lcd_spi_data Alias Spic_data
         #elseif Varptr( "Lcd_ctrl_port") = Varptr( "Portd")
            Const Lcd_spi_ctrl = Spid_ctrl
            Lcd_spi_status Alias Spid_status
            Lcd_spi_data Alias Spid_data
         #elseif Varptr( "Lcd_ctrl_port") = Varptr( "Porte")
            Const Lcd_spi_ctrl = Spie_ctrl
            Lcd_spi_status Alias Spie_status
            Lcd_spi_data Alias Spie_data
         #elseif Varptr( "Lcd_ctrl_port") = Varptr( "Portf")
            Const Lcd_spi_ctrl = Spif_ctrl
            Lcd_spi_status Alias Spif_status
            Lcd_spi_data Alias Spif_data
         #endif

         Const Lcd_spi_ctrl_init = &B1_1_0_1_00_00          ' Clk2x, Enable, Data Order = MSB, Master, Mode 0, Prescaler Clk/4
      #else
         Const Lcd_spi_ctrl = Spcr
         Lcd_spi_status Alias Spsr
         Lcd_spi_data Alias Spdr
         Const Lcd_spi_ctrl_init = &B0_1_0_1_0_0_00         ' No INT, Enable, MSB, Master, CPOL=0, CPHA=0, CLK/4
         Const Lcd_spi_status_init = &B00_00000_1           ' CLKx2
      #endif

      Macro Lcd_ctrl_init
         !in R16, Lcd_ctrl_ddr                              ' configure SPI output pins
         !ori R16, Lcd_ctrl_output_mask
         !out Lcd_ctrl_ddr, R16

         #if _xmega = False
            Lcd_spi_status = Lcd_spi_status_init
         #endif

         'Config Spic = Hard , Master = Yes , Mode = 0 , Clockdiv = Clk2 , Data_order = Msb , Ss = None
         !ldi R16, Lcd_spi_ctrl_init
         !sts Lcd_spi_ctrl, R16
      End Macro
   #endif

   Macro Lcd_write_cmd
      !lds R16, Lcd_ctrl_port                               ' CS, DC low
      !andi R16, Lcd_clear_cs_dc
      !sts Lcd_ctrl_port, R16
      #if Lcd_use_soft_spi = False                          ' write byte
         Lcd_spi_data = Lcd_databyte
         While Lcd_spi_status.spif = 0 : Wend
         !lds R16, Lcd_ctrl_port
      #else
         Lcd_spi_out_bitbang
      #endif
      !ori R16, Lcd_set_dc                                  ' DC high
      !sts Lcd_ctrl_port, R16
   End Macro

   Macro Lcd_write_data
      #if Lcd_use_soft_spi = False                          ' write byte
         Lcd_spi_data = Lcd_databyte
         While Lcd_spi_status.spif = 0 : Wend
      #else
         !lds R16, Lcd_ctrl_port
         Lcd_spi_out_bitbang
      #endif
   End Macro

   Macro Lcd_spi_end                                        ' CS high
      !lds R16, Lcd_ctrl_port
      !ori R16, Lcd_set_cs
      !sts Lcd_ctrl_port, R16
   End Macro
#else

'==============================================================================
   ' Parallel mode subroutines
   #if _xmega = True
      Const Lcd_data_ddr_1 = Lcd_data_port_1 - 4
      #if Lcd_enable_16bit = True
         Const Lcd_data_ddr_2 = Lcd_data_port_2 - 4
      #endif
   #else
      Const Lcd_data_ddr_1 = Lcd_data_port_1 + 1
      #if Lcd_enable_16bit = True
         Const Lcd_data_ddr_2 = Lcd_data_port_2 + 1
      #endif
   #endif

   Const Lcd_pin_wr_bm = 2 ^ Lcd_pin_wr
   Const Lcd_pin_cs_bm = 2 ^ Lcd_pin_cs

   Const Lcd_set_cs = 2 ^ Lcd_pin_cs
   Const Lcd_write_mask = Not(2 ^ Lcd_pin_cs + 2 ^ Lcd_pin_dc + 2 ^ Lcd_pin_wr + 2 ^ Lcd_pin_rd)
   Const Lcd_write_idle_mask = 2 ^ Lcd_pin_cs + 2 ^ Lcd_pin_dc + 2 ^ Lcd_pin_wr + 2 ^ Lcd_pin_rd
   Const Lcd_write_cmd_mask = 2 ^ Lcd_pin_rd + Lcd_write_mask
   Const Lcd_write_data_mask = 2 ^ Lcd_pin_dc + 2 ^ Lcd_pin_rd + Lcd_write_mask

   Macro Lcd_ctrl_init
      'Config Lcd_data_port = Output
      !ldi R16, &HFF
      !Out Lcd_data_ddr_1 , R16
      #if Lcd_enable_16bit = True
         !Out Lcd_data_ddr_2 , R16
      #endif
   End Macro

   Macro Lcd_write_cmd
      !out lcd_data_port_1, R17
      !in R16, Lcd_ctrl_port
      !andi R16, Lcd_write_cmd_mask
      !out Lcd_ctrl_port, R16
      !ori R16, Lcd_write_idle_mask
      !out Lcd_ctrL_port, R16
   End Macro

   Macro Lcd_write_data
      !out lcd_data_port_1, R17
      !in R16, Lcd_ctrl_port
      !andi R16, Lcd_write_data_mask
      !out Lcd_ctrl_port, R16
      !ori R16, Lcd_write_idle_mask
      !out Lcd_ctrL_port, R16
   End Macro

   #if Lcd_enable_16bit = True
      Macro Lcd_write_data_16bit
         !out lcd_data_port_2, R18
         Lcd_write_data
      End Macro
   #endif
#endif

'==============================================================================

#if Lcd_enable_backlight_pwm = True
   #if _xmega = True
      #if Varptr( "Lcd_backlight_port") = Varptr( "Portc")
         #if Lcd_pin_backlight < 4
            Const Lcd_pwm_ctrla = Tcc0_ctrla
         #else
            Const Lcd_pwm_ctrla = Tcc1_ctrla
         #endif
      #elseif Varptr( "Lcd_backlight_port") = Varptr( "Portd")
         #if Lcd_pin_backlight < 4
            Const Lcd_pwm_ctrla = Tcd0_ctrla
         #else
            Const Lcd_pwm_ctrla = Tcd1_ctrla
         #endif
      #elseif Varptr( "Lcd_backlight_port") = Varptr( "Porte")
         #if Lcd_pin_backlight < 4
            Const Lcd_pwm_ctrla = Tce0_ctrla
         #else
            Const Lcd_pwm_ctrla = Tce1_ctrla
         #endif
      #elseif Varptr( "Lcd_backlight_port") = Varptr( "Portf")
         #if Lcd_pin_backlight < 4
            Const Lcd_pwm_ctrla = Tcf0_ctrla
         #else
            Const Lcd_pwm_ctrla = Tcf1_ctrla
         #endif
      #endif
      Const Lcd_pwm_ctrlb = Lcd_pwm_ctrla + 1
      Const Lcd_pwm_ctrle = Lcd_pwm_ctrla + 4
      Const Lcd_pwm_perl = Lcd_pwm_ctrla + &H26
      Const Lcd_pwm_perh = Lcd_pwm_perl + 1
      Const Lcd_pwm_ccl = Lcd_pwm_ctrla + &H28 +(lcd_pin_backlight * 2)
      Const Lcd_pwm_cch = Lcd_pwm_ccl + 1
      Const Lcd_pwm_ctrla_init = 5                          ' Prescaler (1: Clk, 2: Clk/2, 3: Clk/4, 4: Clk/8, 5: Clk/64, 6: Clk/256, 7: Clk/1024)
      Const Lcd_pwm_ctrlb_init = 2 ^(lcd_pin_backlight + 4) + 3       ' Capture Unit (according to pin setting) and Waveform mode (Single Slope)
      Const Lcd_pwm_ctrle_init = 0                          ' Normal (16Bit) mode

      Macro Lcd_pwm_init
         'Config Lcd_pwm_timer = Pwm , Prescale = 64 , Comparea = Enabled , Resolution = 8       ' 3 KHz
         !ldi R16,Lcd_pwm_ctrla_init
         !sts lcd_pwm_ctrla, R16
         !ldi R16,Lcd_pwm_ctrlb_init
         !sts lcd_pwm_ctrlb, R16
         !ldi R16,Lcd_pwm_ctrle_init
         !sts lcd_pwm_ctrle, R16

         !ldi R16, 255
         !clr R17
         !sts Lcd_pwm_perl, R16
         !sts lcd_pwm_perh, R17
      End Macro

      Sub Lcd_set_pwm(byval Value As Byte)
         R16 = Value
         !clr R17
         !sts Lcd_pwm_ccl, R16
         !sts Lcd_pwm_cch, R17
      End Sub
   #else
      Macro Lcd_pwm_init
         #if Varptr( "Lcd_backlight_timer" ) = Varptr( "TIMER0")
            Config Lcd_backlight_timer = Pwm , Prescale = 8 , Pwm = On , Compare_pwm = Clear_up
         #elseif Varptr( "Lcd_backlight_timer" ) = Varptr( "TIMER1")
            Config Lcd_backlight_timer = Pwm , Prescale = 8 , Pwm = 8 , Compare_a_pwm = Clear_up
         #elseif Varptr( "Lcd_backlight_timer" ) = Varptr( "TIMER2")
            #if _chip = 60                                  ' m328
               Config Lcd_backlight_timer = Pwm , Prescale = 8 , Pwm = On , Compare_b_pwm = Clear_up
            #else
               #if Varexist( "Compare2a")
                   Config Lcd_backlight_timer = Pwm , Prescale = 8 , Pwm = On , Compare_a_pwm = Clear_up
               #else
                   Config Lcd_backlight_timer = Pwm , Prescale = 8 , Pwm = On , Compare_pwm = Clear_up
               #endif
            #endif
         #endif
      End Macro

      Sub Lcd_set_pwm(byval Value As Byte)
         #if Varptr( "Lcd_backlight_timer" ) = Varptr( "TIMER0")
            Compare0a = Value
         #elseif Varptr( "Lcd_backlight_timer" ) = Varptr( "TIMER1")
            Compare1a = Value
         #elseif Varptr( "Lcd_backlight_timer" ) = Varptr( "TIMER2")
            #if _chip = 60                                  ' m328
               Compare2b = Value
            #else
               #if Varexist( "Compare2a")
                  Compare2a = Value
               #else
                  Compare2 = Value
               #endif
            #endif
         #endif
      End Sub
   #endif
#endif

'==============================================================================
' Initializes the control interface (parallel/serial), backlight control and the display
Sub Lcd_init()
   !in R16, Lcd_ctrl_port                                   ' set controls in inactive state
   !ori R16, Lcd_write_idle_mask
   !out Lcd_ctrl_port, R16

   'Config Lcd_ctrl_port = &B11111111
   !in R16, Lcd_ctrl_ddr                                    ' control pins as output
   !ori R16, Lcd_write_idle_mask
   !out Lcd_ctrl_ddr, R16

   !in R16, Lcd_reset_port                                  ' set reset in inactive state
   !ori R16, Lcd_reset_high
   !out Lcd_reset_port, R16

   'Config Pind.0 = Output
   !in R16, Lcd_reset_ddr                                   ' reset pin as output
   !ori R16, Lcd_reset_high
   !out Lcd_reset_ddr, R16

   #if Lcd_enable_backlight_pwm = True                      ' init PWM timer
      Lcd_pwm_init
   #endif
   #if Lcd_enable_backlight_pwm = True Or Lcd_enable_backlight = True
      !in R16, Lcd_backlight_ddr                            ' backlight pin as output
      !ori R16, Lcd_backlight_high
      !out Lcd_backlight_ddr, R16
   #endif

   Lcd_ctrl_init                                            ' init Ctrl port (serial/parallel mode)

   Lcd_screen_width = Lcd_width - 1
   Lcd_screen_height = Lcd_height - 1

   'Lcd_reset = False
   !in R16, Lcd_reset_port                                  ' reset low
   !andi R16, Lcd_reset_low
   !out Lcd_reset_port, R16
   Waitms 10

   'Lcd_ctrl_port = Lcd_ctrl_port Or Lcd_write_idle_mask
   !in R16, Lcd_ctrl_port                                   ' ctrl pins high
   !ori R16, Lcd_write_idle_mask
   !out Lcd_ctrl_port, R16

   'Lcd_reset = True
   !in R16, Lcd_reset_port                                  ' reset high
   !ori R16, Lcd_reset_high
   !out Lcd_reset_port, R16
   Waitms 20

   #if Lcd_alternative_init = False
      Lcd_databyte = &HCF : Lcd_write_cmd                   ' Power Control B
      Lcd_databyte = &H00 : Lcd_write_data                  ' always &H00
      Lcd_databyte = &H8B : Lcd_write_data                  ' 81 PCEQ off                           DB!
      Lcd_databyte = &H30 : Lcd_write_data                  ' ESD protection
      #if Lcd_enable_spi = True
         Lcd_spi_end
      #endif

      Lcd_databyte = &HED : Lcd_write_cmd                   ' Power-On sequence control
      Lcd_databyte = &H67 : Lcd_write_data                  ' 64 soft start                         DB!
      Lcd_databyte = &H03 : Lcd_write_data                  ' power on sequence
      Lcd_databyte = &H12 : Lcd_write_data                  ' power on sequence
      Lcd_databyte = &H81 : Lcd_write_data                  ' DDVDH enhance on
      #if Lcd_enable_spi = True
         Lcd_spi_end
      #endif

      Lcd_databyte = &HE8 : Lcd_write_cmd                   ' Driver timing control A
      Lcd_databyte = &H85 : Lcd_write_data                  ' non-overlap
      Lcd_databyte = &H10 : Lcd_write_data                  ' 01 EQ timing                          DB!
      Lcd_databyte = &H7A : Lcd_write_data                  ' 79 Pre-charge timing                  DB!
      #if Lcd_enable_spi = True
         Lcd_spi_end
      #endif

      Lcd_databyte = &HCB : Lcd_write_cmd                   ' Power Control A
      Lcd_databyte = &H39 : Lcd_write_data                  ' always &H39
      Lcd_databyte = &H2C : Lcd_write_data                  ' always &H2C
      Lcd_databyte = &H00 : Lcd_write_data                  ' always &H00
      Lcd_databyte = &H34 : Lcd_write_data                  ' Vcore = 1.6V
      Lcd_databyte = &H02 : Lcd_write_data                  ' DDVDH = 5.6V
      #if Lcd_enable_spi = True
         Lcd_spi_end
      #endif

      Lcd_databyte = &HF7 : Lcd_write_cmd                   ' Pump ratio control
      Lcd_databyte = &H20 : Lcd_write_data                  ' DDVDH=2xVCI
      #if Lcd_enable_spi = True
         Lcd_spi_end
      #endif

      Lcd_databyte = &HEA : Lcd_write_cmd                   ' Driver timing control B
      Lcd_databyte = &H00 : Lcd_write_data                  ' Gate driver timing
      Lcd_databyte = &H00 : Lcd_write_data                  ' always &H00
      #if Lcd_enable_spi = True
         Lcd_spi_end
      #endif

      Lcd_databyte = &HC0 : Lcd_write_cmd                   ' Power control
      Lcd_databyte = &H1B : Lcd_write_data                  ' VRH[5:0]
      #if Lcd_enable_spi = True
         Lcd_spi_end
      #endif

      Lcd_databyte = &HC1 : Lcd_write_cmd                   ' Power control
      Lcd_databyte = &H10 : Lcd_write_data                  ' SAP[2:0];BT[3:0]
      #if Lcd_enable_spi = True
         Lcd_spi_end
      #endif

      Lcd_databyte = &HC5 : Lcd_write_cmd                   ' VC0M control 1
      Lcd_databyte = &H3F : Lcd_write_data
      Lcd_databyte = &H3C : Lcd_write_data
      #if Lcd_enable_spi = True
         Lcd_spi_end
      #endif

      Lcd_databyte = &HC7 : Lcd_write_cmd                   ' VC0M control 2
      Lcd_databyte = &HB7 : Lcd_write_data
      #if Lcd_enable_spi = True
         Lcd_spi_end
      #endif

      Lcd_databyte = &H36 : Lcd_write_cmd                   ' Memory Access Control
      Lcd_databyte = &H08 : Lcd_write_data
      #if Lcd_enable_spi = True
         Lcd_spi_end
      #endif

      Lcd_databyte = &H3A : Lcd_write_cmd                   ' Pixel format
      Lcd_databyte = &H55 : Lcd_write_data                  ' GRAM Data: &H55 = 16bit/pixel (2 bytes), &H56 = 18bit/pixel (3 bytes)
      #if Lcd_enable_spi = True
         Lcd_spi_end
      #endif

      Lcd_databyte = &HB1 : Lcd_write_cmd                   ' frame rate control
      Lcd_databyte = &H00 : Lcd_write_data
      Lcd_databyte = &H1B : Lcd_write_data
      #if Lcd_enable_spi = True
         Lcd_spi_end
      #endif

      Lcd_databyte = &HB6 : Lcd_write_cmd                   ' Display Function Control
      Lcd_databyte = &H0A : Lcd_write_data
      Lcd_databyte = &HA2 : Lcd_write_data
      #if Lcd_enable_spi = True
         Lcd_spi_end
      #endif

      Lcd_databyte = &HF2 : Lcd_write_cmd                   ' 3Gamma Function Disable
      Lcd_databyte = &H00 : Lcd_write_data
      #if Lcd_enable_spi = True
         Lcd_spi_end
      #endif

      Lcd_databyte = &H26 : Lcd_write_cmd                   ' Gamma curve selected
      Lcd_databyte = &H01 : Lcd_write_data
      #if Lcd_enable_spi = True
         Lcd_spi_end
      #endif

      Lcd_databyte = &HE0 : Lcd_write_cmd                   ' Set Gamma positive
      Lcd_databyte = &H0F : Lcd_write_data
      Lcd_databyte = &H2A : Lcd_write_data
      Lcd_databyte = &H28 : Lcd_write_data
      Lcd_databyte = &H08 : Lcd_write_data
      Lcd_databyte = &H0E : Lcd_write_data
      Lcd_databyte = &H08 : Lcd_write_data
      Lcd_databyte = &H54 : Lcd_write_data
      Lcd_databyte = &HA9 : Lcd_write_data
      Lcd_databyte = &H43 : Lcd_write_data
      Lcd_databyte = &H0A : Lcd_write_data
      Lcd_databyte = &H0F : Lcd_write_data
      Lcd_databyte = &H00 : Lcd_write_data
      Lcd_databyte = &H00 : Lcd_write_data
      Lcd_databyte = &H00 : Lcd_write_data
      Lcd_databyte = &H00 : Lcd_write_data
      #if Lcd_enable_spi = True
         Lcd_spi_end
      #endif

      Lcd_databyte = &HE1 : Lcd_write_cmd                   ' Set Gamma negative
      Lcd_databyte = &H00 : Lcd_write_data
      Lcd_databyte = &H15 : Lcd_write_data
      Lcd_databyte = &H17 : Lcd_write_data
      Lcd_databyte = &H07 : Lcd_write_data
      Lcd_databyte = &H11 : Lcd_write_data
      Lcd_databyte = &H06 : Lcd_write_data
      Lcd_databyte = &H2B : Lcd_write_data
      Lcd_databyte = &H56 : Lcd_write_data
      Lcd_databyte = &H3C : Lcd_write_data
      Lcd_databyte = &H05 : Lcd_write_data
      Lcd_databyte = &H10 : Lcd_write_data
      Lcd_databyte = &H0F : Lcd_write_data
      Lcd_databyte = &H3F : Lcd_write_data
      Lcd_databyte = &H3F : Lcd_write_data
      Lcd_databyte = &H0F : Lcd_write_data
      #if Lcd_enable_spi = True
         Lcd_spi_end
      #endif

      Lcd_databyte = &H11 : Lcd_write_cmd                   ' Exit Sleep
      #if Lcd_enable_spi = True
         Lcd_spi_end
      #endif
   #else
      ' Alternative init sequence from Hkipnik
      Lcd_databyte = &HCB : Lcd_write_cmd                   ' Power Control A
      Lcd_databyte = &H39 : Lcd_write_data
      Lcd_databyte = &H2C : Lcd_write_data
      Lcd_databyte = &H00 : Lcd_write_data
      Lcd_databyte = &H34 : Lcd_write_data
      Lcd_databyte = &H02 : Lcd_write_data
      #if Lcd_enable_spi = True
         Lcd_spi_end
      #endif

      Lcd_databyte = &HCF : Lcd_write_cmd                   ' Power Control B
      Lcd_databyte = &H00 : Lcd_write_data                  ' always &H00
      Lcd_databyte = &HC1 : Lcd_write_data                  ' 81 PCEQ off                           DB!
      Lcd_databyte = &H30 : Lcd_write_data                  ' ESD protection
      #if Lcd_enable_spi = True
         Lcd_spi_end
      #endif

      Lcd_databyte = &HE8 : Lcd_write_cmd                   ' Driver timing control A
      Lcd_databyte = &H85 : Lcd_write_data                  ' non-overlap
      Lcd_databyte = &H00 : Lcd_write_data                  ' 01 EQ timing                          DB!
      Lcd_databyte = &H78 : Lcd_write_data                  ' 79 Pre-charge timing                  DB!
      #if Lcd_enable_spi = True
         Lcd_spi_end
      #endif

      Lcd_databyte = &HEA : Lcd_write_cmd                   ' Driver timing control B
      Lcd_databyte = &H00 : Lcd_write_data                  ' Gate driver timing
      Lcd_databyte = &H00 : Lcd_write_data                  ' always &H00
      #if Lcd_enable_spi = True
         Lcd_spi_end
      #endif


      Lcd_databyte = &HED : Lcd_write_cmd                   ' Power-On sequence control
      Lcd_databyte = &H64 : Lcd_write_data                  ' 64 soft start                         DB!
      Lcd_databyte = &H03 : Lcd_write_data                  ' power on sequence
      Lcd_databyte = &H12 : Lcd_write_data                  ' power on sequence
      Lcd_databyte = &H81 : Lcd_write_data                  ' DDVDH enhance on
      #if Lcd_enable_spi = True
         Lcd_spi_end
      #endif

      Lcd_databyte = &HF7 : Lcd_write_cmd                   ' Pump ratio control
      Lcd_databyte = &H20 : Lcd_write_data                  ' DDVDH=2xVCI
      #if Lcd_enable_spi = True
         Lcd_spi_end
      #endif

      Lcd_databyte = &HC0 : Lcd_write_cmd                   ' Power control
      Lcd_databyte = &H23 : Lcd_write_data                  ' VRH[5:0]
      #if Lcd_enable_spi = True
         Lcd_spi_end
      #endif

      Lcd_databyte = &HC1 : Lcd_write_cmd                   ' Power control
      Lcd_databyte = &H10 : Lcd_write_data                  ' SAP[2:0];BT[3:0]
      #if Lcd_enable_spi = True
         Lcd_spi_end
      #endif

      Lcd_databyte = &HC5 : Lcd_write_cmd                   ' VC0M control 1
      Lcd_databyte = &H3E : Lcd_write_data
      Lcd_databyte = &H28 : Lcd_write_data
      #if Lcd_enable_spi = True
         Lcd_spi_end
      #endif

      Lcd_databyte = &HC7 : Lcd_write_cmd                   ' VC0M control 2
      Lcd_databyte = &H86 : Lcd_write_data
      #if Lcd_enable_spi = True
         Lcd_spi_end
      #endif

      Lcd_databyte = &H36 : Lcd_write_cmd                   ' Memory Access Control
      Lcd_databyte = &H08 : Lcd_write_data                  ' Hkipnik original: Lcd_mode = 1 -> &B0100_1000; Lcd_mode = else -> &B1110_1000
      #if Lcd_enable_spi = True
         Lcd_spi_end
      #endif

      Lcd_databyte = &H3A : Lcd_write_cmd                   ' Pixel format
      Lcd_databyte = &H55 : Lcd_write_data                  ' GRAM Data: &H55 = 16bit/pixel (2 bytes), &H56 = 18bit/pixel (3 bytes)
      #if Lcd_enable_spi = True
         Lcd_spi_end
      #endif

      Lcd_databyte = &HB1 : Lcd_write_cmd                   ' frame rate control (In Normal Mode/Full Colors)
      Lcd_databyte = &H00 : Lcd_write_data
      Lcd_databyte = &H18 : Lcd_write_data
      #if Lcd_enable_spi = True
         Lcd_spi_end
      #endif

      Lcd_databyte = &HB6 : Lcd_write_cmd                   ' Display Function Control
      Lcd_databyte = &H08 : Lcd_write_data
      Lcd_databyte = &H82 : Lcd_write_data
      Lcd_databyte = &H27 : Lcd_write_data
      #if Lcd_enable_spi = True
         Lcd_spi_end
      #endif

      Lcd_databyte = &HF2 : Lcd_write_cmd                   ' 3Gamma Function Disable
      Lcd_databyte = &H00 : Lcd_write_data
      #if Lcd_enable_spi = True
         Lcd_spi_end
      #endif

      Lcd_databyte = &H26 : Lcd_write_cmd                   ' Gamma curve selected
      Lcd_databyte = &H01 : Lcd_write_data
      #if Lcd_enable_spi = True
         Lcd_spi_end
      #endif


      Lcd_databyte = &HE0 : Lcd_write_cmd                   ' Set Gamma positive
      Lcd_databyte = &H0F : Lcd_write_data
      Lcd_databyte = &H31 : Lcd_write_data
      Lcd_databyte = &H2B : Lcd_write_data
      Lcd_databyte = &H0C : Lcd_write_data
      Lcd_databyte = &H0E : Lcd_write_data
      Lcd_databyte = &H08 : Lcd_write_data
      Lcd_databyte = &H4E : Lcd_write_data
      Lcd_databyte = &HF1 : Lcd_write_data
      Lcd_databyte = &H37 : Lcd_write_data
      Lcd_databyte = &H07 : Lcd_write_data
      Lcd_databyte = &H10 : Lcd_write_data
      Lcd_databyte = &H03 : Lcd_write_data
      Lcd_databyte = &H0E : Lcd_write_data
      Lcd_databyte = &H09 : Lcd_write_data
      Lcd_databyte = &H00 : Lcd_write_data
      #if Lcd_enable_spi = True
         Lcd_spi_end
      #endif

      Lcd_databyte = &HE1 : Lcd_write_cmd                   ' Set Gamma negative
      Lcd_databyte = &H00 : Lcd_write_data
      Lcd_databyte = &H0E : Lcd_write_data
      Lcd_databyte = &H14 : Lcd_write_data
      Lcd_databyte = &H03 : Lcd_write_data
      Lcd_databyte = &H11 : Lcd_write_data
      Lcd_databyte = &H07 : Lcd_write_data
      Lcd_databyte = &H31 : Lcd_write_data
      Lcd_databyte = &HC1 : Lcd_write_data
      Lcd_databyte = &H48 : Lcd_write_data
      Lcd_databyte = &H08 : Lcd_write_data
      Lcd_databyte = &H0F : Lcd_write_data
      Lcd_databyte = &H0C : Lcd_write_data
      Lcd_databyte = &H31 : Lcd_write_data
      Lcd_databyte = &H36 : Lcd_write_data
      Lcd_databyte = &H0F : Lcd_write_data
      #if Lcd_enable_spi = True
         Lcd_spi_end
      #endif
   #endif

   Waitms 5
   Lcd_databyte = &H29 : Lcd_write_cmd                      ' Display On
   #if Lcd_enable_spi = True
      Lcd_spi_end
   #endif
   Waitms 10

   Lcd_clear &H0000
   'Waitms 10
   #if Lcd_enable_backlight = True Or Lcd_enable_backlight_pwm = True
      Lcd_backlight_dim 0 , &HFF , 400                      ' backlight on
   #endif
End Sub

'==============================================================================

#if Lcd_enable_backlight = True Or Lcd_enable_backlight_pwm = True

   ' Sets the backlight value (0: Off; 1-254: PWM, 255: On)
   ' enters standby mode when display is off (0) and reenables the display when the backlight is activated again
   Sub Lcd_backlight(byval Value As Byte)
      Local Lcd_off As Byte
      Lcd_off = 2
      #if Lcd_enable_backlight_pwm = True
         #if Lcd_invert_backlight = False
            Lcd_set_pwm Value
            If Value = 0 Then
               If Lcd_sleepmode = False Then Lcd_off = True
            Else
               If Lcd_sleepmode = True Then Lcd_off = False
            End If
         #else
            Value = &HFF - Value
            Lcd_set_pwm Value
            If Value = &HFF Then
               If Lcd_sleepmode = False Then Lcd_off = True
            Else
               If Lcd_sleepmode = True Then Lcd_off = False
            End If
         #endif
      #else
         #if Lcd_invert_backlight = False
            If Value = 255 Then
               'Lcd_ctrl_port.lcd_pin_backlight = True
               !in R16, Lcd_backlight_port
               !ori R16, Lcd_backlight_high
               !out Lcd_backlight_port, R16
               If Lcd_sleepmode = True Then Lcd_off = False
            Else
               'Lcd_ctrl_port.lcd_pin_backlight = False
               !in R16, Lcd_backlight_port
               !andi R16, Lcd_backlight_low
               !out Lcd_backlight_port, R16
               If Lcd_sleepmode = False Then Lcd_off = True
            End If
         #else
            If Value = 0 Then
               'Lcd_ctrl_port.lcd_pin_backlight = False
               !in R16, Lcd_backlight_port
               !andi R16, Lcd_backlight_low
               !out Lcd_backlight_port, R16
               If Lcd_sleepmode = True Then Lcd_off = False
            Else
               'Lcd_ctrl_port.lcd_pin_backlight = True
               !in R16, Lcd_backlight_port
               !ori R16, Lcd_backlight_high
               !out Lcd_backlight_port, R16
               If Lcd_sleepmode = False Then Lcd_off = True
            End If
         #endif
      #endif
      If Lcd_off <> 2 Then
         Lcd_sleep Lcd_off
      End If
   End Sub

'==============================================================================

   ' smoothly dims the backlight
   Sub Lcd_backlight_dim(byval Startvalue As Byte , Byval Endvalue As Byte , Byval Millisecs As Word)
      Local Totalus As Dword
      Local Stepsize As Byte

      Totalus = Millisecs * 1000
      If Startvalue < Endvalue Then
         Stepsize = Endvalue - Startvalue
         Totalus = Totalus / Stepsize
         Millisecs = Totalus
         For Stepsize = Startvalue To Endvalue
            Lcd_backlight Stepsize
            Waitus Millisecs
         Next
      Else
         Stepsize = Startvalue - Endvalue
         Totalus = Totalus / Stepsize
         Millisecs = Totalus
         For Stepsize = Startvalue To Endvalue Step -1
            Lcd_backlight Stepsize
            Waitus Millisecs
         Next
      End If
   End Sub
#endif

'==============================================================================
Dim Lcd_sleepmode As Bit

Sub Lcd_sleep(byval Value As Byte)
   If Value = False Then
      Lcd_databyte = &H11                                   ' exit sleep
   Else
      Lcd_databyte = &H10                                   ' enter sleep
   End If
   Lcd_write_cmd
   If Value = False Then Waitms 120
   Lcd_sleepmode = Value
   #if Lcd_enable_spi = True
      Lcd_spi_end
   #endif
End Sub

'==============================================================================

' madctl_mx = &H40
' madctl_my = &H80
' madctl_mv = &H20
' madctl_ml = &H10
' madctl_rgb = &H00
' madctl_bgr = &H08
' madctl_mh = &H04

' current rotation
Dim Lcd_rotation As Byte

' Rotates the display (0: Portrait (Standard), 1: Landscape, 2: Portrait 180°, 3: Landscape 180°)
Sub Lcd_set_rotation(byval Rotation As Byte)
   Lcd_rotation = Rotation Mod 4
   Lcd_databyte = &H36 : Lcd_write_cmd                      ' madctl
   Select Case Lcd_rotation
   Case 0:
      Lcd_screen_width = Lcd_width - 1
      Lcd_screen_height = Lcd_height - 1
      Lcd_databyte = &H08                                   ' madctl_bgr (?)
   Case 1:
      Lcd_screen_width = Lcd_height - 1
      Lcd_screen_height = Lcd_width - 1
      Lcd_databyte = &H68                                   ' madctl_mv + madctl_mx + madctl_bgr
   Case 2:
      Lcd_screen_width = Lcd_width - 1
      Lcd_screen_height = Lcd_height - 1
      Lcd_databyte = &HC8                                   ' madctl_mx + madctl_my + madctl_bgr
   Case 3:
      Lcd_screen_width = Lcd_height - 1
      Lcd_screen_height = Lcd_width - 1
      Lcd_databyte = &HA8                                   ' madctl_mv + madctl_my + madctl_bgr
   End Select
   Lcd_write_data
   #if Lcd_enable_spi = True
      Lcd_spi_end
   #endif
End Sub

'==============================================================================

' define the window to write pixel data to
Sub Lcd_set_window(byval X1 As Word , Byval Y1 As Word , Byval X2 As Word , Byval Y2 As Word)
   Lcd_databyte = &H2A : Lcd_write_cmd
   !ldd R26, Y+6                                            ' x1
   !ldd R27, Y+7
   !ld R18, X+
   !ld R17, X
   Lcd_write_data                                           ' h
   !mov R17, R18
   Lcd_write_data                                           ' l
   !ldd R26, Y+2                                            ' x2
   !ldd R27, Y+3
   !ld R18, X+
   !ld R17, X
   Lcd_write_data                                           ' h
   !mov R17, R18
   Lcd_write_data                                           ' l
   #if Lcd_enable_spi = True
      Lcd_spi_end
   #endif

   Lcd_databyte = &H2B : Lcd_write_cmd
   !ldd R26, Y+4                                            ' y1
   !ldd R27, Y+5
   !ld R18, X+
   !ld R17, X
   Lcd_write_data                                           ' h
   !mov R17, R18
   Lcd_write_data                                           ' l
   !ldd R26, Y+0                                            ' y2
   !ldd R27, Y+1
   !ld R18, X+
   !ld R17, X
   Lcd_write_data                                           ' h
   !mov R17, R18
   Lcd_write_data                                           ' l
   #if Lcd_enable_spi = True
      Lcd_spi_end
   #endif

   Lcd_databyte = &H2C : Lcd_write_cmd
End Sub


'-------------------------------------------------------------------------------
' Filled rectangle
'-------------------------------------------------------------------------------
Sub Lcd_fill_rect(byval X1 As Word , Byval Y1 As Word , Byval X2 As Word , Byval Y2 As Word , Byval Color As Word)
   If Lcd_screen_width < X1 Then X1 = Lcd_screen_width
   If Lcd_screen_width < X2 Then X2 = Lcd_screen_width
   If Lcd_screen_height < Y1 Then Y1 = Lcd_screen_height
   If Lcd_screen_height < Y2 Then Y2 = Lcd_screen_height
   If X1 > X2 Then Swap X1 , X2
   If Y1 > Y2 Then Swap Y1 , Y2
   Lcd_set_window X1 , Y1 , X2 , Y2

   !ldd R26, Y+6                                            ' Y1
   !ldd R27, Y+7
   !ld R20, X+
   !ld R21, X
   !ldd R26, Y+2                                            ' Y2
   !ldd R27, Y+3
   !ld R24, X+
   !ld R25, X
   !SUB R24, R20                                            ' Y2 = Y2 - Y1 + 1
   !sbc R25, R21
   !adiw R24, 1

   !ldd R26, Y+8                                            ' X1
   !ldd R27, Y+9
   !ld R18, X+
   !ld R19, X
   !ldd R26, Y+4                                            ' X2
   !ldd R27, Y+5
   !ld R30, X+
   !ld R31, X
   !Sub R30 , R18                                           ' X2 = X2 - X1 + 1
   !sbc R31, R19
   !adiw R30, 1
   !movw R20, R30
'   !mov R21, R31

   !ldd R26, Y+0                                            ' Colorlow = Low(color)
   !ldd R27, Y+1                                            ' Colorhigh = High(color)
   #if Lcd_enable_16bit = False
      !ld R18, X+
      !ld R19, X
   #else
      !ld R17, X+
      !ld R18, X
   #endif

   Lcd_rect_y_loop:                                         ' For Y = Y1 To Y2
      !movw R30, R20                                        '    For X = X1 To X2
'      !mov R31, R21
      Lcd_rect_x_loop:
         #if Lcd_enable_16bit = False
            !mov R17, R19                                   '      write pixel data
            Lcd_write_data
            !mov R17, R18
            Lcd_write_data
         #else
            Lcd_write_data_16bit
         #endif
      !sbiw R30, 1                                          '    Next
      !brne Lcd_rect_x_loop
   !sbiw R24, 1                                             ' Next
   !brne Lcd_rect_y_loop

   #if Lcd_enable_spi = True
      Lcd_spi_end
   #endif
End Sub

'==============================================================================

Sub Lcd_set_pixel(byval X As Word , Byval Y As Word , Byval Color As Word)
   Lcd_databyte = &H2A
   Lcd_write_cmd
   !ldd R26, Y+4
   !ldd R27, Y+5
   !ld R1, X+
   !ld R2, X
   !mov R17, R2
   Lcd_write_data
   !mov R17, R1
   Lcd_write_data
   !mov R17, R2
   Lcd_write_data
   !mov R17, R1
   Lcd_write_data
   #if Lcd_enable_spi = True
      Lcd_spi_end
   #endif

   Lcd_databyte = &H2B
   Lcd_write_cmd
   !ldd R26, Y+2
   !ldd R27, Y+3
   !ld R1, X+
   !ld R2, X
   !mov R17, R2
   Lcd_write_data
   !mov R17, R1
   Lcd_write_data
   !mov R17, R2
   Lcd_write_data
   !mov R17, R1
   Lcd_write_data
   #if Lcd_enable_spi = True
      Lcd_spi_end
   #endif

   Lcd_databyte = &H2C
   Lcd_write_cmd
   !ldd R26, Y+0
   !ldd R27, Y+1
   #if Lcd_enable_16bit = False
      !ld R1, X+
      !ld R17, X
      Lcd_write_data
      !mov R17, R1
      Lcd_write_data
   #else
      !ld R17, X+
      !ld R18, X
      Lcd_write_data_16bit
   #endif
   #if Lcd_enable_spi = True
      Lcd_spi_end
   #endif
End Sub

'-------------------------------------------------------------------------------
' Draw 16-bit BIN Picture from Flash      X start - Y start - Width - Height
'               before Call Sub you must Restore the Label
'-------------------------------------------------------------------------------
Sub Lcd_pic_flash(byval Xs As Word , Byval Ys As Word , Byval Width As Byte , Byval Height As Word)
   Local Count As Dword , Length As Dword
   Local X2 As Word , Y2 As Word
   #if Lcd_enable_16bit = False
      Local Color As Byte
   #else
      Local Color As Word
   #endif
   Length = Width * Height
   X2 = Width + Xs : Y2 = Height + Ys
   Decr X2 : Decr Y2
   Lcd_set_window Xs , Ys , X2 , Y2
   For Count = 1 To Length
      #if Lcd_enable_16bit = False
         Read Color : Lcd_databyte = Color : Lcd_write_data
         Read Color : Lcd_databyte = Color : Lcd_write_data
      #else
         Read Color
         Lcd_databyte = High(color)
         Lcd_databyte2 = Low(color)
         Lcd_write_data_16bit
      #endif
   Next Count
   #if Lcd_enable_spi = True
      Lcd_spi_end
   #endif
End Sub

'-------------------------------------------------------------------------------
' Draw 16-bit BIN Picture from SPI-ROM
'                X start - Y start - Width - Height - Address
'-------------------------------------------------------------------------------
Sub Lcd_pic_spi(byval Xs As Word , Byval Ys As Word , Byval Width As Word , Byval Height As Word , Byval Address As Dword)
   Local Count As Dword , Length As Dword
   Local X2 As Word , Y2 As Word
   #if Lcd_enable_16bit = False
      Local Color As Byte
   #else
      Local Color As Word
   #endif
   Length = Width * Height
   X2 = Width + Xs : Y2 = Height + Ys
   Decr X2 : Decr Y2
   Flash_beginread Address
   Lcd_set_window Xs , Ys , X2 , Y2
   For Count = 1 To Length
      #if Lcd_enable_16bit = False
         Color = Flash_readbyte() : Lcd_databyte = Color : Lcd_write_data
         Color = Flash_readbyte() : Lcd_databyte = Color : Lcd_write_data
      #else
         Color = Flash_readbyte()
         Lcd_databyte = Color
         Color = Flash_readbyte()
         Lcd_databyte2 = Color
         Lcd_write_data_16bit
      #endif
   Next Count
   Flash_endread
End Sub

#if _huge = True                                            ' external SRAM attached
   ' pointer to SRAM location where to start storing the pixel data (not set automatically, needs an init value first!)
   Dim Lcd_ram_pics_free_ptr As Dword

   ' loads a BIN picture from flash to a SRAM location (set by Lcd_ram_pics_free_ptr) and returns the location of the current picture
   Function Lcd_load_pic_flash2ram(byval Width As Word , Byval Height As Word) As Dword
      Local Length As Dword
      Local Picaddress As Dword
#else
   ' same as above, normal memory range
   Dim Lcd_ram_pics_free_ptr As Word

   Function Lcd_load_pic_flash2ram(byval Width As Word , Byval Height As Word) As Word
      Local Length As Word
      Local Picaddress As Word
#endif
   Local Databyte As Byte
   Length = Width * Height
   Length = Length * 2
   Picaddress = Lcd_ram_pics_free_ptr
   Do
      Read Databyte
      Out Lcd_ram_pics_free_ptr , Databyte
      Incr Lcd_ram_pics_free_ptr
      Decr Length
   Loop Until Length = 0
   Lcd_load_pic_flash2ram = Picaddress
End Function

' Draws a BIN picture from the SRAM
#if _huge = True
   Sub Lcd_pic_ram(byval Xs As Word , Byval Ys As Word , Byval Width As Word , Byval Height As Word , Byval Address As Dword)
      Local Ramptemp As Byte
#else
   Sub Lcd_pic_ram(byval Xs As Word , Byval Ys As Word , Byval Width As Word , Byval Height As Word , Byval Address As Word)
#endif
   Local X2 As Word , Y2 As Word
   #if _huge = True
      Ramptemp = Rampz
   #endif
   X2 = Width + Xs : Y2 = Height + Ys
   Decr X2 : Decr Y2
   Lcd_set_window Xs , Ys , X2 , Y2

   !ldd R26, Y+8                                            ' height
   !ldd R27, Y+9
   !ld R24, X+
   !ld R25, X

   !ldd R26, Y+10                                           ' width
   !ldd R27, Y+11
   !ld R12, X+
   !ld R13, X

   !ldd R26, Y+6                                            ' Address
   !ldd R27, Y+7
   !ld R30, X+
   !ld R31, X+
   #if _huge = True
      !ld R16, X
      !out Rampz, R16
   #endif

   Lcd_pic_ram_y_loop:
      !movw R26, R12
      Lcd_pic_ram_x_loop:
         #if Lcd_enable_16bit = False
            !ld R17, Z+
            Lcd_write_data
            !ld R17, Z+
            Lcd_write_data
         #else
            !ld R18, Z+
            !ld R17, Z+
            Lcd_write_data_16bit
         #endif
      !sbiw R26, 1
      !brne Lcd_pic_ram_x_loop
   !sbiw R24, 1
   !brne Lcd_pic_ram_y_loop

   #if Lcd_enable_spi = True
      Lcd_spi_end
   #endif
   #if _huge = True
      Rampz = Ramptemp
   #endif
End Sub

'-------------------------------------------------------------------------------
' Draw BGF Picture              X Start -  Y Start
'                before Call Sub you must Restore the Label
'-------------------------------------------------------------------------------
Sub Lcd_pic_bgf(byval Xoffset As Word , Byval Yoffset As Word)
   Local Ysize As Word , Xsize As Word , Imagesize As Word
   Local L As Word , Pixel As Byte , Repeat As Byte
   Local Color_l As Byte , Color_h As Byte
   Read Pixel                                               'Read the first 4 bytes from the BGF, this contains info about the image
   Ysize = Pixel + Yoffset
   Ysize = Ysize - 1
   Read Pixel
   Xsize = Pixel + Xoffset
   Xsize = Xsize - 1
   Read Imagesize
   Color_l = Xoffset
   Color_h = Xsize
   Lcd_set_window Color_l , Yoffset , Color_h , Ysize
   Do
      Read Pixel                                            'Read the value for the pixel
      If Pixel = &HAA Then                                  'BGF file is RLE coded &HAA represents this, the byte after &HAA is the number of times that the byte before &HAA wil be repeated
         Read Repeat                                        'Byte after &HAA contains the number of times to repeat
         Repeat = Repeat - 1
         For L = 1 To Repeat                                'Loop the repeat
            #if Lcd_enable_16bit = False
               Lcd_databyte = Color_h : Lcd_write_data      'Write the pixel data to the display
               Lcd_databyte = Color_l : Lcd_write_data
            #else
               Lcd_databyte = Color_l
               Lcd_databyte2 = Color_h
               Lcd_write_data_16bit
            #endif
         Next L                                             'Next repeat loop
         Imagesize = Imagesize - Repeat                     'Decrease the bytes still to be processed
      Else                                                  'It's a regular byte, not RLE encoded
         Color_l = 0 : Color_h = 0                          'Convert the 8bit RRRGGGBB bgc data to 16 bit RRRRRGGGGGGBBBBB
         If Pixel.0 = True Then Color_l = &H0A
         If Pixel.1 = True Then Color_l = Color_l Or &H15
         If Pixel.2 = True Then
            Color_l = Color_l Or &H20
            Color_h = &H01
         End If
         If Pixel.3 = True Then
            Color_l = Color_l Or &H40
            Color_h = Color_h Or &H02
         End If
         If Pixel.4 = True Then
            Color_l = Color_l Or &H80
            Color_h = Color_h Or &H04
         End If
         If Pixel.5 = True Then Color_h = Color_h Or &H20
         If Pixel.6 = True Then Color_h = Color_h Or &H48
         If Pixel.7 = True Then Color_h = Color_h Or &H90
         #if Lcd_enable_16bit = False
            Lcd_databyte = Color_h : Lcd_write_data         'Write the pixel data to the display
            Lcd_databyte = Color_l : Lcd_write_data
         #else
            Lcd_databyte = Color_l
            Lcd_databyte2 = Color_h
            Lcd_write_data_16bit
         #endif
         Imagesize = Imagesize - 1                          'Decrease the bytes to process
      End If
   Loop Until Imagesize = 0                                 'Do until all bytes have been processed
   #if Lcd_enable_spi = True
      Lcd_spi_end
   #endif
End Sub


'-------------------------------------------------------------------------------
' Clear LCD with color
'-------------------------------------------------------------------------------
Sub Lcd_clear(byval Color As Word)
   Lcd_fill_rect 0 , 0 , Lcd_screen_width , Lcd_screen_height , Color
End Sub


'-------------------------------------------------------------------------------
' Invert the display colors
'-------------------------------------------------------------------------------
Sub Lcd_invert(byval Inverted As Byte )
   If Inverted = False Then
      Lcd_databyte = &H20
   Else
      Lcd_databyte = &H21
   End If
   Lcd_write_cmd
   #if Lcd_enable_spi = True
      Lcd_spi_end
   #endif
End Sub


'-------------------------------------------------------------------------------
' Text
'-------------------------------------------------------------------------------

Dim Lcd_chartable As Word
#if Varexist( "RAMPZ")
   Dim Lcd_chartable_rampz As Byte
#endif
Dim Lcd_tempword As Word

Sub Lcd_loadfont()
   !sts {Lcd_chartable}, _DPTRL
   !sts {Lcd_chartable+1}, _DPTRH
   #if Varexist( "RAMPZ")
      Lcd_chartable_rampz = ___readrampz
   #endif
End Sub

Sub Lcd_text(byval S As String , Byval Xoffset As Word , Byval Yoffset As Word , Byval Forecolor As Word , Byval Backcolor As Word)
   Local A As Word , Pixels As Byte , Count As Byte , Charcount As Byte , Temp As Word
   Local Row As Byte , Block As Byte , Byteseach As Byte , Blocksize As Byte
   Local Colums As Byte , Columcount As Byte , Rowcount As Byte , Stringsize As Byte
   Local Pixelcount As Byte , Offset As Word
   Local Pointer As Word
   Local X As Word , Y As Word

   Stringsize = Len(s) - 1                                  'Size of the text string -1 because we must start with 0
   !Lds _DPTRL , {Lcd_chartable}                            'Restore Font Label
   !Lds _DPTRH , {Lcd_chartable+1}
   #if Varexist( "RAMPZ")
      ___readrampz = Lcd_chartable_rampz
   #endif
   Read Block
   Read Byteseach
   Read Blocksize
   Colums = Blocksize / Block                               'Calculate the numbers of columns
   Row = Block * 8                                          'Row is always 8 pixels high = 1 byte, so working with row in steps of 8.
   Row = Row - 1                                            'Want to start with row=0 instead of 1
   Colums = Colums - 1                                      'Same for the colums
   Pointer = Varptr(s)
   For Charcount = 0 To Stringsize                          'Loop for the numbers of characters that must be displayed
      Offset = Inp(pointer)                                 'Font files start with character 32
      Offset = Offset - 32
      Incr Pointer
      If Lcd_digit_font = 1 And Offset > 14 Then Offset = Offset - 15       'For reduced Digit Font
      Offset = Offset * Blocksize
      Offset = Offset + 4
      Lcd_tempword = Lcd_chartable + Offset                 'Point to Character
      !Lds _DPTRL , {Lcd_tempword}
      !Lds _DPTRH , {Lcd_tempword+1}
      #if Varexist( "RAMPZ")
         ___readrampz = Lcd_chartable_rampz
      #endif

      Temp = Charcount * Byteseach
      Temp = Temp + Xoffset
      For Rowcount = 0 To Row Step 8                        'Loop for numbers of rows
         Y = Yoffset + Rowcount
         X = Temp
         For Columcount = 0 To Colums                       'Loop for numbers of Columns
            Read Pixels
            For Pixelcount = 0 To 7                         'Loop for 8 pixels to be set or not
               If Pixels.pixelcount = 1 Then
                  Lcd_set_pixel X , Y , Forecolor
               Else
                  Lcd_set_pixel X , Y , Backcolor
               End If
               If Lcd_color_font = False Then
                  Incr Y
               Else
                  Incr X
               End If
            Next Pixelcount
            If Lcd_color_font = False Then
               Y = Y - 8                                    'Do some calculation to get the character on the correct Xposition
               Incr X                                       'Increment Y position
            Else
               X = X - 8
               Incr Y
            End If
         Next Columcount
      Next Rowcount
   Next Charcount
End Sub


'-------------------------------------------------------------------------------
' Draw Line (Bresenham)  left-top (x1,y1) right-bottom(x2,y2),
'-------------------------------------------------------------------------------
Sub Lcd_line(byval X1 As Word , Byval Y1 As Word , Byval X2 As Word , Byval Y2 As Word , Byval Color As Word)
   Local Adx As Integer , Ady As Integer
   Local Sdx As Integer , Sdy As Integer
   Local Pdx As Integer , Pdy As Integer
   Local Ddx As Integer , Ddy As Integer
   Local Es As Integer , El As Integer
   Local Error As Integer
   Local I As Integer

   Adx = X2 - X1
   Ady = Y2 - Y1

   'Adx = Abs(dx)
   'Sdx = Sgn(dx)
   If Adx < 0 Then
      Adx = -adx
      Sdx = -1
   Else
      Sdx = 1
   End If
   'Ady = Abs(dy)
   'Sdy = Sgn(dy)
   If Ady < 0 Then
      Ady = -ady
      Sdy = -1
   Else
      Sdy = 1
   End If

   If Adx > Ady Then
      Pdx = Sdx : Pdy = 0
      Ddx = Sdx : Ddy = Sdy
      Es = Ady : El = Adx
   Else
      Pdx = 0 : Pdy = Sdy
      Ddx = Sdx : Ddy = Sdy
      Es = Adx : El = Ady
   End If

   Lcd_set_pixel X1 , Y1 , Color
   Error = El / 2

   For I = 1 To El
      Error = Error - Es
      If Error < 0 Then
         Error = Error + El
         X1 = X1 + Ddx : Y1 = Y1 + Ddy
      Else
         X1 = X1 + Pdx : Y1 = Y1 + Pdy
      End If
      Lcd_set_pixel X1 , Y1 , Color
   Next
End Sub


'-------------------------------------------------------------------------------
' Draw Line (Bresenham)  left-top (x1,y1) right-bottom(x2,y2), with color gradient (slow)
'-------------------------------------------------------------------------------
Sub Lcd_line_gradient(byval X1 As Word , Byval Y1 As Word , Byval X2 As Word , Byval Y2 As Word , Byval Color1 As Word , Byval Color2 As Word)
   Local Adx As Integer , Ady As Integer
   Local Sdx As Integer , Sdy As Integer
   Local Pdx As Integer , Pdy As Integer
   Local Ddx As Integer , Ddy As Integer
   Local Es As Integer , El As Integer
   Local Error As Integer
   Local I As Integer

   Local R1 As Byte , G1 As Byte , B1 As Byte
   Local R2 As Byte , G2 As Byte , B2 As Byte

   Local R As Single , G As Single , B As Single
   Local Rinc As Single , Ginc As Single , Binc As Single

   Color16to24 Color1 , R1 , G1 , B1
   Color16to24 Color2 , R2 , G2 , B2

   Adx = X2 - X1
   Ady = Y2 - Y1

   G = Adx
   G = G * G
   B = Ady
   B = B * B
   B = G + B
   B = Sqr(b)
   B = B - 1

   Rinc = R2
   Rinc = Rinc - R1
   Rinc = Rinc / B

   Ginc = G2
   Ginc = Ginc - G1
   Ginc = Ginc / B

   Binc = B2
   Binc = Binc - B1
   Binc = Binc / B

   R = R1
   G = G1
   B = B1

   'Adx = Abs(dx)
   'Sdx = Sgn(dx)
   If Adx < 0 Then
      Adx = -adx
      Sdx = -1
   Else
      Sdx = 1
   End If
   'Ady = Abs(dy)
   'Sdy = Sgn(dy)
   If Ady < 0 Then
      Ady = -ady
      Sdy = -1
   Else
      Sdy = 1
   End If

   If Adx > Ady Then
      Pdx = Sdx : Pdy = 0
      Ddx = Sdx : Ddy = Sdy
      Es = Ady : El = Adx
   Else
      Pdx = 0 : Pdy = Sdy
      Ddx = Sdx : Ddy = Sdy
      Es = Adx : El = Ady
   End If

   Color1 = Color24to16(r1 , G1 , B1)
   Lcd_set_pixel X1 , Y1 , Color1
   Error = El / 2

   For I = 1 To El
      R = R + Rinc
      G = G + Ginc
      B = B + Binc
      R1 = R + .5
      G1 = G + .5
      B1 = B + .5
      Color1 = Color24to16(r1 , G1 , B1)
      Error = Error - Es
      If Error < 0 Then
         Error = Error + El
         X1 = X1 + Ddx : Y1 = Y1 + Ddy
      Else
         X1 = X1 + Pdx : Y1 = Y1 + Pdy
      End If
      Lcd_set_pixel X1 , Y1 , Color1
   Next
End Sub


'-------------------------------------------------------------------------------
' Circle (Bresenham)
'-------------------------------------------------------------------------------
Sub Lcd_circle(byval Xcenter As Word , Byval Ycenter As Word , Byval Radius As Word , Byval Color As Word)
   Local X As Word , Y As Word
   Local Xoffset As Integer , Yoffset As Integer
   Local D As Integer
   Local Error As Integer
   Xoffset = Radius
   Yoffset = 0
   Error = Radius
   X = Xcenter + Xoffset
   Y = Ycenter + Yoffset
   Incr Lcd_screen_width
   Incr Lcd_screen_height
   If X < Lcd_screen_width And Y < 320 Then
      Lcd_set_pixel X , Y , Color
      If Xcenter >= Xoffset Then
         X = Xcenter - Xoffset
         Lcd_set_pixel X , Y , Color
      End If
      Y = Ycenter + Xoffset
      If Y < Lcd_screen_height Then
         X = Xcenter
         Lcd_set_pixel X , Y , Color
         If Ycenter >= Xoffset Then
            Y = Ycenter - Xoffset
            Lcd_set_pixel X , Y , Color
         End If
      End If
   End If

   While Yoffset < Xoffset
      D = Yoffset * 2
      Incr D
      Incr Yoffset
      Error = Error - D
      If Error < 0 Then
         D = Xoffset * 2
         D = 1 - D
         Xoffset = Xoffset - 1
         Error = Error - D
      End If
      X = Xcenter + Xoffset
      Y = Ycenter + Yoffset
      If X < Lcd_screen_width And Y < Lcd_screen_height Then Lcd_set_pixel X , Y , Color
      X = Xcenter - Xoffset
      If X < Lcd_screen_width And Y < Lcd_screen_height Then Lcd_set_pixel X , Y , Color
      Y = Ycenter - Yoffset
      If X < Lcd_screen_width And Y < Lcd_screen_height Then Lcd_set_pixel X , Y , Color
      X = Xcenter + Xoffset
      If X < Lcd_screen_width And Y < Lcd_screen_height Then Lcd_set_pixel X , Y , Color
      X = Xcenter + Yoffset
      Y = Ycenter + Xoffset
      If X < Lcd_screen_width And Y < Lcd_screen_height Then Lcd_set_pixel X , Y , Color
      X = Xcenter - Yoffset
      If X < Lcd_screen_width And Y < Lcd_screen_height Then Lcd_set_pixel X , Y , Color
      Y = Ycenter - Xoffset
      If X < Lcd_screen_width And Y < Lcd_screen_height Then Lcd_set_pixel X , Y , Color
      X = Xcenter + Yoffset
      If X < Lcd_screen_width And Y < Lcd_screen_height Then Lcd_set_pixel X , Y , Color
   Wend
   Decr Lcd_screen_width
   Decr Lcd_screen_height
End Sub


'-------------------------------------------------------------------------------
' Fill Circle (slow)
'-------------------------------------------------------------------------------
Sub Lcd_fill_circle(byval X As Word , Byval Y As Word , Byval Radius As Word , Byval Color As Word)
   Local Count_y As Integer , Count_x As Integer
   Local Y1 As Integer , X1 As Integer , Xy_radius As Long
   Local Y11 As Long , X11 As Long , Xy As Long
   Local Intradius As Integer
   Local Xpos As Word , Ypos As Word

   Intradius = Radius
   Xy_radius = Intradius * Intradius
   Y1 = -intradius
   X1 = -intradius

   For Count_y = Y1 To Intradius
      Y11 = Count_y * Count_y
      For Count_x = X1 To Intradius
         X11 = Count_x * Count_x

         Xy = X
         Xy = Xy + Count_x
         Xpos = Xy

         Xy = Y
         Xy = Xy + Count_y
         Ypos = Xy

         Xy = X11 + Y11
         If Xy <= Xy_radius And 0 <= Xpos And Xpos < Lcd_screen_width And 0 <= Ypos And Ypos < Lcd_screen_height Then
            Lcd_set_pixel Xpos , Ypos , Color
         End If
      Next Count_x
   Next Count_y
End Sub


'-------------------------------------------------------------------------------
' Ellipse (Bresenham)
'-------------------------------------------------------------------------------
Sub Lcd_ellipse(byval X_center As Word , Byval Y_center As Word , Byval X_radius As Word , Byval Y_radius As Word , Byval Color As Word)
   Local A2 As Long , B2 As Long
   Local Fa2 As Long , Fb2 As Long
   Local X_ellipse As Long , Y_ellipse As Long , Sigma As Long
   Local X_calc As Long , Y_calc As Long

   A2 = X_radius * X_radius
   B2 = Y_radius * Y_radius
   Fa2 = 4 * A2
   Fb2 = 4 * B2

   X_ellipse = 2 * B2
   Sigma = 2 * Y_radius
   Sigma = 1 - Sigma
   Sigma = Sigma * A2
   Sigma = X_ellipse + Sigma
   X_ellipse = 0
   Y_ellipse = Y_radius
   Do
      X_calc = X_center + X_ellipse
      Y_calc = Y_center + Y_ellipse
      Lcd_set_pixel X_calc , Y_calc , Color
      X_calc = X_center - X_ellipse
      Lcd_set_pixel X_calc , Y_calc , Color
      Y_calc = Y_center - Y_ellipse
      Lcd_set_pixel X_calc , Y_calc , Color
      X_calc = X_center + X_ellipse
      Lcd_set_pixel X_calc , Y_calc , Color

      If Sigma >= 0 Then
         X_calc = 1 - Y_ellipse
         X_calc = X_calc * Fa2
         Sigma = Sigma + X_calc
         Y_ellipse = Y_ellipse - 1
      End If
      X_calc = 4 * X_ellipse
      X_calc = X_calc + 6
      X_calc = X_calc * B2
      Sigma = Sigma + X_calc

      X_calc = X_ellipse * B2
      Y_calc = Y_ellipse * A2

      If X_calc > Y_calc Then Exit Do
      X_ellipse = X_ellipse + 1
   Loop

   X_ellipse = 2 * A2
   Sigma = 2 * X_radius
   Sigma = 1 - Sigma
   Sigma = Sigma * B2
   Sigma = X_ellipse + Sigma
   X_ellipse = X_radius
   Y_ellipse = 0
   Do
      X_calc = X_center + X_ellipse
      Y_calc = Y_center + Y_ellipse
      Lcd_set_pixel X_calc , Y_calc , Color
      X_calc = X_center - X_ellipse
      Lcd_set_pixel X_calc , Y_calc , Color
      Y_calc = Y_center - Y_ellipse
      Lcd_set_pixel X_calc , Y_calc , Color
      X_calc = X_center + X_ellipse
      Lcd_set_pixel X_calc , Y_calc , Color

      If Sigma >= 0 Then
         X_calc = 1 - X_ellipse
         X_calc = X_calc * Fb2
         Sigma = Sigma + X_calc
         X_ellipse = X_ellipse - 1
      End If
      X_calc = 4 * Y_ellipse
      X_calc = X_calc + 6
      X_calc = X_calc * A2
      Sigma = Sigma + X_calc

      X_calc = X_ellipse * B2
      Y_calc = Y_ellipse * A2
      If Y_calc > X_calc Then Exit Do
      Y_ellipse = Y_ellipse + 1
   Loop
End Sub


'-------------------------------------------------------------------------------
' Rectangle
'-------------------------------------------------------------------------------
Sub Lcd_rect(byval X1 As Word , Byval Y1 As Word , Byval X2 As Word , Byval Y2 As Word , Byval Color As Word)
   Lcd_fill_rect X1 , Y1 , X2 , Y1 , Color
   Lcd_fill_rect X1 , Y1 , X1 , Y2 , Color
   Lcd_fill_rect X1 , Y2 , X2 , Y2 , Color
   Lcd_fill_rect X2 , Y1 , X2 , Y2 , Color
End Sub