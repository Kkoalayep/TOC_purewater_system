/*--------------------------------------------------------------------------
// 文件名：uart.c
// 描述：  usart 驱动 
// 设计者：EU电子
// 深圳EU电子出品-版权所有-翻版必究
// EU-热爱嵌入式开发
// http://euse.taobao.com
//-------------------------------------------------------------------------*/
/* INCLUDES */
#include "Prohead.h"
#include "Driver.h"
#include "uart.h"

//开串口 并执行初始化
//8位数据位 无校验 1位起始位/1位停止位 允许收发中断	宏定义BAUDRATE设定波特率 低优先级中断 
void UARTON(void)
{

		//波特率设置
		#define	 UART_BAUDDEF  9600
		GPIO_InitTypeDef    GPIO_InitStructure;  
		USART_InitTypeDef   USART_InitStructure; 

		//使能串口时钟
		RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);

		GPIO_PinAFConfig(GPIOA,GPIO_PinSource9,GPIO_AF_USART1);//使能复用功能USART1
		GPIO_PinAFConfig(GPIOA,GPIO_PinSource10,GPIO_AF_USART1);
		//管脚配置
		GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;			
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;		
		GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;	//推挽复用输出
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;   
		GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;		//使能上拉
		GPIO_Init(GPIOA, &GPIO_InitStructure);			/* TXIO */	
	
		GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;			 	
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;   
		GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;		
		GPIO_Init(GPIOA, &GPIO_InitStructure); 			/* RXIO */	

		//串口工作模式配置
		USART_InitStructure.USART_BaudRate = UART_BAUDDEF;								
		USART_InitStructure.USART_WordLength = USART_WordLength_8b; 				 	
		USART_InitStructure.USART_StopBits = USART_StopBits_1;							
		USART_InitStructure.USART_Parity = USART_Parity_No ; 						
		USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None; 
		USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx; 
		USART_Init(USART1, &USART_InitStructure);	
		USART_Cmd(USART1, ENABLE);	
}

/*******************************************************************************
* Function Name  : fputc
* Description    : Retargets the C library printf function to the USART.
* Input          : int ch, FILE *f
* Output         : None
* Return         : None
* 备注： 需要调用此函数，要把 Target 中的 Use Micro LIB 勾上，不然会出错
*******************************************************************************/
int fputc(int ch, FILE *f)
{
  /* Place your implementation of fputc here */
  /* e.g. write a character to the USART */
  USART_SendData(USART1, (u8) ch);

  /* Loop until the end of transmission */
  while(USART_GetFlagStatus(USART1, USART_FLAG_TC) == RESET)
  {
  }

  return ch;
}

