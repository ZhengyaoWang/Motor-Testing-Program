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

#define UART_BUFFER_SIZE 64
#define MAX_MOTORS 2 // Adjust this according to the number of motors you want to control

uint8_t txBuffer[8]; //待发送数据数组

void sendCommand(uint8_t slaveAddr, uint16_t speed, uint8_t acc, int32_t absoluteAxis);
void ParseAndRun(void);
void GroupedRun(void);
void readRealTimeLocation(uint8_t slaveAddr);
boolean_t waitingForPositionACK(void);
uint8_t waitingForACK(uint32_t delayTime);
void runFail(void);
void NVIC_INIT(void);

extern volatile char uartBuffer[UART_BUFFER_SIZE]; // External declaration, defined in uart.c
extern volatile int uartBufferIndex;

uint8_t motorAddrs[MAX_MOTORS];
uint16_t speeds[MAX_MOTORS];
uint8_t accs[MAX_MOTORS];
int32_t motorAngles[MAX_MOTORS];
uint8_t motorCount = 0;


int main(void)
{
    NVIC_INIT();
    LED_Init();         //初始化LED灯
    Timer2_Init();      //初始化定时器2
    CAN_INIT();         //初始化CAN
    UART_Init(9600);

    Delay_ms(3000);     //延时3000ms等待电机启动

    while (1)
    {
        LED_Toggle();

        // Check if a complete message has been received
        if (uartBufferIndex > 0 && (uartBuffer[uartBufferIndex - 1] == '\0'))
        {
            ParseAndRun(); // Parse and execute the command
            uartBufferIndex = 0; // Reset the buffer index after processing the message
        }

        Delay_ms(50); //延时50ms
    }
}

void ParseAndRun(void)
{
    uint8_t rx_slaveAddr;
    uint16_t rx_speed;
    uint8_t rx_acc;
    int32_t rx_axis;

    // Example input format: "1 60 30 4000\n"
    if (sscanf((char*)uartBuffer, "%hhu %hu %hhu %d", &rx_slaveAddr, &rx_speed, &rx_acc, &rx_axis) == 4)
    {
        // Store parsed values into arrays for grouped execution
        motorAddrs[motorCount] = rx_slaveAddr;
        speeds[motorCount] = rx_speed;
        accs[motorCount] = rx_acc;
        motorAngles[motorCount] = rx_axis;
        motorCount++;

        if (motorCount >= MAX_MOTORS) // Once all motor commands are parsed
        {
            GroupedRun(); // Run all motors in a grouped manner
            motorCount = 0; // Reset motor count after execution
        }
    }
    else
    {
        UART_SendString("Invalid input format\r\n");
    }
}

void GroupedRun(void)
{
    // Step 1: Send commands to all motors sequentially
    
	int maxWaitTime = 0; // Initialize to 0 or a very small value

	for (int i = 0; i < motorCount; i++) {
			int estimatedWaitTimeForACK = motorAngles[i]/16384/(speeds[i]*6)*1000*2.1;
                  			
			if (estimatedWaitTimeForACK > maxWaitTime) {
					maxWaitTime = estimatedWaitTimeForACK; // Update maxWaitTime if current value is higher
			}
	}
	
	for (int i = 0; i < motorCount; i++)
	
    {
        sendCommand(motorAddrs[i], speeds[i], accs[i], motorAngles[i]);
			
    }

    // Step 2: Wait for ack=1 from all motors
    for (int i = 0; i < motorCount; i++)
    {
        while (waitingForACK(maxWaitTime) != 1)
        {
            runFail();// Optionally add a timeout here to prevent infinite loops
            // and handle communication failures gracefully
        }
    }

    // Step 3: Wait for ack=2 from all motors
    for (int i = 0; i < motorCount; i++)
    {
        while (waitingForACK(maxWaitTime) != 2)
        {
            runFail();// Again, handle timeouts or failures as needed
        }
    }

    // Once all ACKs are received, the work cycle is complete
}

void sendCommand(uint8_t slaveAddr, uint16_t speed, uint8_t acc, int32_t absAxis)
{
    CAN_ID = slaveAddr; // ID
    txBuffer[0] = 0xF5; // Function Code (Byte0) means position Mode 3
    txBuffer[1] = (speed >> 8) & 0x00FF; // Higher 8 Bits of the Speed (Byte1)
    txBuffer[2] = speed & 0x00FF; // Lower 8 Bits of the Speed (Byte2)
    txBuffer[3] = acc; // Acceleration Byte (Byte3)
    txBuffer[4] = (absAxis >> 16) & 0xFF; // Absolute Position bit23 - bit16 (Byte4)
    txBuffer[5] = (absAxis >> 8) & 0xFF; // Absolute Position bit15 - bit8 (Byte5)
    txBuffer[6] = (absAxis >> 0) & 0xFF; // Absolute Position bit7 - bit0 (Byte6)

    CanTransfer(txBuffer, 8);
}

uint8_t waitingForACK(uint32_t delayTime)
{
    uint8_t retVal = 0; //返回值
    unsigned long sTime = millis(); //计时起始时刻
    unsigned long time; //当前时刻
    uint8_t rxByte;

    while (1)
    {
        if (CAN_RxDone == TRUE) //CAN接收到数据
        {
            CAN_RxDone = FALSE;
            rxByte = CanRxBuf.DLC;
            CAN_ID = CanRxBuf.StdId;
            if (CanRxBuf.Data[rxByte - 1] == canCRC_ATM(CanRxBuf.Data, rxByte - 1))
            {
                retVal = CanRxBuf.Data[1]; //校验正确
                break;
            }
        }
				
				////////////////////////////////////////
				
        time = millis();
        if ((delayTime != 0) && ((time - sTime) > delayTime)) //判断是否超时
        {
            retVal = 0;
            break; //超时，退出while(1)
        }
    }
    return retVal;
}

boolean_t waitingForPositionACK(void)
{
    boolean_t retVal = FALSE; //返回值
    unsigned long sTime = millis();    //计时起始时刻
    unsigned long time;    //当前时刻
    uint8_t rxByte;

    while (1)
    {
        if (CAN_RxDone == TRUE)  //CAN接收到数据
        {
            CAN_RxDone = FALSE;
            rxByte = CanRxBuf.DLC;
            CAN_ID = CanRxBuf.StdId;
            if (CanRxBuf.Data[rxByte - 1] == canCRC_ATM(CanRxBuf.Data, rxByte - 1))
            {
                retVal = TRUE;   //校验正确
                break;
            }
        }

        time = millis();
        if ((time - sTime) > 3000)   //判断是否超时
        {
            retVal = FALSE;
            break;  //超时，退出while(1)
        }
    }
    return retVal;
}

void runFail(void)
{
    while (1) //快速闪灯，提示运行失败
    {
        LED_ON();
        Delay_ms(200);
        LED_OFF();
        Delay_ms(200);
    }
}

void NVIC_INIT(void)
{    
    // Set the NVIC priority grouping to 2 bits for preemption priority
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
    
    // Configure NVIC for USB LP CAN1 RX0 interrupt (as before)
    NVIC_InitTypeDef NVIC_InitStructure;
    NVIC_InitStructure.NVIC_IRQChannel = USB_LP_CAN1_RX0_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

    // Configure NVIC for USART1 interrupt
    NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;  // Specify the USART1 interrupt channel
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1; // Set a different priority if needed
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
}
