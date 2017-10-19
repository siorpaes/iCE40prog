ICE40PROG = ice40prog
ICE40PROGOBJS = ice40prog.o ./mpsse/mpsse.o ./mpsse/support.o ./mpsse/fast.o

ICE40PROGBB = ice40progbb
ICE40PROBBGOBJS = ice40progbb.o


CFLAGS = -Wall -I./mpsse -I/usr/include/libftdi1 -DLIBFTDI1=1
LDLIBS = -lftdi1


all: $(ICE40PROG) $(ICE40PROGBB)

$(ICE40PROG): $(ICE40PROGOBJS)

$(ICE40PROGBB): $(ICE40PROGOBJSBB)

clean:
	rm -f $(ICE40PROG) $(ICE40PROGBB) *.o ./mpsse/*.o *~
