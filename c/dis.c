/* dis.c (ejo@ufo) 11 Sep 23  Modified: 11 Sep 23  17:32 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include "io.h"
#include "dis.h"
#include "instructions.h"
#include "constants.h"

typedef struct {
  int opcode;
  char *mnemonic;
  int has_operand;
  char *description;
} instruction_t;


instruction_t opcode_table[51] = {
  { 0, "Invalid",    0, "Invalid opcode."},
  { 1, "LoadV",      1, "LoadV value: load the value into AC"},
  { 2, "LoadA",      1, "LoadA address: load the value at address into AC"},
  { 3, "LoadI",      1, "LoadI address: load the value from the address found at address into AC"},
  { 4, "LoadX",      1, "LoadX address: load the value at (address+X) into AC"},
  { 5, "LoadY",      1, "LoadY address: load the value at (address+Y) into AC"},
  { 6, "LoadSPX",    0, "LoadSPX: load the value at (SP+X) into AC"},
  { 7, "Store",      1, "Store address: store AC into address"},
  { 8, "Get",        0, "Get: Load a random int from 1 to 100 into AC"},
  { 9, "Put",        1, "Put port: write AC to screen. port 1=int, port 2=char"},
  {10, "AddX",       0, "AddX: add the value in X to AC"},
  {11, "AddY",       0, "AddY: add the value in Y to AC"},
  {12, "SubX",       0, "SubX: subtract the value in X from AC"},
  {13, "SubY",       0, "SubY: subtract the value in Y from AC"},
  {14, "CopyToX",    0, "CopyToX: copy the value in AC to X"},
  {15, "CopyFromX",  0, "CopyFromX: copy the value in X to AC"},
  {16, "CopyToY",    0, "CopyToY: copy the value in AC to Y"},
  {17, "CopyFromY",  0, "CopyFromY: copy the value in Y to AC"},
  {18, "CopyToSP",   0, "CopyToSP: copy the value in AC to SP"},
  {19, "CopyFromSP", 0, "CopyFromSP: copy the value in SP to AC"},
  {20, "Jump",       1, "Jump address: jump to address"},
  {21, "JumpEQ",     1, "JumpEQ address: jump to address if AC is zero"},
  {22, "JumpNE",     1, "JumpEQ address: jump to address if AC is not zero"},
  {23, "Call",       1, "Call address: push return address onto stack, jump to the address."},
  {24, "Return",     0, "Return: pop return address from stack and jump to address."},
  {25, "IncX",       0, "IncX: Increment the value in X"},
  {26, "DecX",       0, "DecX: Decrement the value in X"},
  {27, "Push",       0, "Push: push AC onto stack"},
  {28, "Pop",        0, "Pop: pop from stack into AC"},
  {29, "Interrupt",  0, "Interrupt: perform a system call"},
  {30, "IReturn",    0, "IReturn: return from system call"},
  { 0, "Invalid",    0, "Invalid opcode."},  
  { 0, "Invalid",    0, "Invalid opcode."},  
  { 0, "Invalid",    0, "Invalid opcode."},
  { 0, "Invalid",    0, "Invalid opcode."},
  { 0, "Invalid",    0, "Invalid opcode."},
  { 0, "Invalid",    0, "Invalid opcode."},  
  { 0, "Invalid",    0, "Invalid opcode."},  
  { 0, "Invalid",    0, "Invalid opcode."},
  { 0, "Invalid",    0, "Invalid opcode."},
  { 0, "Invalid",    0, "Invalid opcode."},
  { 0, "Invalid",    0, "Invalid opcode."},  
  { 0, "Invalid",    0, "Invalid opcode."},  
  { 0, "Invalid",    0, "Invalid opcode."},
  { 0, "Invalid",    0, "Invalid opcode."},
  { 0, "Invalid",    0, "Invalid opcode."},
  { 0, "Invalid",    0, "Invalid opcode."},  
  { 0, "Invalid",    0, "Invalid opcode."},  
  { 0, "Invalid",    0, "Invalid opcode."},
  { 0, "Invalid",    0, "Invalid opcode."},
  {50, "End",        0, "End: end execution"},
};


instruction_t decode(int address)
{
  instruction_t instruction;
  int           opcode;
  
  opcode = read_memory(address);

  if (!VALID_INSTRUCTION(opcode)) {
    return opcode_table[0];
  }

  instruction = opcode_table[opcode];
    
  return instruction;
  
}

void disassemble_memory(FILE *fp, region_t region)
{
  int           address;
  int           operand;
  int           base;
  int           final;
  instruction_t instruction;
  char          r;

  switch(region) {
    case USER_REGION:		/* USER */
      base = USER_PROGRAM_LOAD;
      final = TIMER_PROGRAM_LOAD;
      break;
    case TIMER_REGION:		/* TIMER */
      base = TIMER_PROGRAM_LOAD;
      final = INTERRUPT_PROGRAM_LOAD;
      break;
    case INTERRUPT_REGION:	/* INTERRUPT */
      base = INTERRUPT_PROGRAM_LOAD;
      final = SSTACK_BASE;
      break;
    case ALL_REGIONS:
      base = USER_PROGRAM_LOAD;
      final = SSTACK_BASE;
      break;
  }
  

  for(address = base; address < final; ) {

    if (address < TIMER_PROGRAM_LOAD) {
      r = 'U';
    }
    else
      r = (address < INTERRUPT_PROGRAM_LOAD)?'T':'I';
	
    
    instruction = decode(address);

    if (instruction.opcode == INVALID) {
      address ++;
      continue;
    }

    switch (instruction.has_operand) {
      case 1:
	operand = read_memory(address+1);
	fprintf(CONSOLE, "[ DIS] %c [%04d] %16s %8d // %s\n",
		r, address, instruction.mnemonic, operand, instruction.description);
	address += 2;
	break;
      case 0:
	fprintf(CONSOLE, "[ DIS] %c [%04d] %16s %8s // %s\n",
		r, address, instruction.mnemonic, "", instruction.description);
	address ++;
	break;
    }
  }
}
