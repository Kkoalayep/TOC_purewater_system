#include "svo.h"

uint8_t svo_state = 0;
void SVO_Init(void)
{
  GPIO_InitTypeDef  GPIO_InitStructure;
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);//使能GPIOA时钟
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;//普通输出模式
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;//推挽输出
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;//100MHz
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_DOWN;//下拉
  GPIO_Init(GPIOA, &GPIO_InitStructure);//初始化
	GPIO_ResetBits(GPIOA,GPIO_Pin_6);//GPIOA6设置低，关闭
}

void SVO_Console(uint8_t state)
{
	if(state == 0)
	{
		GPIO_ResetBits(GPIOA,GPIO_Pin_6);
	}
	if(state == 1)
	{
		GPIO_SetBits(GPIOA,GPIO_Pin_6);
	}
}

