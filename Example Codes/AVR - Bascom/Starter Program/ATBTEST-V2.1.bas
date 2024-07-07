$regfile = "m2560def.dat"
$hwstack = 128
$swstack = 128
$framesize = 32
$crystal = 1000000

Config Submode = New
Const False = 0
Const True = 1
Const Lcd_enable_16bit = True
Const Lcd_ctrl_port = Portk
Const Lcd_data_port_1 = Porta
Const Lcd_data_port_2 = Portl
Const Lcd_reset_pin = 1
Const Lcd_pin_wr = 3
Const Lcd_pin_rd = 2
Const Lcd_pin_dc = 4
Const Lcd_pin_cs = 0




$include "ILI9341.inc"
$include "ProportionalFont.inc"


Config Timer1 = Timer , Prescale = 1
Config Timer4 = Pwm , Pwm = 8 , Compare_a_pwm = Clear_up , Compare_b_pwm = Clear_up , Prescale = 8
Config Timer0 = Pwm , Pwm = On , Compare_a_pwm = Clear_up , Compare_b_pwm = Clear_up , Prescale = 1

Config Lcd = 16x2
Config Lcdpin = Pin , Db4 = Porta.4 , Db5 = Porta.5 , Db6 = Porta.6 , Db7 = Porta.7 , E = Porte.1 , Rs = Porte.5
'Config Graphlcd = 128 * 64sed , Dataport = Porta , Controlport = Porte , Ce = 7 , Ce2 = 6 , Cd = 5 , Rd = 6 , Reset = 2 , Enable = 3
Config Portb = Output
Config Porth = Output
Config Portg = Output
Config Portd = Output
Config Porte = Output
Config Portf = Output
Config Portc = Output
Config Porta = Output
Config Portl = Output
Config Portk = Output
Config Portj = Output

Const Turn_stop = 0
Const Turn_left = 1
Const Turn_right = 2

Declare Sub Turn()
Declare Sub Motor(byval Direction As Byte)
Declare Sub Stepper(byval Direction As Byte , Byval Steps As Byte)
Declare Sub Segment(byval Value As Byte , Byval Seg_number As Byte)

Declare Sub Glcd_init()
Declare Sub Glcd_disp_on()
Declare Sub Glcd_y_address(byval Cmd As Byte)
Declare Sub Glcd_x_address(byval Cmd As Byte)
Declare Sub Glcd_z_address(byval Cmd As Byte)
Declare Sub Glcd_data(byval Datas As Byte)
Declare Sub Glcd_clear()
Declare Sub Glcd_draw_circle(byval Xc As Byte , Byval Yc As Byte , Byval R As Byte)
Declare Sub Glcd_set_pixel(x As Byte , Y As Byte)


Dim 7seg_value As Byte , Location As Byte
Dim Step_value As Byte
Dim Xn As Integer , Nn As Integer
Dim Dot_matrix As Byte

Dim Pixels As Word
Dim R As Byte , G As Byte , B As Byte
Dim X As Word , Y As Word , Color As Word
Dim X2 As Word , Y2 As Word , Radius As Word
Dim Statustext As String * 30
Dim Pwm_value As Byte
Dim Pwm_counter As Byte
Restore Color8x8
Lcd_color_font = True
Lcd_loadfont
Restore Timesnewroman_10pt
Lcd_tt_loadfont
Lcd_init



Clock_out Alias Portj.3                                     ' SRCLK - 74HC595
Data_out Alias Portj.4                                      ' SER - of the first 74HC595
Latch_out Alias Porth.2                                     ' RCLK

' Control pins
Glcd_rw Alias Porte.3
Glcd_rs Alias Portb.2
Glcd_cs Alias Portj.4
Glcd_csa Alias Portj.7
Glcd_csb Alias Porth.6
Glcd_rst Alias Portj.3

Led_white Alias Portb.4
Led_green Alias Portb.5
Led_red Alias Portb.6
Led_pink Alias Porth.7

Dcm_en_pwm Alias Portb.7
Dcm_in_1 Alias Portg.3
Dcm_in_2 Alias Portg.4

Sem_con Alias Portj.7

Stm_in_1 Alias Portg.1
Stm_in_2 Alias Portg.0
Stm_in_3 Alias Portd.6
Stm_in_4 Alias Portd.7

Dm_rst Alias Portd.5
Dm_clk Alias Portd.4
Row Alias Portc


Sub Random_color()
   R = Rnd(255)
   G = Rnd(255)
   B = Rnd(255)
   Color = Color24to16(r , G , B)
End Sub

Reset Porte.4

Enable Interrupts
Enable Timer1
On Timer1 7seg
Timer1 = 155
Start Timer1
Do

7seg_value = 0
'TFT test
' =========== Clear Screen ============
For Pixels = 1 To 5
   Random_color
   Lcd_clear Color
Next
Waitms 300
Lcd_clear &H0000

Incr 7seg_value
' =========== Boxes ============
For Pixels = 1 To 100
   Random_color
   X = Rnd(lcd_screen_width)
   Y = Rnd(lcd_screen_height)
   X2 = Rnd(lcd_screen_width)
   Y2 = Rnd(lcd_screen_height)
   Lcd_rect X , Y , X2 , Y2 , Color
Next
Waitms 300
Lcd_clear &H0000

Incr 7seg_value
' =========== Filled Circles ============
For Pixels = 1 To 10
   Random_color
   Y = Rnd(lcd_screen_width)
   Y2 = Rnd(lcd_screen_height)
   Radius = 10
   Lcd_fill_circle Y , Y2 , Radius , Color
Next
Waitms 300



Incr 7seg_value

Initlcd
Cls
Lcd "TESLA"
Locate 2 , 3
Lcd "ELECTRONIC"

'start pwm generation for Servo motor
Start Timer4
Pwm4b = 10


Incr 7seg_value

Toggle Led_white
Toggle Led_green
Toggle Led_red
Toggle Led_pink


'DC motor test
Motor Turn_left
Wait 1
Motor Turn_right
Wait 1
Motor Turn_stop


Incr 7seg_value

'END pwm generation for Servo motor
Pwm_value = 40
Wait 1
Pwm_value = 80

Toggle Led_white
Toggle Led_green
Toggle Led_red
Toggle Led_pink


Incr 7seg_value

'run stepper motor
Stepper Turn_left , 10
Waitms 300
Stepper Turn_right , 10





Incr 7seg_value
'GLCD test
Glcd_init
Glcd_clear
Glcd_draw_circle 20 , 10 , 10
'check push buttons

'clear TFT
Lcd_clear &H0000

'audio generator via PWM
Start Timer0
For Nn = 10 To 200 Step 50
   Pwm0b = Nn
   Waitms 100
Next
Pwm0b = 0

Loop

Sub Turn()
   Stm_in_1 = Step_value.0
   Stm_in_2 = Step_value.2
   Stm_in_3 = Step_value.1
   Stm_in_4 = Step_value.3
End Sub

'  $include "font8x8.font"

7seg:
   Stop Timer1
   Timer1 = 63055
   Start Timer1

   Incr Pwm_counter
   If Pwm_counter > 99 Then Pwm_counter = 0
   If Pwm_counter > Pwm_value Then
      Set Sem_con
   Else
      Reset Sem_con
   End If
   '7segment display test
   Incr Location
   If Location > 6 Then Location = 1
   Segment 7seg_value , Location

   Incr Dot_matrix
   If Dot_matrix > 8 Then
      Dot_matrix = 0
      Set Dm_rst
      Waitus 100
      Reset Dm_rst
   End If
   Row = Lookup(dot_matrix , A)
   Toggle Dm_clk
   Waitus 100
   Toggle Dm_clk
Return


Sub Motor(byval Direction As Byte)
   If Direction = Turn_left Then
      Reset Dcm_in_1
      Set Dcm_in_2
      Set Dcm_en_pwm
   Elseif Direction = Turn_right Then
      Set Dcm_in_1
      Reset Dcm_in_2
      Set Dcm_en_pwm
   Else
      Reset Dcm_in_1
      Reset Dcm_in_2
      Reset Dcm_en_pwm
   End If
End Sub

Sub Stepper(byval Direction As Byte , Byval Steps As Byte)
   For Nn = 1 To Steps
      If Direction = Turn_left Then Step_value = &H01
      If Direction = Turn_right Then Step_value = &H08
      For Xn = 1 To 2
         Turn
         If Direction = Turn_left Then Rotate Step_value , Left , 2
         If Direction = Turn_right Then Rotate Step_value , Right , 2

         Waitms 100
      Next
   Next
End Sub


Sub Segment(byval Value As Byte , Byval Seg_number As Byte)
   Dim Seg_pos As Word
   Dim Buffer As Word
   If Value > 9 Then Value = 9
   Reset Latch_out
   Seg_pos = &H01
   Shift Seg_pos , Left , Seg_number
   Seg_pos = Not Seg_pos
   Shift Seg_pos , Left , 8
   Buffer = Lookup(value , Segments)
   Buffer = Not Buffer
   Buffer = Buffer Or Seg_pos
   Shiftout Data_out , Clock_out , Buffer , 0 , 16 , 10
   Set Latch_out
   Waitus 10
   Reset Latch_out
End Sub

'''''''''''''''''''''GLCD LIB KS0108'''''''''''''''''''
' Initialize GLCD
Sub Glcd_init()
  ' Reset GLCD
  Reset Glcd_rst
  Waitms 10
  Set Glcd_rst
  Waitms 10

  Glcd_disp_on
End Sub

' Turn on GLCD
Sub Glcd_disp_on()
  Reset Glcd_rs                                             ' Set RS to 0 (command mode)
  Reset Glcd_rw
  Porta = 63
  Reset Glcd_cs                                             ' Select GLCD
  Waitus 10
  Set Glcd_cs                                               ' Deselect GLCD
End Sub

' Send Y to GLCD
Sub Glcd_y_address(cmd As Byte)
   Local Tempbit As Byte
   Tempbit = Cmd And 63
  Reset Glcd_rs                                             ' Set RS to 0 (command mode)
  Reset Glcd_rw
  Porta = Tempbit + 64                                      ' Send command
  Reset Glcd_cs                                             ' Select GLCD
  Waitus 10
  Set Glcd_cs                                               ' Deselect GLCD
End Sub

' Send X to GLCD
Sub Glcd_x_address(cmd As Byte)
   Local Tempbit As Byte
   Tempbit = Cmd And 7
  Reset Glcd_rs                                             ' Set RS to 0 (command mode)
  Reset Glcd_rw
  Porta = Tempbit + 184                                     ' Send command
  Reset Glcd_cs                                             ' Select GLCD
  Waitus 10
  Set Glcd_cs                                               ' Deselect GLCD
End Sub

' Send Z to GLCD
Sub Glcd_z_address(cmd As Byte)
   Local Tempbit As Byte
   Tempbit = Cmd And 63
  Reset Glcd_rs                                             ' Set RS to 0 (command mode)
  Reset Glcd_rw
  Porta = Tempbit + 192                                     ' Send command
  Reset Glcd_cs                                             ' Select GLCD
  Waitus 10
  Set Glcd_cs                                               ' Deselect GLCD
End Sub

' Send data to GLCD
Sub Glcd_data(datas As Byte)
  Set Glcd_rs                                               ' Set RS to 1 (data mode)
  Reset Glcd_rw
  Porta = Datas                                             ' Send data
  Reset Glcd_cs                                             ' Select GLCD
  Waitus 10
  Set Glcd_cs                                               ' Deselect GLCD
End Sub

' Clear screen
Sub Glcd_clear()
  Local Gtemp As Byte , Page As Byte , Col As Byte
  For Page = 0 To 7                                         ' 8 pages
    Glcd_x_address Page
    For Col = 0 To 127                                      ' 128 columns
        If Col > 63 Then
            Set Glcd_csa
            Reset Glcd_csb
            Gtemp = Col - 64
        Else
            Reset Glcd_csa
            Set Glcd_csb
            Gtemp = Col
        End If
        Glcd_y_address Gtemp
        Glcd_data &H00                                      ' Clear pixel
    Next
  Next
End Sub

' Draw circle
Sub Glcd_draw_circle(xc As Byte , Yc As Byte , R As Byte)
  Local X As Byte , Y As Byte , D As Byte , Gtempx As Byte , Gtempy As Byte
  X = 0
  Y = R
  D = 2 * R
  D = 3 - D

  While X <= Y
    Gtempx = Xc + X
    Gtempy = Yc + Y
    Glcd_set_pixel Gtempx , Gtempy                          ' Octant 1
    Gtempx = Xc + Y
    Gtempy = Yc + X
    Glcd_set_pixel Gtempx , Gtempy                          ' Octant 2
    Gtempx = Xc - Y
    Gtempy = Yc + X
    Glcd_set_pixel Gtempx , Gtempy                          ' Octant 3
    Gtempx = Xc - X
    Gtempy = Yc + Y
    Glcd_set_pixel Gtempx , Gtempy                          ' Octant 4
    Gtempx = Xc - X
    Gtempy = Yc - Y
    Glcd_set_pixel Gtempx , Gtempy                          ' Octant 5
    Gtempx = Xc - Y
    Gtempy = Yc - X
    Glcd_set_pixel Gtempx , Gtempy                          ' Octant 6
    Gtempx = Xc + Y
    Gtempy = Yc - X
    Glcd_set_pixel Gtempx , Gtempy                          ' Octant 7
    Gtempx = Xc + X
    Gtempy = Yc - Y
    Glcd_set_pixel Gtempx , Gtempy                          ' Octant 8

    If D < 0 Then                                           ' Select E
      Gtempx = 4 * X
      Gtempx = Gtempx + 10
      D = D + Gtempx
    Else                                                    ' Select SE
      Gtempx = X - Y
      Gtempx = 4 * Gtempx
      Gtempx = Gtempx + 10
      D = D + Gtempx
      Y = Y - 1
    End If
    X = X + 1
  Wend
End Sub

' Set pixel on screen
Sub Glcd_set_pixel(x As Byte , Y As Byte)
  Local Page As Byte , Col As Byte , Bitpix As Byte , Gtemp3 As Byte
  If X > 63 Then
      Set Glcd_csa
      Reset Glcd_csb
  Else
      Reset Glcd_csa
      Set Glcd_csb
  End If

  Page = Y \ 8                                              ' Calculate page number (0-7)
  Col = X                                                   ' Calculate column number (0-127)
  Bitpix = Y Mod 8                                          ' Calculate bit number (0-7)
  Gtemp3 = &HB0 Or Page
  Glcd_x_address Gtemp3                                     ' Set page address
  Gtemp3 = Col
  Shift Gtemp3 , Right , 4
  Gtemp3 = &H10 Or Gtemp3
  Glcd_y_address Gtemp3                                     ' Set column address MSB
  Gtemp3 = Col And &H0F
  Glcd_z_address Gtemp3                                     ' Set column address LSB
  Gtemp3 = &H01
  Shift Gtemp3 , Left , Bitpix
  Glcd_data Gtemp3                                          ' Set pixel
End Sub

A:
Data 198 , 198 , 198 , 254 , 254 , 108 , 56 , 16 , 0

Segments:
Data 1 , 48 , 18 , 6 , 76 , 36 , 32 , 15 , 0 , 4


$include "data\color8x8.font"

$include "data\TimesNewRoman_10pt.inc"