/*--------------------------------------------------------------------------
// �ļ�����uart.c
// ������  usart ���� 
// ����ߣ�EU����
// ����EU���ӳ�Ʒ-��Ȩ����-����ؾ�
// EU-�Ȱ�Ƕ��ʽ����
// http://euse.taobao.com
//-------------------------------------------------------------------------*/
/* INCLUDES */
#include "Prohead.h"
#include "Driver.h"
#include "uart.h"

//������ ��ִ�г�ʼ��
//8λ����λ ��У�� 1λ��ʼλ/1λֹͣλ �����շ��ж�	�궨��BAUDRATE�趨������ �����ȼ��ж� 
void UARTON(void)
{

		//����������
		#define	 UART_BAUDDEF  9600
		GPIO_InitTypeDef    GPIO_InitStructure;  
		USART_InitTypeDef   USART_InitStructure; 

		//ʹ�ܴ���ʱ��
		RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);

		GPIO_PinAFConfig(GPIOA,GPIO_PinSource9,GPIO_AF_USART1);//ʹ�ܸ��ù���USART1
		GPIO_PinAFConfig(GPIOA,GPIO_PinSource10,GPIO_AF_USART1);
		//�ܽ�����
		GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;			
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;		
		GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;	//���츴�����
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;   
		GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;		//ʹ������
		GPIO_Init(GPIOA, &GPIO_InitStructure);			/* TXIO */	
	
		GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;			 	
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;   
		GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;		
		GPIO_Init(GPIOA, &GPIO_InitStructure); 			/* RXIO */	

		//���ڹ���ģʽ����
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
* ��ע�� ��Ҫ���ô˺�����Ҫ�� Target �е� Use Micro LIB ���ϣ���Ȼ�����
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

