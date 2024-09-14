#ifndef __SERVO57D_H
#define __SERVO57D_H
#include "stm32f10x.h" 



extern void speedModeRun(uint8_t slaveAddr,uint8_t dir,uint16_t speed,uint8_t acc);
extern uint8_t getCheckSum(uint8_t *buffer,uint8_t size);
extern uint8_t waitingForACK(void);

#endif
