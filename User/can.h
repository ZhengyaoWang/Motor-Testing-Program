#ifndef __MKS_CAN_H
#define __MKS_CAN_H
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


#define  CAN_FILTER_EXTID_H(EXTID)       ((uint16_t)  (((EXTID<<3)|0x04)>>16) )
#define  CAN_FILTER_EXTID_L(EXTID)       ((uint16_t)  (( EXTID<<3)|0x04) )
#define  CAN_STD_ID_H_MASK_DONT_CARE     0x0000
#define  CAN_STD_ID_L_MASK_DONT_CARE     0x0000



extern void CAN_INIT(void);
extern uint8_t canCRC_ATM(uint8_t *buf,uint8_t len);
extern void CanTransfer(uint8_t *buf,uint8_t len);
extern CanRxMsg CanRxBuf;
extern CanTxMsg CanTxBuf;
extern uint16_t  CAN_ID;
extern boolean_t CAN_RxDone;

#endif
