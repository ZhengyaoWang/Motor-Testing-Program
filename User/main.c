#include "stm32f10x.h"
#include "stm32f10x_rcc.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_can.h"
#include "uart.h"
#include <string.h>
#include <stdio.h>

#include "Delay.h"
#include "timer.h"
#include "led.h"
#include "can.h"
#include "mksPulse.h"

uint8_t txBuffer[8];      //��������������

void sendCommand(uint8_t slaveAddr,uint16_t speed,uint8_t acc,int32_t absoluteAxis);
uint8_t waitingForACK(uint32_t delayTime);
void Run(uint8_t slaveAddr, uint16_t speed, uint8_t acc, int32_t axis);
void runFail(void);
void NVIC_INIT(void);

int main(void)
{
	NVIC_INIT();
	LED_Init();					//��ʼ��LED��
	Timer2_Init();			//��ʼ����ʱ��2
	CAN_INIT();					//��ʼ��CAN
	Delay_ms(3000);     //��ʱ3000ms�ȴ��������
	
	while(1)
	{   
		LED_Toggle();

    Run(1, 1000, 150, 163840*20);
		Run(2, 1000, 150, 163840*20);
		ackStatus = waitingForACK(0);
		Delay_ms(1000);     //��ʱ1000ms
    Run(1, 900, 150, 0);
		Run(2, 900, 150, 0);
		Delay_ms(1000);     //��ʱ1000ms
	}
}

/*
���ܣ����ڷ���λ��ģʽ����ָ��
���룺slaveAddr �ӻ���ַ
      speed     �����ٶ�
      acc       ���ٶ�
      absAxis   ��������
*/
void sendCommand(uint8_t slaveAddr,uint16_t speed,uint8_t acc,int32_t absAxis)
{
	CAN_ID = slaveAddr;				//ID
    txBuffer[0] = 0xF5;       //Function Code (Byte0) means position Mode 3
    txBuffer[1] = (speed>>8)&0x00FF; //Higher 8 Bits of the Speed (Byte1)
    txBuffer[2] = speed&0x00FF;     //Lower 8 Bits of the Speed (Byte2)
    txBuffer[3] = acc;            //Acceleration Byte (Byte3)
    txBuffer[4] = (absAxis >> 16)&0xFF;  //Absolute Position bit23 - bit16 (Byte4)
    txBuffer[5] = (absAxis >> 8)&0xFF;   //Absolute Position bit15 - bit8 (Byte5)
    txBuffer[6] = (absAxis >> 0)&0xFF;   //Absolute Position bit7 - bit0 (Byte6)
	
	CanTransfer(txBuffer,8);
}

/*
���ܣ��ȴ���λ��Ӧ�����ó�ʱʱ��Ϊ3000ms
���룺
  delayTime �ȴ�ʱ��(ms), 
  delayTime = 0 ,���޵ȴ�
�����
  λ��ģʽ3���ƿ�ʼ    1
  λ��ģʽ3�������    2
  λ��ģʽ3����ʧ��    0
  ��ʱ��Ӧ��          0
*/
uint8_t waitingForACK(uint32_t delayTime)
{
    boolean_t retVal = FALSE; //����ֵ
    unsigned long sTime;  		//��ʱ��ʼʱ��
    unsigned long time;  			//��ǰʱ��
    uint8_t rxByte;      

    sTime = millis();    //��ȡ��ǰʱ��
    while(1)
    {
		if(CAN_RxDone == TRUE)  //CAN���յ�����
		{
			CAN_RxDone = FALSE;
			rxByte = CanRxBuf.DLC;
			CAN_ID = CanRxBuf.StdId;
			if(CanRxBuf.Data[rxByte-1] == canCRC_ATM(CanRxBuf.Data,rxByte-1))
			{
				retVal = CanRxBuf.Data[1];   //У����ȷ
				break;
			}				
		}

        time = millis();
        if((delayTime != 0) && ((time - sTime) > delayTime))   //�ж��Ƿ�ʱ
        {
            retVal = 0;
            break;                    //��ʱ���˳�while(1)
        }
    }
    return(retVal);
}

/*
���ܣ�����λ�����кʹ�����
���룺slaveAddr �ӻ���ַ
      speed     �����ٶ�
      acc       ���ٶ�
      axis      ��������
*/
void Run(uint8_t slaveAddr, uint16_t speed, uint8_t acc, int32_t axis)
{
    sendCommand(slaveAddr, speed, acc, axis); //����λ��ģʽ����ָ��
    uint8_t ackStatus = waitingForACK(3000);        //�ȴ����Ӧ��

    /*if (ackStatus == 1)                             //λ�ÿ��ƿ�ʼ
    {
        ackStatus = waitingForACK(0);               //�ȴ�λ�ÿ������Ӧ��
        if (ackStatus != 2)                         //δ�յ�λ�����Ӧ��
        {
            runFail();
        }
    }
    else                                            //λ�ÿ���ʧ��
    {
        runFail();
    }*/
}

//����ʧ��
void runFail(void)
{
    while(1)                //�������ƣ���ʾ����ʧ��
    {
        LED_ON();
        Delay_ms(200);
        LED_OFF();
        Delay_ms(200);
    }	
}

/**
	* @brief   ����NVIC������
	* @param   ��
	* @retval  ��
	*/
void NVIC_INIT(void)
{	
	// 2bit��ռ���ȼ�λ
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
	NVIC_InitTypeDef NVIC_InitStructure;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannel = USB_LP_CAN1_RX0_IRQn;
	NVIC_Init(&NVIC_InitStructure);
}
