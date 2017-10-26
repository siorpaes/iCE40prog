# iCE40prog
Very essential tools for iCE40 FPGAs bitstream programming using
bare bones FTDI C232HM MPSSE cable

Tested on Ubuntu 16.04.2 and Cygwin with UPDuino and Go Board boards

http://gnarlygrey.atspace.cc/development-platform.html#upduino

https://www.nandland.com/

Upduino board pinout

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

Go Board can only work with the bitbanged version of the tool as MISO and MOSI signals
are configured for iCE40 master mode (i.e.: loads bitstream from external serial flash).

Prerequisites: libftdi1.
On Cygwin use Zadig to install WinUSB driver.

libmpsse essential sources are embedded in the project for 
convenience as there's no Debian package for it yet.

