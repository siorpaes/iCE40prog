# iCE40prog
Very essential tool for iCE40 FPGAs bitstream programming using
bare bones FTDI C232HM MPSSE cable

Tested with UPDuino board 
http://gnarlygrey.atspace.cc/development-platform.html#upduino

Pinout

|Signal   | FTDI cable color |
|---------|:----------------:|
| CLK     |  Orange          |
| MOSI    |  Yellow          |
| CS      |  White           |
| RESE    |  Blue            |


Prerequisites
libftdi
apt-get install libftdi-dev

libmpsse essential sources are embedded in the project for 
convenience as there's no Debian package for it yet.

