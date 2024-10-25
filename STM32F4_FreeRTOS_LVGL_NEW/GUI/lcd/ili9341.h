#ifndef __Lcdlib__
#define __Lcdlib__

#include "lvgl.h"

extern uint16_t *XDown;
extern uint16_t *YDown;

/*--------------------------------------------------------------------------
// 文件名：Lcdlib.h
// 描述：  LCD库头文件
// 设计者：EU电子
// 创建日期：2013年10月16日
// 深圳EU电子出品-版权所有-翻版必究
// EU-热爱嵌入式开发
// http://euse.taobao.com
//-------------------------------------------------------------------------*/
typedef unsigned char       u8;
typedef uint16_t           u16;

/*-------------------------------------------------------------------------------------------------------
*  向外声明					 
-------------------------------------------------------------------------------------------------------*/
//颜色值
#define BLACK          0x0000   //黑

#define RED            0xF800   //红  
#define ORANGE 		   	 0xFB00   //橙
#define YELLOW         0xFFE0   //黄
#define GREEN          0x07E0   //绿
#define BLUE           0x001F   //蓝
#define PURPLE		   	 0x881F   //紫
#define GRAY  		     0X8430   //灰
#define WHITE          0xFFFF   //白

#define GOLDEN         0XBC40   //金
#define LIGHTBLUE      0x051F   //浅蓝
#define MAGENTA        0xF81F   //样红
#define CYAN           0x7FFF   //青
//----- RGB565原理 ------
//xxxxxxxx xxxxxxxx
//rrrrrggg gggbbbbb
//11111000 00000000 红 0xF800
//00000111 11100000 绿 0x07E0
//00000000 00011111 蓝 0x001F

//lcd direction
#define  LCD_DIRECT        1           		   //1 横屏  2 竖屏

//lcd bus chose
#define  LCD_BUSTYPE       1                 //1:16位总线  2:8位总线

/*-------------------------------------------------------------------------------------------------------
*  向外接口								 
-------------------------------------------------------------------------------------------------------*/
//------------------------- APP USER LIB -----------------------------
//lcd初始化
void LCD_ili9341Init(void);
//lcd清屏幕
void LCD_ClrScr(u16 BackColor);

//--------------------------------------------------------------------
//---------------- 屏幕左上角为原点,x指行, y指列 ---------------------
//--------------------------------------------------------------------
//------------------------- 点 -----------------------------
//在x0,y0位置显示一个颜色为Color点
void LCD_DisAPoint(u16 x0, u16 y0, u16 Color);
u16 LCD_ReadAPoint(u16 x0, u16 y0);

//------------------------- 线 -----------------------------
//在x0,y0位置显示一条直线(dir:横向1/纵向2),长度为lenth,颜色为color
void LCD_DisALine(u16 x0, u16 y0, u8 dir, u16 lenth, u16 color);
//在x0,y0 ~ 对角x1,y1,画一个颜色为Color的矩形边框
void LCD_DisABorder(u16 x0, u16 y0, u16 x1, u16 y1, u16 Color);
//在x0,y0位置画一个半径为r,颜色为Color的圆框
void LCD_DisALoop(u16 x0, u16 y0, u8 r, u16 Color);

//------------------------- 面 -----------------------------
//在x0,y0位置开始,显示一个宽度为wide,颜色为Color的正方形
void LCD_DisASquare(u16 x0, u16 y0, u16 wide, u16 Color);   
//在x0,y0 ~ x1,y1位置处,显示一个颜色为Color的长方形
void LCD_DisARectangular(u16 x0, u16 y0, u16 x1, u16 y1, u16 Color);

//--------------------- 英文字符 ---------------------------
//在x0,y0位置显示一个ASC字符,颜色为fColor,背景色为bColor
void LCD_DisASCString16x8(u16 x0, u16 y0, u8 *s, u16 fColor, u16 bColor);

//--------------------- 中文字符 ---------------------------
//在x0,y0位置显示一汉字字符串,颜色为fColor,背景色为bColor
void LCD_DisGB2312String16x16(u16 x0, u16 y0, u8 *s, u16 fColor, u16 bColor);
//在x0,y0位置显示一汉字字符串,颜色为fColor,背景色为bColor
void LCD_DisGB2312String32x32(u16 x0, u16 y0, u8 *s, u16 fColor, u16 bColor);

//------------------------ 图片 -----------------------------
//在x0,y0位置显示一副高为high,宽为wide的图片, 图片信息按RGB565组织(大端格式每个像素点占用2个字节)
//图片信息头地址指针为pData
void LCD_DisAPhoto(u16 x0, u16 y0, u16 high, u16 wide, u8 *pData);
void LVGL_LCD_DisARectangular(u16 x0, u16 y0, u16 x1, u16 y1, lv_color_t *Color);


//触摸数据采样处理 并转换为坐标
int Touch_GetSite(u16 *pRow, u16 *pColumn);
void Touch_GetVal(u16 *X, u16 *Y);


#endif



