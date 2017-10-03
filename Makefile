ICE40PROG = ice40prog
ICE40PROGOBJS = ice40prog.o ./mpsse/mpsse.o ./mpsse/support.o ./mpsse/fast.o

CFLAGS = -Wall -I./mpsse -I/usr/include/libftdi -DLIBFTDI1=1
LDLIBS = -lftdi1

all: $(ICE40PROG)

$(ICE40PROG): $(ICE40PROGOBJS)

clean:
	rm -f $(ICE40PROG) *.o ./mpsse/*.o *~


