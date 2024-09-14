#include "stm32f10x.h"                  // Device header

#include "timer.h"
#include "mksPulse.h"

uint8_t stpStatus = 0;

void mksPulseInit(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	/*-----------使能PA,PC端口时钟------------*/
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
	
	/*-----------配置 EN/STP/DIR 端口为输出------------*/
	GPIO_InitStructure.GPIO_Pin = En_PIN | Stp_PIN | Dir_PIN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	GPIO_SetBits(En_PORT,En_PIN);					//En 置高电平
	GPIO_ResetBits(Dir_PORT, Dir_PIN);		//Dir置低电平
	GPIO_ResetBits(Stp_PORT, Stp_PIN);		//Stp置低电平
	
	Timer3_Init();	
}

void mksPulseRun(void)
{
	GPIO_ResetBits(En_PORT,En_PIN);					//En 置低电平
	GPIO_SetBits(Dir_PORT, Dir_PIN);			//Dir置低电平
	
	TIM_Cmd(TIM3, ENABLE);									//使能TIM3, 翻转Stp信号，产生脉冲
}

