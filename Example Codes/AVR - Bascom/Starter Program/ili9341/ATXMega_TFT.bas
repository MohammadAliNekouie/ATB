$regfile = "xm128a1def.dat"
$crystal = 48000000
$hwstack = 256
$swstack = 256
$framesize = 256

Config Submode = New
Const False = 0
Const True = 1

' use the internal 2MHz clock with the PLL for the desired system clock
$include "XMegaPll.inc"

' status leds
Config Porte = Output
Porte = &HFF

' debug comm
Config Com1 = 115200 , Mode = Asynchroneous , Parity = None , Stopbits = 1 , Databits = 8
Open "COM1:" For Binary As #1

' button SW7
Config Xpin = Portr.1 , Outpull = Pullup , Sense = Falling
Portr_int1mask.1 = True
On Portr_int1 Switch_isr
Enable Portr_int1 , Lo

' 24 Fps @ 48 MHz
Config Tcd0 = Normal , Prescale = 1024
Tcd0_per = 1953
On Tcd0_ovf Tcd0_isr
Enable Tcd0_ovf , Lo

' 10 KHz
Config Tcd1 = Normal , Prescale = 8
Tcd1_per = 600
Stop Tcd1
On Tcd1_ovf Tcd1_isr
Enable Tcd1_ovf , Lo

' =========== XRAM ============
'the XPLAINED has a 64 MBit SDRAM which is 8 MByte, it is connected in 3 port, 4 bit databus mode
'in the PDF of the SDRAM you can see it is connected as 16 Meg x 4. Refreshcount is 4K and the row address is A0-A11, column addressing is A0-A9
$xramsize = 8388608                                         ' 8 MByte
Config Xram = 3port , Sdbus = 4 , Sdcol = 10 , Sdcas = 3 , Sdrow = 12 , Refresh = 500 , Initdelay = 3200 , Modedelay = 2 , Rowcycledelay = 7 , Rowprechargedelay = 7 , Wrdelay = 1 , Esrdelay = 7 , Rowcoldelay = 7 , Modesel3 = Sdram , Adrsize3 = 8m , Baseadr3 = &H0000
'the config above will set the port registers correct. it will also wait for Ebi_cs3_ctrlb.7
Const Xramstart = _hwstackstart + 1
Const Xramend = Xramstart + _xramsize

' =========== TFT Screen ============
'Const Lcd_enable_spi = True
'Const Lcd_use_soft_spi = True
Const Lcd_enable_16bit = True
Const Lcd_reset_port = Portr
$include "ILI9341.inc"
$include "ProportionalFont.inc"

' =========== SPI DataFlash ============
'Const Flash_use_soft_spi = True
Const Flash_port_ss = Portq
Const Flash_pin_ss = 2
Const Flash_enable_statusled = True
Const Flash_invert_statusled = True
$include "SPI-Flash.inc"
$include "DataFlashFiles.inc"
Const Xramdataend = Xramstart + Flash_totalsize

' =========== Eye candies ============
$include "Matrix.inc"
$include "3D-Engine.inc"

' =========== Touch Screen ============
Const Touch_updaterate = 200
Const Touch_ctrl_port = Varptr( "Porta")
Const Touch_measure_resistance = 2
Const Touch_resistance_x = 221
Const Touch_resistance_y = 577
$include "4WireResistiveTouch.inc"


Function Str_rounded(byval Value As Single , Byval Decimals As Byte) As String * 16
   Local Temp As Single
   Local Strvalue As String * 16
   Local Index As Byte , Ptr As Word
   Tmpval = Decimals
   Temp = 10.0
   Temp = Temp ^ Tmpval
   Value = Value * Temp
   Value = Round(value)
   Value = Value / Temp
   Strvalue = Str(value)

   Ptr = Varptr(strvalue)
   Index = Charpos(strvalue , "." , 0 , Speed)
   Ptr = Ptr + Index
   Ptr = Ptr + 2
   Out Ptr , 0

   Str_rounded = Strvalue
End Function

Sub Show_stats(byval Text As String * 50 , Byval Iterations As Word)
   Tmpval = Stopwatch
   Tmpval = Tmpval / Iterations
   Tmpval = 10000 / Tmpval
   Statustext = Str_rounded(tmpval , 2)
   Statustext = Text + "/sec: " + Statustext
   Show_text
End Sub

Sub Show_text()
   X = 0
   X2 = Lcd_screen_width
   Y = Lcd_screen_height - 20
   Y2 = Lcd_screen_height
   Lcd_fill_rect X , Y , X2 , Y2 , Color_black
   Y = Y + 4
   Lcd_tt_text Statustext , 10 , Y , Color_white
   Waitms 3000
   Lcd_clear &H0000
   Stopwatch = 0
End Sub

Sub Draw_homescreen()
   Lcd_clear &H0000
   Restore Segoescript_31pt_b
   Lcd_tt_loadfont
   X = Lcd_screen_width - 134
   X = X / 2
   Lcd_tt_text "DreiD" , X , 10 , Color_red

   Restore Alarmclock_18_5pt_b
   Lcd_tt_loadfont
   X = Lcd_screen_width - 180
   X = X / 2
   Y = Lcd_screen_height - 40
   Lcd_tt_text "ILI9341 + Touch" , X , Y , Color_blue

   Restore Timesnewroman_10pt
   Lcd_tt_loadfont
End Sub

Sub Random_color()
   R = Rnd(255)
   G = Rnd(255)
   B = Rnd(255)
   Color = Color24to16(r , G , B)
End Sub

Dim Rot As Byte , Demo_mode As Byte
Dim Switch1_pressed As Bit , Switch_pressed As Bit
Dim Stopwatch As Dword

Dim Pixels As Word
Dim R As Byte , G As Byte , B As Byte
Dim X As Word , Y As Word , Color As Word
Dim X2 As Word , Y2 As Word , Radius As Word

Dim Fps_update As Bit
Dim Rotation_x As Single , Rotation_y As Single , Rotation_z As Single
Dim Acceleration_x As Single , Acceleration_y As Single , Acceleration_z As Single
Dim Cube_r As Single , Cube_g As Single , Cube_b As Single
Dim Cube_dr As Single , Cube_dg As Single , Cube_db As Single
Dim Cube_change_color As Byte
Dim Tmpval As Single

Dim Statustext As String * 30 , Previous_statustext As String * 30 , Status_update As Bit
Dim Touchstatus As Byte , Touchfirst As Boolean

Dim Rampicture As Dword , Videoframe As Dword

Config Priority = Static , Vector = Application , Lo = Enabled , Med = Disabled , Hi = Disabled
Enable Interrupts

' =========== Load fonts ============
Restore Color8x8
Lcd_color_font = True
Lcd_loadfont

Restore Timesnewroman_10pt
Lcd_tt_loadfont

' =========== Load 3D model ============
Dreid_fov = 250
Dreid_viewer_distance = 7
Dreid_project_x = 120
Dreid_project_y = 160
Restore Dreid_demo_cube
Dreid_load_model

' =========== Load everything from the SPI DataFlash into the XRAM ============
Flash_init
Flash_readto 0 , Xramstart , Flash_totalsize
Flash_stop
Lcd_ram_pics_free_ptr = Xramdataend

' =========== Load image from program flash into the XRAM ============
Restore Coffee
Rampicture = Lcd_load_pic_flash2ram(160 , 120)


' =========== Start ============
Lcd_init
Touch_init

Do
   If Switch_pressed = True Then
      Demo_mode = 1 - Demo_mode
      Switch_pressed = False
      Waitms 1000
      If Demo_mode = 1 Then Draw_homescreen
   End If

   If Demo_mode = 0 Then
      ' =========== Clear Screen ============
      Start Tcd1
      For Pixels = 1 To 100
         Random_color
         Lcd_clear Color
      Next
      Lcd_clear &H0000
      Stop Tcd1
      Show_stats "Screen clears" , 100

      ' =========== Gradient Fill ============
      R = 0
      G = 0
      B = 0
      X2 = 0
      Color = 0
      Start Tcd1
      Y = 0
      For Pixels = 1 To 512
         Radius = Color
         Random_color
         For G = 0 To 32
            X2 = Color16alpha(color , Radius , G)
            Lcd_fill_rect 0 , Y , Lcd_screen_width , Y , X2
            Y = Y + 1
            If Y = Lcd_height Then Y = 0
         Next
      Next
      Stop Tcd1
      Show_stats "Gradient fills" , 512

      ' =========== Pixels ============
      Start Tcd1
      For Pixels = 1 To 65535
         Random_color
         X = Rnd(lcd_screen_width)
         Y = Rnd(lcd_screen_height)
         Lcd_set_pixel X , Y , Color
      Next
      Stop Tcd1
      Show_stats "Pixels" , 65535

      ' =========== Lines ============
      Start Tcd1
      For Pixels = 1 To 3000
         Random_color
         X = Rnd(lcd_screen_width)
         Y = Rnd(lcd_screen_height)
         X2 = Rnd(lcd_screen_width)
         Y2 = Rnd(lcd_screen_height)
         Lcd_line X , Y , X2 , Y2 , Color
      Next
      Stop Tcd1
      Show_stats "Lines" , 3000

      ' =========== Boxes ============
      Start Tcd1
      For Pixels = 1 To 10000
         Random_color
         X = Rnd(lcd_screen_width)
         Y = Rnd(lcd_screen_height)
         X2 = Rnd(lcd_screen_width)
         Y2 = Rnd(lcd_screen_height)
         Lcd_rect X , Y , X2 , Y2 , Color
      Next
      Stop Tcd1
      Show_stats "Boxes" , 10000

      ' =========== Circles ============
      Start Tcd1
      For Pixels = 1 To 1000
         Random_color
         X = Rnd(lcd_screen_width)
         Y = Rnd(lcd_screen_height)
         Radius = Rnd(200)
         Lcd_circle X , Y , Radius , Color
      Next
      Stop Tcd1
      Show_stats "Circles" , 1000

      ' =========== Filled Circles ============
      Start Tcd1
      For Pixels = 1 To 50
         Random_color
         X = Rnd(lcd_screen_width)
         Y = Rnd(lcd_screen_height)
         Radius = Rnd(80)
         Lcd_fill_circle X , Y , Radius , Color
      Next
      Stop Tcd1
      Show_stats "Filled circles" , 50

      ' =========== Ellipses ============
      Start Tcd1
      For Pixels = 1 To 1000
         Random_color
         X = Rnd(lcd_screen_width)
         Y = Rnd(lcd_screen_height)
         Radius = Rnd(200)
         Y2 = Rnd(200)
         Lcd_ellipse X , Y , Radius , Y2 , Color
      Next
      Stop Tcd1
      Show_stats "Ellipses" , 1000

      ' =========== SRAM BIN Pics ============
      Start Tcd1
      Lcd_pic_ram 0 , 0 , Flash_boats_bin_width , Flash_boats_bin_height , Xramstart + Flash_boats_bin
      Stop Tcd1
      Waitms 3000
      Start Tcd1
      Lcd_pic_ram 0 , 0 , Flash_coast_bin_width , Flash_coast_bin_height , Xramstart + Flash_coast_bin
      Stop Tcd1
      Waitms 3000
      Start Tcd1
      Lcd_pic_ram 0 , 0 , Flash_colors_bin_width , Flash_colors_bin_height , Xramstart + Flash_colors_bin
      Stop Tcd1
      Waitms 3000
      Start Tcd1
      Lcd_pic_ram 0 , 0 , Flash_desert_bin_width , Flash_desert_bin_height , Xramstart + Flash_desert_bin
      Stop Tcd1
      Waitms 3000
      Start Tcd1
      Lcd_pic_ram 0 , 0 , Flash_eiffeltower_bin_width , Flash_eiffeltower_bin_height , Xramstart + Flash_eiffeltower_bin
      Stop Tcd1
      Waitms 3000
      Start Tcd1
      Lcd_pic_ram 0 , 0 , Flash_orange_bin_width , Flash_orange_bin_height , Xramstart + Flash_orange_bin
      Stop Tcd1
      Waitms 3000
      Start Tcd1
      Lcd_pic_ram 0 , 0 , Flash_pcb_bin_width , Flash_pcb_bin_height , Xramstart + Flash_pcb_bin
      Stop Tcd1
      Waitms 3000
      Start Tcd1
      Lcd_pic_ram 0 , 0 , Flash_spices_bin_width , Flash_spices_bin_height , Xramstart + Flash_spices_bin
      Stop Tcd1
      Waitms 3000
      Show_stats "240x320 BIN pics (SRAM)" , 8

      ' =========== Flash BIN Pics ============
      Start Tcd1
      For Pixels = 1 To 50
         X = Rnd(80)
         Y = Rnd(200)
         Restore Coffee
         Lcd_pic_flash X , Y , 160 , 120
      Next
      Stop Tcd1
      Show_stats "160x120 BIN pics (Flash)" , 50

      ' =========== RAM BIN Pics ============
      Start Tcd1
      For Pixels = 1 To 50
         X = Rnd(80)
         Y = Rnd(200)
         Lcd_pic_ram X , Y , 160 , 120 , Rampicture
      Next
      Stop Tcd1
      Show_stats "160x120 BIN pics (SRAM)" , 50

      ' =========== Flash BGF Pics ============
      Start Tcd1
      For Pixels = 1 To 50
         X = Rnd(108)
         Y = Rnd(188)
         Restore Animals
         Lcd_pic_bgf X , Y
      Next
      Stop Tcd1
      Show_stats "132x132 BGF pics (Flash)" , 50

      ' =========== Video from RAM BIN pics ============
      Videoframe = Xramstart + Flash_frame_000_bin
      X = 320
      Do
         If Fps_update = True Then
            Fps_update = False
            Lcd_pic_ram 66 , 80 , Flash_frame_000_bin_width , Flash_frame_000_bin_height , Videoframe
            Videoframe = Videoframe + Flash_frame_000_bin_size
            If Videoframe = Xramdataend Then
               Videoframe = Xramstart + Flash_frame_000_bin
            End If
            X = X - 1
         End If
      Loop Until X = 0
      Statustext = "107x160, 24 fps animation"
      Show_text

      ' =========== Fixed Font Text ============
      Start Tcd1
      For Pixels = 1 To 1000
         Random_color
         X = Rnd(220)
         Y = Rnd(310)
         Lcd_text "Text" , X , Y , Color , Color_black
      Next
      Stop Tcd1
      Show_stats "Fixed font texts" , 1000

      ' =========== Proportional Font Text ============
      Start Tcd1
      For Pixels = 1 To 1000
         Random_color
         X = Rnd(220)
         Y = Rnd(310)
         Lcd_tt_text "Text" , X , Y , Color
      Next
      Stop Tcd1
      Show_stats "Proportional font texts" , 1000


      ' =========== Matrix code rain ============
      Tcd0_per = 1200
      Videoframe = 0
      Matrix_init &H14A4 , Color_aquamarine
      Do
         If Fps_update = True Then
            Fps_update = False
            Start Tcd1
            Matrix_update
            Stop Tcd1
            Videoframe = Videoframe + 1
            If Videoframe = 400 Then
               Matrix_set_colors Color_navy , Color_royalblue4
            Elseif Videoframe = 800 Then
               Matrix_set_colors Color_indigo , Color_darkorchid1
            Elseif Videoframe = 1200 Then
               Matrix_set_colors Color_maroon , Color_indianred
            Elseif Videoframe = 1600 Then
               Exit Do
            End If
         End If
      Loop
      Tcd0_per = 1953
      Tmpval = Stopwatch
      Tmpval = 9999360 / Tmpval
      'Tmpval = 25.6 * Tmpval
      Statustext = Str_rounded(tmpval , 2)
      Statustext = "Matrix code rain, render FPS: " + Statustext
      Restore Timesnewroman_10pt
      Lcd_tt_loadfont
      Show_text

      ' =========== Rotation ============
      For Pixels = 0 To 4
         Lcd_set_rotation Pixels
         Lcd_clear Color_white
         Y = Lcd_screen_height - 20
         Lcd_fill_rect 0 , Y , Lcd_screen_width , Lcd_screen_height , Color_black
         Y = Y + 4
         Statustext = "Rotation " + Str(lcd_rotation)
         Lcd_tt_text Statustext , 10 , Y , Color_white
         X = Lcd_screen_width - 80
         X = X / 2
         Y = Lcd_screen_height - 155
         Y = Y / 2
         'Restore Hand
         'Lcd_pic_flash X , Y , 80 , 155
         'Lcd_pic_ram X , Y , 80 , 155 , Xramstart + Flash_hand_bin
         Waitms 1500
      Next

      ' =========== Backlight Dimming ============
      X = 0
      Lcd_clear Color_white
      Y = Lcd_screen_height - 20
      Lcd_fill_rect X , Y , X2 , Y2 , Color_black
      Y = Y + 4
      Lcd_tt_text "Backlight dimming" , 10 , Y , Color_white
      Lcd_backlight_dim 255 , 1 , 1500                      ' dim to 0 would enable the screen's standby mode
      Waitms 200
      Lcd_backlight_dim 1 , 255 , 1500

      ' =========== Screen Standby ============
      Lcd_clear Color_black
      Y = Lcd_screen_height - 16
      Lcd_tt_text "Standby" , 10 , Y , Color_white
      Waitms 2000
      Lcd_backlight 0
      Waitms 1000
      Lcd_backlight 255
      Waitms 1000

      ' =========== Invert ============
      Lcd_pic_ram 0 , 0 , Flash_colors_bin_width , Flash_colors_bin_height , Xramstart + Flash_colors_bin
      Y = Lcd_screen_height - 20
      Lcd_fill_rect X , Y , X2 , Y2 , Color_black
      Y = Y + 4
      Lcd_tt_text "Invert" , 10 , Y , Color_white
      Waitms 1000
      Lcd_invert True
      Waitms 3000
      Lcd_invert False
      Waitms 1000

      ' =========== Switch Demo Mode ============
      Lcd_clear Color_black
      Y = Lcd_screen_height - 20
      Lcd_fill_rect X , Y , X2 , Y2 , Color_black
      Y = Y + 4
      Lcd_tt_text "Touch calibration" , 10 , Y , Color_white
      Waitms 3000
      Demo_mode = 1
      Touch_reset_calibration
      Touch_calibrate
      Draw_homescreen
   Else
      If Touch_touched = True Then
         Touchstatus = Touch_get_sample()
         If Touchstatus = True Then
            X = Touch_x
            If Touchfirst = True Then
               X2 = X
               Y2 = Touch_y
               Touchfirst = False
            End If
            Lcd_line X , Touch_y , X2 , Y2 , Color_yellow
            Lcd_set_pixel X2 , Y2 , Color_violet
            X2 = X
            Y2 = Touch_y
            Statustext = "X: " + Str(x)
            Statustext = Statustext + ", Y: "
            Statustext = Statustext + Str(touch_y)
            Status_update = True
         End If
         If Touch_dragging = True Then Porte.2 = False
      Else
         Porte.2 = True
         If Touchfirst = False Then
            Statustext = ""
            Status_update = True
         End If
         Touchfirst = True
      End If

      If Fps_update = True Then
         Fps_update = False

         Videoframe = Videoframe + 1
         If 23 < Videoframe Then
            Tmpval = Stopwatch
            Tmpval = 10000 / Tmpval
            Tmpval = 24 * Tmpval
            Statustext = Str_rounded(tmpval , 2)
            Statustext = "Render FPS: " + Statustext
            Status_update = True
            Stopwatch = 0
            Videoframe = 0
         End If

         If Touch_touched = False Then
            Start Tcd1
            Tmpval = Deg2rad(acceleration_x)
            Tmpval = Sin(tmpval)
            Tmpval = Tmpval * 2.5
            Acceleration_x = Acceleration_x + 0.17
            If Acceleration_x >= 360 Then Acceleration_x = 0
            Rotation_x = Rotation_x + Tmpval

            Tmpval = Deg2rad(acceleration_y)
            Tmpval = Sin(tmpval)
            Tmpval = Tmpval * 2.5
            Acceleration_y = Acceleration_y + 0.29
            If Acceleration_y >= 360 Then Acceleration_y = 0
            Rotation_y = Rotation_y + Tmpval

            Tmpval = Deg2rad(acceleration_z)
            Tmpval = Sin(tmpval)
            Tmpval = Tmpval * 2.5
            Acceleration_z = Acceleration_z + 0.37
            If Acceleration_z >= 360 Then Acceleration_z = 0
            Rotation_z = Rotation_z + Tmpval

            If Cube_change_color = 0 Then
               Cube_change_color = 100

               Tmpval = Cube_change_color
               R = Rnd(256)
               G = Rnd(256)
               B = Rnd(256)
               Cube_dr = R
               Cube_dr = Cube_dr - Cube_r
               Cube_dr = Cube_dr / Tmpval
               Cube_dg = G
               Cube_dg = Cube_dg - Cube_g
               Cube_dg = Cube_dg / Tmpval
               Cube_db = B
               Cube_db = Cube_db - Cube_b
               Cube_db = Cube_db / Tmpval
            Else
               Cube_change_color = Cube_change_color - 1
               Cube_r = Cube_r + Cube_dr
               Cube_g = Cube_g + Cube_dg
               Cube_b = Cube_b + Cube_db
               R = Cube_r
               G = Cube_g
               B = Cube_b
               Dreid_forecolor = Color24to16(r , G , B)
            End If

            Dreid_rotate Rotation_x , Rotation_y , Rotation_z
            Dreid_update
            Stop Tcd1
         End If

         If Status_update = True Then
            Status_update = False
            If Previous_statustext <> "" Then Lcd_tt_text Previous_statustext , 30 , 265 , Color_black
            If Statustext <> "" Then Lcd_tt_text Statustext , 30 , 265 , Color_white
            Previous_statustext = Statustext
         End If

         If Porte.1 = False Then
            Porte.1 = True
            Touch_clicked = False
         End If
         If Touch_clicked = True Then
            Porte.1 = False
         End If
      End If
   End If
Loop
End

Switch_isr:
   Switch_pressed = True
Return

Tcd0_isr:
   Fps_update = True
Return

Tcd1_isr:
   Incr Stopwatch
Return

$include "data\color8x8.font"
'$include "data\font8x8.font"

$include "data\Dreid_DemoCube.inc"

$include "data\SegoeScript_31pt_b.inc"
$include "data\alarmclock_18_5pt_b.inc"
$include "data\MatrixCodeNFI_12pt.inc"
$include "data\TimesNewRoman_10pt.inc"

$inc Coffee , Nosize , "data\Coffee.bin"
'$inc Hand , Nosize , "data\Hand.bin"

Animals:
$bgf "data\Animals.bgc"