$regfile = "m128def.dat"
'$regfile = "xm128a1def.dat"
$hwstack = 64
$swstack = 64
$framesize = 64

Config Submode = New
Const False = 0
Const True = 1

#if _xmega = True
   $crystal = 32000000
   $include "XMegaPLL.inc"
#else
   $crystal = 16000000
#endif

$include "ILI9341.inc"

$include "ProportionalFont.inc"

Sub Random_color()
   R = Rnd(255)
   G = Rnd(255)
   B = Rnd(255)
   Color = Color24to16(r , G , B)
End Sub

Dim Pixels As Word
Dim R As Byte , G As Byte , B As Byte
Dim X As Word , Y As Word , Color As Word
Dim X2 As Word , Y2 As Word , Radius As Word
Dim Statustext As String * 30

Restore Color8x8
Lcd_loadfont

Restore Timesnewroman_10pt
Lcd_tt_loadfont

Enable Interrupts

Lcd_init

Do
   ' =========== Clear Screen ============
   For Pixels = 1 To 99
      Random_color
      Lcd_clear Color
   Next
   Waitms 3000
   Lcd_clear &H0000

   ' =========== Gradient Fill (Color16 functions) ============
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
   Lcd_backlight_dim 255 , 1 , 1500                                          ' dim to 0 would enable the screen's standby mode
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
   Y = Lcd_screen_height - 20
   Lcd_fill_rect X , Y , X2 , Y2 , Color_black
   Y = Y + 4
   Lcd_tt_text "Invert" , 10 , Y , Color_white
   Waitms 1000
   Lcd_invert True
   Waitms 3000
   Lcd_invert False
   Waitms 1000
Loop
End

$include "data\color8x8.font"

$include "data\TimesNewRoman_10pt.inc"

$inc Coffee , Nosize , "data\Coffee.bin"
$inc Hand , Nosize , "data\Hand.bin"

Animals:
$bgf "data\Animals.bgc"