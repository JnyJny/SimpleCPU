
TARGETS = cpu memory driver

SRC = $(TARGETS:=.c)
OBJ = $(SRC:.c=.o)

CFLAGS=
LFLAGS= 

all: $(TARGETS)

.c.o: $(OBJ)
	cc -c $< $(CFLAGS)

cpu: cpu.o
	cc -o $@ $< $(LFLAGS)

memory: memory.o
	cc -o $@ $< $(LFLAGS)

driver: driver.o
	cc -o $@ $< $(LFLAGS)

clean:
	rm -f $(OBJ)

immaculate: clean
	rm -f $(TARGETS)
