
TARGETS = cpu memory loader asm dis

SRC = cpu.c instruction.c memory.c io.c microcode.c object.c
OBJ = $(SRC:.c=.o)
HDR = $(SRC:.c=.h) constants.h

CFLAGS=
LFLAGS= 

all: $(TARGETS)

.c.o: $(SRC) $(HDR)
	cc -c $< $(CFLAGS) $(DEBUG)

CPU_OBJS= cpu.o io.o instruction.o microcode.o
CPU_HDRS= $(CPU_OBJS:.o=.h)

LDR_OBJS= loader.o io.o object.o
LDR_HDRS= $(LDR_OBJS:.o=.h)

DIS_OBJS= dis.o instruction.o object.o
DIS_HDRS= $(DIS_OBJS:.o=.h)

ASM_OBJS= asm.o instruction.o object.o
ASM_HDRS= $(ASM_OBJS:.o=.h)

cpu: $(CPU_OBJS) $(CPH_HDRS)
	cc -o $@ $(CPU_OBJS) $(LFLAGS)

memory: memory.o io.h
	cc -o $@ $< $(LFLAGS)

loader: $(LDR_OBJS) $(LDR_HDRS)
	cc -o $@ $(LDR_OBJS)  $(LFLAGS)

asm: asm.c $(ASM_OBJS) $(ASM_HDRS)
	cc -o $@ $(ASM_OBJS) $(LFLAGS)

dis: dis.c $(DIS_OBJS) $(DIS_HDRS)
	cc -o $@ $(DIS_OBJS) $(LFLAGS)

clean:
	rm -f $(OBJ) trace.memory

immaculate: clean
	rm -f $(TARGETS)
