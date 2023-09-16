/* cpu.c (ejo@ufo)  7 Sep 23  Modified:  7 Sep 23  16:10 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <libgen.h>

#include "constants.h"
#include "cpu.h"
#include "microcode.h"
#include "io.h"

#define OPTSTR "dt:"
extern char *optarg;
extern int opterr;


cpu_t cpu;

void fetch(cpu_t *cpu);
void execute(cpu_t *cpu);
int  machine_check(cpu_t *cpu);

void m_load(word_t address, word_t *destination);
void m_store(word_t address, word_t value);

void dump_cpu(FILE *fp, cpu_t *cpu, char *prefix);


int main(int argc, char *argv[])
{
  int      opt;
  int      timer_fired;
  int      running;

  cpu.r_ir     = 0;
  cpu.r_pc     = USER_PROGRAM_LOAD;
  cpu.r_sp     = USTACK_BASE;
  cpu.r_ac     = 0;
  cpu.r_x      = 0;
  cpu.r_y      = 0;
  cpu.r_cycles = 0;
  
  cpu.timer_interval = 0;
  cpu.c_mode       = USER_MODE;
  cpu.c_interrupts = 1;
  cpu.c_debug      = 0;
  cpu.c_fault      = 0;
  cpu.c_stop       = 0;
  cpu.c_step       = 0;
  
  cpu.registers.error.value = 0;
  
  cpu.load = m_load;
  cpu.store = m_store;

  while((opt = getopt(argc, argv, OPTSTR)) != EOF)
    switch(opt) {
      case 't':
	cpu.timer_interval = atoi(optarg);
	break;
      case 'd':
	cpu.c_debug = 1;
	break;
      case '?':
      case 'h':
	fprintf(stderr,"Usage: %s [-d] [-t timer_interval]\n", basename(argv[0]));
	exit(EXIT_FAILURE);
	/* NOTREACHED */
	break;
    }

  if (cpu.c_debug)
    fprintf(CONSOLE, "[ CPU] Initializing microcode.\n");

  cpu.instruction_decode = alloc_dispatch_table();

  for(int i=0; i<NUM_OPCODES; i++) 
    cpu.instruction_decode[i].microcode = (gmicrocode_f )microcode_table[i];

  cpu.instruction = cpu.instruction_decode[0];

  if (cpu.c_debug) {
    fprintf(CONSOLE, "[ CPU] Start. Timer %d Debug %d\n",
	    cpu.timer_interval, cpu.c_debug);
    dump_cpu(CONSOLE, &cpu, "START");
  }

  do {
    
    if (TAKE_TIMER_INTERRUPT(&cpu))
      timer_interrupt(&cpu);

    if (machine_check(&cpu))
      break;

    fetch(&cpu);

    if (machine_check(&cpu))
      break;

    execute(&cpu);

    if (machine_check(&cpu))
      break;
    
  } while(!cpu.c_stop);

  dump_cpu(CONSOLE, &cpu, "FINAL");

  if (cpu.c_debug)
    fprintf(CONSOLE, "[ CPU] End. Executed %d instructions.\n",
	    cpu.r_cycles);
  
    
  return EXIT_SUCCESS;
}

void fetch(cpu_t *cpu)
{
  
  cpu->load(cpu->r_pc, &cpu->r_ir);

  if (cpu->c_fault)
    return;

  cpu->instruction = cpu->instruction_decode[INVALID_OPCODE];

  if (!(VALID_OPCODE(cpu->r_ir)) ) {
    cpu->c_fault = 1;
    cpu->e_fetch = 1;
    cpu->e_range = 1;
    return ;
  }

  cpu->instruction = cpu->instruction_decode[cpu->r_ir];
  cpu->instruction.address = cpu->r_pc;

  if (cpu->instruction.has_operand)
    cpu->load(++cpu->r_pc, &cpu->instruction.operand);
    
  //dump_cpu(CONSOLE, cpu, "FETCH");
  
  return;
}


void execute(cpu_t *cpu)
{
  if (!VALID_OPCODE(cpu->r_ir)) {
    cpu->c_fault = 1;
    cpu->e_execute = 1;
    return ;
  }

  cpu->instruction.microcode(cpu);
  
  if (NOT_CTI(cpu->r_ir))
    cpu->r_pc ++;
  
  cpu->r_cycles++;

  dump_cpu(CONSOLE, cpu, "EXECUTE");
  
  return;
}


int machine_check(cpu_t *cpu)
{
  if (!cpu->c_fault)
    return 0;
  
  cpu->c_debug = 1;
  dump_cpu(CONSOLE, cpu, "CHECK");
  return 1;
}


void m_load(word_t address, word_t *dst)
{
  io_t io = {address, 0, 0, IO_RD, 0};
  
  if (ADDRESS_FAULT(address, cpu.c_mode)) {
    cpu.c_fault = 1;
    cpu.e_load = 1;
    cpu.e_perm = 1;
    return ;
  }

  if (write(MEM_WR_CHANNEL, &io, sizeof(io)) != sizeof(io)) {
    cpu.c_fault = 1;
    cpu.e_load = 1;
    return ;
  }

  if (read(MEM_RD_CHANNEL, &io, sizeof(io)) != sizeof(io)) {
    cpu.c_fault = 1;
    cpu.e_load = 1;
    return ;
  }

  if (io.error) {
    cpu.c_fault = 1;
    cpu.e_load = 1;
    return;
  }

  *dst = io.value;
}


void m_store(word_t address, word_t value)
{
  io_t io = {address, value, 0, IO_WR, 0};
  
  if (ADDRESS_FAULT(address, cpu.c_mode)) {
    cpu.c_fault = 1;
    cpu.e_store = 1;
    cpu.e_perm = 1;
    return ;
  }

  if (write(MEM_WR_CHANNEL, &io, sizeof(io)) != sizeof(io)) {
    /* error */
    cpu.c_fault = 1;
    cpu.e_store = 1;
    return ;
  }

  if (read(MEM_RD_CHANNEL, &io, sizeof(io)) != sizeof(io)) {
    /* error */
    cpu.c_fault = 1;
    cpu.e_store = 1;
    return ;
  }

  if (io.error) {
    cpu.c_fault = 1;
    cpu.e_store = 1;
  }
  
  return ;
}

#define HR "--[%-7s]--------------------------------------------------\n"

void dump_cpu(FILE *fp, cpu_t *cpu, char *prefix)
{
  word_t stack_base;

  if (!cpu->c_debug)
    return ;

  fprintf(fp, HR, prefix?prefix:"????");

  fputs("[ CPU] [in] ", fp);
  iprintf(fp, &cpu->instruction);
  
  fprintf(fp, "[ CPU] [pc] %08d [ir] %08d [sp] %08d [mode] %s\n",
	  cpu->r_pc, cpu->r_ir, cpu->r_sp, cpu->c_mode?" sys":"user");
  fprintf(fp, "[ CPU] [ac] %08d [ x] %08d [ y] %08d [debg] %s\n",
	  cpu->r_ac, cpu->r_x, cpu->r_y, cpu->c_debug?"on":"off");
  fprintf(fp, "[ CPU] [cy] %08d [ti] %08d [ft] %8s [intr] %s\n",
	  cpu->r_cycles,
	  cpu->timer_interval,
	  cpu->c_fault?"true":"false",
	  cpu->c_interrupts?"enable":"disable");
  
  if (cpu->r_control) {
    fprintf(fp, "[ CPU] [cr] %08x mode:%d intr:%d debug:%d fault:%d stop:%d step:%d\n",
	    cpu->r_control,
	    cpu->c_mode, cpu->c_interrupts,
	    cpu->c_debug, cpu->c_fault,
	    cpu->c_stop, cpu->c_step);
  }

  if (cpu->r_error) {
    fprintf(fp, "[ CPU] [er] %08x fetch:%d decode:%d execute:%d load:%d store:%d range:%d perm:%d underflow:%d overflow:%d invalid:%d\n",
	    cpu->r_error,
	    cpu->e_fetch,
	    cpu->e_decode,
	    cpu->e_execute,
	    cpu->e_load,
	    cpu->e_store,
	    cpu->e_range,
	    cpu->e_perm,
	    cpu->e_underflow,
	    cpu->e_overflow,
	    cpu->e_invalid);
  }

  stack_base = (cpu->c_mode)?SSTACK_BASE:USTACK_BASE;

  for (int address=cpu->r_sp+1; address <= stack_base; address++) {
    word_t word;
    cpu->load(address, &word);
    fprintf(CONSOLE, "[ CPU] [stack %c] sp=%08d:%08d\n",
	    cpu->c_mode?'S':'U', address, word);
  }
}
