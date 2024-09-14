#ifndef __MKS_PULSES_H
#define __MKS_PULSES_H
#include "stm32f10x.h" 

/*---------定义脉冲接口端口------------*/
#define En_PORT	GPIOA					 
#define En_PIN	GPIO_Pin_4		//En  PA4
#define Stp_PORT	GPIOA
#define Stp_PIN	GPIO_Pin_5		//Stp	PA5
#define Dir_PORT	GPIOA
#define Dir_PIN	GPIO_Pin_6		//Dir	PA6

extern uint8_t stpStatus;

extern void mksPulseInit(void);
extern void mksPulseRun(void);

#endif
