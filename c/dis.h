/* dis.h (ejo@ufo) 11 Sep 23  Modified: 11 Sep 23  17:33 */


#ifndef DIS_H
#define DIS_H

typedef enum {
  USER_REGION,
  TIMER_REGION,
  INTERRUPT_REGION,
  ALL_REGIONS,
} region_t;

void disassemble_memory(FILE *fp, region_t region);

#endif	/* DIS_H */
