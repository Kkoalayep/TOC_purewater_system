#include "led.h"

uint8_t led_state = 0;

void LED_Init(void)
{
  GPIO_InitTypeDef  GPIO_InitStructure;
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);//使能GPIOF时钟
  //GPIOF9,F10初始化设置
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;//普通输出模式
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;//推挽输出
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;//100MHz
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_DOWN;//下拉
  GPIO_Init(GPIOA, &GPIO_InitStructure);//初始化
	GPIO_ResetBits(GPIOA,GPIO_Pin_3);//GPIOA4设置低，关闭
}

void LED_Console(uint8_t state)
{
	if(state == 0)
	{
		GPIO_ResetBits(GPIOA,GPIO_Pin_3);
	}
	if(state == 1)
	{
		GPIO_SetBits(GPIOA,GPIO_Pin_3);
	}
}

