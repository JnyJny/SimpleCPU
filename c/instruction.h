#ifndef INSTRUCTION_H
#define INSTRUCTION_H



typedef enum {
  INVALID,    
  LOADV,      
  LOADA,      
  LOADI,      
  LOADX,      
  LOADY,      
  LOADSPX,    
  STORE,      
  GET,        
  PUT,        
  ADDX,       
  ADDY,       
  SUBX,       
  SUBY,       
  COPYTOX,    
  COPYFROMX,  
  COPYTOY,    
  COPYFROMY,  
  COPYTOSP,   
  COPYFROMSP, 
  JUMP,       
  JUMPEQ,     
  JUMPNE,     
  CALL,       
  RETURN,     
  INCX,       
  DECX,       
  PUSH,       
  POP,
  INTERRUPT,  
  IRETURN,    
  END=50,
} opcode_t;

typedef void (*microcode_f)(void *);

typedef struct {
  opcode_t      opcode;
  unsigned int  mode:3;
  unsigned int  has_operand:1;
  unsigned int  pad:28;
  microcode_f   microcode;
  int           address;
  int           operand;
  char         *mnemonic;
  char         *description;
} instruction_t;


#define VALID_OPCODE(OPCODE) (((OPCODE)==END)||(((OPCODE)>=LOADV)&&((OPCODE)<=IRETURN)))
#define       IS_CTI(OPCODE) (((OPCODE)>=JUMP)&&((OPCODE)<=RETURN))||(((OPCODE)==INTERRUPT)||((OPCODE)==IRETURN))
#define      NOT_CTI(OPCODE) (!(IS_CTI(OPCODE)))

#endif	/* INSTRUCTION_H */
