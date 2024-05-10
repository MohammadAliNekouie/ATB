$nocompile

'************** config Clock 2MHz internal, PLL x24 ****************************
Const Pll_mul = _xtal / 2000000
Sub Set_pll()                                                                   '
   Local A As Byte
   Out &H50 , &B00000001                                                        ' enable  2MHz Oscillator
   Out &H55 , Pll_mul                                                           '&B00011000                                                           'PLL Clocksource = 2MHz internal Osc.
   Out &H34 , &HD8                                                              'No write protection for 4 cycles                                         '
   Out &H41 , &B00000000                                                        'Prescaler= No division Prescaler A, B and C
   While A.0 = 0                                                                'Wait for 2MHz Osc. to be ready
      A = Inp(&H51)
   Wend
   Out &H50 , &B00010001                                                        ' enable  PLL
   While A.4 = 0                                                                'Wait for PLL to be ready
      A = Inp(&H51)
   Wend
   Out &H34 , &HD8                                                              'No write protection for 4 cycles                                         '
   Out &H40 , &B00000100                                                        'Select PLL as system Clock
   Out &H34 , &HD8                                                              'No write protection for 4 cycles                                         '
   Out &H42 , &B00000001                                                        'Lock System Clock Settings
End Sub
Set_pll