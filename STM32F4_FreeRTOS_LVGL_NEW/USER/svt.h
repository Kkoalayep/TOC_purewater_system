#ifndef __SVT_H__
#define __SVT_H__


#include "stm32f4xx.h"


extern uint8_t svt_state;
void SVT_Console(uint8_t state);
void SVT_Init(void);



#endif

