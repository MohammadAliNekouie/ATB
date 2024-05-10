$regfile = "xm128a1def.dat"
'$regfile = "m128def.dat"
$crystal = 32000000
$hwstack = 64
$swstack = 64
$framesize = 64

Config Submode = New
Const False = 0
Const True = 1

' XMega settings
#if _xmega = True
   ' sets the clock frequency using the PLL with the internal 2MHz oscillator
   $include "XMegaPll.inc"

   ' Host communication
   Config Com1 = 115200 , Mode = Asynchroneous , Parity = None , Stopbits = 1 , Databits = 8

   ' use the watchdog for communication timeouts
   Config Watchdog = 1000

   ' Xplained DataFlash SS
   Const Flash_port_ss = Portq
   Const Flash_pin_ss = 2

' ATMega settings
#else
   ' Host communication
   $baud = 115200
   Config Com1 = Dummy , Synchrone = 0 , Parity = None , Stopbits = 1 , Databits = 8 , Clockpol = 0

   ' use the watchdog for communication timeouts
   Config Watchdog = 1024
#endif

Open "COM1:" For Binary As #1

Const Flash_enable_statusled = True                         '
$include "SPI-Flash.inc"

Dim Flashbuffer(256) As Byte
Dim Shakedhands As Boolean
Dim Sector As Byte , Page As Word , Address As Dword

Flash_init

Do
   ' wait for command
   Inputbin #1 , Sector
   Select Case Sector

   ' Handshake/Exit
   Case &HAA:
      Shakedhands = Not Shakedhands                         ' some kind of accidental write protection
      Printbin #1 , &HF0                                    ' ack

   ' erase a sector (address bits 23..16)
   Case &H01:
      Reset Watchdog
      Start Watchdog
      Inputbin #1 , Sector                                  ' read sector address
      Stop Watchdog
      If Shakedhands = True Then Flash_sector_erase Sector
      Printbin #1 , &HF0                                    ' ack

   ' erase whole memory
   Case &H02:
      If Shakedhands = True Then Flash_bulk_erase
      Printbin #1 , &HF0                                    ' ack

   ' write a page (256 Bytes, address bits 23..8)
   Case &H03:
      Reset Watchdog
      Start Watchdog
      Inputbin #1 , Page                                    ' read page address
      Inputbin #1 , Flashbuffer(1) ; 256                    ' read 256 bytes page data
      Stop Watchdog
      Swap Page
      If Shakedhands = True Then Flash_write_page Page , Flashbuffer(1)
      Printbin #1 , &HF0                                    ' ack

   ' read a page (256 Bytes, address bits 23..8)
   Case &H04:
      Reset Watchdog
      Start Watchdog
      Inputbin #1 , Page                                    ' read page address
      Stop Watchdog
      Swap Page
      Address = Page
      Shift Address , Left , 8
      If Shakedhands = True Then Flash_read Address , Flashbuffer(1) , 256
      Printbin #1 , Flashbuffer(1) ; 256                    ' write page data

   ' read the device ID data (81 Bytes)
   Case &H05:
      If Shakedhands = True Then Flash_get_info Flashbuffer(1)
      Printbin #1 , Flashbuffer(1) ; 81

   End Select
Loop