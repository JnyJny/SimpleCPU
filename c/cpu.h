/* cpu.h (ejo@ufo)  7 Sep 23  Modified:  7 Sep 23  17:29 */

#ifndef USER_H
#define USER_H

#include "constants.h"
#include "instruction.h"
#include "io.h"

#define CPU "cpu"

typedef void (*load_f) (word_t address, word_t *destination);
typedef void (*store_f)(word_t address, word_t value);

typedef struct {
  word_t       ir;		/* instruction register */
  word_t       pc;		/* program counter */
  word_t       sp;		/* stack pointer */
  word_t       ac;		/* accumulator */
  word_t       x;		/* operand X */
  word_t       y;		/* operand Y */
  unsigned int cycles;		/* number of instructions executed */
  
  union {
    struct {
      unsigned int mode:1;		/* USER, SYSTEM */
      unsigned int interrupts:1;	/* 0=off 1=enable */
      unsigned int debug:1;		/* 0=off 1=enable */
      unsigned int fault:1;		/* 0=ok, 1=fault */
      unsigned int stop:1;              /* 0=run, 1=stop */
      unsigned int step:1;              /* 0=run, 1=step */
      unsigned int pad:26;		/* reserved bits */
    } fields;
    unsigned int   value;
  } control;
  
  union {
    struct {
      unsigned int fetch:1;
      unsigned int decode:1;
      unsigned int execute:1;
      unsigned int load:1;
      unsigned int store:1;
      unsigned int range:1;
      unsigned int perm:1;
      unsigned int underflow:1;
      unsigned int overflow:1;
      unsigned int invalid:1;
      unsigned int pad:23;
    } fields;
    unsigned int value;
  } error;
  
} registers_t;


typedef struct {
  registers_t    registers;
  load_f         load;		 /* load function pointer */
  store_f        store;		 /* store function pointer */
  unsigned int   timer_interval; /* # cycles to fire timer interrupts */
  instruction_t  instruction;	 /* last instruction decoded */  
  instruction_t *instruction_decode;
} cpu_t;

#define r_ir         registers.ir
#define r_pc         registers.pc
#define r_sp         registers.sp
#define r_ac         registers.ac
#define r_x          registers.x
#define r_y          registers.y
#define r_cycles     registers.cycles
#define r_control    registers.control.value
#define r_error      registers.error.value


#define c_mode       registers.control.fields.mode
#define c_interrupts registers.control.fields.interrupts
#define c_debug      registers.control.fields.debug
#define c_fault      registers.control.fields.fault
#define c_stop       registers.control.fields.stop
#define c_step       registers.control.fields.step

#define e_fetch      registers.error.fields.fetch
#define e_decode     registers.error.fields.decode
#define e_execute    registers.error.fields.execute
#define e_load       registers.error.fields.load
#define e_store      registers.error.fields.store
#define e_perm       registers.error.fields.perm
#define e_range      registers.error.fields.range
#define e_underflow  registers.error.fields.underflow
#define e_overflow   registers.error.fields.overflow
#define e_invalid    registers.error.fields.invalid

#define TAKE_TIMER_INTERRUPT(CPU) ((CPU)->r_cycles && (CPU)->timer_interval && (((CPU)->r_cycles %(CPU)->timer_interval) == 0))


#endif	/* USER_H */
