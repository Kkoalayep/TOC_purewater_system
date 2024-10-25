#ifndef __TOC_SENSOR_H__
#define __TOC_SENSOR_H__
#include "stm32f4xx.h"
#include "ds18b20.h"

#define PRE_PORT     GPIOA                   //GPIO端口
#define PRE_PIN      GPIO_Pin_1              //GPIO引脚
#define PRE_APB_RCC  RCC_APB2Periph_ADC1     //ADC时钟
#define PRE_AHB_RCC  RCC_AHB1Periph_GPIOA    //GPIOA时钟
#define ADCx         ADC1
#define ADCx_CH      1

//Toc
#define RES2 820.0   //运放电阻，与硬件电路有关
#define ECREF 200.0 //极片输入电压，与硬件电路相关
void Adc_Toc_Init(void); 	//ADC通道初始化
void Get_Toc_Adc(void); 	//获得某个通道值 
void Get_EC_value(uint8_t count);

void EC_Start_Get(void);
void EC_Stop_Get(void);
void TOC_Get(void);


extern uint16_t toc_adc_value;
extern float toc_value;
extern float EC_Start_value;
extern float EC_Stop_value;



extern float temp_value;

#endif


