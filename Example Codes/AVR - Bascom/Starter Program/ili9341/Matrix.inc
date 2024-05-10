$nocompile

Const Matrix_font_width = 8
Const Matrix_font_height = 14
Const Matrix_letters_x = Int((lcd_width) / Matrix_font_width)
Const Matrix_letters_y = Int((lcd_height ) / Matrix_font_height) + 1
Const Matrix_letters_count = Matrix_letters_x * Matrix_letters_y
Const Matrix_2nd_highlight = Int((matrix_letters_y / 3) * 2)
Const Matrix_trail_stop = Matrix_letters_y + 5

Dim Matrix_color_highlight As Word
Dim Matrix_color_base_r As Byte , Matrix_color_base_g As Byte , Matrix_color_base_b As Byte

Dim Matrix_start(matrix_letters_x) As Integer
Dim Matrix_start_char(matrix_letters_x) As Byte
Dim Matrix_highlight_char(matrix_letters_x) As Byte
Dim Matrix_speed(matrix_letters_x) As Byte
Dim Matrix_updatecounter(matrix_letters_x) As Byte

Dim Matrix_letters(matrix_letters_count) As Byte
Dim Matrix_letters_color(matrix_letters_count) As Word
Dim Matrix_letters_visible(matrix_letters_count) As Byte

Sub Matrix_init(byval Basecolor As Word , Byval Highlightcolor As Word)
   Local Index As Word , Trail As Byte
   Matrix_color_highlight = Highlightcolor
   Color16to24 Basecolor , Matrix_color_base_r , Matrix_color_base_g , Matrix_color_base_b
   For Index = 1 To Matrix_letters_count
      Matrix_randomize_letter Index
      Matrix_letters_visible(index) = 0
   Next
   For Trail = 1 To Matrix_letters_x
      Matrix_randomize_trail Trail
   Next
   Restore Matrixcodenfi_12pt
   Lcd_tt_loadfont
End Sub

Sub Matrix_set_colors(byval Basecolor As Word , Byval Highlightcolor As Word)
   Local Index As Word
   Matrix_color_highlight = Highlightcolor
   Color16to24 Basecolor , Matrix_color_base_r , Matrix_color_base_g , Matrix_color_base_b
   For Index = 1 To Matrix_letters_count
      Matrix_randomize_letter_color Index
   Next
End Sub

Function Matrix_random_char() As Byte
   Local Randomchar As Byte
   Do
      Randomchar = Rnd(94)
      Randomchar = Randomchar + 33
   Loop Until Randomchar <> 64
   Matrix_random_char = Randomchar
End Function

Sub Matrix_randomize_letter(byref Index As Word)
   Matrix_letters(index) = Matrix_random_char()
   Matrix_randomize_letter_color Index
End Sub

Sub Matrix_randomize_letter_color(byref Index As Word)
   Local R As Byte , G As Byte , B As Byte
   R = Matrix_color_base_r + Rnd(32)
   G = Matrix_color_base_g + Rnd(32)
   B = Matrix_color_base_b + Rnd(32)
   Matrix_letters_color(index) = Color24to16(r , G , B)
End Sub

Sub Matrix_randomize_trail(trail As Byte)
   Matrix_start_char(trail) = Matrix_random_char()
   Matrix_highlight_char(trail) = Matrix_random_char()
   Matrix_speed(trail) = Rnd(3) + 1
   Matrix_start(trail) = 0
End Sub

Sub Matrix_update()
   Local Trail As Byte
   Local Trailend As Integer
   Local Index As Word
   Local X As Byte , Rand As Byte

   ' random chars
   For Trail = 1 To 5
      X = Rnd(matrix_letters_x) + 1
      Trailend = Rnd(matrix_letters_y)
      Index = Matrix_getindex(x , Trailend)
      Matrix_randomize_letter Index
      Rand = Matrix_letters_visible(index)
      If 1 < Rand Then
         Matrix_draw_char X , Trailend , 0 , 0
         Matrix_draw_char X , Trailend , 0 , Rand
      End If
   Next

   ' update trails
   For Trail = 1 To Matrix_letters_x
      Rand = Rnd(3)
      If 0 < Rand Then
         If Matrix_updatecounter(trail) = 0 Then
            Matrix_updatecounter(trail) = Matrix_speed(trail)
            ' move
            Matrix_start(trail) = Matrix_start(trail) + 1
            ' draw new highlight char
            Trailend = Matrix_start(trail)
            Matrix_draw_char Trail , Trailend , Matrix_start_char(trail) , 1
            Trailend = Trailend - 1
            ' redraw old highlight char
            Matrix_draw_char Trail , Trailend , 0 , 0
            Matrix_draw_char Trail , Trailend , 0 , 2
            Trailend = Trailend - 1
            Matrix_draw_char Trail , Trailend , 0 , 3

            ' draw 2nd highlight char
            Trailend = Matrix_start(trail) - Matrix_2nd_highlight
            Matrix_draw_char Trail , Trailend , 0 , 0
            Matrix_draw_char Trail , Trailend , Matrix_highlight_char(trail) , 1
            Trailend = Trailend - 1
            Matrix_draw_char Trail , Trailend , 0 , 0
            Matrix_draw_char Trail , Trailend , 0 , 2
            Trailend = Trailend - 1
            Matrix_draw_char Trail , Trailend , 0 , 3

            ' delete trail end
            Trailend = Matrix_start(trail) - Matrix_letters_y
            Matrix_draw_char Trail , Trailend , 0 , 0
            If Trailend = Matrix_trail_stop Then
               Matrix_randomize_trail Trail
            Else
               Trailend = Trailend + 1
               Matrix_draw_char Trail , Trailend , 0 , 4
               Trailend = Trailend + 1
               Matrix_draw_char Trail , Trailend , 0 , 5
               Trailend = Trailend + 1
               Matrix_draw_char Trail , Trailend , 0 , 6
            End If
         Else
            Matrix_updatecounter(trail) = Matrix_updatecounter(trail) - 1
         End If
      End If
   Next
End Sub

Function Matrix_getindex(byval Char_x As Byte , Char_y As Integer) As Word
   Local Index As Word
   Char_x = Char_x - 1
   Index = Char_y * Matrix_letters_x
   Index = Index + Char_x
   Index = Index + 1
   Matrix_getindex = Index
End Function

Sub Matrix_draw_char(byval Char_x As Byte , Char_y As Integer , Byval Char As Byte , Byval Colorcode As Byte)
   Local Index As Word
   Local X1 As Word , Y1 As Word
   Local X2 As Word , Y2 As Word
   Local Color As Word
   Local Char_str As String * 1
   If 0 <= Char_y And Char_y < Matrix_letters_y Then
      Index = Matrix_getindex(char_x , Char_y)
      Matrix_letters_visible(index) = Colorcode
      Char_x = Char_x - 1
      X1 = Char_x * Matrix_font_width
      Y1 = Char_y * Matrix_font_height

      If Colorcode = 0 Then
         X2 = X1 + Matrix_font_width
         X2 = X2 - 1
         Y2 = Y1 + Matrix_font_height
         Y2 = Y2 - 1
         Lcd_fill_rect X1 , Y1 , X2 , Y2 , 0
      Else
         If Colorcode = 1 Then
            Color = Matrix_color_highlight                                      ' highlighted letter
         Else
            Color = Matrix_letters_color(index)
            Select Case Colorcode
            Case 2:
               Color = Color16alpha(matrix_color_highlight , Color , 16)
            Case 4:
               Color = Color16alpha(color_black , Color , 24)
            Case 5:
               Color = Color16alpha(color_black , Color , 16)
            Case 6:
               Color = Color16alpha(color_black , Color , 8)
            End Select
         End If

         If Char = 0 Then Char = Matrix_letters(index)
         Char_str = Chr(char)
         Lcd_tt_text Char_str , X1 , Y1 , Color
      End If
   End If
End Sub