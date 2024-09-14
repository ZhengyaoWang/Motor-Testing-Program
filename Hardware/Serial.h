#ifndef __SERIAL_H
#define __SERIAL_H
#include "stm32f10x.h" 


#ifndef TRUE
  /** Value is true (boolean_t type) */
  #define TRUE        ((boolean_t) 1u)
#endif

#ifndef FALSE
  /** Value is false (boolean_t type) */
  #define FALSE       ((boolean_t) 0u)
#endif  

typedef uint8_t      boolean_t;


typedef struct {
	boolean_t	available;
	uint8_t	rxByte;
} Serial_t;  
extern volatile Serial_t Serial;

void Serial_Init(void);
void Serial_writeByte(uint8_t Byte);
void Serial_writeArray(uint8_t *Array, uint16_t Length);
void Serial_SendString(char *String);
void Serial_SendNumber(uint32_t Number, uint8_t Length);
void Serial_Printf(char *format, ...);

boolean_t Serial_Available(void);
uint8_t Serial_readByte(void);

#endif
