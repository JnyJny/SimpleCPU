#ifndef INSTRUCTION_H
#define INSTRUCTION_H

#include "io.h"

typedef enum {
  INVALID_OPCODE=0,    
  LOADV_OPCODE,      
  LOADA_OPCODE,      
  LOADI_OPCODE,      
  LOADX_OPCODE,      
  LOADY_OPCODE,      
  LOADSPX_OPCODE,    
  STORE_OPCODE,      
  GET_OPCODE,        
  PUT_OPCODE,        
  ADDX_OPCODE,       
  ADDY_OPCODE,       
  SUBX_OPCODE,       
  SUBY_OPCODE,       
  COPYTOX_OPCODE,    
  COPYFROMX_OPCODE,  
  COPYTOY_OPCODE,    
  COPYFROMY_OPCODE,  
  COPYTOSP_OPCODE,   
  COPYFROMSP_OPCODE, 
  JUMP_OPCODE,       
  JUMPEQ_OPCODE,     
  JUMPNE_OPCODE,     
  CALL_OPCODE,       
  RETURN_OPCODE,     
  INCX_OPCODE,       
  DECX_OPCODE,       
  PUSH_OPCODE,       
  POP_OPCODE,
  INTERRUPT_OPCODE,  
  IRETURN_OPCODE,
  END_OPCODE=50,    
  NUM_OPCODES,
} opcode_t;


typedef void (*microcode_f)(void *);

typedef struct {
  opcode_t      opcode;
  unsigned int  has_operand:1;
  unsigned int  mode:3;
  unsigned int  pad:28;
  microcode_f   microcode;
  int           address;
  int           operand;
  char         *mnemonic;
  char         *description;

} instruction_t;


#define VALID_OPCODE(OPCODE) (((OPCODE)==END_OPCODE)||(((OPCODE)>=LOADV_OPCODE)&&((OPCODE)<=IRETURN_OPCODE)))
#define       IS_CTI(OPCODE) (((OPCODE)>=JUMP_OPCODE)&&((OPCODE)<=RETURN_OPCODE))||(((OPCODE)==INTERRUPT_OPCODE)||((OPCODE)==IRETURN_OPCODE))
#define      NOT_CTI(OPCODE) (!(IS_CTI(OPCODE)))

instruction_t *alloc_dispatch_table(void);
void           free_dispatch_table(instruction_t *table);

int iprintf(FILE *fp, instruction_t *instruction);

#endif	/* INSTRUCTION_H */
