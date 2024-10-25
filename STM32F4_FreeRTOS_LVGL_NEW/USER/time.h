#ifndef __TIME_H__
#define __TIME_H__
#include "stm32f4xx.h"
#include "stdio.h"
#include "string.h"

extern uint8_t rtc_time[40];
extern uint8_t rtc_date[40];
extern uint8_t rtc_start[40];

uint8_t My_RTC_Init(void);						//RTC初始化
ErrorStatus RTC_Set_Time(uint8_t hour,uint8_t min,uint8_t sec,uint8_t ampm);			//RTC时间设置
ErrorStatus RTC_Set_Date(uint8_t year,uint8_t month,uint8_t date,uint8_t week); 		//RTC日期设置
void RTC_Set_AlarmA(uint8_t week,uint8_t hour,uint8_t min,uint8_t sec);		//设置闹钟时间(按星期闹铃,24小时制)
void RTC_Set_WakeUp(uint32_t wksel,uint16_t cnt);					//周期性唤醒定时器设置
void My_RTC_Get(void);//获取时间到BUF备用



#endif

