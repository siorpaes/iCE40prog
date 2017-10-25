# iCE40prog
Very essential tool for iCE40 FPGAs SRAM bitstream programming using
bare bones FTDI C232HM MPSSE cable

Tested with UPDuino board 

http://gnarlygrey.atspace.cc/development-platform.html#upduino

on Ubuntu 16.04.2 and Cygwin

Pinout

|Signal   | FTDI cable color | UPDuino  |
|---------|:----------------:|:--------:|
| CLK     |  Orange          |  JP2-3   |
| MOSI    |  Yellow          |  JP2-1   |
| CS      |  White           |  JP2-5   |
| RESET   |  Blue            |  JP1-2   |
| Vdd     |  Red             |  JP2-2   |
| GND     |  Black           |  JP2-6   |


Leave J1 unpopulated. A blinky LED bitstream example is provided for testing
purposes on UPDuino board.


Prerequisites: libftdi1
On Cygwin use Zadig to install WinUSB driver.

libmpsse essential sources are embedded in the project for 
convenience as there's no Debian package for it yet.

