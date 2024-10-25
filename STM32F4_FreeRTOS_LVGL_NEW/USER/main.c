#include "stm32f4xx.h"
#include "FreeRTOS.h"
#include "task.h"
#include "Driver.h"
#include "ili9341.h"
#include "lvgl.h"
#include "lv_port_disp_template.h"
#include "lv_port_indev_template.h"
#include "gui_guider.h"
#include "events_init.h"
#include "stdio.h"
#include "string.h"
#include "stdlib.h"

#include "SDcard.h"

#include "time.h"
#include "light.h"
#include "motor.h"
#include "svo.h"
#include "svt.h"
#include "led.h"
#include "toc_sensor.h"

#include "uart.h"
#include "console_logic.h"

//#include "prohead.h"
//#include "diskio.h"
//#include "ff.h"

#define MOTOR_DELAY_S  10            //蠕动泵开启时间单位s
#define LIGHT_DELAY_S  20  //3*60    //紫外灯开启时间单位s
#define CLEAR_DELAY_S  10            //清洗模式时间单位s

//FATFS  FileSys; //file system object
//FIL    File;//File object
//FRESULT  FileResult; //File function return code (FRESULT)
//UINT NbCount ; //file number count 
//__align(4) u8 TxBuffer[512] = "你好测试";

//void DIS_Delayms(uint16_t Num)
//{
//   	uint16_t Timer;
//		while(Num--)
//		{
//		 	Timer = 25000;
//			while(Timer--); 
//		}
//}

lv_ui guider_ui;

unsigned char touch_state_val = 0;
char display_str[20] = {0};

uint8_t sdcard_state = 0;	

void vTask1(void *pvParameters)   //翻转LED0
{
	GPIO_InitTypeDef GPIO_InitStructure;
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOF, ENABLE);
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;//LED1对应IO口
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;//普通输出模式
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;//推挽输出
  GPIO_InitStructure.GPIO_Speed = GPIO_Low_Speed;//100MHz
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_DOWN;//上拉
  GPIO_Init(GPIOF, &GPIO_InitStructure);
	while(1)
	{
		GPIO_SetBits(GPIOF,GPIO_Pin_9);
		vTaskDelay(300);
		GPIO_ResetBits(GPIOF,GPIO_Pin_9);
		vTaskDelay(300);
	}
}

void vTask2(void *pvParameters)   //翻转LED1
{
	GPIO_InitTypeDef GPIO_InitStructure;
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;//LED1对应IO口
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;//普通输出模式
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;//推挽输出
  GPIO_InitStructure.GPIO_Speed = GPIO_Low_Speed;//100MHz
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_DOWN;//上拉
  GPIO_Init(GPIOF, &GPIO_InitStructure);
	while(1)
	{
		GPIO_SetBits(GPIOF,GPIO_Pin_10);
		vTaskDelay(500);
		GPIO_ResetBits(GPIOF,GPIO_Pin_10);
		vTaskDelay(500);
	}
}

void vTask3(void *pvParameters)  //LVGL线程，ADC获取
{
	LCD_ili9341Init();                              //屏幕初始化
	

	
	lv_init();
	lv_port_disp_init();
	lv_port_indev_init();
	
	
	setup_ui(&guider_ui);
	events_init(&guider_ui);
	printf(" test hello world\r\n");
	while(1)
	{
		Touch_GetSite(XDown, YDown);
		//My_RTC_Get();
		
		DS18B20_Get_Temp();
		
		
		console_logic_run(motor_state,light_state,svo_state,svt_state,oxidize_state,clear_state);
		
		
		memset(display_str,0,sizeof(display_str)/sizeof(display_str[0]));
	  sprintf(display_str,"%.1f",temp_value);
	  lv_obj_set_style_text_font(guider_ui.screen_temp_lable, &lv_font_simfang_24, 0);
	  lv_label_set_text(guider_ui.screen_temp_lable, display_str);
		
		memset(display_str,0,sizeof(display_str)/sizeof(display_str[0]));
	  sprintf(display_str,"%.2f",EC_Start_value);
	  lv_obj_set_style_text_font(guider_ui.screen_start_conductivity_lable, &lv_font_simfang_24, 0);
	  lv_label_set_text(guider_ui.screen_start_conductivity_lable, display_str);
		
		memset(display_str,0,sizeof(display_str)/sizeof(display_str[0]));
	  sprintf(display_str,"%.2f",EC_Stop_value);
	  lv_obj_set_style_text_font(guider_ui.screen_end_conductivity_lable, &lv_font_simfang_24, 0);
	  lv_label_set_text(guider_ui.screen_end_conductivity_lable, display_str);
		
		memset(display_str,0,sizeof(display_str)/sizeof(display_str[0]));
	  sprintf(display_str,"%.2f",toc_value);
	  lv_obj_set_style_text_font(guider_ui.screen_toc_lable, &lv_font_simfang_24, 0);
	  lv_label_set_text(guider_ui.screen_toc_lable, display_str);
		
		
		if(oxidize_state == 1)  //oxidize mode
		{
			lv_obj_set_style_text_font(guider_ui.screen_oxidize_state_lable, &lv_font_simfang_24, 0);
			lv_label_set_text(guider_ui.screen_oxidize_state_lable, "运行中");
			
		}
		if(oxidize_state == 0)
		{
			lv_obj_set_style_text_font(guider_ui.screen_oxidize_state_lable, &lv_font_simfang_24, 0);
			lv_label_set_text(guider_ui.screen_oxidize_state_lable, "已停止");
		}
		
		if(clear_state == 1)  //clear mode
		{
			lv_obj_set_style_text_font(guider_ui.screen_clear_state_lable, &lv_font_simfang_24, 0);
			lv_label_set_text(guider_ui.screen_clear_state_lable, "运行中");
			
		}
		if(clear_state == 0)
		{
			lv_obj_set_style_text_font(guider_ui.screen_clear_state_lable, &lv_font_simfang_24, 0);
			lv_label_set_text(guider_ui.screen_clear_state_lable, "已停止");
		}
	
		if(motor_state == 1)  //motor
		{
			lv_obj_set_style_text_font(guider_ui.screen_motor_state_lable, &lv_font_simfang_24, 0);
			lv_label_set_text(guider_ui.screen_motor_state_lable, "开");
			lv_obj_add_state(guider_ui.screen_motor_sw, LV_STATE_CHECKED);
			
		}
		if(motor_state == 0)
		{
			lv_obj_set_style_text_font(guider_ui.screen_motor_state_lable, &lv_font_simfang_24, 0);
			lv_label_set_text(guider_ui.screen_motor_state_lable, "关");
			lv_obj_clear_state(guider_ui.screen_motor_sw, LV_STATE_CHECKED);
		}
		
		
		
		if(light_state == 1)  //light
		{
			lv_obj_set_style_text_font(guider_ui.screen_light_state_lable, &lv_font_simfang_24, 0);
			lv_label_set_text(guider_ui.screen_light_state_lable, "开");
			lv_obj_add_state(guider_ui.screen_light_sw, LV_STATE_CHECKED);
		}
		if(light_state == 0)
		{
			lv_obj_set_style_text_font(guider_ui.screen_light_state_lable, &lv_font_simfang_24, 0);
			lv_label_set_text(guider_ui.screen_light_state_lable, "关");
			lv_obj_clear_state(guider_ui.screen_light_sw, LV_STATE_CHECKED);
		}
		
		if(svo_state == 1)  //svo
		{
			lv_obj_set_style_text_font(guider_ui.screen_svo_state_lable, &lv_font_simfang_24, 0);
			lv_label_set_text(guider_ui.screen_svo_state_lable, "开");
			lv_obj_add_state(guider_ui.screen_svo_sw, LV_STATE_CHECKED);
		}
		if(svo_state == 0)
		{
			lv_obj_set_style_text_font(guider_ui.screen_svo_state_lable, &lv_font_simfang_24, 0);
			lv_label_set_text(guider_ui.screen_svo_state_lable, "关");
			lv_obj_clear_state(guider_ui.screen_svo_sw, LV_STATE_CHECKED);
		}
		
		if(svt_state == 1)  //svt
		{
			lv_obj_set_style_text_font(guider_ui.screen_svt_state_lable, &lv_font_simfang_24, 0);
			lv_label_set_text(guider_ui.screen_svt_state_lable, "开");
			lv_obj_add_state(guider_ui.screen_svt_sw, LV_STATE_CHECKED);
		}
		if(svt_state == 0)
		{
			lv_obj_set_style_text_font(guider_ui.screen_svt_state_lable, &lv_font_simfang_24, 0);
			lv_label_set_text(guider_ui.screen_svt_state_lable, "关");
			lv_obj_clear_state(guider_ui.screen_svt_sw, LV_STATE_CHECKED);
		}
		
		lv_task_handler();
		vTaskDelay(1);
	}
}

void vTask4(void *pvParameters)  //保持LVGL心跳
{
	while(1)
	{
		lv_tick_inc(1);
		vTaskDelay(1);
	}
}

void vTask5(void *pvParameters)  
{
	uint16_t motor_delay = 0;
	uint16_t light_delay = 0;
	uint16_t clear_delay = 0;
	while(1)
	{
		if(clear_state == 1)
		{
			clear_state = 2;
			motor_state = 1;
			svo_state   = 1;
			svt_state   = 1;
			led_state   = 0;
		}
		if(clear_state == 3)
		{
			motor_state = 0;
			svo_state   = 0;
			svt_state   = 0;
		}
		if(oxidize_state == 1)
		{
			motor_delay++;
	    light_delay++;
			toc_value      = 0;
			if(motor_delay <= MOTOR_DELAY_S)
			{
				motor_state = 1;
				
				if(motor_delay == MOTOR_DELAY_S)
				{
					motor_state = 0;
					EC_Start_value = 0;
			    EC_Stop_value  = 0;
					EC_Start_Get();
					printf(" start ec %.2f",EC_Start_value);
				}
			}
			if(motor_delay > MOTOR_DELAY_S)
			{
				
			}
			if((MOTOR_DELAY_S <light_delay)  &&  (light_delay< LIGHT_DELAY_S))
			{
				light_state = 1;
			}
			if(light_delay > LIGHT_DELAY_S)
			{
				light_state = 0;
				
				motor_delay = 0;
	      light_delay = 0;
				oxidize_state = 0;
				led_state = 1;
				EC_Stop_Get();
				TOC_Get();
				printf(" stop ec %.2f TOC %.2f\r\n",EC_Stop_value,toc_value);
			}
		}
		vTaskDelay(1000);   //1秒延时
	}
}





int main(void)
{
	Driver_MCU_Init();                              //系统频率/分频设置
	
	SysTICK_SET();                                  //滴答定时器设置
	
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2); //绑定中断向量组2
	
	
//	disk_initialize(0);
//	f_mount(&FileSys,"",0);
//	FileResult = f_open(&File, "0:/EU_TT.TXT",FA_CREATE_ALWAYS|FA_WRITE);
//	if(FileResult == FR_OK)
//	{
//			//printf("Creat EU_DEMO.TXT File OK \r\n");

//			FileResult = f_write(&File,TxBuffer,sizeof(TxBuffer),&NbCount);//往文件里面写数据
//			f_close(&File);
//	}
//	f_mount(0, "", 0);
	
	My_RTC_Init();
	UARTON();
	
	LIGHT_Init();
	MOTOR_Init();
	SVO_Init();
	SVT_Init();
	LED_Init();
	Adc_Toc_Init();
	DS18B20_Init();

	xTaskCreate(vTask1,"LED1",64,NULL,1,NULL);      //翻转LED1(单片机状态指示)
	xTaskCreate(vTask2,"LED2",64,NULL,1,NULL);      //翻转LED2(单片机状态指示)
																								  
	xTaskCreate(vTask3,"TASK3",1024,NULL,1,NULL);   //更新画面、获取触摸值、控制水泵、TOC获取                            一定注意分配堆栈大小，大了无法创建成功。
	xTaskCreate(vTask4,"TASK4",128,NULL,1,NULL);    //保持LVGL心跳。
	xTaskCreate(vTask5,"TASK5",512,NULL,1,NULL);    //控制逻辑

	vTaskStartScheduler();
	
	while(1)
	{
		;
	}
}





