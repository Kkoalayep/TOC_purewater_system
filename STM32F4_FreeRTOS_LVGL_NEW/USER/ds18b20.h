#ifndef __DS18B20_H__
#define __DS18B20_H__
#include "stm32f4xx.h"

#define HIGH  1
#define LOW   0

extern float temp_value;

//带参宏，可以像内联函数一样使用,输出高电平或低电平
#define DS18B20_DATA_OUT(a)	if (a)	\
					GPIO_SetBits(GPIOA,GPIO_Pin_2);\
					else		\
					GPIO_ResetBits(GPIOA,GPIO_Pin_2)
 //读取引脚的电平
#define  DS18B20_DATA_IN()	   GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_2)

typedef struct
{
	uint8_t  humi_int;		//湿度的整数部分
	uint8_t  humi_deci;	 	//湿度的小数部分
	uint8_t  temp_int;	 	//温度的整数部分
	uint8_t  temp_deci;	 	//温度的小数部分
	uint8_t  check_sum;	 	//校验和
		                 
}DS18B20_Data_TypeDef;

uint8_t DS18B20_Init(void);
float DS18B20_Get_Temp(void);

void delay_init(uint8_t SYSCLK);

#endif

