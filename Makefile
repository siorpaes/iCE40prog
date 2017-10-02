ICE40PROG = ice40prog
ICE40PROGOBJS = ice40prog.o ./mpsse/mpsse.c ./mpsse/support.c ./mpsse/fast.c

CFLAGS = -Wall -I./mpsse
LDLIBS = -lftdi

all: $(ICE40PROG)

$(ICE40PROG): $(ICE40PROGOBJS)

clean:
	rm -f $(ICE40PROG) *.o *~


