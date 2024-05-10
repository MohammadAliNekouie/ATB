' SPI-DataFlash Library 1.0,  Apr. 2015
' Supports S25FL devices (AT25DF compatible)
' http://mat.midlight.eu/wiki/index.php?title=SPI_DataFlash_Library

$nocompile
 Config Error = Ignore , 380 = Ignore
'Set to true to use software SPI instead of hardware modules, any GPIOs will work
#if Not Varexist( "Flash_use_soft_spi")
   Const Flash_use_soft_spi = False
#endif

' False only configures the SS pin, does not enable and configure the SPI interface,
' this needs to be done manually (useful if the memory shares the SPI interface with other hardware)
#if Not Varexist( "Flash_init_spi")
   Const Flash_init_spi = True
#endif

' XMega
#if _xmega = True
   ' Sets the port of the SPI interface
   #if Not Varexist( "Flash_port_spi")
      Const Flash_port_spi = Portc
   #endif

   ' Set the SPI pins (CS, CLK, SDO and SDI are set to the hardware SPI pins by default)
   #if Not Varexist( "Flash_pin_cs")
      Const Flash_pin_cs = 4
   #endif

   #if Not Varexist( "Flash_pin_clk")
      Const Flash_pin_clk = 7
   #endif

   #if Not Varexist( "Flash_pin_sdo")
      Const Flash_pin_sdo = 5
   #endif

   #if Not Varexist( "Flash_pin_sdi")
      Const Flash_pin_sdi = 6
   #endif

' ATMega
#else
   ' Sets the port of the SPI interface
   #if Not Varexist( "Flash_port_spi")
      Const Flash_port_spi = Portb
   #endif

   ' Set the SPI pins (CS, CLK, SDO and SDI are set to the hardware SPI pins by default)
   #if Not Varexist( "Flash_pin_cs")
      Const Flash_pin_cs = 0
   #endif

   #if Not Varexist( "Flash_pin_clk")
      Const Flash_pin_clk = 1
   #endif

   #if Not Varexist( "Flash_pin_sdo")
      Const Flash_pin_sdo = 2
   #endif

   #if Not Varexist( "Flash_pin_sdi")
      Const Flash_pin_sdi = 3
   #endif
#endif

' (HW SPI) If the memory's SS signal is not connected to the HW SPI CS pin, the correct pin needs to be set
#if Not Varexist( "Flash_port_ss")
   Const Flash_port_ss = Flash_port_spi
#endif

#if Not Varexist( "Flash_pin_ss")
   Const Flash_pin_ss = Flash_pin_cs
#endif

' Enable the status LED. Active during ongoing transmissions
#if Not Varexist( "Flash_enable_statusled")
   Const Flash_enable_statusled = False
#endif

#if Flash_enable_statusled = True
   ' Invert the status LED (active low)
   #if Not Varexist( "Flash_invert_statusled")
      Const Flash_invert_statusled = False
   #endif

   ' Set the pin of the status LED
   #if _xmega = True
      Flash_statusled Alias Porte.0
   #else
      Flash_statusled Alias Portb.4
   #endif
#endif

Const Spiflash_available = True

Const Flash_cmd_rdid = &H9F
Const Flash_cmd_read_status = &H05
Const Flash_cmd_read = &H03
Const Flash_cmd_write_enable = &H06
Const Flash_cmd_sector_erase = &HD8
Const Flash_cmd_bulk_erase = &H60
Const Flash_cmd_program_page = &H02

#if _xmega = True
   Const Flash_ddr_spi = Flash_port_spi - 4
   Const Flash_pin_spi = Flash_port_spi + 4
   Const Flash_ddr_ss = Flash_port_ss - 4

   #if Varptr( "Flash_port_spi") = Varptr( "Portc")
      Const Flash_spi_ctrl = Spic_ctrl
      Flash_spi_status Alias Spic_status
      Flash_spi_data Alias Spic_data
   #elseif Varptr( "Flash_port_spi") = Varptr( "Portd")
      Const Flash_spi_ctrl = Spid_ctrl
      Flash_spi_status Alias Spid_status
      Flash_spi_data Alias Spid_data
   #elseif Varptr( "Flash_port_spi") = Varptr( "Porte")
      Const Flash_spi_ctrl = Spie_ctrl
      Flash_spi_status Alias Spie_status
      Flash_spi_data Alias Spie_data
   #elseif Varptr( "Flash_port_spi") = Varptr( "Portf")
      Const Flash_spi_ctrl = Spif_ctrl
      Flash_spi_status Alias Spif_status
      Flash_spi_data Alias Spif_data
   #endif

   Const Flash_spi_ctrl_init = &B1_1_0_1_00_00                                  ' Clk2x, Enable, Data Order = MSB, Master, Mode 0, Prescaler Clk/4
#else
   Const Flash_ddr_spi = Flash_port_spi + 1
   Const Flash_pin_spi = Flash_port_spi + 2
   Const Flash_ddr_ss = Flash_port_ss + 1

   Const Flash_spi_ctrl = Spcr
   Flash_spi_status Alias Spsr
   Flash_spi_data Alias Spdr
   Const Flash_spi_ctrl_init = &H0_1_0_1_0_0_00                                 ' No INT, Enable, MSB, Master, CPOL=0, CPHA=0, CLK/4
   Const Flash_spi_status_init = &H00_00000_1                                   ' CLKx2
#endif

Const Flash_set_cs = 2 ^ Flash_pin_cs
Const Flash_set_clk = 2 ^ Flash_pin_clk
Const Flash_clr_clk = Not Flash_set_clk
Const Flash_set_sdo = 2 ^ Flash_pin_sdo
Const Flash_clr_sdo = Not Flash_set_sdo
Const Flash_read_sdi = 2 ^ Flash_pin_sdi
Const Flash_set_ss = 2 ^ Flash_pin_ss
Const Flash_clr_ss = Not Flash_set_ss

Const Flash_ddr_spi_init = Flash_set_cs + Flash_set_clk + Flash_set_sdo

' Initializes the soft-/hardware SPI interface if enabled and configures the SS pin
Sub Flash_init()
   #if Flash_init_spi = True
      !in R16, Flash_ddr_spi                                ' config and set SPI DDR (CS, CLK, SDO as output)
      !ori R16, flash_ddr_spi_init
      !out Flash_ddr_spi, R16
      !in R16, Flash_port_spi                               ' config and set SPI PORT
      !ori R16, flash_set_cs
      !out Flash_port_spi, R16
      #if Flash_use_soft_spi = False
         #if _xmega = False
            Flash_spi_status = Flash_spi_status_init
         #endif
         !ldi R16, Flash_spi_ctrl_init
         !sts Flash_spi_ctrl, R16
      #endif
   #endif

   !in R16, Flash_ddr_ss                                    ' config as output and set SS
   !ori R16, flash_set_ss
   !out Flash_ddr_ss, R16
   Flash_ss_high

   #if Flash_enable_statusled = True
      Config Flash_statusled = Output
      #if Flash_invert_statusled = True
         Toggle Flash_statusled
      #endif
   #endif
End Sub

' Deactivates the SPI interface
Sub Flash_stop()
   #if Flash_use_soft_spi = False
      !clr R0
      !out Flash_spi_ctrl, R0
   #endif
End Sub

' Sets the CS pin
Sub Flash_ss_high()
   !in R16, Flash_port_ss
   !ori R16, flash_set_ss
   !out Flash_port_ss, R16
   #if Flash_enable_statusled = True
      Toggle Flash_statusled
   #endif
End Sub

' Clears the CS pin
Sub Flash_ss_low()
   !in R16, Flash_port_ss
   !andi R16, flash_clr_ss
   !out Flash_port_ss, R16
   #if Flash_enable_statusled = True
      Toggle Flash_statusled
   #endif
End Sub

' Writes a byte to the SPI bus
Sub Flash_writebyte(byval Databyte As Byte)
   #if Flash_use_soft_spi = False
      Flash_spi_data = Databyte
      While Flash_spi_status = 0 : Wend
   #else
      R17 = Databyte
      !in R16, flash_port_spi
      !ldi R22, &H80
      Flash_bitbang_out_loop:
         !Mov R23, R17
         !AND R23, R22                                      ' get MSB
         !BREQ Flash_bitbang_out_zero                       ' 0 or 1?
            !ori R16, flash_set_sdo                         ' set 1
            !rjmp Flash_bitbang_out_continue
         Flash_bitbang_out_zero:
            !andi R16, flash_clr_sdo                        ' set 0
         Flash_bitbang_out_continue:
         !ori R16, flash_set_clk                            ' set clock
         !out flash_port_spi, R16
         !andi R16, flash_clr_clk                           ' clear clock
         !out flash_port_spi, R16
         !LSR R22                                           ' advance bit position
      !BRNE Flash_bitbang_out_loop                          ' continue loop if bits left to transfer
   #endif
End Sub

' Reads a byte from the SPI bus
Function Flash_readbyte() As Byte
   Local Databyte As Byte
   #if Flash_use_soft_spi = False
      Flash_spi_data = 0
      While Flash_spi_status = 0 : Wend
      Databyte = Flash_spi_data
   #else
      !in R16, flash_port_spi
      !clr R17
      !ldi R22, &H80
      Flash_bitbang_in_loop:
         !ori R16, flash_set_clk                            ' set clock
         !out flash_port_spi, R16
         !in R23, flash_pin_spi
         !ANDI R23, Flash_read_sdi                          ' get MSB
         !BREQ Flash_bitbang_in_zero                        ' 0 or 1?
            !OR R17, R22                                    ' set 1
         Flash_bitbang_in_zero:
         !andi R16, flash_clr_clk                           ' clear clock
         !out flash_port_spi, R16
         !LSR R22                                           ' advance bit position
      !BRNE Flash_bitbang_in_loop                                                ' continue loop if bits left to transfer
      Databyte = R17
   #endif
   Flash_readbyte = Databyte
End Function


' Sends the Write Enable command. Needed prior to any write operation.
Sub Flash_writeenable()
   Flash_ss_low
   Flash_writebyte Flash_cmd_write_enable
   Flash_ss_high
End Sub

' Continously reads the status register and waits for the WIP bit to be cleared
Sub Flash_status_wait()
   Local Status As Byte
   Flash_ss_low
   Flash_writebyte Flash_cmd_read_status
   Do
      Status = Flash_readbyte()
   Loop Until Status.0 = 0
   Flash_ss_high
End Sub

' Writes a page (256 Bytes, address &HXXXX00)
Sub Flash_write_page(byval Address As Word , Byref Pagedata() As Byte)
   Local Pagelow As Byte , Pagehigh As Byte
   Pagelow = Address
   Shift Address , Right , 8
   Pagehigh = Address
   Flash_writeenable
   Flash_ss_low
   Flash_writebyte Flash_cmd_program_page
   Flash_writebyte Pagehigh
   Flash_writebyte Pagelow
   Flash_writebyte 0
   Address = Varptr(pagedata(1))
   For Pagelow = 0 To 255
      Pagehigh = Inp(address)
      Flash_writebyte Pagehigh
      Incr Address
   Next
   Flash_ss_high
   Flash_status_wait
End Sub

' Initiates a read sequence at byte address (&HXXXXXX)
Sub Flash_beginread(byval Address As Dword)
   Local Pageoffset As Byte , Pagelow As Byte , Pagehigh As Byte
   Pageoffset = Address
   Shift Address , Right , 8
   Pagelow = Address
   Shift Address , Right , 8
   Pagehigh = Address
   Flash_ss_low
   Flash_writebyte Flash_cmd_read
   Flash_writebyte Pagehigh
   Flash_writebyte Pagelow
   Flash_writebyte Pageoffset
End Sub

' Stops an ongoing read sequence
Sub Flash_endread()
   Flash_ss_high
End Sub

' Reads [Length] bytes starting at the byte address (&HXXXXXX) to a array
Sub Flash_read(byval Address As Dword , Byref Spiflashbuffer() As Byte , Byval Length As Word)
   Local Dataindex As Word
   Flash_beginread Address
   For Dataindex = 1 To Length
      Spiflashbuffer(dataindex) = Flash_readbyte()
   Next
   Flash_ss_high
End Sub

' Reads [Length] bytes starting at the byte address (&HXXXXXX) to a SRAM memory address (&HXXXXXX or &HXXXX)
#if _xmega = True
   Sub Flash_readto(byval Sourceaddress As Dword , Byval Destinationaddress As Dword , Byval Length As Dword)
#else
   Sub Flash_readto(byval Sourceaddress As Dword , Byval Destinationaddress As Word , Byval Length As Word)
#endif
   Local Databyte As Byte
   Flash_beginread Sourceaddress
   Do
      Databyte = Flash_readbyte()
      Out Destinationaddress , Databyte
      Incr Destinationaddress
      Decr Length
   Loop Until Length = 0
   Flash_ss_high
End Sub

' Erases (sets to &HFF) a sector (64Kb, &HXX0000), needed prior to page writes
Sub Flash_sector_erase(byval Sector As Byte)
   Flash_writeenable
   Flash_ss_low
   Flash_writebyte Flash_cmd_sector_erase
   Flash_writebyte Sector
   Flash_writebyte 0
   Flash_writebyte 0
   Flash_ss_high
   Flash_status_wait
End Sub

' Erases (sets to &HFF) the complete memory
Sub Flash_bulk_erase()
   Flash_writeenable
   Flash_ss_low
   Flash_writebyte Flash_cmd_bulk_erase
   Flash_ss_high
   Flash_status_wait
End Sub

' Reads the 81 ID Bytes
Sub Flash_get_info(byref Spiflashbuffer() As Byte)
   Local Dataindex As Byte
   Flash_ss_low
   Flash_writebyte Flash_cmd_rdid
   For Dataindex = 1 To 81
      Spiflashbuffer(dataindex) = Flash_readbyte()
   Next
   Flash_ss_high
End Sub