$regfile = "m1284def.dat"
'$regfile = "m328def.dat"
$crystal = 16000000
$hwstack = 128
$swstack = 160
$framesize = 128

Config Submode = New

Const False = 0
Const True = 1

Const Lcd_enable_spi = True
$include "ILI9341.inc"

$include "ProportionalFont.inc"

'$include "Matrix.inc"

$include "3D-Engine.inc"

Const Touch_measure_resistance = 2
Const Touch_resistance_x = 221
Const Touch_resistance_y = 577
$include "4WireResistiveTouch.inc"

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

Dim Demo_mode As Byte
Dim Stopwatch As Dword

Dim Pixels As Word
Dim R As Byte , G As Byte , B As Byte
Dim X As Word , Y As Word , Color As Word
Dim X2 As Word , Y2 As Word , Radius As Word

Dim Rotation_x As Single , Rotation_y As Single , Rotation_z As Single
Dim Acceleration_x As Single , Acceleration_y As Single , Acceleration_z As Single
Dim Cube_r As Single , Cube_g As Single , Cube_b As Single
Dim Cube_dr As Single , Cube_dg As Single , Cube_db As Single
Dim Cube_change_color As Byte
Dim Tmpval As Single

Dim Statustext As String * 30 , Previous_statustext As String * 30 , Status_update As Bit
Dim Touchstatus As Byte , Touchfirst As Boolean

Dim Videoframe As Dword

Enable Interrupts

Restore Color8x8
Lcd_loadfont

Restore Timesnewroman_10pt
Lcd_tt_loadfont

Dreid_fov = 250
Dreid_viewer_distance = 7
Dreid_project_x = 120
Dreid_project_y = 160
Restore Dreid_demo_cube
Dreid_load_model

Lcd_init
'Touch_reset_calibration
Touch_init

Do
   If Demo_mode = 0 Then
      ' =========== Clear Screen ============
      For Pixels = 1 To 99
         Random_color
         Lcd_clear Color
      Next
      Waitms 3000
      Lcd_clear &H0000

      ' =========== Gradient Fill ============
      R = 0
      G = 0
      B = 0
      X2 = 0
      Color = 0
      Y = 0
      For Pixels = 0 To 512
         Radius = Color
         Random_color
         For G = 0 To 32
            X2 = Color16alpha(color , Radius , G)
            Lcd_fill_rect 0 , Y , Lcd_screen_width , Y , X2
            Y = Y + 1
            If Y = Lcd_height Then Y = 0
         Next
      Next
      Waitms 3000
      Lcd_clear &H0000

      ' =========== Pixels ============
      For Pixels = 1 To 65535
         Random_color
         X = Rnd(lcd_screen_width)
         Y = Rnd(lcd_screen_height)
         Lcd_set_pixel X , Y , Color
      Next
      Waitms 3000
      Lcd_clear &H0000

      ' =========== Lines ============
      For Pixels = 1 To 3000
         Random_color
         X = Rnd(lcd_screen_width)
         Y = Rnd(lcd_screen_height)
         X2 = Rnd(lcd_screen_width)
         Y2 = Rnd(lcd_screen_height)
         Lcd_line X , Y , X2 , Y2 , Color
      Next
      Waitms 3000
      Lcd_clear &H0000

      ' =========== Boxes ============
      For Pixels = 1 To 10000
         Random_color
         X = Rnd(lcd_screen_width)
         Y = Rnd(lcd_screen_height)
         X2 = Rnd(lcd_screen_width)
         Y2 = Rnd(lcd_screen_height)
         Lcd_rect X , Y , X2 , Y2 , Color
      Next
      Waitms 3000
      Lcd_clear &H0000

      ' =========== Circles ============
      For Pixels = 1 To 1000
         Random_color
         Y = Rnd(lcd_screen_width)
         Y2 = Rnd(lcd_screen_height)
         Radius = Rnd(200)
         Lcd_circle Y , Y2 , Radius , Color
      Next
      Waitms 3000
      Lcd_clear &H0000

      ' =========== Filled Circles ============
      For Pixels = 1 To 50
         Random_color
         Y = Rnd(lcd_screen_width)
         Y2 = Rnd(lcd_screen_height)
         Radius = Rnd(80)
         Lcd_fill_circle Y , Y2 , Radius , Color
      Next
      Waitms 3000
      Lcd_clear &H0000

      ' =========== Flash BIN Pics ============
      For Pixels = 1 To 50
         X = Rnd(80)
         Y = Rnd(200)
         Restore Coffee
         Lcd_pic_flash X , Y , 160 , 120
      Next
      Waitms 3000
      Lcd_clear &H0000

      ' =========== Flash BGF Pics ============
      For Pixels = 1 To 50
         X = Rnd(108)
         Y = Rnd(188)
         Restore Animals
         Lcd_pic_bgf X , Y
      Next
      Waitms 3000
      Lcd_clear &H0000

      ' =========== Fixed Font Text ============
      For Pixels = 1 To 1000
         Random_color
         X = Rnd(220)
         Y = Rnd(310)
         Lcd_text "Text" , X , Y , Color , Color_black
      Next
      Waitms 3000
      Lcd_clear &H0000

      ' =========== Proportional Font Text ============
      For Pixels = 1 To 1000
         Random_color
         X = Rnd(220)
         Y = Rnd(310)
         Lcd_tt_text "Text" , X , Y , Color
      Next
      Waitms 3000
      Lcd_clear &H0000

'      Videoframe = 0
'      Matrix_init &H14A4 , Color_aquamarine
'      Do
'         Matrix_update
'         Videoframe = Videoframe + 1
'         If Videoframe = 250 Then
'            Matrix_set_colors Color_navy , Color_royalblue4
'         Elseif Videoframe = 500 Then
'            Matrix_set_colors Color_indigo , Color_darkorchid1
'         Elseif Videoframe = 750 Then
'            Matrix_set_colors Color_maroon , Color_indianred
'         Elseif Videoframe = 1000 Then
'            Exit Do
'         End If
'      Loop
'      Restore Timesnewroman_10pt
'      Lcd_tt_loadfont
'      Waitms 3000
'      Lcd_clear &H0000

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
         Restore Hand
         Lcd_pic_flash X , Y , 80 , 155
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
      'Lcd_pic_ram 0 , 0 , Flash_colors_bin_width , Flash_colors_bin_height , Xramstart + Flash_colors_bin
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
      'Touch_reset_calibration
      'Touch_calibrate
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
      Else
         If Touchfirst = False Then
            Statustext = ""
            Status_update = True
         End If
         Touchfirst = True
      End If

      Videoframe = Videoframe + 1
      If 23 < Videoframe Then
         Tmpval = Stopwatch
         Tmpval = 10000 / Tmpval
         Tmpval = 24 * Tmpval
         Statustext = Str(tmpval)
         Statustext = "Render FPS: " + Statustext
         Status_update = True
         Stopwatch = 0
         Videoframe = 0
      End If

      If Touch_touched = False Then
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
      End If

      If Status_update = True Then
         Status_update = False
         If Previous_statustext <> "" Then Lcd_tt_text Previous_statustext , 30 , 265 , Color_black
         If Statustext <> "" Then Lcd_tt_text Statustext , 30 , 265 , Color_white
         Previous_statustext = Statustext
      End If
   End If
Loop
End

$include "data\color8x8.font"

$include "data\Dreid_DemoCube.inc"

$include "data\SegoeScript_31pt_b.inc"
$include "data\alarmclock_18_5pt_b.inc"
'$include "data\MatrixCodeNFI_12pt.inc"
$include "data\TimesNewRoman_10pt.inc"

$inc Coffee , Nosize , "data\Coffee.bin"
$inc Hand , Nosize , "data\Hand.bin"

Animals:
$bgf "data\Animals.bgc"