#include "stm32f10x.h"                  // Device header
#include "led.h"                  // Device header

void LED_Init(void)
{
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);
	
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Pin = led_pin;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(led_port, &GPIO_InitStructure);
	
	GPIO_SetBits(led_port, led_pin);
}

void LED_ON(void)
{
	GPIO_ResetBits(led_port, led_pin);
}

void LED_OFF(void)
{
	GPIO_SetBits(led_port, led_pin);
}

void LED_Toggle(void)
{
	if (GPIO_ReadOutputDataBit(led_port, led_pin) == 0)
	{
		GPIO_SetBits(led_port, led_pin);
	}
	else
	{
		GPIO_ResetBits(led_port, led_pin);
	}
}

