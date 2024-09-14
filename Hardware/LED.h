#ifndef __LED_H
#define __LED_H

#define led_port	GPIOC
#define led_pin		GPIO_Pin_13

void LED_Init(void);
void LED_ON(void);
void LED_OFF(void);
void LED_Toggle(void);

#endif
