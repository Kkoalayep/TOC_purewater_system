#ifndef __Driver__
#define __Driver__
/*--------------------------------------------------------------------------
// 文件名：Driver.h
// 描述：  系统驱动头文件
// 设计者：EU电子
// 创建日期：2013年11月10日
// 深圳EU电子出品-版权所有-翻版必究
// EU-热爱嵌入式开发
// http://euse.taobao.com
//-------------------------------------------------------------------------*/

#include "stm32f4xx.h"


typedef unsigned char       u8;
typedef unsigned short      u16;

/*-------------------------------------------------------------------------------------------------------
*  向外接口	为应用层提供的接口										 
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



