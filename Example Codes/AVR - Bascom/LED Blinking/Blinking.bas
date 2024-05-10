$regfile = "m2560def.dat"
$hwstack = 128
$swstack = 128
$framesize = 32
$crystal = 1000000

Config Portb = Output

Do

Toggle portb.4
Waitms 200 

Loop
End
