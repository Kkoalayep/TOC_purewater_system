#ifndef __UART
#define __UART
#include "stdio.h"
#include "stm32f4xx_usart.h"
extern void UARTON(void);
extern int fputc(int ch, FILE *f);

#endif  // __UART
