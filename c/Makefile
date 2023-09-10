
TARGETS = cpu memory loader

SRC = cpu.c memory.c io.c instructions.c loader.c
OBJ = $(SRC:.c=.o)

CFLAGS=
LFLAGS= 

all: $(TARGETS)

.c.o:
	cc -c $< $(CFLAGS)

cpu: cpu.o io.o instructions.o
	cc -o $@ $^ $(LFLAGS)

memory: memory.o
	cc -o $@ $< $(LFLAGS)

loader: loader.o
	cc -o $@ $< $(LFLAGS)

clean:
	rm -f $(OBJ)

immaculate: clean
	rm -f $(TARGETS)
