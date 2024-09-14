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

uint8_t txBuffer[8];      //待发送数据数组

void sendCommand(uint8_t slaveAddr,uint16_t speed,uint8_t acc,int32_t absoluteAxis);
uint8_t waitingForACK(uint32_t delayTime);
void Run(uint8_t slaveAddr, uint16_t speed, uint8_t acc, int32_t axis);
void runFail(void);
void NVIC_INIT(void);

int main(void)
{
	NVIC_INIT();
	LED_Init();					//初始化LED灯
	Timer2_Init();			//初始化定时器2
	CAN_INIT();					//初始化CAN
	Delay_ms(3000);     //延时3000ms等待电机启动
	
	while(1)
	{   
		LED_Toggle();

    Run(1, 1000, 150, 163840*20);
		Run(2, 1000, 150, 163840*20);
		ackStatus = waitingForACK(0);
		Delay_ms(1000);     //延时1000ms
    Run(1, 900, 150, 0);
		Run(2, 900, 150, 0);
		Delay_ms(1000);     //延时1000ms
	}
}

/*
功能：串口发送位置模式运行指令
输入：slaveAddr 从机地址
      speed     运行速度
      acc       加速度
      absAxis   绝对坐标
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
功能：等待下位机应答，设置超时时间为3000ms
输入：
  delayTime 等待时间(ms), 
  delayTime = 0 ,无限等待
输出：
  位置模式3控制开始    1
  位置模式3控制完成    2
  位置模式3控制失败    0
  超时无应答          0
*/
uint8_t waitingForACK(uint32_t delayTime)
{
    boolean_t retVal = FALSE; //返回值
    unsigned long sTime;  		//计时起始时刻
    unsigned long time;  			//当前时刻
    uint8_t rxByte;      

    sTime = millis();    //获取当前时刻
    while(1)
    {
		if(CAN_RxDone == TRUE)  //CAN接收到数据
		{
			CAN_RxDone = FALSE;
			rxByte = CanRxBuf.DLC;
			CAN_ID = CanRxBuf.StdId;
			if(CanRxBuf.Data[rxByte-1] == canCRC_ATM(CanRxBuf.Data,rxByte-1))
			{
				retVal = CanRxBuf.Data[1];   //校验正确
				break;
			}				
		}

        time = millis();
        if((delayTime != 0) && ((time - sTime) > delayTime))   //判断是否超时
        {
            retVal = 0;
            break;                    //超时，退出while(1)
        }
    }
    return(retVal);
}

/*
功能：处理位置运行和错误检查
输入：slaveAddr 从机地址
      speed     运行速度
      acc       加速度
      axis      绝对坐标
*/
void Run(uint8_t slaveAddr, uint16_t speed, uint8_t acc, int32_t axis)
{
    sendCommand(slaveAddr, speed, acc, axis); //发送位置模式运行指令
    uint8_t ackStatus = waitingForACK(3000);        //等待电机应答

    /*if (ackStatus == 1)                             //位置控制开始
    {
        ackStatus = waitingForACK(0);               //等待位置控制完成应答
        if (ackStatus != 2)                         //未收到位置完成应答
        {
            runFail();
        }
    }
    else                                            //位置控制失败
    {
        runFail();
    }*/
}

//运行失败
void runFail(void)
{
    while(1)                //快速闪灯，提示运行失败
    {
        LED_ON();
        Delay_ms(200);
        LED_OFF();
        Delay_ms(200);
    }	
}

/**
	* @brief   配置NVIC控制器
	* @param   无
	* @retval  无
	*/
void NVIC_INIT(void)
{	
	// 2bit抢占优先级位
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
	NVIC_InitTypeDef NVIC_InitStructure;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannel = USB_LP_CAN1_RX0_IRQn;
	NVIC_Init(&NVIC_InitStructure);
}
