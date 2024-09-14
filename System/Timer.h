#ifndef __TIMER_H
#define __TIMER_H
#include "stm32f10x.h" 

extern void Timer2_Init(void);
extern void Timer3_Init(void);
extern uint64_t millis(void);

#endif
