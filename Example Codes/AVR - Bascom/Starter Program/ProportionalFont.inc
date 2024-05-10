' Proportional Font Library 1.3, 2014-2017
' Web: http://mat.midlight.eu/wiki/index.php/Bascom_Proportional_Fonts
$nocompile

#if Not Varexist( "False")
   Const False = 0
   Const True = 1
#endif



Dim Lcd_tt_fontheight As Byte                               ' max. height in pixels of the current loaded font
Dim Lcd_tt_fontspacing As Byte                              ' ranging from -15 to +15, bias value 15

Dim Lcd_tt_chars As Byte
Dim Lcd_tt_rlecompressed As Bit
Dim Lcd_tt_chartable_ptr As Word
#if Varexist( "RAMPZ")
   Dim Lcd_tt_chartable_rampz As Byte
#endif
Dim Lcd_tt_blocksize As Byte
Dim Lcd_tt_fontdisplace As Byte



Macro Lcd_tt_load_byte
   !movw ZL, R8
   #if Varexist( "RAMPZ")
      !Out Rampz , R10
      !elpm R11, Z+
      !In R10, Rampz
      !Out Rampz , R0
   #else
      !lpm R11, Z+
   #endif
   !movw R8, ZL
End Macro



' Loads font data, use    Restore [Fontlabel]    first
Sub Lcd_tt_loadfont()
   !sts {Lcd_tt_chartable_ptr}, _DPTRL
   !sts {Lcd_tt_chartable_ptr+1}, _DPTRH
   #if Varexist( "RAMPZ")
      Lcd_tt_chartable_rampz = ___readrampz
   #endif
   Read Lcd_tt_fontheight
   Read Lcd_tt_fontspacing
   Lcd_tt_rlecompressed = Lcd_tt_fontspacing.5
   Lcd_tt_fontspacing = Lcd_tt_fontspacing And &H1F
   Read Lcd_tt_chars
   Lcd_tt_chartable_ptr = Lcd_tt_chartable_ptr + 3
   Lcd_tt_blocksize = Lcd_tt_fontheight + 7
   Lcd_tt_blocksize = Lcd_tt_blocksize / 8
   Lcd_tt_fontdisplace = Lcd_tt_blocksize * 8
End Sub



' Draws a text on the screen using the previously loaded font
Sub Lcd_tt_text(byval Text As String * 50 , Byval X As Word , Byval Y As Word , Byval Forecolor As Word)
   Local Stringindex As Byte
   Local Stringlength As Byte
   Local Stringpointer As Word
   Local Stringchar As Byte
   Local Charindex As Byte
   Local Tablechar As Byte
   Local Tempindex As Byte
   Local Fontdata As Byte
   Local Tempbyte As Byte
   Local Xpos As Word
   Local Previous As Byte
   Local Repeat As Byte
   Local Firstbyte As Byte

   Stringlength = Len(text)
   Stringpointer = Varptr(text)

   For Stringindex = 0 To Stringlength
      #if Varexist( "RAMPZ")
         !clr R0
         !out RAMPZ, R0
      #endif
      Stringchar = Inp(stringpointer)
      Stringpointer = Stringpointer + 1

      ' restore chartable pointer
      !LDS _DPTRL, {Lcd_tt_chartable_ptr}
      !LDS _DPTRH, {Lcd_tt_chartable_ptr+1}
      #if Varexist( "RAMPZ")
         !lds R10, {Lcd_tt_chartable_rampz}
      #endif

      ' search for char in chartable
      Fontdata = False
      For Charindex = 1 To Lcd_tt_chars
         Lcd_tt_load_byte
         Tablechar = R11
         If Stringchar = Tablechar Then
            Tempindex = Lcd_tt_chars - Charindex            ' jump to end of char table
            R23 = Tempindex
            !Add _DPTRL, R23
            !ldi R23, 0
            !adc _DPTRH, R23
            #if Varexist( "RAMPZ")
               !brcc tt_skip_ramp1
                  !inc R10
               Tt_skip_ramp1:
            #endif
            Fontdata = True
            Exit For
         End If
      Next

      If Fontdata = True Then                               ' char is in chartable
         ' skip preceding chars in fonttable
         If 1 < Charindex Then
            Charindex = Charindex - 2
            For Stringchar = Charindex To 0 Step -1

               Lcd_tt_load_byte                             ' char width
               Tablechar = R11
               If Lcd_tt_rlecompressed = True Then
                  Lcd_tt_load_byte                          ' font data byte size
                  Tablechar = R11
               Else
                  Tablechar = Tablechar * Lcd_tt_blocksize
               End If
               R23 = Tablechar
               !Add _DPTRL, R23
               !ldi R23, 0
               !adc _DPTRH, R23
               #if Varexist( "RAMPZ")
                  !brcc tt_skip_ramp2
                     !inc R10
                  Tt_skip_ramp2:
               #endif
            Next
         Else
            Charindex = Charindex - 1
         End If

         ' read font data and draw char
         Repeat = 0
         Firstbyte = True
         Lcd_tt_load_byte                                   ' char width
         Tablechar = R11
         If Lcd_tt_rlecompressed = True Then Lcd_tt_load_byte       ' font data byte size, not needed now
         For Charindex = 1 To Lcd_tt_blocksize              ' blocks
            Xpos = X
            For Stringchar = 1 To Tablechar                 ' width
               If Lcd_tt_rlecompressed = True Then
                  If Repeat = 0 Then
                     Lcd_tt_load_byte
                     Fontdata = R11
                     If Firstbyte = False Then
                        If Previous = Fontdata Then         ' not first data byte and same as previous one
                           Lcd_tt_load_byte
                           Repeat = R11
                        End If
                     Else
                        Firstbyte = False
                     End If
                     Previous = Fontdata
                  Else
                     Repeat = Repeat - 1
                  End If
               Else
                  Lcd_tt_load_byte
                  Fontdata = R11
               End If
               R11 = Fontdata
               !ldi R16, &H80
               !mov R12, R16
               Font_pixel_loop:                             ' draw block
                  !mov R13, R11
                  !and R13, R12
                  !breq Font_pixel_dontset
                     #if Varexist( "Ili9341")
                        Lcd_set_pixel Xpos , Y , Forecolor
                     #else
                        ' ### OTHER DISPLAYS ###
                        !push R10
                        !push R11
                        !push R12
                        !Push R13

                        Pset Xpos , Y , 1

                        !pop R13
                        !pop R12
                        !pop R11
                        !pop R10
                        ' ######################
                     #endif
                  Font_pixel_dontset:
                  Y = Y + 1
               !lsr R12
               !brne Font_pixel_loop
               Y = Y - 8
               Xpos = Xpos + 1
            Next
            Y = Y + 8
         Next

         X = X + Tablechar
         X = X - 15
         X = X + Lcd_tt_fontspacing
         Y = Y - Lcd_tt_fontdisplace
      End If
   Next
End Sub



' returns the width of a text using the loaded font
Function Lcd_tt_textwidth(byval Text As String) As Word
   Local Stringindex As Byte
   Local Stringlength As Byte
   Local Stringpointer As Word
   Local Stringchar As Byte
   Local Charindex As Byte
   Local Tablechar As Byte
   Local Tempindex As Byte
   Local Fontdata As Byte
   Local X As Word

   X = 0
   Stringlength = Len(text) - 1                             ' WARUM -1 ?
   Stringpointer = Varptr(text)

   For Stringindex = 0 To Stringlength
      #if Varexist( "RAMPZ") And _huge = False
         !clr R0
         !out RAMPZ, R0
      #endif
      Stringchar = Inp(stringpointer)
      Stringpointer = Stringpointer + 1

      ' restore chartable pointer
      !LDS _DPTRL, {Lcd_tt_chartable_ptr}
      !LDS _DPTRH, {Lcd_tt_chartable_ptr+1}
      #if Varexist( "RAMPZ")
         !lds R10, {Lcd_tt_chartable_rampz}
      #endif

      ' search for char in chartable
      Fontdata = False
      For Charindex = 1 To Lcd_tt_chars
         Lcd_tt_load_byte
         Tablechar = R11
         If Stringchar = Tablechar Then
            Tempindex = Lcd_tt_chars - Charindex            ' jump to end of char table
            R23 = Tempindex
            !Add _DPTRL, R23
            !ldi R23, 0
            !adc _DPTRH, R23
            #if Varexist( "RAMPZ")
               !brcc tt_skip_ramp3
                  !inc R10
               Tt_skip_ramp3:
            #endif
            Fontdata = True
            Exit For
         End If
      Next

      If Fontdata = True Then                               ' char is in chartable
         ' skip preceding chars in fonttable
         If 1 < Charindex Then
            Charindex = Charindex - 2
            For Stringchar = Charindex To 0 Step -1
               Lcd_tt_load_byte                             ' char width
               Tablechar = R11
               If Lcd_tt_rlecompressed = True Then
                  Lcd_tt_load_byte                          ' font data byte size
                  Tablechar = R11
               Else
                  Tablechar = Tablechar * Lcd_tt_blocksize
               End If
               R23 = Tablechar
               !Add _DPTRL, R23
               !ldi R23, 0
               !adc _DPTRH, R23
               #if Varexist( "RAMPZ")
                  !brcc tt_skip_ramp4
                     !inc R10
                  Tt_skip_ramp4:
               #endif
            Next
         Else
            Charindex = Charindex - 1
         End If

         ' read font data
         Lcd_tt_load_byte                                   ' char width
         X = X + R11
         X = X - 15
         X = X + Lcd_tt_fontspacing
      End If
   Next
   Lcd_tt_textwidth = X
End Function