#ifndef __Driver__
#define __Driver__
/*--------------------------------------------------------------------------
// �ļ�����Driver.h
// ������  ϵͳ����ͷ�ļ�
// ����ߣ�EU����
// �������ڣ�2013��11��10��
// ����EU���ӳ�Ʒ-��Ȩ����-����ؾ�
// EU-�Ȱ�Ƕ��ʽ����
// http://euse.taobao.com
//-------------------------------------------------------------------------*/

#include "stm32f4xx.h"


typedef unsigned char       u8;
typedef unsigned short      u16;

/*-------------------------------------------------------------------------------------------------------
*  ����ӿ�	ΪӦ�ò��ṩ�Ľӿ�										 
-------------------------------------------------------------------------------------------------------*/
//mcu
void Driver_MCU_Init(void);
void SysTICK_SET(void);
//lcd
void Driver_LcdBacklight(unsigned char sta);
void Driver_LcdReset(unsigned char sta);
//spi
void Driver_SPION(void);
void Driver_SPIOFF(void);
void Driver_SPICSEN(void);
void Driver_SPICSUN(void);
void Driver_SPISend(u8 Data);
u8 Driver_SPIRecv(void);
//fsmc
void Driver_LcdFSMCON(void);
void Driver_LcdFSMCWriteReg(u16 Index);
void Driver_LcdFSMCWriteData(u16 Data);
u16 Driver_LcdFSMCReadData(void);
//Touch
void Driver_SPICS(unsigned char sta);
void Driver_SPISCK(unsigned char sta);
void Driver_SPIMOSI(unsigned char sta);
unsigned char Driver_SPIMISO(void);
unsigned char Driver_TINT(void);

//sd card
extern void Driver_SDIOInit(void);

#endif



