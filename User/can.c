#include <string.h>
#include "can.h"

CanRxMsg CanRxBuf;
CanTxMsg CanTxBuf;
uint16_t  CAN_ID;
boolean_t CAN_RxDone = FALSE;

uint8_t canCRC_ATM(uint8_t *buf,uint8_t len);
/**
	* @brief   初始化CAN 
	* @param   无
	* @retval  无
	*/
void CAN_INIT(void)
{
	/*----- 使能CAN 外设时钟-----*/
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB | RCC_APB2Periph_AFIO, ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_CAN1, ENABLE);
	
	/*----- 初始化CAN引脚-----*/
	// PB9 - CAN_TX
	GPIO_InitTypeDef  GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;				/* 复用推挽输出 */
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	// PB8 - CAN_RX
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;					/* 上拉输入 */
	GPIO_Init(GPIOB, &GPIO_InitStructure);


	/*----- 重映射CAN引脚 -----*/
	GPIO_PinRemapConfig(GPIO_Remap1_CAN1, ENABLE);

	/*----- 初始化CAN -----*/
	CAN_InitTypeDef	CAN_InitStructure;
	CAN_StructInit(&CAN_InitStructure);
	CAN_InitStructure.CAN_TTCM = DISABLE;					// 关闭时间触发通讯模式
	CAN_InitStructure.CAN_ABOM = DISABLE;					// 开启自动离线管理
	CAN_InitStructure.CAN_AWUM = DISABLE;					// 关闭自动唤醒模式
	CAN_InitStructure.CAN_NART = DISABLE;					// 关闭非自动重传模式	DISABLE-自动重传
	CAN_InitStructure.CAN_RFLM = DISABLE;					// 接收FIFO锁定模式		DISABLE-溢出时新报文会覆盖原有报文
	CAN_InitStructure.CAN_TXFP = DISABLE;					// 发送FIFO优先级			DISABLE-优先级取决于报文标识符
	CAN_InitStructure.CAN_Mode = CAN_Mode_Normal;	// 正常工作模式
	CAN_InitStructure.CAN_SJW = CAN_SJW_1tq;			// 重新同步跳跃宽度为SJW + 1个时间单位
	CAN_InitStructure.CAN_BS1 = CAN_BS1_9tq;
	CAN_InitStructure.CAN_BS2 = CAN_BS2_2tq;
	CAN_InitStructure.CAN_Prescaler = 6;					// 波特率 = 36M / 6 / (1 + 9 + 2) = 0.5，即500K
	CAN_Init(CAN1, &CAN_InitStructure);

// 初始化CAN过滤器
	CAN_FilterInitTypeDef	CAN_FilterInitStructure;
	CAN_FilterInitStructure.CAN_FilterNumber = 1;											// 过滤器1
	CAN_FilterInitStructure.CAN_FilterMode = CAN_FilterMode_IdMask;		// 掩码模式
	CAN_FilterInitStructure.CAN_FilterScale = CAN_FilterScale_32bit;	// 32位过滤器位宽
	CAN_FilterInitStructure.CAN_FilterIdHigh = CAN_FILTER_EXTID_H(0x00005678);;						// 过滤器标识符的高16位值
	CAN_FilterInitStructure.CAN_FilterIdLow = CAN_FILTER_EXTID_L(0x00005678);;						// 过滤器标识符的低16位值
	CAN_FilterInitStructure.CAN_FilterMaskIdHigh = CAN_STD_ID_H_MASK_DONT_CARE;						// 过滤器屏蔽标识符的高16位值
	CAN_FilterInitStructure.CAN_FilterMaskIdLow = CAN_STD_ID_L_MASK_DONT_CARE;						// 过滤器屏蔽标识符的低16位值
	CAN_FilterInitStructure.CAN_FilterFIFOAssignment = CAN_FIFO0;			// 指向过滤器的FIFO为0
	CAN_FilterInitStructure.CAN_FilterActivation = ENABLE;						// 使能过滤器
	CAN_FilterInit(&CAN_FilterInitStructure);
	
	// 初始化CAN中断
	CAN_ITConfig(CAN1, CAN_IT_FMP0, ENABLE);													// 使能RX0一包数据接收中断
}


/**
	* @brief   CAN1_RX0接收中断
	* @param   无
	* @retval  无
	*/
/*void USB_LP_CAN1_RX0_IRQHandler(void)
{
	// 接收一帧数据
	CAN_Receive(CAN1, CAN_FIFO0, (CanRxMsg *)(&CanRxBuf));
	// 置位接收标志
	CAN_RxDone = TRUE;
}*/

void USB_LP_CAN1_RX0_IRQHandler(void)
{
    CAN_Receive(CAN1, CAN_FIFO0, &CanRxBuf); // Assuming CAN1 is used, replace as necessary
    CAN_RxDone = TRUE; // Set the flag to indicate a message has been received
    CAN_ClearITPendingBit(CAN1, CAN_IT_FMP0); // Clear the interrupt flag
}


//CAN发出标准帧
void CanTransfer(uint8_t *buf,uint8_t len)
{
    uint8_t i = 0;
    /* Init Transmit frame*/
    CanTxBuf.StdId   = CAN_ID; 					
    CanTxBuf.IDE     = CAN_ID_STD;  
    CanTxBuf.RTR     = CAN_RTR_DATA;    
    CanTxBuf.DLC     = len;  
		
	 memcpy(CanTxBuf.Data,buf,len-1);		
	 CanTxBuf.Data[len-1] = canCRC_ATM(buf,len-1);
	
	CAN_Transmit(CAN1, &CanTxBuf);
}

//计算校验和
uint8_t canCRC_ATM(uint8_t *buf,uint8_t len) //CRC_SUM8
{
	uint32_t i;
	uint8_t check_sum;
	uint32_t sum = 0;
	
	for(i=0;i<len;i++)
	{
		sum += buf[i];
	}
	sum += CAN_ID;
	check_sum = sum & 0xFF;
	return check_sum;
}