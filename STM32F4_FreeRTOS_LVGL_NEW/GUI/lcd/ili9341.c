#include "driver.h"
#include "ili9341.h"

uint16_t X_Down;
uint16_t Y_Down;

uint16_t * XDown = &X_Down;
uint16_t * YDown = &Y_Down;

















/*-------------------------------------------------------------------------------------------------------
*  �ڲ�����								 
-------------------------------------------------------------------------------------------------------*/
//lcd drivers
#define	 LCD_RESET_H  		Driver_LcdReset(1)
#define	 LCD_RESET_L  		Driver_LcdReset(0)

//lcd resolution 
#if LCD_DIRECT==1
	#define  LCD_ROW_NUM    320                //����
	#define  LCD_COL_NUM    240                //����
#else
	#define  LCD_ROW_NUM    320                //����
	#define  LCD_COL_NUM    240                //����
#endif

/*-------------------------------------------------------------------------------------------------------
*  ִ�д���													 
-------------------------------------------------------------------------------------------------------*/
/********************************************************************************************************
*  Function: LCD_WriteReg				                                                           
*  Object: lcd write reg
*  Input: index
*  Output: none                                  
*  brief: none
********************************************************************************************************/
void LCD_WriteReg(u16 Index)
{
		Driver_LcdFSMCWriteReg(Index);
}

/********************************************************************************************************
*  Function: LCD_WriteData				                                                           
*  Object: lcd write data
*  Input: index and data
*  Output: none                                  
*  brief: none
********************************************************************************************************/
void LCD_WriteData(u16 Data)
{
		Driver_LcdFSMCWriteData(Data);
}

/********************************************************************************************************
*  Function: LCD_ReadData				                                                           
*  Object: lcd read data
*  Input: none
*  Output: data                                  
*  brief: none
********************************************************************************************************/
u16 LCD_ReadData(void)
{
		return Driver_LcdFSMCReadData();
}

/********************************************************************************************************
*  Function: LCD_Delayms				                                                           
*  Object: lcd init wait..
*  Input: Num
*  Output: none                                  
*  brief: time = Num * 1ms
********************************************************************************************************/
void LCD_Delayms(u8 Num)
{
		u16 Timer;
		while(Num--)
		{
		 	Timer = 5000;
			while(Timer--); 
		}
}

/********************************************************************************************************
*  Function: LCD_PortInit				                                                           
*  Object: lcd port init
*  Input: none
*  Output: none                                  
*  brief: none
********************************************************************************************************/
void LCD_PortInit(void)
{
		Driver_LcdFSMCON();
}

/********************************************************************************************************
*  Function: LCD_Reset				                                                           
*  Object: lcd reset control
*  Input: none
*  Output: none                                  
*  brief: none
********************************************************************************************************/
void LCD_Reset(void)
{
		LCD_RESET_L;
		LCD_Delayms(150);
		LCD_RESET_H;
		LCD_Delayms(50);
}

/********************************************************************************************************
*  Function: Driver_LcdReset				                                                           
*  Object: lcd reset control
*  Input: sta
*  Output: none                                  
*  brief: none
********************************************************************************************************/
void LCD_ili9341Init(void)
{ 
		//-- LCD PORT INIT --
		LCD_PortInit();
		//-- LCD RESET--
		//LCD_Reset();
		//-------------- Initial Sequence ---------------
		//************* Start Initial Sequence **********//	
		LCD_WriteReg(0xCF);  
		LCD_WriteData(0x00); 
		LCD_WriteData(0xC1); 
		LCD_WriteData(0X30); 
		LCD_WriteReg(0xED);  
		LCD_WriteData(0x64); 
		LCD_WriteData(0x03); 
		LCD_WriteData(0X12); 
		LCD_WriteData(0X81); 
		LCD_WriteReg(0xE8);  
		LCD_WriteData(0x85); 
		LCD_WriteData(0x10); 
		LCD_WriteData(0x7A); 
		LCD_WriteReg(0xCB);  
		LCD_WriteData(0x39); 
		LCD_WriteData(0x2C); 
		LCD_WriteData(0x00); 
		LCD_WriteData(0x34); 
		LCD_WriteData(0x02); 
		LCD_WriteReg(0xF7);  
		LCD_WriteData(0x20); 
		LCD_WriteReg(0xEA);  
		LCD_WriteData(0x00); 
		LCD_WriteData(0x00); 
		LCD_WriteReg(0xC0);    //Power control 
		LCD_WriteData(0x1B);   //VRH[5:0] 
		LCD_WriteReg(0xC1);    //Power control 
		LCD_WriteData(0x01);   //SAP[2:0];BT[3:0] 
		LCD_WriteReg(0xC5);    //VCM control 
		LCD_WriteData(0x30); 	 //3F
		LCD_WriteData(0x30); 	 //3C
		LCD_WriteReg(0xC7);    //VCM control2 
		LCD_WriteData(0XB7); 
		LCD_WriteReg(0x36);    // Memory Access Control 
		LCD_WriteData(0x48); 
		LCD_WriteReg(0x3A);   
		LCD_WriteData(0x55); 
		LCD_WriteReg(0xB1);   
		LCD_WriteData(0x00);   
		LCD_WriteData(0x1A); 
		LCD_WriteReg(0xB6);    // Display Function Control 
		LCD_WriteData(0x0A); 
		LCD_WriteData(0xA2); 
		LCD_WriteReg(0xF2);    // 3Gamma Function Disable 
		LCD_WriteData(0x00); 
		LCD_WriteReg(0x26);    //Gamma curve selected 
		LCD_WriteData(0x01); 
		LCD_WriteReg(0xE0);    //Set Gamma 
		LCD_WriteData(0x0F); 
		LCD_WriteData(0x2A); 
		LCD_WriteData(0x28); 
		LCD_WriteData(0x08); 
		LCD_WriteData(0x0E); 
		LCD_WriteData(0x08); 
		LCD_WriteData(0x54); 
		LCD_WriteData(0XA9); 
		LCD_WriteData(0x43); 
		LCD_WriteData(0x0A); 
		LCD_WriteData(0x0F); 
		LCD_WriteData(0x00); 
		LCD_WriteData(0x00); 
		LCD_WriteData(0x00); 
		LCD_WriteData(0x00); 		 
		LCD_WriteReg(0XE1);    //Set Gamma 
		LCD_WriteData(0x00); 
		LCD_WriteData(0x15); 
		LCD_WriteData(0x17); 
		LCD_WriteData(0x07); 
		LCD_WriteData(0x11); 
		LCD_WriteData(0x06); 
		LCD_WriteData(0x2B); 
		LCD_WriteData(0x56); 
		LCD_WriteData(0x3C); 
		LCD_WriteData(0x05); 
		LCD_WriteData(0x10); 
		LCD_WriteData(0x0F); 
		LCD_WriteData(0x3F); 
		LCD_WriteData(0x3F); 
		LCD_WriteData(0x0F); 
		LCD_WriteReg(0x2B); 
		LCD_WriteData(0x00);
		LCD_WriteData(0x00);
		LCD_WriteData(0x01);
		LCD_WriteData(0x3f);
		LCD_WriteReg(0x2A); 
		LCD_WriteData(0x00);
		LCD_WriteData(0x00);
		LCD_WriteData(0x00);
		LCD_WriteData(0xef);	 
		LCD_WriteReg(0x11); //Exit Sleep
		LCD_Delayms(120);
		LCD_WriteReg(0x29); //display on
   	LCD_WriteReg(0x36);  
		LCD_WriteData(0xC9);
		
		
		Driver_LcdBacklight(1);
		
}

/********************************************************************************************************
*  Function: LCD_OpenWin				                                                           
*  Object: lcd open window for display
*  Input: x0,y0, x1, y1
*  Output: none                                  
*  brief: none
********************************************************************************************************/
void LCD_OpenWin(u16 x0, u16 y0, u16 x1, u16 y1)
{
		LCD_WriteReg(0x2A);	
		LCD_WriteData(y0>>8);
		LCD_WriteData(0x00FF&y0);		
		LCD_WriteData(y1>>8);
		LCD_WriteData(0x00FF&y1);
		LCD_WriteReg(0x2B);	
		LCD_WriteData(x0>>8);
		LCD_WriteData(0x00FF&x0);		
		LCD_WriteData(x1>>8);
		LCD_WriteData(0x00FF&x1);
		LCD_WriteReg(0x2C);
}

/********************************************************************************************************
*  Function: LCD_ClrScr				                                                           
*  Object: lcd clear screen
*  Input: backcolor
*  Output: none                                  
*  brief: none
********************************************************************************************************/
void LCD_ClrScr(u16 BackColor)
{
		u16 i,j;
		LCD_OpenWin(0, 0, LCD_ROW_NUM-1, LCD_COL_NUM-1);
		for(i = 0; i < LCD_ROW_NUM; i++)
			 for(j =0; j < LCD_COL_NUM; j++)
					 LCD_WriteData(BackColor);
}

/********************************************************************************************************
*  Function: LCD_DisAPoint				                                                           
*  Object: Display a point at screen
*  Input: site and color
*  Output: none                                  
*  brief: none
********************************************************************************************************/
void LCD_DisAPoint(u16 x0, u16 y0, u16 Color)
{
		LCD_DisASquare(x0, y0, 1, Color);
}

/********************************************************************************************************
*  Function: LCD_ReadAPoint				                                                           
*  Object: Display a point at screen
*  Input: site and color
*  Output: none                                  
*  brief: none
********************************************************************************************************/
u16 LCD_ReadAPoint(u16 x0, u16 y0)
{
		u16 r,g,b;
		LCD_OpenWin(x0, y0, x0, y0);
		LCD_WriteReg(0x2e);
		r = LCD_ReadData();
	
		r = LCD_ReadData();
		b = LCD_ReadData();
		g = r&0XFF;
		g<<=8;
	
		return (((r>>11)<<11)|((g>>10)<<5)|(b>>11));
}

/********************************************************************************************************
*  Function: LCD_DisALine				                                                           
*  Object: Display a line
*  Input: site dir lenth wide color
*  Output: none                                  
*  brief: none
********************************************************************************************************/
void LCD_DisALine(u16 x0, u16 y0, u8 dir, u16 lenth, u16 color)
{
		u16 x1,y1;
		x1 = x0;
		y1 = y0;
		if(dir == 1)
			y1 = y1 + lenth;
		else
			x1 = x1 + lenth;
		LCD_DisARectangular(x0, y0, x1, y1, color);
}

/********************************************************************************************************
*  Function: LCD_DisABorder				                                                           
*  Object: Display a border
*  Input: x0, y0, x1, y1, color
*  Output: none                                  
*  brief: none
********************************************************************************************************/
void LCD_DisABorder(u16 x0, u16 y0, u16 x1, u16 y1, u16 Color)
{
		LCD_DisALine(x0, y0, 1, y1-y0, Color);
		LCD_DisALine(x1, y0, 1, y1-y0, Color);
		LCD_DisALine(x0, y0, 2, x1-x0, Color);
		LCD_DisALine(x0, y1, 2, x1-x0, Color);
}

/********************************************************************************************************
*  Function: LCD_DisALoop				                                                           
*  Object: Display a loop
*  Input: site,radius and color
*  Output: none                                  
*  brief: none
********************************************************************************************************/
void LCD_DisALoop(u16 x0, u16 y0, u8 r, u16 Color)
{
		s16 a,b,next;
		a	=	0;
		b = r;	  
		next = 3 - (r<<1);            
		while(a <= b)
		{
				LCD_DisAPoint(x0+a, y0-b, Color);             
				LCD_DisAPoint(x0+b, y0-a, Color);                      
				LCD_DisAPoint(x0+b, y0+a, Color);                          
				LCD_DisAPoint(x0+a, y0+b, Color);             
				LCD_DisAPoint(x0-a, y0+b, Color);                  
				LCD_DisAPoint(x0-b, y0+a, Color);             
				LCD_DisAPoint(x0-a, y0-b, Color);                          
				LCD_DisAPoint(x0-b, y0-a, Color);              	         
				a++;
				//use the bresenham    
				if(next<0)
					next += 4*a+6;	  
				else
				{
						next += 10+4*(a-b);   
						b--;
				} 						    
		}
} 

/********************************************************************************************************
*  Function: LCD_DisASquare				                                                           
*  Object: Display a square
*  Input: start point, wide, color
*  Output: none                                  
*  brief: none
********************************************************************************************************/
void LCD_DisASquare(u16 x0, u16 y0, u16 wide, u16 Color)
{
		u16 i,j;
		LCD_OpenWin(320-x0, y0, 320-x0+wide-1, y0+wide-1);
		for(i = 0; i < wide; i++)
			for(j = 0; j < wide; j++)
					 LCD_WriteData(Color);
}

/********************************************************************************************************
*  Function: LCD_DisARectangular				                                                           
*  Object: Display a rectangular
*  Input: start point, end point, color
*  Output: none                                  
*  brief: none
********************************************************************************************************/
void LCD_DisARectangular(u16 x0, u16 y0, u16 x1, u16 y1, u16 Color)
{
		u16 i,j;
		LCD_OpenWin(x0, y0, x1, y1);
		for(i = 0; i <= x1-x0; i++)
			 for(j = 0; j <= y1-y0; j++)
					 LCD_WriteData(Color);
}

void LVGL_LCD_DisARectangular(u16 x0, u16 y0, u16 x1, u16 y1, lv_color_t *Color)
{
		u16 i,j;
		LCD_OpenWin(x0, y0, x1, y1);
		for(i = 0; i <= x1-x0; i++)
			 for(j = 0; j <= y1-y0; j++)
					 LCD_WriteData(Color->full);
}

/********************************************************************************************************
*  Function: LCD_DisAPhoto				                                                           
*  Object: display a photo
*  Input: Site(x0,y0), high and wide, pData
*  Output: none                                  
*  brief: none
********************************************************************************************************/
void LCD_DisAPhoto(u16 x0, u16 y0, u16 high, u16 wide, u8 *pData)
{
		u32 i,lenth;
		lenth = high * wide * 2;  		//RGB565 每一像素点占用两个字节
		LCD_OpenWin(x0, y0, x0+high-1, y0+wide-1);
		for(i = 0; i < lenth; i+=2)
			LCD_WriteData((*(pData+i))*0x100 + (*(pData+i+1)));
}

/*-------------------------------------------------------------------------------------------------------
*  触摸屏校正代码			 
-------------------------------------------------------------------------------------------------------*/
//屏幕分辨率
#define LCD_ROWNUM 320
#define LCD_COLUMNNUM 240

//X Y值类型声明
enum{
	X_TYPE = 1,
	Y_TYPE = 2
};
//数值增减类型
enum{
	NUM_ADD = 1,
	NUM_SUBB = 2
};

//----------- 经调校后 获得这部分的参数 可进行写参数保存起来 ---------
//随着ROW行值增加 触摸值变化
//是X还是Y变化
u8 RowChgXYType;
//数值是变大还是变小
u8 RowChgNumType;
//范围值是多少
u16 RowChgNumMAX;
u16 RowChgNumMIN;

//随着COLUMN列值增加 触摸值变化
//是X还是Y变化
u8 ColumnChgXYType;
//数值是变大还是变小
u8 ColumnChgNumType;
//范围值是多少
u16 ColumnChgNumMAX;
u16 ColumnChgNumMIN;
//------------------------------------------------------------------

//导入经校准过的触摸屏幕 结果值
//！！如觉得不够准确 请执行屏幕触摸调校 然后将调校显示结果 赋值给下方变量
void LoadTouchFaceVal(void)
{
		//EU-TFT屏幕类型
		enum{
			TFT32_1,      //EU-TFT32第1批屏幕
			TFT32_2       //EU-TFT32第2批屏幕 现在最新为这个
		};
		//定义使用的屏幕类型
		u8 TFTType = TFT32_2;  
		//根据屏幕类型进行参数配置
		if(TFTType == TFT32_1)
		{
				//ROW
				RowChgXYType = 2;
				RowChgNumType = 1;
				RowChgNumMAX = 3935;  
				RowChgNumMIN = 406;  
				//COLUMN
				ColumnChgXYType = 1;
				ColumnChgNumType = 2;
				ColumnChgNumMAX = 3895; 
				ColumnChgNumMIN = 146; 
		}
		else if(TFTType == TFT32_2)
		{
				//ROW
				RowChgXYType = 2;
				RowChgNumType = 1;
				RowChgNumMAX = 3931;  
				RowChgNumMIN = 360; 
				//COLUMN
				ColumnChgXYType = 1;
				ColumnChgNumType = 1;
				ColumnChgNumMAX = 3918; 
				ColumnChgNumMIN = 158; 
		}
}

//通过触摸AD获得的X Y值, 计算屏幕所指向的行列位置值
//函数返回1 表示获得有效的位置 返回0 表示触摸位置无效 或者空按
bool GetScrSite(u16 X, u16 Y, u16 *Row, u16 *Column)
{
		if(RowChgXYType == X_TYPE)
		{
				if((X > RowChgNumMIN && X < RowChgNumMAX) && (Y > ColumnChgNumMIN && Y < ColumnChgNumMAX))
				{
						//计算ROW
						if(RowChgNumType == NUM_ADD)
						{
								*Row = (X-RowChgNumMIN)/((RowChgNumMAX - RowChgNumMIN)/LCD_ROWNUM);
						}
						else
						{
								*Row = (X-RowChgNumMIN)/((RowChgNumMAX - RowChgNumMIN)/LCD_ROWNUM);
								*Row = LCD_ROWNUM - *Row;
						}
						//计算COLUMN
						if(ColumnChgNumType == NUM_ADD)
						{
								*Column = (Y-ColumnChgNumMIN)/((ColumnChgNumMAX - ColumnChgNumMIN)/LCD_COLUMNNUM);
						}
						else
						{
								*Column = (Y-ColumnChgNumMIN)/((ColumnChgNumMAX - ColumnChgNumMIN)/LCD_COLUMNNUM);
								*Column = LCD_COLUMNNUM - *Column;
						}
						return 1;
				}
				return 0;
		}
		else
		{
				if((Y > RowChgNumMIN && Y < RowChgNumMAX) && (X > ColumnChgNumMIN && X < ColumnChgNumMAX))
				{
						//计算ROW
						if(RowChgNumType == NUM_ADD)
						{
								*Row = (Y-RowChgNumMIN)/((RowChgNumMAX - RowChgNumMIN)/LCD_ROWNUM);
						}
						else
						{
								*Row = (Y-RowChgNumMIN)/((RowChgNumMAX - RowChgNumMIN)/LCD_ROWNUM);
								*Row = LCD_ROWNUM - *Row;
						}
						//计算COLUMN
						if(ColumnChgNumType == NUM_ADD)
						{
								*Column = (X-ColumnChgNumMIN)/((ColumnChgNumMAX - ColumnChgNumMIN)/LCD_COLUMNNUM);
						}
						else
						{
								*Column = (X-ColumnChgNumMIN)/((ColumnChgNumMAX - ColumnChgNumMIN)/LCD_COLUMNNUM);
								*Column = LCD_COLUMNNUM - *Column;
						}
						return 1;
				}
				return 0;
		}
}

/*-------------------------------------------------------------------------------------------------------
*  触摸IC检测代码					 
-------------------------------------------------------------------------------------------------------*/
//往SPI接口发送数据

#define CHSBIT7  0x0080

void Touch_SPIWrite(u8 Val)
{
    u8 i;
    Driver_SPISCK(0);
    for(i=0; i<8; i++)
		{
				if(Val&CHSBIT7)
					Driver_SPIMOSI(1);
				else
					Driver_SPIMOSI(0);
				Val <<= 1;
        Driver_SPISCK(0);
        __nop();
				__nop();
				__nop();
        Driver_SPISCK(1);
        __nop();
				__nop();
				__nop();
    }
}

//从SPI接口接收数据
u16 Touch_SPIRead(void)
{
    u8 i;
    u16 Val = 0;
    for(i=0; i<12; i++)
		{
        Val <<= 1;
        Driver_SPISCK(1);
        __nop();
				__nop();
				__nop();
        Driver_SPISCK(0);
        __nop();
				__nop();
				__nop();
        if(Driver_SPIMISO())
					Val++;
    }
    return Val;
}

//读一次触摸检测值
void Touch_GetVal(u16 *X, u16 *Y)
{
    Driver_SPICS(0);
    Touch_SPIWrite(0x90);   //读取Y轴 检测值
    Driver_SPISCK(1);
    __nop();
		__nop();
		__nop();
    Driver_SPISCK(0);
    __nop();
		__nop();
		__nop();
    *Y = Touch_SPIRead();
    Touch_SPIWrite(0xd0);  //读取X轴 检测值
    Driver_SPISCK(1);
    __nop();
		__nop();
		__nop();
    Driver_SPISCK(0);
    __nop();
		__nop();
		__nop();
    *X = Touch_SPIRead();
    Driver_SPICS(1);
}
static bool Flag = 1;
//触摸数据采样处理 并转换为坐标
int Touch_GetSite(u16 *pRow, u16 *pColumn)
{
		u16 TouchPointX, TouchPointY;
		
		if(Flag)
		{
				//首次运行先导入触摸屏幕校正值
				LoadTouchFaceVal();
				Flag = 0;
		}
		Touch_GetVal(&TouchPointX, &TouchPointY);   /* 获取触摸X Y值 */
	  if(GetScrSite(TouchPointX, TouchPointY, pRow, pColumn))  /* 将触摸X Y值转化为屏幕行列值 */
		{	
			*pRow = *pRow;
			return 1;
		}
		else
		{
			return 0;
		}
}

