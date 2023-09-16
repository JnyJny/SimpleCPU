/* microcode.c (ejo@ufo) 10 Sep 23  Modified: 10 Sep 23  09:34 */

#define MICROCODE_C

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include "constants.h"
#include "microcode.h"

microcode_f microcode_table[NUM_OPCODES] = {
  invalid_instruction,
  load_value,	       
  load_addr,	       
  load_indirect,      
  load_index_x,       
  load_index_y,       
  load_sp_x,	       
  store_addr,	       
  get,		       
  put_port,	       
  add_x,	       
  add_y,	       
  sub_x,	       
  sub_y,	       
  copy_to_x,	       
  copy_from_x,	       
  copy_to_y,	       
  copy_from_y,	       
  copy_to_sp,	       
  copy_from_sp,       
  jump_addr,	       
  jump_eq_addr,       
  jump_ne_addr,       
  call_addr,	       
  ret,		       
  inc_x,	       
  dec_x,	       
  push,	       
  pop,		       
  interrupt,	       
  iret,	       
  invalid_instruction,
  invalid_instruction,
  invalid_instruction,
  invalid_instruction,
  invalid_instruction,
  invalid_instruction,
  invalid_instruction,
  invalid_instruction,
  invalid_instruction,
  invalid_instruction,
  invalid_instruction,
  invalid_instruction,
  invalid_instruction,
  invalid_instruction,
  invalid_instruction,
  invalid_instruction,		       
  invalid_instruction,		       
  invalid_instruction,  
  invalid_instruction,
  end
};

/* Instruction Implementation
 */



void invalid_instruction(cpu_t *cpu)
{
  cpu->c_fault = 1;
  cpu->e_execute = 1;
  cpu->e_invalid = 1;
}


void load_value(cpu_t *cpu)
{
  /* Load value: Load the value into AC */

  cpu->r_ac = cpu->instruction.operand;
}

void load_addr(cpu_t *cpu)
{
  /* Load addr: Load the value at address into AC */

  cpu->load(cpu->instruction.operand, &cpu->r_ac);
}

void load_indirect(cpu_t *cpu)
{
  /* LoadInd addr: Load value from address at address */
  word_t addr;

  cpu->load(cpu->instruction.operand, &addr);
  cpu->load(addr, &cpu->r_ac);
}

void load_index_x(cpu_t *cpu)
{
  /* LoadIdxX addr: Load value (addr+X) into AC */
  word_t addr;

  cpu->load(cpu->instruction.operand + cpu->r_x, &cpu->r_ac);
}

void load_index_y(cpu_t *cpu)
{
  /* LoadIdxY addr: Load value (addr+Y) into AC */
  
  cpu->load(cpu->instruction.operand + cpu->r_y, &cpu->r_ac);
}

void load_sp_x(cpu_t *cpu)
{
  /* LoadSpX: Load from (SP+X) info AC */
  
  cpu->load(cpu->r_sp + cpu->r_x, &cpu->r_ac);
}

void store_addr(cpu_t *cpu)
{
  /* Store addr: Store AC into address */

  cpu->store(cpu->instruction.operand, cpu->r_ac);
}

void get(cpu_t *cpu)
{
  /* Get: random int from 1 to 100 into AC */
  
  cpu->r_ac = 1 + (rand() % 100);
}

void put_port(cpu_t *cpu)
{
  /* Put port: port1, write AC as int, port2 write AC as char */

  if (cpu->c_debug) {
    fprintf(CONSOLE, "[PUT] port=%d AC=%d\n",
	    cpu->instruction.operand,
	    cpu->r_ac);
    return;
  }
  
  switch(cpu->instruction.operand) {
    case 1:
      fprintf(CONSOLE, "%d", cpu->r_ac);
      break;
    case 2:
      fprintf(CONSOLE, "%c", cpu->r_ac);
      break;
    default:
      cpu->c_fault = 1;
      cpu->e_execute = 1;
      cpu->e_range = 1;
      break;
  }
}

void add_x(cpu_t *cpu)
{
  /* AddX: add X to AC */
  
  cpu->r_ac += cpu->r_x;
}

void add_y(cpu_t *cpu)
{
  /* AddY: add Y to AC */
  
  cpu->r_ac += cpu->r_y;
}

void sub_x(cpu_t *cpu)
{
  /* SubX: subtract X from AC */
  
  cpu->r_ac -= cpu->r_x;
}

void sub_y(cpu_t *cpu)
{
  /* SubY: subtract Y from AC */
  
  cpu->r_ac -= cpu->r_y;
}

void copy_to_x(cpu_t *cpu)
{
  /* CopyToX: Copy AC to X */
  
  cpu->r_x = cpu->r_ac;
}

void copy_from_x(cpu_t *cpu)
{
  /* CopyFromX: Copy X to AC */
  
  cpu->r_ac = cpu->r_x;
}

void copy_to_y(cpu_t *cpu)
{
  /* CopyToY: Copy AC to Y */
  
  cpu->r_y = cpu->r_ac;
}

void copy_from_y(cpu_t *cpu)
{
  /* CopyFromY: Copy Y to AC */
  
  cpu->r_ac = cpu->r_y;
}

void copy_to_sp(cpu_t *cpu)
{
  /* CopyToSP: Copy AC to SP */
  
  cpu->r_sp = cpu->r_ac;
}

void copy_from_sp(cpu_t *cpu)
{
  /* CopyFromSP: Copy SP to AC */
  
  cpu->r_ac = cpu->r_sp;
}

void jump_addr(cpu_t *cpu)
{
  /* Jump Addr: Jump to the address */

  cpu->r_pc = cpu->instruction.operand;
}

void jump_eq_addr(cpu_t *cpu)
{
  /* JumpIfEqual addr: jump to addr if AC is zero */
  
  if (cpu->r_ac == 0)
    cpu->r_pc = cpu->instruction.operand;
  else
    cpu->r_pc++;		/* advance PC past operand */
}

void jump_ne_addr(cpu_t *cpu)
{
  /* JumpIfNotEqual addr: Jump to addr if AC is not zero */

  if (cpu->r_ac != 0)
    cpu->r_pc = cpu->instruction.operand;
  else
    cpu->r_pc++;		/* advance PC past operand */
}

void call_addr(cpu_t *cpu)
{
  /* Call addr: Push return address onto stack, jump to addr */
  
  cpu->store(cpu->r_sp, cpu->r_pc+1);
  cpu->r_sp--;
  cpu->r_pc = cpu->instruction.operand;
}

void ret(cpu_t *cpu)
{
  /* Ret: Pop return address from stack, jump to address */

  cpu->r_sp++;
  cpu->load(cpu->r_sp, &cpu->r_pc);
  // TODO check for stack  [under|over] flow
}

void inc_x(cpu_t *cpu)
{
  /* IncX: increment the value in X */
  cpu->r_x++;
}

void dec_x(cpu_t *cpu)
{
  /* DecX: decrement the value in X */
  cpu->r_x--;
}

void push(cpu_t *cpu)
{
  /* Push: push AC onto Stack */

  cpu->store(cpu->r_sp--, cpu->r_ac);
  // TODO check for stack  [under|over] flow
}

void pop(cpu_t *cpu)
{
  /* Pop: pop stack into AC */

  cpu->load(++cpu->r_sp, &cpu->r_ac);
  // TODO check for stack  [under|over] flow
}

void interrupt(cpu_t *cpu)
{
  /* Int: perform system call
   * Switch to system mode and stack
   * Push SP and PC onto system stack
   * Disable interrupts
   */
  
  word_t u_sp;
  word_t u_pc;
  
  if (!cpu->c_interrupts)
    return;
  
  cpu->c_mode = SYSTEM_MODE;
  cpu->c_interrupts = 0;

  u_sp = cpu->r_sp;
  u_pc = cpu->r_pc;
  
  cpu->r_sp = SSTACK_BASE;
  cpu->store(cpu->r_sp--, u_sp);
  cpu->store(cpu->r_sp--, u_pc + 1);
  
  cpu->r_pc = INTERRUPT_PROGRAM_LOAD;

  // TODO check for stack  [under|over] flow
}

void timer_interrupt(cpu_t *cpu)
{
  /* Timer Interrupt
   * Push PC and SP onto system stack
   * Disable interrupts
   * Switch to system mode
   * Swithc to system stack
   */
  word_t u_sp;
  word_t u_pc;
  
  if (!cpu->c_interrupts)
    return ;
  
  cpu->c_mode = SYSTEM_MODE;
  cpu->c_interrupts = 0;

  u_sp = cpu->r_sp;
  u_pc = cpu->r_pc;
    
  cpu->r_sp = SSTACK_BASE;
  cpu->store(cpu->r_sp--, u_sp);
  cpu->store(cpu->r_sp--, u_pc);
  
  cpu->r_pc = TIMER_PROGRAM_LOAD;

  // TODO check for stack  [under|over] flow
}

void iret(cpu_t *cpu)
{
  /* IRet: return from system call
   * Pop PC and SP from system stack
   */
  
  /* pop PC and SP from system stack */
  cpu->load(++cpu->r_sp, &cpu->r_pc);
  cpu->load(++cpu->r_sp, &cpu->r_sp);
  
  /* enable interrupts and return to user mode */
  cpu->c_interrupts = 1;
  cpu->c_mode = USER_MODE;

  // TODO check for stack  [under|over] flow
}

void end(cpu_t *cpu)
{
  /* End executation */
  cpu->c_stop = 1;
}


