/* instruction.c (ejo@ufo) 13 Sep 23  Modified: 13 Sep 23  09:46 */

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

#include "constants.h"
#include "instruction.h"




instruction_t init_instruction(int opcode, int has_op, char *mnemonic, char *description)
{
  instruction_t i = { opcode, has_op, 0, 0, (void *)NULL, 0, 0, mnemonic, description };

  return i;
}

instruction_t *alloc_dispatch_table(void)
{
  instruction_t *tbl;

  if (!(tbl = calloc(NUM_OPCODES, sizeof(instruction_t)))) {
    perror("calloc:dispatch_table");
    exit(EXIT_FAILURE);
  }

  tbl[0]  = init_instruction(INVALID_OPCODE,    0, "Invalid",    "Invalid opcode.");
  tbl[1]  = init_instruction(LOADV_OPCODE,      1, "LoadV",      "LoadV value: load the value into AC");
  tbl[2]  = init_instruction(LOADA_OPCODE,      1, "LoadA",      "LoadA address: load the value at address into AC");
  tbl[3]  = init_instruction(LOADI_OPCODE,      1, "LoadI",      "LoadI address: load the value from the address found at address into AC");
  tbl[4]  = init_instruction(LOADX_OPCODE,      1, "LoadX",      "LoadX address: load the value at (address+X) into AC");
  tbl[5]  = init_instruction(LOADY_OPCODE,      1, "LoadY",      "LoadY address: load the value at (address+Y) into AC");
  tbl[6]  = init_instruction(LOADSPX_OPCODE,    0, "LoadSPX",    "LoadSPX: load the value at (SP+X) into AC");
  tbl[7]  = init_instruction(STORE_OPCODE,      1, "Store",      "Store address: store AC into address");
  tbl[8]  = init_instruction(GET_OPCODE,        0, "Get",        "Get: Load a random int from 1 to 100 into AC");
  tbl[9]  = init_instruction(PUT_OPCODE,        1, "Put",        "Put port: write AC to screen. port 1=int, port 2=char");
  tbl[10] = init_instruction(ADDX_OPCODE,       0, "AddX",       "AddX: add the value in X to AC");
  tbl[11] = init_instruction(ADDY_OPCODE,       0, "AddY",       "AddY: add the value in Y to AC");
  tbl[12] = init_instruction(SUBX_OPCODE,       0, "SubX",       "SubX: subtract the value in X from AC");
  tbl[13] = init_instruction(SUBY_OPCODE,       0, "SubY",       "SubY: subtract the value in Y from AC");
  tbl[14] = init_instruction(COPYTOX_OPCODE,    0, "CopyToX",    "CopyToX: copy the value in AC to X");
  tbl[15] = init_instruction(COPYFROMX_OPCODE,  0, "CopyFromX",  "CopyFromX: copy the value in X to AC");
  tbl[16] = init_instruction(COPYTOY_OPCODE,    0, "CopyToY",    "CopyToY: copy the value in AC to Y");
  tbl[17] = init_instruction(COPYFROMY_OPCODE,  0, "CopyFromY",  "CopyFromY: copy the value in Y to AC");
  tbl[18] = init_instruction(COPYTOSP_OPCODE,   0, "CopyToSP",   "CopyToSP: copy the value in AC to SP");
  tbl[19] = init_instruction(COPYFROMSP_OPCODE, 0, "CopyFromSP", "CopyFromSP: copy the value in SP to AC");
  tbl[20] = init_instruction(JUMP_OPCODE,       1, "Jump",       "Jump address: jump to address");
  tbl[21] = init_instruction(JUMPEQ_OPCODE,     1, "JumpEQ",     "JumpEQ address: jump to address if AC is zero");
  tbl[22] = init_instruction(JUMPNE_OPCODE,     1, "JumpNE",     "JumpEQ address: jump to address if AC is not zero");
  tbl[23] = init_instruction(CALL_OPCODE,       1, "Call",       "Call address: push return address onto stack, jump to the address.");
  tbl[24] = init_instruction(RETURN_OPCODE,     0, "Return",     "Return: pop return address from stack and jump to address.");
  tbl[25] = init_instruction(INCX_OPCODE,       0, "IncX",       "IncX: Increment the value in X");
  tbl[26] = init_instruction(DECX_OPCODE,       0, "DecX",       "DecX: Decrement the value in X");
  tbl[27] = init_instruction(PUSH_OPCODE,       0, "Push",       "Push: push AC onto stack");
  tbl[28] = init_instruction(POP_OPCODE,        0, "Pop",        "Pop: pop from stack into AC");
  tbl[29] = init_instruction(INTERRUPT_OPCODE,  0, "Interrupt",  "Interrupt: perform a system call");
  tbl[30] = init_instruction(IRETURN_OPCODE,    0, "IReturn",    "IReturn: return from system call");
  tbl[32] = init_instruction(INVALID_OPCODE,    0, "Invalid",    "Invalid opcode.");  
  tbl[33] = init_instruction(INVALID_OPCODE,    0, "Invalid",    "Invalid opcode.");
  tbl[34] = init_instruction(INVALID_OPCODE,    0, "Invalid",    "Invalid opcode.");
  tbl[35] = init_instruction(INVALID_OPCODE,    0, "Invalid",    "Invalid opcode.");
  tbl[36] = init_instruction(INVALID_OPCODE,    0, "Invalid",    "Invalid opcode.");  
  tbl[37] = init_instruction(INVALID_OPCODE,    0, "Invalid",    "Invalid opcode.");  
  tbl[38] = init_instruction(INVALID_OPCODE,    0, "Invalid",    "Invalid opcode.");
  tbl[39] = init_instruction(INVALID_OPCODE,    0, "Invalid",    "Invalid opcode.");
  tbl[40] = init_instruction(INVALID_OPCODE,    0, "Invalid",    "Invalid opcode.");
  tbl[41] = init_instruction(INVALID_OPCODE,    0, "Invalid",    "Invalid opcode.");  
  tbl[42] = init_instruction(INVALID_OPCODE,    0, "Invalid",    "Invalid opcode.");  
  tbl[43] = init_instruction(INVALID_OPCODE,    0, "Invalid",    "Invalid opcode.");
  tbl[44] = init_instruction(INVALID_OPCODE,    0, "Invalid",    "Invalid opcode.");
  tbl[45] = init_instruction(INVALID_OPCODE,    0, "Invalid",    "Invalid opcode.");
  tbl[46] = init_instruction(INVALID_OPCODE,    0, "Invalid",    "Invalid opcode.");  
  tbl[47] = init_instruction(INVALID_OPCODE,    0, "Invalid",    "Invalid opcode.");  
  tbl[48] = init_instruction(INVALID_OPCODE,    0, "Invalid",    "Invalid opcode.");
  tbl[49] = init_instruction(INVALID_OPCODE,    0, "Invalid",    "Invalid opcode.");
  tbl[50] = init_instruction(END_OPCODE,        0, "End",        "End: end execution");
  
  return tbl;
}

void free_dispatch_table(instruction_t *table)
{
  if(table)
    free(table);
}


int iprintf(FILE *fp, instruction_t *instruction)
{
  if ((!fp) || (!instruction)) {
    errno = EINVAL;
    return -1;
  }

  if (UADDRESS(instruction->address))
    instruction->mode = USER_MODE;
  else
    instruction->mode = SYSTEM_MODE;
  
  if (instruction->has_operand) {
    fprintf(fp, "%08d %-10s %4d // %c %-s\n",
	    instruction->address,
	    instruction->mnemonic,
	    instruction->operand,
	    instruction->mode?'S':'U',	    
	    instruction->description);
	return 0;
  }

  fprintf(fp, "%08d %-10s      // %c %-s\n",
	  instruction->address,
	  instruction->mnemonic,
	  instruction->mode?'S':'U',	  
	  instruction->description);

  return 0;
}
