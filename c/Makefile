
TARGETS = cpu memory loader

SRC = cpu.c memory.c io.c instructions.c loader.c dis.c
OBJ = $(SRC:.c=.o)
HDR = $(SRC:.c=.h)

CFLAGS=
LFLAGS= 

all: $(TARGETS)

.c.o: $(SRC) $(HDR)
	cc -c $< $(CFLAGS) $(DEBUG)

CPU_OBJS= io.o instructions.o cpu.o
CPU_HDRS= $(CPU_OBJS:.o=.h)

cpu: $(CPU_OBJS) $(CPH_HDRS)
	cc -o $@ $(CPU_OBJS) $(LFLAGS)

memory: memory.o io.h
	cc -o $@ $< $(LFLAGS)

loader: loader.o io.o dis.o dis.h io.h 
	cc -o $@ $< io.o dis.o $(LFLAGS)

clean:
	rm -f $(OBJ) trace.memory

immaculate: clean
	rm -f $(TARGETS)
