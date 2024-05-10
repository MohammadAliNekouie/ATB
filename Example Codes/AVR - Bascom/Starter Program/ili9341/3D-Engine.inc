$nocompile

#if Not Varexist( "False")
   Const False = 0
   Const True = 1
#endif

#if Not Varexist( "Dreid_max_points")
   Const Dreid_max_points = 8
#endif

#if Not Varexist( "Dreid_max_faces")
   Const Dreid_max_faces = 6
#endif

#if Not Varexist( "Dreid_max_vertices")
   Const Dreid_max_vertices = 4
#endif

#if Not Varexist( "Dreid_enable_z_gradient")
   Const Dreid_enable_z_gradient = False
#endif

Const Dreid_max_vertice_points = Dreid_max_faces * Dreid_max_vertices

Dim Dreid_points As Byte
Dim Dreid_point_x(dreid_max_points) As Single
Dim Dreid_point_y(dreid_max_points) As Single
Dim Dreid_point_z(dreid_max_points) As Single
Dim Dreid_transformed_x(dreid_max_points) As Single
Dim Dreid_transformed_y(dreid_max_points) As Single
Dim Dreid_transformed_z(dreid_max_points) As Single

Dim Dreid_faces As Byte
Dim Dreid_face_vertices(dreid_max_faces) As Byte

Dim Dreid_vertice_points(dreid_max_vertice_points) As Byte

Dim Dreid_visible_screen As Byte
Dim Dreid_screen1_x(dreid_max_points) As Word
Dim Dreid_screen1_y(dreid_max_points) As Word
Dim Dreid_screen2_x(dreid_max_points) As Word
Dim Dreid_screen2_y(dreid_max_points) As Word

Dim Dreid_project_x As Word
Dim Dreid_project_y As Word       
Dim Dreid_fov As Single
Dim Dreid_viewer_distance As Single

Dim Dreid_firstdraw As Bit

Dim Dreid_backcolor As Word
Dim Dreid_forecolor As Word

#if Dreid_enable_z_gradient = True
   Dim Dreid_color_far As Word
   Dim Dreid_color(dreid_max_points) As Word
#endif

Sub Dreid_rotate(byval Rotate_x As Single , Byval Rotate_y As Single , Byval Rotate_z As Single)
   Local Index As Byte
   Local Tmp1 As Single , Tmp2 As Single
   Local Sina As Single , Cosa As Single

   For Index = 1 To Dreid_points
      Dreid_transformed_x(index) = Dreid_point_x(index)
      Dreid_transformed_y(index) = Dreid_point_y(index)
      Dreid_transformed_z(index) = Dreid_point_z(index)
   Next

   Rotate_x = Deg2rad(rotate_x)
   Rotate_y = Deg2rad(rotate_y)
   Rotate_z = Deg2rad(rotate_z)

   ' rotate around x-axis by given angle
   Sina = Sin(rotate_x)
   Cosa = Cos(rotate_x)
   For Index = 1 To Dreid_points
      Tmp1 = Dreid_transformed_y(index) * Cosa
      Tmp2 = Dreid_transformed_z(index) * Sina
      Tmp1 = Tmp1 - Tmp2
      Tmp2 = Dreid_transformed_y(index) * Sina
      Dreid_transformed_y(index) = Tmp1
      Tmp1 = Dreid_transformed_z(index) * Cosa
      Dreid_transformed_z(index) = Tmp1 + Tmp2
   Next

   ' rotate around y-axis by given angle
   Sina = Sin(rotate_y)
   Cosa = Cos(rotate_y)
   For Index = 1 To Dreid_points
      Tmp1 = Dreid_transformed_z(index) * Cosa
      Tmp2 = Dreid_transformed_x(index) * Sina
      Tmp1 = Tmp1 - Tmp2
      Tmp2 = Dreid_transformed_z(index) * Sina
      Dreid_transformed_z(index) = Tmp1
      Tmp1 = Dreid_transformed_x(index) * Cosa
      Dreid_transformed_x(index) = Tmp1 + Tmp2
   Next

   ' rotate around z-axis by given angle
   Sina = Sin(rotate_z)
   Cosa = Cos(rotate_z)
   For Index = 1 To Dreid_points
      Tmp1 = Dreid_transformed_x(index) * Cosa
      Tmp2 = Dreid_transformed_y(index) * Sina
      Tmp1 = Tmp1 - Tmp2
      Tmp2 = Dreid_transformed_x(index) * Sina
      Dreid_transformed_x(index) = Tmp1
      Tmp1 = Dreid_transformed_y(index) * Cosa
      Dreid_transformed_y(index) = Tmp1 + Tmp2
   Next
End Sub

Sub Dreid_move(byval Move_x As Single , Byval Move_y As Single , Byval Move_z As Single)
   Local Index As Byte
   Local As Single , Tmp2 As Single

End Sub

Sub Dreid_update()
   Local Index As Byte
   Local Tmp1 As Single , Tmp2 As Single

   #if Dreid_enable_z_gradient = True
      Local R1 As Byte , G1 As Byte , B1 As Byte
      Local R2 As Byte , G2 As Byte , B2 As Byte

      Local R As Byte , G As Byte , B As Byte
      Local Z As Single , Zmin As Single , Zmax As Single , Color2 As Word
      Zmax = 0
   #endif

   ' transform 3D points to 2D screen coordinates (perspective projection)
   Dreid_visible_screen = 1 - Dreid_visible_screen
   For Index = 1 To Dreid_points
      Tmp1 = Dreid_viewer_distance + Dreid_transformed_z(index)
      Tmp1 = Dreid_fov / Tmp1
      Tmp2 = Dreid_transformed_x(index) * Tmp1
      Tmp2 = Tmp2 + Dreid_project_x
      Tmp1 = Dreid_transformed_y(index) * Tmp1
      Tmp1 = -tmp1
      Tmp1 = Tmp1 + Dreid_project_y

      If Dreid_visible_screen = 0 Then
         Dreid_screen1_x(index) = Tmp2
         Dreid_screen1_y(index) = Tmp1
      Else
         Dreid_screen2_x(index) = Tmp2
         Dreid_screen2_y(index) = Tmp1
      End If

      #if Dreid_enable_z_gradient = True
         If Zmax < Dreid_transformed_z(index) Then Zmax = Dreid_transformed_z(index)
      #endif
   Next
   #if Dreid_enable_z_gradient = True
      Zmin = Zmax
      For Index = 1 To Dreid_points
         If Dreid_transformed_z(index) < Zmin Then Zmin = Dreid_transformed_z(index)
      Next
      Zmax = Zmax - Zmin
      Color16to24 Dreid_forecolor , R1 , G1 , B1
      Color16to24 Dreid_color_far , R2 , G2 , B2

      For Index = 1 To Dreid_points
         Z = Dreid_transformed_z(index)
         Z = Z - Zmin
         R = Dreid_interpolate_color(r1 , R2 , Z , Zmax)
         G = Dreid_interpolate_color(g1 , G2 , Z , Zmax)
         B = Dreid_interpolate_color(b1 , B2 , Z , Zmax)
         Dreid_color(index) = Color24to16(r , G , B)
      Next
   #endif

   ' delete old lines
   If Dreid_firstdraw = False Then
      Dreid_visible_screen = 1 - Dreid_visible_screen
      Dreid_draw Dreid_backcolor
      ' draw updated lines
      Dreid_visible_screen = 1 - Dreid_visible_screen
   Else
      Dreid_firstdraw = False
   End If
   Dreid_draw Dreid_forecolor
End Sub

Sub Dreid_draw(byval Color As Word)
   Local Index As Byte , Index2 As Byte , Index3 As Byte , Index4 As Byte , Index5 As Byte
   Local X1 As Word , X2 As Word , Y1 As Word , Y2 As Word

   For Index = 1 To Dreid_faces
      For Index2 = 1 To Dreid_face_vertices(index)
         Index3 = Index - 1
         Index3 = Index3 * Dreid_max_vertices
         Index3 = Index3 + Index2
         Index4 = Dreid_vertice_points(index3)
         If Index2 = Dreid_face_vertices(index) Then
            Index3 = Index - 1
            Index3 = Index3 * Dreid_max_vertices
            Index3 = Index3 + 1
            Index3 = Dreid_vertice_points(index3)
         Else
            Index3 = Dreid_vertice_points(index3 + 1)
         End If
         #if Varexist( "Ili9341")
            If Dreid_visible_screen = 0 Then
               X1 = Dreid_screen1_x(index3)
               X2 = Dreid_screen1_x(index4)
               Y1 = Dreid_screen1_y(index3)
               Y2 = Dreid_screen1_y(index4)
            Else
               X1 = Dreid_screen2_x(index3)
               X2 = Dreid_screen2_x(index4)
               Y1 = Dreid_screen2_y(index3)
               Y2 = Dreid_screen2_y(index4)
            End If
            #if Dreid_enable_z_gradient = False
               Lcd_line X1 , Y1 , X2 , Y2 , Color
            #else
               If Color = Dreid_backcolor Then
                  Lcd_line X1 , Y1 , X2 , Y2 , Dreid_backcolor
               Else
                  Lcd_line_gradient X1 , Y1 , X2 , Y2 , Dreid_color(index3) , Dreid_color(index4)
               End If
            #endif
         #else
            If Dreid_visible_screen = 0 Then

            Else

            End If
         #endif
      Next
   Next
End Sub


#if Dreid_enable_z_gradient = True
   Function Dreid_interpolate_color(byval Startvalue As Byte , Byval Endvalue As Byte , Byval Value As Single , Byval Valuemax As Single) As Byte
      Value = Value / Valuemax
      Valuemax = Endvalue
      Valuemax = Valuemax - Startvalue
      Value = Value * Valuemax
      Value = Value + .5
      Endvalue = Value
      Startvalue = Startvalue + Endvalue
      Dreid_interpolate_color = Startvalue
   End Function
#endif

Sub Dreid_load_model()
   Local Index As Byte , Index2 As Byte , Index3 As Byte
   Dreid_firstdraw = True
   Read Dreid_points
   For Index = 1 To Dreid_points
      Read Dreid_point_x(index)
      Read Dreid_point_y(index)
      Read Dreid_point_z(index)
   Next
   Read Dreid_faces

   For Index = 1 To Dreid_faces
      Read Dreid_face_vertices(index)
      For Index2 = 1 To Dreid_face_vertices(index)
         Index3 = Index - 1
         Index3 = Index3 * Dreid_max_vertices
         Index3 = Index3 + Index2
         Read Dreid_vertice_points(index3)
      Next
   Next
End Sub