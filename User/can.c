#include <string.h>
#include "can.h"

CanRxMsg CanRxBuf;
CanTxMsg CanTxBuf;
uint16_t  CAN_ID;
boolean_t CAN_RxDone = FALSE;

uint8_t canCRC_ATM(uint8_t *buf,uint8_t len);
/**
	* @brief   ��ʼ��CAN 
	* @param   ��
	* @retval  ��
	*/
void CAN_INIT(void)
{
	/*----- ʹ��CAN ����ʱ��-----*/
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB | RCC_APB2Periph_AFIO, ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_CAN1, ENABLE);
	
	/*----- ��ʼ��CAN����-----*/
	// PB9 - CAN_TX
	GPIO_InitTypeDef  GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;				/* ����������� */
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	// PB8 - CAN_RX
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;					/* �������� */
	GPIO_Init(GPIOB, &GPIO_InitStructure);


	/*----- ��ӳ��CAN���� -----*/
	GPIO_PinRemapConfig(GPIO_Remap1_CAN1, ENABLE);

	/*----- ��ʼ��CAN -----*/
	CAN_InitTypeDef	CAN_InitStructure;
	CAN_StructInit(&CAN_InitStructure);
	CAN_InitStructure.CAN_TTCM = DISABLE;					// �ر�ʱ�䴥��ͨѶģʽ
	CAN_InitStructure.CAN_ABOM = DISABLE;					// �����Զ����߹���
	CAN_InitStructure.CAN_AWUM = DISABLE;					// �ر��Զ�����ģʽ
	CAN_InitStructure.CAN_NART = DISABLE;					// �رշ��Զ��ش�ģʽ	DISABLE-�Զ��ش�
	CAN_InitStructure.CAN_RFLM = DISABLE;					// ����FIFO����ģʽ		DISABLE-���ʱ�±��ĻḲ��ԭ�б���
	CAN_InitStructure.CAN_TXFP = DISABLE;					// ����FIFO���ȼ�			DISABLE-���ȼ�ȡ���ڱ��ı�ʶ��
	CAN_InitStructure.CAN_Mode = CAN_Mode_Normal;	// ��������ģʽ
	CAN_InitStructure.CAN_SJW = CAN_SJW_1tq;			// ����ͬ����Ծ���ΪSJW + 1��ʱ�䵥λ
	CAN_InitStructure.CAN_BS1 = CAN_BS1_9tq;
	CAN_InitStructure.CAN_BS2 = CAN_BS2_2tq;
	CAN_InitStructure.CAN_Prescaler = 6;					// ������ = 36M / 6 / (1 + 9 + 2) = 0.5����500K
	CAN_Init(CAN1, &CAN_InitStructure);

// ��ʼ��CAN������
	CAN_FilterInitTypeDef	CAN_FilterInitStructure;
	CAN_FilterInitStructure.CAN_FilterNumber = 1;											// ������1
	CAN_FilterInitStructure.CAN_FilterMode = CAN_FilterMode_IdMask;		// ����ģʽ
	CAN_FilterInitStructure.CAN_FilterScale = CAN_FilterScale_32bit;	// 32λ������λ��
	CAN_FilterInitStructure.CAN_FilterIdHigh = CAN_FILTER_EXTID_H(0x00005678);;						// ��������ʶ���ĸ�16λֵ
	CAN_FilterInitStructure.CAN_FilterIdLow = CAN_FILTER_EXTID_L(0x00005678);;						// ��������ʶ���ĵ�16λֵ
	CAN_FilterInitStructure.CAN_FilterMaskIdHigh = CAN_STD_ID_H_MASK_DONT_CARE;						// ���������α�ʶ���ĸ�16λֵ
	CAN_FilterInitStructure.CAN_FilterMaskIdLow = CAN_STD_ID_L_MASK_DONT_CARE;						// ���������α�ʶ���ĵ�16λֵ
	CAN_FilterInitStructure.CAN_FilterFIFOAssignment = CAN_FIFO0;			// ָ���������FIFOΪ0
	CAN_FilterInitStructure.CAN_FilterActivation = ENABLE;						// ʹ�ܹ�����
	CAN_FilterInit(&CAN_FilterInitStructure);
	
	// ��ʼ��CAN�ж�
	CAN_ITConfig(CAN1, CAN_IT_FMP0, ENABLE);													// ʹ��RX0һ�����ݽ����ж�
}


/**
	* @brief   CAN1_RX0�����ж�
	* @param   ��
	* @retval  ��
	*/
/*void USB_LP_CAN1_RX0_IRQHandler(void)
{
	// ����һ֡����
	CAN_Receive(CAN1, CAN_FIFO0, (CanRxMsg *)(&CanRxBuf));
	// ��λ���ձ�־
	CAN_RxDone = TRUE;
}*/

void USB_LP_CAN1_RX0_IRQHandler(void)
{
    CAN_Receive(CAN1, CAN_FIFO0, &CanRxBuf); // Assuming CAN1 is used, replace as necessary
    CAN_RxDone = TRUE; // Set the flag to indicate a message has been received
    CAN_ClearITPendingBit(CAN1, CAN_IT_FMP0); // Clear the interrupt flag
}


//CAN������׼֡
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

//����У���
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