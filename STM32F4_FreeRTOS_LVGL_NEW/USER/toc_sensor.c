#include "toc_sensor.h"

uint16_t toc_adc_value = 0;
float toc_value = 0.0;

float EC_voltage;
float EC_value=0.0,voltage_value;
float temp_value=250;
float compensationCoefficient=1.0;//温度校准系数
float compensationVolatge;
float kValue=1.0;
float kValue_Low=1.0;  //校准时进行修改
float kValue_High=1.0; //校准时进行修改
float rawEC=0.0;
float EC_valueTemp=0.0;

float EC_Start_value = 0;
float EC_Stop_value  = 0;


//初始化ADC															   
void  Adc_Toc_Init(void)
{    
  GPIO_InitTypeDef  GPIO_InitStructure;
	ADC_CommonInitTypeDef ADC_CommonInitStructure;
	ADC_InitTypeDef       ADC_InitStructure;
	
  RCC_AHB1PeriphClockCmd(PRE_AHB_RCC, ENABLE);     //使能GPIOA时钟
  RCC_APB2PeriphClockCmd(PRE_APB_RCC, ENABLE);     //使能ADC1时钟

  //先初始化ADC1通道5 IO口
  GPIO_InitStructure.GPIO_Pin  = PRE_PIN;          //PA5 通道5
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AN;     //模拟输入
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL ;//不带上下拉
  GPIO_Init(PRE_PORT, &GPIO_InitStructure);        //初始化  
 
	RCC_APB2PeriphResetCmd(PRE_APB_RCC,ENABLE);	  //ADC1复位
	RCC_APB2PeriphResetCmd(PRE_APB_RCC,DISABLE);	//复位结束	 
 
	
  ADC_CommonInitStructure.ADC_Mode = ADC_Mode_Independent;//独立模式
  ADC_CommonInitStructure.ADC_TwoSamplingDelay = ADC_TwoSamplingDelay_5Cycles;//两个采样阶段之间的延迟5个时钟
  ADC_CommonInitStructure.ADC_DMAAccessMode = ADC_DMAAccessMode_Disabled; //DMA失能
  ADC_CommonInitStructure.ADC_Prescaler = ADC_Prescaler_Div4;//预分频4分频。ADCCLK=PCLK2/4=84/4=21Mhz,ADC时钟最好不要超过36Mhz 
  ADC_CommonInit(&ADC_CommonInitStructure);//初始化
	
  ADC_InitStructure.ADC_Resolution = ADC_Resolution_12b;//12位模式
  ADC_InitStructure.ADC_ScanConvMode = DISABLE;//非扫描模式	
  ADC_InitStructure.ADC_ContinuousConvMode = DISABLE;//关闭连续转换
  ADC_InitStructure.ADC_ExternalTrigConvEdge = ADC_ExternalTrigConvEdge_None;//禁止触发检测，使用软件触发
  ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;//右对齐	
  ADC_InitStructure.ADC_NbrOfConversion = 1;//1个转换在规则序列中 也就是只转换规则序列1 
  ADC_Init(ADCx, &ADC_InitStructure);//ADC初始化
	
	ADC_Cmd(ADCx, ENABLE);//开启AD转换器	
}

//获得ADC值
//ch: @ref ADC_channels 
//通道值 0~16取值范围为：ADC_Channel_0~ADC_Channel_16
//返回值:转换结果
void  Get_Toc_Adc(void)
{
	//设置指定ADC的规则组通道，一个序列，采样时间
	ADC_RegularChannelConfig(ADCx, ADCx_CH, 1, ADC_SampleTime_480Cycles );	//ADC通道,480个周期,提高采样时间可以提高精确度			    
	ADC_SoftwareStartConv(ADCx);		//使能指定的ADC的软件转换启动功能	
	while(!ADC_GetFlagStatus(ADCx, ADC_FLAG_EOC ));//等待转换结束
	toc_adc_value = ADC_GetConversionValue(ADCx);	//返回最近一次ADC规则组的转换结果
}

void Get_EC_value(uint8_t count)
{
	uint8_t i = 0;
	uint16_t tmp = 0;
	for(i=0;i<count;i++)
	{
			Get_Toc_Adc();
			tmp += toc_adc_value;
	}
	tmp = tmp/count; //取平均
	EC_voltage =(float) tmp/4096*3300; // 读取转换的AD值
	rawEC = 1000*EC_voltage/RES2/ECREF;
	EC_valueTemp=rawEC*kValue;
	
	/*First Range:(0,2); Second Range:(2,20)*/
	if(EC_valueTemp>2.0)
	{
	  kValue=kValue_High;
	}
	else if(EC_valueTemp<=2.0)
	{
	  kValue=kValue_Low;
	}
	EC_value=rawEC*kValue;
	//compensationCoefficient=1.0+0.0185*((temp_value/10)-25.0); 
	compensationCoefficient=1.0+0.0185*(temp_value-25.0); 
	EC_value=EC_value/compensationCoefficient;
	if((EC_value<=0)){EC_value=0;}
	if((EC_value>20)){EC_value=20;}//20mS/cm
}


void EC_Start_Get(void)
{
	Get_EC_value(10);
	EC_Start_value = EC_value;
}

void EC_Stop_Get(void)
{
	Get_EC_value(10);
	EC_Stop_value = EC_value;
}

void TOC_Get(void)
{
	toc_value = 119.13*(EC_Stop_value - EC_Start_value) + 59.164;
}




