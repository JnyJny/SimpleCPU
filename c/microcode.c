/* instructions.c (ejo@ufo) 10 Sep 23  Modified: 10 Sep 23  09:34 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include "constants.h"
#include "microcode.h"
#include "instruction.h"
#include "io.h"

microcode_f microcode[NUM_OPCODES] = {
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

#define STATE_P(P) ((state_t *)(P))

void invalid_instruction(void *state)
{
  fprintf(CONSOLE,"Invalid Instruction: [pc] %08d [ir] %08d\n",
	  STATE_P(state)->pc, STATE_P(state)->ir);
}


void load_value(void *state)
{
  /* Load value: Load the value into AC */
  
  // STATE_P(state)->ac = STATE_P(state)->read_memory(++STATE_P(state)->pc);

  STATE_P(state)->ac = STATE_P(state)->instruction.operand;
}

void load_addr(void *state)
{
  /* Load addr: Load the value at address into AC */

  STATE_P(state)->ac = STATE_P(state)->read_memory(STATE_P(state)->instruction.operand);
}

void load_indirect(void *state)
{
  /* LoadInd addr: Load value from address at address */
  int addr;

  addr = STATE_P(state)->read_memory(STATE_P(state)->instruction.operand);
  STATE_P(state)->ac = STATE_P(state)->read_memory(addr);
}

void load_index_x(void *state)
{
  /* LoadIdxX addr: Load value (addr+X) into AC */
  int addr;

  addr = STATE_P(state)->instruction.operand;
  STATE_P(state)->ac = STATE_P(state)->read_memory(addr + STATE_P(state)->x);
}

void load_index_y(void *state)
{
  /* LoadIdxY addr: Load value (addr+Y) into AC */
  int addr;
  
  addr = STATE_P(state)->instruction.operand;
  STATE_P(state)->ac = STATE_P(state)->read_memory(addr + STATE_P(state)->y);
}

void load_sp_x(void *state)
{
  /* LoadSpX: Load from (Sp+X) info AC */
  int addr;

  addr = STATE_P(state)->sp + STATE_P(state)->x;
  
  STATE_P(state)->ac = STATE_P(state)->read_memory(addr);
}

void store_addr(void *state)
{
  /* Store addr: Store AC into address */
  int addr;


  STATE_P(state)->write_memory(STATE_P(state)->instruction.operand, STATE_P(state)->ac);
}

void get(void *state)
{
  /* Get: random into from 1 to 100 into AC */
  
  STATE_P(state)->ac = 1 + (rand() % 100);
}

void put_port(void *state)
{
  /* Put port: port1, write AC as int, port2 write AC as char */
  int port;
  
  port = STATE_P(state)->instruction.operand;

  if (STATE_P(state)->debug) {
    fprintf(CONSOLE, "[COUT] port=%d AC=%d\n",
	    port,
	    STATE_P(state)->ac);
    return;
  }
  
  switch(port) {
    case 1:
      fprintf(CONSOLE, "%d", STATE_P(state)->ac);
      break;
    case 2:
      fprintf(CONSOLE, "%c", STATE_P(state)->ac);
      break;
    default:
      break;
  }
}

void add_x(void *state)
{
  /* AddX: add X to AC */
  
  STATE_P(state)->ac += STATE_P(state)->x;
}

void add_y(void *state)
{
  /* AddY: add Y to AC */
  
  STATE_P(state)->ac += STATE_P(state)->y;
}

void sub_x(void *state)
{
  /* SubX: subtract X from AC */
  
  STATE_P(state)->ac -= STATE_P(state)->x;
}

void sub_y(void *state)
{
  /* SubY: subtract Y from AC */
  
  STATE_P(state)->ac -= STATE_P(state)->y;
}

void copy_to_x(void *state)
{
  /* CopyToX: Copy AC to X */
  
  STATE_P(state)->x = STATE_P(state)->ac;
}

void copy_from_x(void *state)
{
  /* CopyFromX: Copy X to AC */
  
  STATE_P(state)->ac = STATE_P(state)->x;
}

void copy_to_y(void *state)
{
  /* CopyToY: Copy AC to Y */
  
  STATE_P(state)->y = STATE_P(state)->ac;
}

void copy_from_y(void *state)
{
  /* CopyFromY: Copy Y to AC */
  
  STATE_P(state)->ac = STATE_P(state)->y;
}

void copy_to_sp(void *state)
{
  /* CopyToSP: Copy AC to SP */
  
  STATE_P(state)->sp = STATE_P(state)->ac;
}

void copy_from_sp(void *state)
{
  /* CopyFromSP: Copy SP to AC */
  
  STATE_P(state)->ac = STATE_P(state)->sp;
}

void jump_addr(void *state)
{
  /* Jump Addr: Jump to the address */

  STATE_P(state)->pc = STATE_P(state)->instruction.operand;
}

void jump_eq_addr(void *state)
{
  /* JumpIfEqual addr: jump to addr if AC is zero */
  
  if (STATE_P(state)->ac == 0)
    STATE_P(state)->pc = STATE_P(state)->instruction.operand;
  else
    STATE_P(state)->pc++;
}

void jump_ne_addr(void *state)
{
  /* JumpIfNotEqual addr: Jump to addr if AC is not zero */

  if (STATE_P(state)->ac != 0)
    STATE_P(state)->pc = STATE_P(state)->instruction.operand;
  else
    STATE_P(state)->pc++;
}

void call_addr(void *state)
{
  /* Call addr: Push return address onto stack, jump to addr */
  STATE_P(state)->write_memory(STATE_P(state)->sp--, STATE_P(state)->pc+1);
  STATE_P(state)->pc = STATE_P(state)->instruction.operand;
}

void ret(void *state)
{
  /* Ret: Pop return address from stack, jump to address */

  STATE_P(state)->pc = STATE_P(state)->read_memory(++STATE_P(state)->sp);
}

void inc_x(void *state)
{
  /* IncX: increment the value in X */
  STATE_P(state)->x++;
}

void dec_x(void *state)
{
  /* DecX: decrement the value in X */
  STATE_P(state)->x--;
}

void push(void *state)
{
  /* Push: push AC onto Stack */

  /* TODO  check for stack overflow */
  STATE_P(state)->write_memory(STATE_P(state)->sp--, STATE_P(state)->ac);
}

void pop(void *state)
{
  /* Pop: pop stack into AC */

  /* TODO  check for stack underflow */
  STATE_P(state)->ac = STATE_P(state)->read_memory(STATE_P(state)->sp++);
}

void interrupt(void *state)
{
  /* Int: perform system call */
  int sp;
  
  if (!STATE_P(state)->interrupts)
    return;
  
  STATE_P(state)->mode = INTERRUPT_MODE;
  STATE_P(state)->interrupts = 0;

  sp = STATE_P(state)->sp;
  STATE_P(state)->sp = SSTACK_BASE;
  STATE_P(state)->write_memory(STATE_P(state)->sp--, sp);
  STATE_P(state)->write_memory(STATE_P(state)->sp--, STATE_P(state)->pc);
  STATE_P(state)->pc = INTERRUPT_PROGRAM_LOAD;
}

void timer_interrupt(state_t *state)
{
  int usp;
  int upc;
  
  if (!STATE_P(state)->interrupts)
    return ;
  
  STATE_P(state)->mode = TIMER_MODE;
  STATE_P(state)->interrupts = 0;

  usp = STATE_P(state)->sp;
  upc = STATE_P(state)->pc;
    
  STATE_P(state)->sp = SSTACK_BASE;
  STATE_P(state)->write_memory(STATE_P(state)->sp--, usp);
  STATE_P(state)->write_memory(STATE_P(state)->sp--, upc);
  STATE_P(state)->pc = TIMER_PROGRAM_LOAD;
}

void iret(void *state)
{
  /* IRet: return from system call */

  
  /* pop PC and SP from system stack */
  STATE_P(state)->pc = STATE_P(state)->read_memory(++STATE_P(state)->sp);
  STATE_P(state)->sp = STATE_P(state)->read_memory(++STATE_P(state)->sp);
  /* enable interrupts and return to user mode */
  STATE_P(state)->interrupts = 1;
  STATE_P(state)->mode = USER_MODE;
}

void end(void *state)
{
  /* End executation */
}


