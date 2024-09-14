#include "stm32f10x.h"                  // Device header

#include "timer.h"
#include "mksPulse.h"

uint8_t stpStatus = 0;

void mksPulseInit(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	/*-----------ʹ��PA,PC�˿�ʱ��------------*/
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
	
	/*-----------���� EN/STP/DIR �˿�Ϊ���------------*/
	GPIO_InitStructure.GPIO_Pin = En_PIN | Stp_PIN | Dir_PIN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	GPIO_SetBits(En_PORT,En_PIN);					//En �øߵ�ƽ
	GPIO_ResetBits(Dir_PORT, Dir_PIN);		//Dir�õ͵�ƽ
	GPIO_ResetBits(Stp_PORT, Stp_PIN);		//Stp�õ͵�ƽ
	
	Timer3_Init();	
}

void mksPulseRun(void)
{
	GPIO_ResetBits(En_PORT,En_PIN);					//En �õ͵�ƽ
	GPIO_SetBits(Dir_PORT, Dir_PIN);			//Dir�õ͵�ƽ
	
	TIM_Cmd(TIM3, ENABLE);									//ʹ��TIM3, ��תStp�źţ���������
}

