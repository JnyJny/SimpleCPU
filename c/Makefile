
TARGETS = cpu memory loader asm dis

SRC = cpu.c io.c instruction.c memory.c microcode.c object.c
OBJ = $(SRC:.c=.o)
HDR = $(SRC:.c=.h) constants.h

CFLAGS=
LFLAGS= 

all: $(TARGETS)

.c.o: $(SRC) $(HDR)
	cc -c $< $(CFLAGS) $(DEBUG)

CPU_OBJS= cpu.o instruction.o microcode.o
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



TEST_SRC = tests/program1.s tests/sample1.s tests/sample2.s tests/sample3.s tests/sample4.s
TEST_OBJ = $(TEST_SRC:.s=.o)

tests: $(TEST_PROGRAMS)

.s.o: asm $(TEST_PROGRAMS)
	./asm -o $@ -O $<

clean:
	rm -f $(OBJ) trace.memory *.o

immaculate: clean
	rm -f $(TARGETS)
