#include "uart.h"

#define UART_BUFFER_SIZE 64

volatile char uartBuffer[UART_BUFFER_SIZE];
volatile int uartBufferIndex = 0;

	void UART_Init(uint32_t baudrate)
	{
 // Enable clocks for GPIOA and USART1
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1 | RCC_APB2Periph_GPIOA, ENABLE);

    // Configure PA9 as USART1 Tx (alternate function push-pull)
    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    // Configure PA10 as USART1 Rx (input floating)
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    // USART1 configuration
    USART_InitTypeDef USART_InitStructure;
    USART_InitStructure.USART_BaudRate = baudrate;
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_Parity = USART_Parity_No;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
    USART_Init(USART1, &USART_InitStructure);

    // Enable USART1
    USART_Cmd(USART1, ENABLE);

    // Enable the USART1 Receive interrupt
    USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);

    // Enable the USART1 global interrupt in NVIC
    NVIC_InitTypeDef NVIC_InitStructure;
    NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
	}

	void UART_SendByte(uint8_t byte)
	{
			// Wait until the transmit data register is empty
			while (USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET);
			// Send a byte
			USART_SendData(USART1, byte);
	}

	void UART_SendString(char* string)
	{
			while (*string)
			{
					UART_SendByte(*string++);
			}
	}

	uint8_t UART_ReceiveByte(void)
	{
			// Wait until data is received
			while (USART_GetFlagStatus(USART1, USART_FLAG_RXNE) == RESET);
			// Read the received byte
			return USART_ReceiveData(USART1);
	}

void USART1_IRQHandler(void)
{
    if (USART_GetITStatus(USART1, USART_IT_RXNE) != RESET)
    {
        char receivedChar = USART_ReceiveData(USART1);

        // Store the received character in the buffer
        uartBuffer[uartBufferIndex++] = receivedChar;

        // Prevent buffer overflow by wrapping around or discarding excess data
        if (uartBufferIndex >= UART_BUFFER_SIZE)
        {
            uartBufferIndex = 0; // Reset buffer index if overflow occurs
        }

        // Check if the received character is a newline or carriage return
        if (receivedChar == '\n' || receivedChar == '\r')
        {
            uartBuffer[uartBufferIndex - 1] = '\0'; // Null-terminate the string
            // Don't reset uartBufferIndex here; let the main loop handle it
        }
    }
}
