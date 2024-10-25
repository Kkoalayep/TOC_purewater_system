#include "Driver.h"

#define  True  1
#define  False 0


typedef struct
{
	u16 LCD_REG;
	u16 LCD_RAM;
} LCD_TypeDef;

#define LCD_BASE        ((u32)(0x6C000000 | 0x0000007E))
#define LCD             ((LCD_TypeDef *) LCD_BASE)

/*-------------------------------------------------------------------------------------------------------
*  ��������												 
-------------------------------------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------------------------------------
*  ��������												 
-------------------------------------------------------------------------------------------------------*/

GPIO_InitTypeDef GPIO_InitStructure;

void Driver_MCU_Init(void)
{
		#define PLL_M      8
		#define PLL_N      336	
		#define PLL_P      2	
		#define PLL_Q      7	
  	//--------------------------- CLK INIT, HSE PLL ----------------------------
		ErrorStatus HSEStartUpStatus;
		//RCC reset
		RCC_DeInit();
		//Enable HSE
		RCC_HSEConfig(RCC_HSE_ON); 
		//Wait HSE is ready
		HSEStartUpStatus = RCC_WaitForHSEStartUp();
		//If HSE start fail, wail and wail.
		while(HSEStartUpStatus == ERROR);
		//Set bus clock
		RCC_HCLKConfig(RCC_SYSCLK_Div1);	//(HCLK=SYSCLK)=168MHz
		RCC_PCLK1Config(RCC_HCLK_Div4);		//(PCLK1=SYSCLK/4)=42MHz
		RCC_PCLK2Config(RCC_HCLK_Div2);		//(PCLK2=SYSCLK/2)=84MHz
		//HSE 8M PLL-> 168M	
		//PLL=8MHz * N/ (M*P)=8MHz* 336 /(8*2) = 168MHz
		RCC_PLLConfig(RCC_PLLSource_HSE,PLL_M ,PLL_N ,PLL_P,PLL_Q);
		RCC_PLLCmd(ENABLE); 
		while(RCC_GetFlagStatus(RCC_FLAG_PLLRDY) == RESET);
		//Select PLL as system clock source
		RCC_SYSCLKConfig(RCC_SYSCLKSource_PLLCLK);
		while(RCC_GetSYSCLKSource() != 0x08);
		
		//----------------------------- CLOSE HSI ---------------------------
		RCC_HSICmd(DISABLE);		
		//--------------------------- OPEN GPIO CLK -------------------------
		RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
		RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);		
		RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);
		RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE);
		RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOE, ENABLE);		
		RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOF, ENABLE);
		RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOG, ENABLE);		

		//---------------------------- INT CONFIG ---------------------------
		//2-level interrupt 
		//NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
			
		//---------------------------- JTAG CONFIG ---------------------------
		//JTAG/SWD disable
		//GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable, ENABLE);
		//GPIO_PinRemapConfig(GPIO_Remap_SWJ_Disable, ENABLE);

			//开中断
		__enable_irq();
		
}


void SysTICK_SET(void)
{
	SysTick_Config(SystemCoreClock/1000);
}

/*-------------------------------------------------------------------------------------------------------
*  ִ�д���													 
-------------------------------------------------------------------------------------------------------*/
//-------------------------------------------------------------------------------------------------------
//-------------------------------------- LCD DRIVER -----------------------------------------------------
//-------------------------------------------------------------------------------------------------------
/********************************************************************************************************
*  Function: Driver_LcdBacklight				                                                           
*  Object: lcd backlight control
*  Input: sta
*  Output: none                                  
*  brief: none
********************************************************************************************************/
void Driver_LcdBacklight(unsigned char sta)
{
		static unsigned char StartFlag = True;
		if(StartFlag)
		{
			//PB0
			GPIO_InitTypeDef GPIO_InitStructure;
			GPIO_InitStructure.GPIO_Pin = GPIO_Pin_15;
			GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
			GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;; 
			GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;  
			GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;  
			GPIO_Init(GPIOB, &GPIO_InitStructure);
			StartFlag = False;
		}	
		if(sta)
			GPIO_SetBits(GPIOB, GPIO_Pin_15);
		else
			GPIO_ResetBits(GPIOB, GPIO_Pin_15);
}

/********************************************************************************************************
*  Function: Driver_LcdReset				                                                           
*  Object: lcd reset control
*  Input: sta
*  Output: none                                  
*  brief: none
********************************************************************************************************/
void Driver_LcdReset(unsigned char sta)
{
	;
}

/********************************************************************************************************
*  Function: Driver_LcdFSMCON				                                                           
*  Object: FSMC init
*  Input: none
*  Output: none                                  
*  brief: none
********************************************************************************************************/
void Driver_LcdFSMCON(void)
{
  GPIO_InitTypeDef  GPIO_InitStructure;
	FSMC_NORSRAMInitTypeDef  FSMC_NORSRAMInitStructure;
  FSMC_NORSRAMTimingInitTypeDef  readWriteTiming; 
	FSMC_NORSRAMTimingInitTypeDef  writeTiming;
	
	
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB|RCC_AHB1Periph_GPIOD|RCC_AHB1Periph_GPIOE|RCC_AHB1Periph_GPIOF|RCC_AHB1Periph_GPIOG, ENABLE);//使能PD,PE,PF,PG时钟  
  RCC_AHB3PeriphClockCmd(RCC_AHB3Periph_FSMC,ENABLE);//使能FSMC时钟  
  
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_15;//PB15 推挽输出,控制背光
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;//普通输出模式
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;//推挽输出
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;//100MHz
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;//上拉
  GPIO_Init(GPIOB, &GPIO_InitStructure);//初始化 //PB15 推挽输出,控制背光
	
  GPIO_InitStructure.GPIO_Pin = (3<<0)|(3<<4)|(7<<8)|(3<<14);//PD0,1,4,5,8,9,10,14,15 AF OUT
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;//复用输出
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;//推挽输出
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;//100MHz
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;//上拉
  GPIO_Init(GPIOD, &GPIO_InitStructure);//初始化  
	
  GPIO_InitStructure.GPIO_Pin = (0X1FF<<7);//PE7~15,AF OUT
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;//复用输出
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;//推挽输出
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;//100MHz
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;//上拉
  GPIO_Init(GPIOE, &GPIO_InitStructure);//初始化  

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12;//PF12,FSMC_A6
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;//复用输出
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;//推挽输出
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;//100MHz
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;//上拉
  GPIO_Init(GPIOF, &GPIO_InitStructure);//初始化  

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12;//PF12,FSMC_A6
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;//复用输出
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;//推挽输出
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;//100MHz
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;//上拉
  GPIO_Init(GPIOG, &GPIO_InitStructure);//初始化 

  GPIO_PinAFConfig(GPIOD,GPIO_PinSource0,GPIO_AF_FSMC);//PD0,AF12
  GPIO_PinAFConfig(GPIOD,GPIO_PinSource1,GPIO_AF_FSMC);//PD1,AF12
  GPIO_PinAFConfig(GPIOD,GPIO_PinSource4,GPIO_AF_FSMC);
  GPIO_PinAFConfig(GPIOD,GPIO_PinSource5,GPIO_AF_FSMC); 
  GPIO_PinAFConfig(GPIOD,GPIO_PinSource8,GPIO_AF_FSMC); 
  GPIO_PinAFConfig(GPIOD,GPIO_PinSource9,GPIO_AF_FSMC);
  GPIO_PinAFConfig(GPIOD,GPIO_PinSource10,GPIO_AF_FSMC);
  GPIO_PinAFConfig(GPIOD,GPIO_PinSource14,GPIO_AF_FSMC);
  GPIO_PinAFConfig(GPIOD,GPIO_PinSource15,GPIO_AF_FSMC);//PD15,AF12
 
  GPIO_PinAFConfig(GPIOE,GPIO_PinSource7,GPIO_AF_FSMC);//PE7,AF12
  GPIO_PinAFConfig(GPIOE,GPIO_PinSource8,GPIO_AF_FSMC);
  GPIO_PinAFConfig(GPIOE,GPIO_PinSource9,GPIO_AF_FSMC);
  GPIO_PinAFConfig(GPIOE,GPIO_PinSource10,GPIO_AF_FSMC);
  GPIO_PinAFConfig(GPIOE,GPIO_PinSource11,GPIO_AF_FSMC);
  GPIO_PinAFConfig(GPIOE,GPIO_PinSource12,GPIO_AF_FSMC);
  GPIO_PinAFConfig(GPIOE,GPIO_PinSource13,GPIO_AF_FSMC);
  GPIO_PinAFConfig(GPIOE,GPIO_PinSource14,GPIO_AF_FSMC);
  GPIO_PinAFConfig(GPIOE,GPIO_PinSource15,GPIO_AF_FSMC);//PE15,AF12
 
  GPIO_PinAFConfig(GPIOF,GPIO_PinSource12,GPIO_AF_FSMC);//PF12,AF12
  GPIO_PinAFConfig(GPIOG,GPIO_PinSource12,GPIO_AF_FSMC);


  readWriteTiming.FSMC_AddressSetupTime = 0XF;	 //地址建立时间（ADDSET）为16个HCLK 1/168M=6ns*16=96ns	
  readWriteTiming.FSMC_AddressHoldTime = 0x00;	 //地址保持时间（ADDHLD）模式A未用到	
  readWriteTiming.FSMC_DataSetupTime = 24;			//数据保存时间为25个HCLK	=6*25=150ns
  readWriteTiming.FSMC_BusTurnAroundDuration = 0x00;
  readWriteTiming.FSMC_CLKDivision = 0x00;
  readWriteTiming.FSMC_DataLatency = 0x00;
  readWriteTiming.FSMC_AccessMode = FSMC_AccessMode_A;	 //模式A 
    

	writeTiming.FSMC_AddressSetupTime =8;	      //地址建立时间（ADDSET）为8个HCLK =48ns 
  writeTiming.FSMC_AddressHoldTime = 0x00;	 //地址保持时间（A		
  writeTiming.FSMC_DataSetupTime = 8;		 //数据保存时间为6ns*9个HCLK=54ns
  writeTiming.FSMC_BusTurnAroundDuration = 0x00;
  writeTiming.FSMC_CLKDivision = 0x00;
  writeTiming.FSMC_DataLatency = 0x00;
  writeTiming.FSMC_AccessMode = FSMC_AccessMode_A;	 //模式A 

 
  FSMC_NORSRAMInitStructure.FSMC_Bank = FSMC_Bank1_NORSRAM4;//  这里我们使用NE4 ，也就对应BTCR[6],[7]。
  FSMC_NORSRAMInitStructure.FSMC_DataAddressMux = FSMC_DataAddressMux_Disable; // 不复用数据地址
  FSMC_NORSRAMInitStructure.FSMC_MemoryType =FSMC_MemoryType_SRAM;// FSMC_MemoryType_SRAM;  //SRAM   
  FSMC_NORSRAMInitStructure.FSMC_MemoryDataWidth = FSMC_MemoryDataWidth_16b;//存储器数据宽度为16bit   
  FSMC_NORSRAMInitStructure.FSMC_BurstAccessMode =FSMC_BurstAccessMode_Disable;// FSMC_BurstAccessMode_Disable; 
  FSMC_NORSRAMInitStructure.FSMC_WaitSignalPolarity = FSMC_WaitSignalPolarity_Low;
  FSMC_NORSRAMInitStructure.FSMC_AsynchronousWait=FSMC_AsynchronousWait_Disable; 
  FSMC_NORSRAMInitStructure.FSMC_WrapMode = FSMC_WrapMode_Disable;   
  FSMC_NORSRAMInitStructure.FSMC_WaitSignalActive = FSMC_WaitSignalActive_BeforeWaitState;  
  FSMC_NORSRAMInitStructure.FSMC_WriteOperation = FSMC_WriteOperation_Enable;	//  存储器写使能
  FSMC_NORSRAMInitStructure.FSMC_WaitSignal = FSMC_WaitSignal_Disable;   
  FSMC_NORSRAMInitStructure.FSMC_ExtendedMode = FSMC_ExtendedMode_Enable; // 读写使用不同的时序
  FSMC_NORSRAMInitStructure.FSMC_WriteBurst = FSMC_WriteBurst_Disable; 
  FSMC_NORSRAMInitStructure.FSMC_ReadWriteTimingStruct = &readWriteTiming; //读写时序
  FSMC_NORSRAMInitStructure.FSMC_WriteTimingStruct = &writeTiming;  //写时序

  FSMC_NORSRAMInit(&FSMC_NORSRAMInitStructure);  //初始化FSMC配置

  FSMC_NORSRAMCmd(FSMC_Bank1_NORSRAM4, ENABLE);  // 使能BANK1 
	
  FSMC_Bank1E->BWTR[6]&=~(0XF<<0);//地址建立时间(ADDSET)清零 	 
	FSMC_Bank1E->BWTR[6]&=~(0XF<<8);//数据保存时间清零
	FSMC_Bank1E->BWTR[6]|=3<<0;		//地址建立时间(ADDSET)为3个HCLK =18ns  	 
	FSMC_Bank1E->BWTR[6]|=2<<8; 	//数据保存时间(DATAST)为6ns*3个HCLK=18ns
}


/********************************************************************************************************
*  Function: Driver_SDInit						                                                           
*  Object: 	SDIO 引脚初始化 
*  输入： 无
*  输出： 无	                                     
*  备注： PB10                
********************************************************************************************************/
void Driver_SDIOInit(void)
{
	GPIO_InitTypeDef  GPIO_InitStructure;
	NVIC_InitTypeDef	NVIC_InitStructure;
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SDIO,ENABLE);
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA2,ENABLE);
	RCC_APB2PeriphResetCmd(RCC_APB2Periph_SDIO, ENABLE);//SDIO复位
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8|GPIO_Pin_9|GPIO_Pin_10|GPIO_Pin_11|GPIO_Pin_12;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;//PC 8,9,10,11,12 设置为SDIO复用功能
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;//上拉	
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOC,&GPIO_InitStructure);
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;	//PD2设置为SDIO复用功能
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOD,&GPIO_InitStructure);		
	
	GPIO_PinAFConfig(GPIOC,GPIO_PinSource8, GPIO_AF_SDIO);
	GPIO_PinAFConfig(GPIOC,GPIO_PinSource9, GPIO_AF_SDIO);
	GPIO_PinAFConfig(GPIOC,GPIO_PinSource10,GPIO_AF_SDIO);
	GPIO_PinAFConfig(GPIOC,GPIO_PinSource11,GPIO_AF_SDIO);
	GPIO_PinAFConfig(GPIOC,GPIO_PinSource12,GPIO_AF_SDIO);
	
	GPIO_PinAFConfig(GPIOD,GPIO_PinSource2, GPIO_AF_SDIO);	
	
	RCC_APB2PeriphResetCmd(RCC_APB2Periph_SDIO, DISABLE);//SDIO结束复位	
	
	
  NVIC_InitStructure.NVIC_IRQChannel = SDIO_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority= 10;//抢占优先级1
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 10;		//子优先级1
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//IRQ通道使能
	NVIC_Init(&NVIC_InitStructure);	
	

}

/********************************************************************************************************
*  Function: Driver_LcdFSMCWriteReg				                                                           
*  Object: ͨ��FSMC�ӿ�дREG
*  Input: Index
*  Output: none                                  
*  brief: none
********************************************************************************************************/
void Driver_LcdFSMCWriteReg(u16 Index)
{
		LCD->LCD_REG = Index;
}

/********************************************************************************************************
*  Function: Driver_LcdFSMCWriteData				                                                           
*  Object: ͨ��FSMC�ӿ�дData
*  Input: Data
*  Output: none                                  
*  brief: none
********************************************************************************************************/
void Driver_LcdFSMCWriteData(u16 Data)
{
		LCD->LCD_RAM = Data;
}

/********************************************************************************************************
*  Function: Driver_LcdFSMCReadData				                                                           
*  Object: ͨ��FSMC�ӿڶ�Data
*  Input: none
*  Output: Data                                  
*  brief: none
********************************************************************************************************/
u16 Driver_LcdFSMCReadData(void)
{
		return (LCD->LCD_RAM);
}

//-------------------------------------------------------------------------------------------------------
//-------------------------------------- TOUCH SPI DRIVER -----------------------------------------------
//-------------------------------------------------------------------------------------------------------
/********************************************************************************************************
*  Function: Driver_TouchCS					                                                           
*  Object: touch ic spi enable/unable
*  Input: 1/0
*  Output: none 							                         	                                     
*  brief:	none                                 
********************************************************************************************************/
void Driver_SPICS(unsigned char sta)
{
		static unsigned char StartFlag = True;
		if(StartFlag)
		{
			GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13;
			GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
			GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;; 
			GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;  
			GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;   	
			GPIO_Init(GPIOC, &GPIO_InitStructure);
			StartFlag = False;
		}	
		if(!sta)
			GPIO_ResetBits(GPIOC, GPIO_Pin_13);
		else
			GPIO_SetBits(GPIOC, GPIO_Pin_13);
}

/********************************************************************************************************
*  Function: Driver_SPIMISO					                                                           
*  Object: SPI_MOSI
*  Input: none
*  Output: 1/0 							                         	                                     
*  brief:	none                                   
********************************************************************************************************/
unsigned char Driver_SPIMISO(void)
{
		static unsigned char StartFlag = True;
		if(StartFlag)
		{	
			GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
			GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
			GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;  
			GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP; 	
			GPIO_Init(GPIOB, &GPIO_InitStructure);
			StartFlag = False;
		}
		return GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_2);
}

/********************************************************************************************************
*  Function: Driver_SPIMOSI					                                                           
*  Object: SPI_MOSI
*  Input: 1/0
*  Output: none 							                         	                                     
*  brief:	none                                   
********************************************************************************************************/
void Driver_SPIMOSI(unsigned char sta)
{
		static unsigned char StartFlag = True;
		if(StartFlag)
		{
			GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;
			GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
			GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;; 
			GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;  
			GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP; 	
			GPIO_Init(GPIOF, &GPIO_InitStructure);
			StartFlag = False;
		}	
		if(!sta)
			GPIO_ResetBits(GPIOF, GPIO_Pin_11);
		else
			GPIO_SetBits(GPIOF, GPIO_Pin_11);
}

/********************************************************************************************************
*  Function: Driver_SPISCK					                                                           
*  Object: SPI_SCK
*  Input: 1/0
*  Output: none 							                         	                                     
*  brief:	none  	                                  
********************************************************************************************************/
void Driver_SPISCK(unsigned char sta)
{
		static unsigned char StartFlag = True;
		if(StartFlag)
		{
			GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
			GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
			GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;; 
			GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;  
			GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP; 
			GPIO_Init(GPIOB, &GPIO_InitStructure);
			StartFlag = False;
		}	
		if(!sta)
			GPIO_ResetBits(GPIOB, GPIO_Pin_0);
		else
			GPIO_SetBits(GPIOB, GPIO_Pin_0);
}


unsigned char Driver_TINT(void)
{
		static unsigned char StartFlag = True;
		if(StartFlag)
		{	
			GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;
			GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
			GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;  
			GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_DOWN; 	
			GPIO_Init(GPIOB, &GPIO_InitStructure);
			StartFlag = False;
		}
		return GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_1);
}

