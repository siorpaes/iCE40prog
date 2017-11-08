# iCE40prog
Very essential tool for iCE40 FPGAs *SRAM* bitstream programming using
bare bones FTDI C232HM MPSSE cable. Does not support (yet) serial flash programming.

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


Leave J1 unpopulated. A blinky LED bitstream example is provided for testing purposes on UPDuino board.

Go Board can only work with the bitbanged version of the tool as MISO and MOSI signals
are configured for iCE40 master mode (i.e.: loads bitstream from external serial flash).

Prerequisites: libftdi1.
On Cygwin use Zadig to install WinUSB driver.
On Linux, copy (as root) the contents of udev-rules directory into /etc/udev/rules.d/ if willing to use MPSSE cable as non-root user.

libmpsse essential sources are embedded in the project for convenience as there's no Debian package for it yet.

