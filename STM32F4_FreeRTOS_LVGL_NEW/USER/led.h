#ifndef __LED_H__
#define __LED_H__
#include "stm32f4xx.h"


extern uint8_t led_state;
void LED_Console(uint8_t state);
void LED_Init(void);

#endif


