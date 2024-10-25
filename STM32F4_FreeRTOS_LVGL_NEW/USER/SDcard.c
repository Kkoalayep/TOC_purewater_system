/*--------------------------------------------------------------------------
// 文件名：SDcard.c
// 描述：  SD 驱动及相关函数 
// 设计者：EU电子
// 深圳EU电子出品-版权所有-翻版必究
// EU-热爱嵌入式开发
// http://euse.taobao.com
//-------------------------------------------------------------------------*/
/* INCLUDES */
#include "Prohead.h"
#include "SDcard.h"
#include "Driver.h"
#include "stdio.h"

//************************ Global variables definition *********************************/

static u32 CardType =  SDIO_STD_CAPACITY_SD_CARD_V1_1;
static u32 CSD_Tab[4], CID_Tab[4], RCA = 0;
u32 StopCondition = 0;
SD_Error TransferError = SD_OK;
u32 TransferEnd = 0;
SD_CardInfo SDCardInfo;
__align(4) u8 SDIOBuff[512];

SDIO_InitTypeDef SDIO_InitStructure;
SDIO_CmdInitTypeDef SDIO_CmdInitStructure;
SDIO_DataInitTypeDef SDIO_DataInitStructure;  

SD_Error SD_Init(void);
SD_Error SD_PowerON(void);
SD_Error SD_InitializeCards(void);
SD_Error CmdError(void);
static SD_Error CmdResp1Error(u8 cmd);
static SD_Error CmdResp2Error(void);
static SD_Error CmdResp3Error(void);
static SD_Error CmdResp6Error(u8 cmd, u16 *prca);
static SD_Error CmdResp7Error(void);
SD_Error SD_GetCardInfo(SD_CardInfo *cardinfo);
SD_Error SD_SelectDeselect(u32 addr);
SD_Error SD_EnableWideBusOperation(u32 WideMode);
static SD_Error SDEnWideBus(FunctionalState NewState);
static SD_Error FindSCR(u16 rca, u32 *pscr);
SD_Error SD_ReadBlock(u8 *readbuff, long long  ReadAddr, u16 BlockSize);
SD_Error SD_ReadMultiBlocks(u8 *readbuff, long long  ReadAddr, u16 BlockSize, u32 NumberOfBlocks);
SD_Error SD_WriteBlock(u8 *writebuff, long long WriteAddr, u16 BlockSize);
SD_Error SD_WriteMultiBlocks(u8 *writebuff, long long WriteAddr, u16 BlockSize, u32 NumberOfBlocks);
static SD_Error IsCardProgramming(u8 *pstatus);
u8 convert_from_bytes_to_power_of_two(u16 NumberOfBytes);
void SD_DMA_Config(u32 *BufferDST,u32 DMA_Dir);
u32 SD_DMAEndOfTransferStatus(void);
/********************************************************************************************************
*  Function: SD_Init						                                                           
*  Object: SD卡初始化               
*  输入： 无
*  输出： SD卡状态								                         	                                     
*  备注：                                      
********************************************************************************************************/
SD_Error SD_Init(void)
{
	 SD_Error errorstatus = SD_OK;	

	 Driver_SDIOInit();// SDIO初始化
	 
	 SDIO_DeInit();

   errorstatus=SD_PowerON();			//SD上电

  if (errorstatus != SD_OK)
  {
    /*!< CMD Response TimeOut (wait for CMDSENT flag) */
    return(errorstatus);
  }
	//SD 卡初始化
  errorstatus = SD_InitializeCards();

  if (errorstatus != SD_OK)
  {
    /*!< CMD Response TimeOut (wait for CMDSENT flag) */
    return(errorstatus);
  }

	//配置SD卡时钟
  // SDIOCLK = 48M, SDIO_CK = SDIOCLK/(2 + SDIO_TRANSFER_CLK_DIV) 
	// 48/2 = 24M 
  SDIO_InitStructure.SDIO_ClockDiv = SDIO_TRANSFER_CLK_DIV; 
  SDIO_InitStructure.SDIO_ClockEdge = SDIO_ClockEdge_Rising;
  SDIO_InitStructure.SDIO_ClockBypass = SDIO_ClockBypass_Disable;
  SDIO_InitStructure.SDIO_ClockPowerSave = SDIO_ClockPowerSave_Disable;
  SDIO_InitStructure.SDIO_BusWide = SDIO_BusWide_1b;
  SDIO_InitStructure.SDIO_HardwareFlowControl = SDIO_HardwareFlowControl_Disable;
  SDIO_Init(&SDIO_InitStructure);
  
  if (errorstatus == SD_OK)
  {
		//获取SD卡信息
    errorstatus = SD_GetCardInfo(&SDCardInfo);
  }

  if (errorstatus == SD_OK)
  {
		//选中SD卡
    errorstatus = SD_SelectDeselect((u32) (SDCardInfo.RCA << 16));
  }
	//设置SD卡数据宽度， 4位
  if (errorstatus == SD_OK)
  {
    errorstatus = SD_EnableWideBusOperation(SDIO_BusWide_4b);
  }  

   return(errorstatus);
}


/********************************************************************************************************
*  Function: SD_PowerON						                                                           
*  Object: 	SD卡上电
*  输入： 无
*  输出： 卡应答                                     
*  备注：                 
********************************************************************************************************/
SD_Error SD_PowerON(void)
{
  u8 i ;
	SD_Error errorstatus = SD_OK;
  u32 response = 0, count = 0, validvoltage = 0;
  u32 SDType = SD_STD_CAPACITY;

  ///*配置SDIO外设 */
  ///* SDIOCLK = HCLK, SDIO_CK = HCLK/(2 + SDIO_INIT_CLK_DIV) */
  //*SDIO_CK 初始化时钟不可以超过400 KHz */  
  SDIO_InitStructure.SDIO_ClockDiv = SDIO_INIT_CLK_DIV;
  SDIO_InitStructure.SDIO_ClockEdge = SDIO_ClockEdge_Rising;
  SDIO_InitStructure.SDIO_ClockBypass = SDIO_ClockBypass_Disable;
  SDIO_InitStructure.SDIO_ClockPowerSave = SDIO_ClockPowerSave_Disable;
  SDIO_InitStructure.SDIO_BusWide = SDIO_BusWide_1b;//1 位数据线
  SDIO_InitStructure.SDIO_HardwareFlowControl = SDIO_HardwareFlowControl_Disable;
  SDIO_Init(&SDIO_InitStructure);

  // 使能上电
  SDIO_SetPowerState(SDIO_PowerState_ON);

  //使能SDIO时钟
  SDIO_ClockCmd(ENABLE);

	//发送CMD0，复位SD卡，不需要应答
	SDIO_CmdInitStructure.SDIO_Argument = 0x0;
	SDIO_CmdInitStructure.SDIO_CmdIndex = SD_CMD_GO_IDLE_STATE;
	SDIO_CmdInitStructure.SDIO_Response = SDIO_Response_No;
	SDIO_CmdInitStructure.SDIO_Wait = SDIO_Wait_No;
	SDIO_CmdInitStructure.SDIO_CPSM = SDIO_CPSM_Enable;	
	for(i = 0; i <74; i++)//多次发送，确保可以正常复位
	{

		SDIO_SendCommand(&SDIO_CmdInitStructure);
		errorstatus = CmdError();
		if(errorstatus == SD_OK)
		{
				break;
		}
	}

  if (errorstatus != SD_OK)//CMD0发送超时，返回错误状态
  {
    /*!< CMD Response TimeOut (wait for CMDSENT flag) */
    return(errorstatus);
  }

	//发送CMD8，只有2.0及以后的卡才支持CMD8，MMC卡和V1.x的卡不支持此命令，用来区分卡的特性
  /*!< Argument: - [31:12]: Reserved (shall be set to '0')
               - [11:8]: Supply Voltage (VHS) 0x1 (Range: 2.7-3.6 V)
               - [7:0]: Check Pattern (recommended 0xAA) */
  /*!< CMD Response: R7 */
  SDIO_CmdInitStructure.SDIO_Argument = SD_CHECK_PATTERN;
  SDIO_CmdInitStructure.SDIO_CmdIndex = SDIO_SEND_IF_COND;	//CMD8
  SDIO_CmdInitStructure.SDIO_Response = SDIO_Response_Short;
  SDIO_CmdInitStructure.SDIO_Wait = SDIO_Wait_No;	//关闭等待中断
  SDIO_CmdInitStructure.SDIO_CPSM = SDIO_CPSM_Enable;
  SDIO_SendCommand(&SDIO_CmdInitStructure);

  errorstatus = CmdResp7Error();//检测R7响应

  if (errorstatus == SD_OK)
  {
    CardType = SDIO_STD_CAPACITY_SD_CARD_V2_0; //SD 2.0或者更高的卡
    SDType = SD_HIGH_CAPACITY;
  }
  /*!< CMD55 */
  SDIO_CmdInitStructure.SDIO_Argument = 0x00;
  SDIO_CmdInitStructure.SDIO_CmdIndex = SD_CMD_APP_CMD;
  SDIO_CmdInitStructure.SDIO_Response = SDIO_Response_Short;
  SDIO_CmdInitStructure.SDIO_Wait = SDIO_Wait_No;
  SDIO_CmdInitStructure.SDIO_CPSM = SDIO_CPSM_Enable;
  SDIO_SendCommand(&SDIO_CmdInitStructure);
  errorstatus = CmdResp1Error(SD_CMD_APP_CMD);

  /*!< If errorstatus is Command TimeOut, it is a MMC card */
  /*!< If errorstatus is SD_OK it is a SD card: SD card 2.0 (voltage range mismatch)
     or SD card 1.x */
  if (errorstatus == SD_OK)
  {
    /*!< SD CARD */
    /*!< Send ACMD41 SD_APP_OP_COND with Argument 0x80100000 */
    while ((!validvoltage) && (count < SD_MAX_VOLT_TRIAL))
    {

      /*!< SEND CMD55 APP_CMD with RCA as 0 */
      SDIO_CmdInitStructure.SDIO_Argument = 0x00;
      SDIO_CmdInitStructure.SDIO_CmdIndex = SD_CMD_APP_CMD;
      SDIO_CmdInitStructure.SDIO_Response = SDIO_Response_Short;
      SDIO_CmdInitStructure.SDIO_Wait = SDIO_Wait_No;
      SDIO_CmdInitStructure.SDIO_CPSM = SDIO_CPSM_Enable;
      SDIO_SendCommand(&SDIO_CmdInitStructure);

      errorstatus = CmdResp1Error(SD_CMD_APP_CMD);

      if (errorstatus != SD_OK)
      {
        return(errorstatus);
      }
			//发送ACMD41
      SDIO_CmdInitStructure.SDIO_Argument = SD_VOLTAGE_WINDOW_SD | SDType;
      SDIO_CmdInitStructure.SDIO_CmdIndex = SD_CMD_SD_APP_OP_COND; // ACMD41
      SDIO_CmdInitStructure.SDIO_Response = SDIO_Response_Short;
      SDIO_CmdInitStructure.SDIO_Wait = SDIO_Wait_No;
      SDIO_CmdInitStructure.SDIO_CPSM = SDIO_CPSM_Enable;
      SDIO_SendCommand(&SDIO_CmdInitStructure);

      errorstatus = CmdResp3Error();
      if (errorstatus != SD_OK)
      {
        return(errorstatus);
      }
			response=SDIO->RESP1;		
      validvoltage = (((response >> 31) == 1) ? 1 : 0);
      count++;
    }
    if (count >= SD_MAX_VOLT_TRIAL)
    {
      errorstatus = SD_INVALID_VOLTRANGE;
      return(errorstatus);
    }

    if (response &= SD_HIGH_CAPACITY)
    {
      CardType = SDIO_HIGH_CAPACITY_SD_CARD;
    }

  }
	else
	{
/*!< else MMC Card */
		
	}
  return(errorstatus);
}

/********************************************************************************************************
*  Function: SD_InitializeCards						                                                           
*  Object: 	让卡进入就绪状态
*  输入： 无
*  输出： 卡应答                                    
*  备注：                 
********************************************************************************************************/
SD_Error SD_InitializeCards(void)
{
  SD_Error errorstatus = SD_OK;
  u16 rca = 0x01;

  if (SDIO_GetPowerState() == SDIO_PowerState_OFF)
  {
    errorstatus = SD_REQUEST_NOT_APPLICABLE;
    return(errorstatus);
  }

  if (SDIO_SECURE_DIGITAL_IO_CARD != CardType)
  {
		//发送命令2
    SDIO_CmdInitStructure.SDIO_Argument = 0x0;
    SDIO_CmdInitStructure.SDIO_CmdIndex = SD_CMD_ALL_SEND_CID;
    SDIO_CmdInitStructure.SDIO_Response = SDIO_Response_Long;
    SDIO_CmdInitStructure.SDIO_Wait = SDIO_Wait_No;
    SDIO_CmdInitStructure.SDIO_CPSM = SDIO_CPSM_Enable;
    SDIO_SendCommand(&SDIO_CmdInitStructure);

    errorstatus = CmdResp2Error();

    if (SD_OK != errorstatus)
    {
      return(errorstatus);
    }

    CID_Tab[0] = SDIO_GetResponse(SDIO_RESP1);
    CID_Tab[1] = SDIO_GetResponse(SDIO_RESP2);
    CID_Tab[2] = SDIO_GetResponse(SDIO_RESP3);
    CID_Tab[3] = SDIO_GetResponse(SDIO_RESP4);
  }
  if ((SDIO_STD_CAPACITY_SD_CARD_V1_1 == CardType) 
		||  (SDIO_STD_CAPACITY_SD_CARD_V2_0 == CardType) 
	  ||  (SDIO_SECURE_DIGITAL_IO_COMBO_CARD == CardType)
    ||  (SDIO_HIGH_CAPACITY_SD_CARD == CardType))
  {
		//发送命令3
    /*!< SD Card publishes its RCA. */
    SDIO_CmdInitStructure.SDIO_Argument = 0x00;
    SDIO_CmdInitStructure.SDIO_CmdIndex = SD_CMD_SET_REL_ADDR;
    SDIO_CmdInitStructure.SDIO_Response = SDIO_Response_Short;
    SDIO_CmdInitStructure.SDIO_Wait = SDIO_Wait_No;
    SDIO_CmdInitStructure.SDIO_CPSM = SDIO_CPSM_Enable;
    SDIO_SendCommand(&SDIO_CmdInitStructure);

    errorstatus = CmdResp6Error(SD_CMD_SET_REL_ADDR, &rca);

    if (SD_OK != errorstatus)
    {
      return(errorstatus);
    }
  }

  if (SDIO_SECURE_DIGITAL_IO_CARD != CardType)
  {
    RCA = rca;

    /*!< Send CMD9 SEND_CSD with argument as card's RCA */
    SDIO_CmdInitStructure.SDIO_Argument = (u32)(rca << 16);
    SDIO_CmdInitStructure.SDIO_CmdIndex = SD_CMD_SEND_CSD;
    SDIO_CmdInitStructure.SDIO_Response = SDIO_Response_Long;
    SDIO_CmdInitStructure.SDIO_Wait = SDIO_Wait_No;
    SDIO_CmdInitStructure.SDIO_CPSM = SDIO_CPSM_Enable;
    SDIO_SendCommand(&SDIO_CmdInitStructure);

    errorstatus = CmdResp2Error();

    if (SD_OK != errorstatus)
    {
      return(errorstatus);
    }

    CSD_Tab[0] = SDIO_GetResponse(SDIO_RESP1);
    CSD_Tab[1] = SDIO_GetResponse(SDIO_RESP2);
    CSD_Tab[2] = SDIO_GetResponse(SDIO_RESP3);
    CSD_Tab[3] = SDIO_GetResponse(SDIO_RESP4);
  }

  errorstatus = SD_OK; /*!< All cards get intialized */

  return(errorstatus);
}
/********************************************************************************************************
*  Function: CmdError						                                                           
*  Object: 	检测CMD0是否发送成功
*  输入：　
*  输出： 卡应答                                   
*  备注：                 
********************************************************************************************************/
SD_Error CmdError(void)
{
	SD_Error errorstatus = SD_OK;
	u32 timeout=SDIO_CMD0TIMEOUT;	   
	while(timeout)
	{
		timeout--;
	  if(SDIO_GetFlagStatus(SDIO_FLAG_CMDSENT) != RESET)
		{
			break;
		}
	}	    
	if(timeout==0)return SD_CMD_RSP_TIMEOUT; 
  /*!< Clear all the static flags */	
	SDIO_ClearFlag(SDIO_STATIC_FLAGS);
	return errorstatus;
}	 

/********************************************************************************************************
*  Function: CmdResp1Error						                                                           
*  Object: 	R1应答
*  输入：　
*  输出： 卡应答                                  
*  备注：                 
********************************************************************************************************/
static SD_Error CmdResp1Error(u8 cmd)
{
  while (!(SDIO->STA & (SDIO_FLAG_CCRCFAIL | SDIO_FLAG_CMDREND | SDIO_FLAG_CTIMEOUT)))
  {
  }

  SDIO->ICR = SDIO_STATIC_FLAGS;

  return (SD_Error)(SDIO->RESP1 &  SD_OCR_ERRORBITS);
}

/********************************************************************************************************
*  Function: CmdResp2Error						                                                           
*  Object: 	R2应答
*  输入：　
*  输出： 卡应答                                  
*  备注：                 
********************************************************************************************************/


static SD_Error CmdResp2Error(void)
{
  SD_Error errorstatus = SD_OK;
  u32 status;

  status = SDIO->STA;

  while (!(status & (SDIO_FLAG_CCRCFAIL | SDIO_FLAG_CTIMEOUT | SDIO_FLAG_CMDREND)))
  {
    status = SDIO->STA;
  }

  if (status & SDIO_FLAG_CTIMEOUT)
  {
    errorstatus = SD_CMD_RSP_TIMEOUT;
    SDIO_ClearFlag(SDIO_FLAG_CTIMEOUT);
    return(errorstatus);
  }
  else if (status & SDIO_FLAG_CCRCFAIL)
  {
    errorstatus = SD_CMD_CRC_FAIL;
    SDIO_ClearFlag(SDIO_FLAG_CCRCFAIL);
    return(errorstatus);
  }

  /*!< Clear all the static flags */
  SDIO_ClearFlag(SDIO_STATIC_FLAGS);

  return(errorstatus);
}

/********************************************************************************************************
*  Function: CmdResp3Error						                                                           
*  Object: 	R3应答
*  输入：　
*  输出： 卡应答                                  
*  备注：                 
********************************************************************************************************/
static SD_Error CmdResp3Error(void)
{
  SD_Error errorstatus = SD_OK;
  u32 status;

  status = SDIO->STA;

  while (!(status & (SDIO_FLAG_CCRCFAIL | SDIO_FLAG_CMDREND | SDIO_FLAG_CTIMEOUT)))
  {
    status = SDIO->STA;
  }

  if (status & SDIO_FLAG_CTIMEOUT)
  {
    errorstatus = SD_CMD_RSP_TIMEOUT;
    SDIO_ClearFlag(SDIO_FLAG_CTIMEOUT);
    return(errorstatus);
  }
  /*!< Clear all the static flags */
  SDIO_ClearFlag(SDIO_STATIC_FLAGS);
  return(errorstatus);
}

/********************************************************************************************************
*  Function: CmdResp6Error						                                                           
*  Object: 	R6应答
*  输入：　
*  输出： 卡应答                                     
*  备注：                 
********************************************************************************************************/
static SD_Error CmdResp6Error(u8 cmd, u16 *prca)
{
  SD_Error errorstatus = SD_OK;
  u32 status;
  u32 response_r1;

  status = SDIO->STA;

  while (!(status & (SDIO_FLAG_CCRCFAIL | SDIO_FLAG_CTIMEOUT | SDIO_FLAG_CMDREND)))
  {
    status = SDIO->STA;
  }

  if (status & SDIO_FLAG_CTIMEOUT)
  {
    errorstatus = SD_CMD_RSP_TIMEOUT;
    SDIO_ClearFlag(SDIO_FLAG_CTIMEOUT);
    return(errorstatus);
  }
  else if (status & SDIO_FLAG_CCRCFAIL)
  {
    errorstatus = SD_CMD_CRC_FAIL;
    SDIO_ClearFlag(SDIO_FLAG_CCRCFAIL);
    return(errorstatus);
  }

  /*!< Check response received is of desired command */
  if (SDIO_GetCommandResponse() != cmd)
  {
    errorstatus = SD_ILLEGAL_CMD;
    return(errorstatus);
  }

  /*!< Clear all the static flags */
  SDIO_ClearFlag(SDIO_STATIC_FLAGS);

  /*!< We have received response, retrieve it.  */
  response_r1 = SDIO_GetResponse(SDIO_RESP1);

  if (SD_ALLZERO == (response_r1 & (SD_R6_GENERAL_UNKNOWN_ERROR | SD_R6_ILLEGAL_CMD | SD_R6_COM_CRC_FAILED)))
  {
    *prca = (u16) (response_r1 >> 16);
    return(errorstatus);
  }

  if (response_r1 & SD_R6_GENERAL_UNKNOWN_ERROR)
  {
    return(SD_GENERAL_UNKNOWN_ERROR);
  }

  if (response_r1 & SD_R6_ILLEGAL_CMD)
  {
    return(SD_ILLEGAL_CMD);
  }

  if (response_r1 & SD_R6_COM_CRC_FAILED)
  {
    return(SD_COM_CRC_FAILED);
  }

  return(errorstatus);
}

/********************************************************************************************************
*  Function: CmdResp7Error						                                                           
*  Object: 	R7应答
*  输入：　
*  输出： 卡应答                                   
*  备注：                 
********************************************************************************************************/
static SD_Error CmdResp7Error(void)
{
  SD_Error errorstatus = SD_OK;
  u32 status;
  u32 timeout = SDIO_CMD0TIMEOUT;

  status = SDIO->STA;

  while (!(status & (SDIO_FLAG_CCRCFAIL | SDIO_FLAG_CMDREND | SDIO_FLAG_CTIMEOUT)) && (timeout > 0))
  {
    timeout--;
    status = SDIO->STA;
  }

  if ((timeout == 0) || (status & SDIO_FLAG_CTIMEOUT))
  {
    /*!< Card is not V2.0 complient or card does not support the set voltage range */
    errorstatus = SD_CMD_RSP_TIMEOUT;
    SDIO_ClearFlag(SDIO_FLAG_CTIMEOUT);
    return(errorstatus);
  }

  if (status & SDIO_FLAG_CMDREND)
  {
    /*!< Card is SD V2.0 compliant */
    errorstatus = SD_OK;
    SDIO_ClearFlag(SDIO_FLAG_CMDREND);
    return(errorstatus);
  }
  return(errorstatus);
}

/********************************************************************************************************
*  Function: SD_GetCardInfo						                                                           
*  Object: 	获取SD卡信息
*  输入：　cardinfo
*  输出： 卡应答                                   
*  备注：                 
********************************************************************************************************/
SD_Error SD_GetCardInfo(SD_CardInfo *cardinfo)
{
  SD_Error errorstatus = SD_OK;
  u8 tmp = 0;

  cardinfo->CardType = (u8)CardType;
  cardinfo->RCA = (u16)RCA;

  /*!< Byte 0 */
  tmp = (u8)((CSD_Tab[0] & 0xFF000000) >> 24);
  cardinfo->SD_csd.CSDStruct = (tmp & 0xC0) >> 6;
  cardinfo->SD_csd.SysSpecVersion = (tmp & 0x3C) >> 2;
  cardinfo->SD_csd.Reserved1 = tmp & 0x03;

  /*!< Byte 1 */
  tmp = (u8)((CSD_Tab[0] & 0x00FF0000) >> 16);
  cardinfo->SD_csd.TAAC = tmp;

  /*!< Byte 2 */
  tmp = (u8)((CSD_Tab[0] & 0x0000FF00) >> 8);
  cardinfo->SD_csd.NSAC = tmp;

  /*!< Byte 3 */
  tmp = (u8)(CSD_Tab[0] & 0x000000FF);
  cardinfo->SD_csd.MaxBusClkFrec = tmp;

  /*!< Byte 4 */
  tmp = (u8)((CSD_Tab[1] & 0xFF000000) >> 24);
  cardinfo->SD_csd.CardComdClasses = tmp << 4;

  /*!< Byte 5 */
  tmp = (u8)((CSD_Tab[1] & 0x00FF0000) >> 16);
  cardinfo->SD_csd.CardComdClasses |= (tmp & 0xF0) >> 4;
  cardinfo->SD_csd.RdBlockLen = tmp & 0x0F;

  /*!< Byte 6 */
  tmp = (u8)((CSD_Tab[1] & 0x0000FF00) >> 8);
  cardinfo->SD_csd.PartBlockRead = (tmp & 0x80) >> 7;
  cardinfo->SD_csd.WrBlockMisalign = (tmp & 0x40) >> 6;
  cardinfo->SD_csd.RdBlockMisalign = (tmp & 0x20) >> 5;
  cardinfo->SD_csd.DSRImpl = (tmp & 0x10) >> 4;
  cardinfo->SD_csd.Reserved2 = 0; /*!< Reserved */

  if ((CardType == SDIO_STD_CAPACITY_SD_CARD_V1_1) || (CardType == SDIO_STD_CAPACITY_SD_CARD_V2_0))
  {
    cardinfo->SD_csd.DeviceSize = (tmp & 0x03) << 10;

    /*!< Byte 7 */
    tmp = (u8)(CSD_Tab[1] & 0x000000FF);
    cardinfo->SD_csd.DeviceSize |= (tmp) << 2;

    /*!< Byte 8 */
    tmp = (u8)((CSD_Tab[2] & 0xFF000000) >> 24);
    cardinfo->SD_csd.DeviceSize |= (tmp & 0xC0) >> 6;

    cardinfo->SD_csd.MaxRdCurrentVDDMin = (tmp & 0x38) >> 3;
    cardinfo->SD_csd.MaxRdCurrentVDDMax = (tmp & 0x07);

    /*!< Byte 9 */
    tmp = (u8)((CSD_Tab[2] & 0x00FF0000) >> 16);
    cardinfo->SD_csd.MaxWrCurrentVDDMin = (tmp & 0xE0) >> 5;
    cardinfo->SD_csd.MaxWrCurrentVDDMax = (tmp & 0x1C) >> 2;
    cardinfo->SD_csd.DeviceSizeMul = (tmp & 0x03) << 1;
    /*!< Byte 10 */
    tmp = (u8)((CSD_Tab[2] & 0x0000FF00) >> 8);
    cardinfo->SD_csd.DeviceSizeMul |= (tmp & 0x80) >> 7;
    
    cardinfo->CardCapacity = (cardinfo->SD_csd.DeviceSize + 1) ;
    cardinfo->CardCapacity *= (1 << (cardinfo->SD_csd.DeviceSizeMul + 2));
    cardinfo->CardBlockSize = 1 << (cardinfo->SD_csd.RdBlockLen);
    cardinfo->CardCapacity *= cardinfo->CardBlockSize;
  }
  else if (CardType == SDIO_HIGH_CAPACITY_SD_CARD)
  {
    /*!< Byte 7 */
    tmp = (u8)(CSD_Tab[1] & 0x000000FF);
    cardinfo->SD_csd.DeviceSize = (tmp & 0x3F) << 16;

    /*!< Byte 8 */
    tmp = (u8)((CSD_Tab[2] & 0xFF000000) >> 24);

    cardinfo->SD_csd.DeviceSize |= (tmp << 8);

    /*!< Byte 9 */
    tmp = (u8)((CSD_Tab[2] & 0x00FF0000) >> 16);

    cardinfo->SD_csd.DeviceSize |= (tmp);

    /*!< Byte 10 */
    tmp = (u8)((CSD_Tab[2] & 0x0000FF00) >> 8);
    
    cardinfo->CardCapacity = (cardinfo->SD_csd.DeviceSize + 1) * 512 * 1024;
    cardinfo->CardBlockSize = 512;    
  }


  cardinfo->SD_csd.EraseGrSize = (tmp & 0x40) >> 6;
  cardinfo->SD_csd.EraseGrMul = (tmp & 0x3F) << 1;

  /*!< Byte 11 */
  tmp = (u8)(CSD_Tab[2] & 0x000000FF);
  cardinfo->SD_csd.EraseGrMul |= (tmp & 0x80) >> 7;
  cardinfo->SD_csd.WrProtectGrSize = (tmp & 0x7F);

  /*!< Byte 12 */
  tmp = (u8)((CSD_Tab[3] & 0xFF000000) >> 24);
  cardinfo->SD_csd.WrProtectGrEnable = (tmp & 0x80) >> 7;
  cardinfo->SD_csd.ManDeflECC = (tmp & 0x60) >> 5;
  cardinfo->SD_csd.WrSpeedFact = (tmp & 0x1C) >> 2;
  cardinfo->SD_csd.MaxWrBlockLen = (tmp & 0x03) << 2;

  /*!< Byte 13 */
  tmp = (u8)((CSD_Tab[3] & 0x00FF0000) >> 16);
  cardinfo->SD_csd.MaxWrBlockLen |= (tmp & 0xC0) >> 6;
  cardinfo->SD_csd.WriteBlockPaPartial = (tmp & 0x20) >> 5;
  cardinfo->SD_csd.Reserved3 = 0;
  cardinfo->SD_csd.ContentProtectAppli = (tmp & 0x01);

  /*!< Byte 14 */
  tmp = (u8)((CSD_Tab[3] & 0x0000FF00) >> 8);
  cardinfo->SD_csd.FileFormatGrouop = (tmp & 0x80) >> 7;
  cardinfo->SD_csd.CopyFlag = (tmp & 0x40) >> 6;
  cardinfo->SD_csd.PermWrProtect = (tmp & 0x20) >> 5;
  cardinfo->SD_csd.TempWrProtect = (tmp & 0x10) >> 4;
  cardinfo->SD_csd.FileFormat = (tmp & 0x0C) >> 2;
  cardinfo->SD_csd.ECC = (tmp & 0x03);

  /*!< Byte 15 */
  tmp = (u8)(CSD_Tab[3] & 0x000000FF);
  cardinfo->SD_csd.CSD_CRC = (tmp & 0xFE) >> 1;
  cardinfo->SD_csd.Reserved4 = 1;


  /*!< Byte 0 */
  tmp = (u8)((CID_Tab[0] & 0xFF000000) >> 24);
  cardinfo->SD_cid.ManufacturerID = tmp;

  /*!< Byte 1 */
  tmp = (u8)((CID_Tab[0] & 0x00FF0000) >> 16);
  cardinfo->SD_cid.OEM_AppliID = tmp << 8;

  /*!< Byte 2 */
  tmp = (u8)((CID_Tab[0] & 0x000000FF00) >> 8);
  cardinfo->SD_cid.OEM_AppliID |= tmp;

  /*!< Byte 3 */
  tmp = (u8)(CID_Tab[0] & 0x000000FF);
  cardinfo->SD_cid.ProdName1 = tmp << 24;

  /*!< Byte 4 */
  tmp = (u8)((CID_Tab[1] & 0xFF000000) >> 24);
  cardinfo->SD_cid.ProdName1 |= tmp << 16;

  /*!< Byte 5 */
  tmp = (u8)((CID_Tab[1] & 0x00FF0000) >> 16);
  cardinfo->SD_cid.ProdName1 |= tmp << 8;

  /*!< Byte 6 */
  tmp = (u8)((CID_Tab[1] & 0x0000FF00) >> 8);
  cardinfo->SD_cid.ProdName1 |= tmp;

  /*!< Byte 7 */
  tmp = (u8)(CID_Tab[1] & 0x000000FF);
  cardinfo->SD_cid.ProdName2 = tmp;

  /*!< Byte 8 */
  tmp = (u8)((CID_Tab[2] & 0xFF000000) >> 24);
  cardinfo->SD_cid.ProdRev = tmp;

  /*!< Byte 9 */
  tmp = (u8)((CID_Tab[2] & 0x00FF0000) >> 16);
  cardinfo->SD_cid.ProdSN = tmp << 24;

  /*!< Byte 10 */
  tmp = (u8)((CID_Tab[2] & 0x0000FF00) >> 8);
  cardinfo->SD_cid.ProdSN |= tmp << 16;

  /*!< Byte 11 */
  tmp = (u8)(CID_Tab[2] & 0x000000FF);
  cardinfo->SD_cid.ProdSN |= tmp << 8;

  /*!< Byte 12 */
  tmp = (u8)((CID_Tab[3] & 0xFF000000) >> 24);
  cardinfo->SD_cid.ProdSN |= tmp;

  /*!< Byte 13 */
  tmp = (u8)((CID_Tab[3] & 0x00FF0000) >> 16);
  cardinfo->SD_cid.Reserved1 |= (tmp & 0xF0) >> 4;
  cardinfo->SD_cid.ManufactDate = (tmp & 0x0F) << 8;

  /*!< Byte 14 */
  tmp = (u8)((CID_Tab[3] & 0x0000FF00) >> 8);
  cardinfo->SD_cid.ManufactDate |= tmp;

  /*!< Byte 15 */
  tmp = (u8)(CID_Tab[3] & 0x000000FF);
  cardinfo->SD_cid.CID_CRC = (tmp & 0xFE) >> 1;
  cardinfo->SD_cid.Reserved2 = 1;
  
  return(errorstatus);
}

/********************************************************************************************************
*  Function: SD_SelectDeselect						                                                           
*  Object: 	选中ＳＤ卡
*  输入：　addr
*  输出： 卡应答                                    
*  备注：                 
********************************************************************************************************/
SD_Error SD_SelectDeselect(u32 addr)
{
  SD_Error errorstatus = SD_OK;

  /*!< Send CMD7 SDIO_SEL_DESEL_CARD */
  SDIO_CmdInitStructure.SDIO_Argument =  addr;
  SDIO_CmdInitStructure.SDIO_CmdIndex = SD_CMD_SEL_DESEL_CARD;
  SDIO_CmdInitStructure.SDIO_Response = SDIO_Response_Short;
  SDIO_CmdInitStructure.SDIO_Wait = SDIO_Wait_No;
  SDIO_CmdInitStructure.SDIO_CPSM = SDIO_CPSM_Enable;
  SDIO_SendCommand(&SDIO_CmdInitStructure);

  errorstatus = CmdResp1Error(SD_CMD_SEL_DESEL_CARD);

  return(errorstatus);
}

/********************************************************************************************************
*  Function: SD_EnableWideBusOperation						                                                           
*  Object: 	SDIO总线宽度设置
*  输入：　WideMode
*  输出： 卡应答                                     
*  备注：                 
********************************************************************************************************/
SD_Error SD_EnableWideBusOperation(u32 WideMode)
{
  SD_Error errorstatus = SD_OK;

  /*!< MMC Card doesn't support this feature */
  if (SDIO_MULTIMEDIA_CARD == CardType)
  {
    errorstatus = SD_UNSUPPORTED_FEATURE;
    return(errorstatus);
  }
  else if ((SDIO_STD_CAPACITY_SD_CARD_V1_1 == CardType) 
		|| (SDIO_STD_CAPACITY_SD_CARD_V2_0 == CardType) 
	  || (SDIO_HIGH_CAPACITY_SD_CARD == CardType))
  {
    if (SDIO_BusWide_8b == WideMode)
    {
      errorstatus = SD_UNSUPPORTED_FEATURE;
      return(errorstatus);
    }
    else if (SDIO_BusWide_4b == WideMode)
    {
      errorstatus = SDEnWideBus(ENABLE);

      if (SD_OK == errorstatus)
      {
        /*!< Configure the SDIO peripheral */
        SDIO_InitStructure.SDIO_ClockDiv = SDIO_TRANSFER_CLK_DIV; 
        SDIO_InitStructure.SDIO_ClockEdge = SDIO_ClockEdge_Rising;
        SDIO_InitStructure.SDIO_ClockBypass = SDIO_ClockBypass_Disable;
        SDIO_InitStructure.SDIO_ClockPowerSave = SDIO_ClockPowerSave_Disable;
        SDIO_InitStructure.SDIO_BusWide = SDIO_BusWide_4b;
        SDIO_InitStructure.SDIO_HardwareFlowControl = SDIO_HardwareFlowControl_Disable;
        SDIO_Init(&SDIO_InitStructure);
      }
    }
    else
    {
      errorstatus = SDEnWideBus(DISABLE);

      if (SD_OK == errorstatus)
      {
        /*!< Configure the SDIO peripheral */
        SDIO_InitStructure.SDIO_ClockDiv = SDIO_TRANSFER_CLK_DIV; 
        SDIO_InitStructure.SDIO_ClockEdge = SDIO_ClockEdge_Rising;
        SDIO_InitStructure.SDIO_ClockBypass = SDIO_ClockBypass_Disable;
        SDIO_InitStructure.SDIO_ClockPowerSave = SDIO_ClockPowerSave_Disable;
        SDIO_InitStructure.SDIO_BusWide = SDIO_BusWide_1b;
        SDIO_InitStructure.SDIO_HardwareFlowControl = SDIO_HardwareFlowControl_Disable;
        SDIO_Init(&SDIO_InitStructure);
      }
    }
  }

  return(errorstatus);
}
/********************************************************************************************************
*  Function: SDEnWideBus						                                                           
*  Object: 	SDIO宽总线模式使能
*  输入：　NewState
*  输出： 卡应答	                                     
*  备注：                 
********************************************************************************************************/
static SD_Error SDEnWideBus(FunctionalState NewState)
{
  SD_Error errorstatus = SD_OK;

  u32 scr[2] = {0, 0};

  if (SDIO_GetResponse(SDIO_RESP1) & SD_CARD_LOCKED)
  {
    errorstatus = SD_LOCK_UNLOCK_FAILED;
    return(errorstatus);
  }

  /*!< Get SCR Register */
  errorstatus = FindSCR(RCA, scr);

  if (errorstatus != SD_OK)
  {
    return(errorstatus);
  }

  /*!< If wide bus operation to be enabled */
  if (NewState == ENABLE)
  {
    /*!< If requested card supports wide bus operation */
    if ((scr[1] & SD_WIDE_BUS_SUPPORT) != SD_ALLZERO)
    {
      /*!< Send CMD55 APP_CMD with argument as card's RCA.*/
      SDIO_CmdInitStructure.SDIO_Argument = (u32) RCA << 16;
      SDIO_CmdInitStructure.SDIO_CmdIndex = SD_CMD_APP_CMD;
      SDIO_CmdInitStructure.SDIO_Response = SDIO_Response_Short;
      SDIO_CmdInitStructure.SDIO_Wait = SDIO_Wait_No;
      SDIO_CmdInitStructure.SDIO_CPSM = SDIO_CPSM_Enable;
      SDIO_SendCommand(&SDIO_CmdInitStructure);

      errorstatus = CmdResp1Error(SD_CMD_APP_CMD);

      if (errorstatus != SD_OK)
      {
        return(errorstatus);
      }

      /*!< Send ACMD6 APP_CMD with argument as 2 for wide bus mode */
      SDIO_CmdInitStructure.SDIO_Argument = 0x2;
      SDIO_CmdInitStructure.SDIO_CmdIndex = SD_CMD_APP_SD_SET_BUSWIDTH;
      SDIO_CmdInitStructure.SDIO_Response = SDIO_Response_Short;
      SDIO_CmdInitStructure.SDIO_Wait = SDIO_Wait_No;
      SDIO_CmdInitStructure.SDIO_CPSM = SDIO_CPSM_Enable;
      SDIO_SendCommand(&SDIO_CmdInitStructure);

      errorstatus = CmdResp1Error(SD_CMD_APP_SD_SET_BUSWIDTH);

      if (errorstatus != SD_OK)
      {
        return(errorstatus);
      }
      return(errorstatus);
    }
    else
    {
      errorstatus = SD_REQUEST_NOT_APPLICABLE;
      return(errorstatus);
    }
  }   /*!< If wide bus operation to be disabled */
  else
  {
    /*!< If requested card supports 1 bit mode operation */
    if ((scr[1] & SD_SINGLE_BUS_SUPPORT) != SD_ALLZERO)
    {
      /*!< Send CMD55 APP_CMD with argument as card's RCA.*/
      SDIO_CmdInitStructure.SDIO_Argument = (u32) RCA << 16;
      SDIO_CmdInitStructure.SDIO_CmdIndex = SD_CMD_APP_CMD;
      SDIO_CmdInitStructure.SDIO_Response = SDIO_Response_Short;
      SDIO_CmdInitStructure.SDIO_Wait = SDIO_Wait_No;
      SDIO_CmdInitStructure.SDIO_CPSM = SDIO_CPSM_Enable;
      SDIO_SendCommand(&SDIO_CmdInitStructure);


      errorstatus = CmdResp1Error(SD_CMD_APP_CMD);

      if (errorstatus != SD_OK)
      {
        return(errorstatus);
      }

      /*!< Send ACMD6 APP_CMD with argument as 2 for wide bus mode */
      SDIO_CmdInitStructure.SDIO_Argument = 0x00;
      SDIO_CmdInitStructure.SDIO_CmdIndex = SD_CMD_APP_SD_SET_BUSWIDTH;
      SDIO_CmdInitStructure.SDIO_Response = SDIO_Response_Short;
      SDIO_CmdInitStructure.SDIO_Wait = SDIO_Wait_No;
      SDIO_CmdInitStructure.SDIO_CPSM = SDIO_CPSM_Enable;
      SDIO_SendCommand(&SDIO_CmdInitStructure);

      errorstatus = CmdResp1Error(SD_CMD_APP_SD_SET_BUSWIDTH);

      if (errorstatus != SD_OK)
      {
        return(errorstatus);
      }

      return(errorstatus);
    }
    else
    {
      errorstatus = SD_REQUEST_NOT_APPLICABLE;
      return(errorstatus);
    }
  }
}
/********************************************************************************************************
*  Function: FindSCR						                                                           
*  Object: 	查找SCR寄存器
*  输入：　rca，*pscr
*  输出： 卡应答	                                     
*  备注：                 
********************************************************************************************************/
static SD_Error FindSCR(u16 rca, u32 *pscr)
{
  u32 index = 0;
  SD_Error errorstatus = SD_OK;
  u32 tempscr[2] = {0, 0};

  /*!< Set Block Size To 8 Bytes */
  /*!< Send CMD55 APP_CMD with argument as card's RCA */
  SDIO_CmdInitStructure.SDIO_Argument = (u32)8;
  SDIO_CmdInitStructure.SDIO_CmdIndex = SD_CMD_SET_BLOCKLEN;
  SDIO_CmdInitStructure.SDIO_Response = SDIO_Response_Short;
  SDIO_CmdInitStructure.SDIO_Wait = SDIO_Wait_No;
  SDIO_CmdInitStructure.SDIO_CPSM = SDIO_CPSM_Enable;
  SDIO_SendCommand(&SDIO_CmdInitStructure);

  errorstatus = CmdResp1Error(SD_CMD_SET_BLOCKLEN);

  if (errorstatus != SD_OK)
  {
    return(errorstatus);
  }

  /*!< Send CMD55 APP_CMD with argument as card's RCA */
  SDIO_CmdInitStructure.SDIO_Argument = (u32) RCA << 16;
  SDIO_CmdInitStructure.SDIO_CmdIndex = SD_CMD_APP_CMD;
  SDIO_CmdInitStructure.SDIO_Response = SDIO_Response_Short;
  SDIO_CmdInitStructure.SDIO_Wait = SDIO_Wait_No;
  SDIO_CmdInitStructure.SDIO_CPSM = SDIO_CPSM_Enable;
  SDIO_SendCommand(&SDIO_CmdInitStructure);

  errorstatus = CmdResp1Error(SD_CMD_APP_CMD);

  if (errorstatus != SD_OK)
  {
    return(errorstatus);
  }
  SDIO_DataInitStructure.SDIO_DataTimeOut = SD_DATATIMEOUT;
  SDIO_DataInitStructure.SDIO_DataLength = 8;
  SDIO_DataInitStructure.SDIO_DataBlockSize = SDIO_DataBlockSize_8b;
  SDIO_DataInitStructure.SDIO_TransferDir = SDIO_TransferDir_ToSDIO;
  SDIO_DataInitStructure.SDIO_TransferMode = SDIO_TransferMode_Block;
  SDIO_DataInitStructure.SDIO_DPSM = SDIO_DPSM_Enable;
  SDIO_DataConfig(&SDIO_DataInitStructure);


  /*!< Send ACMD51 SD_APP_SEND_SCR with argument as 0 */
  SDIO_CmdInitStructure.SDIO_Argument = 0x0;
  SDIO_CmdInitStructure.SDIO_CmdIndex = SD_CMD_SD_APP_SEND_SCR;
  SDIO_CmdInitStructure.SDIO_Response = SDIO_Response_Short;
  SDIO_CmdInitStructure.SDIO_Wait = SDIO_Wait_No;
  SDIO_CmdInitStructure.SDIO_CPSM = SDIO_CPSM_Enable;
  SDIO_SendCommand(&SDIO_CmdInitStructure);

  errorstatus = CmdResp1Error(SD_CMD_SD_APP_SEND_SCR);

  if (errorstatus != SD_OK)
  {
    return(errorstatus);
  }

  while (!(SDIO->STA & (SDIO_FLAG_RXOVERR | SDIO_FLAG_DCRCFAIL | SDIO_FLAG_DTIMEOUT | SDIO_FLAG_DBCKEND | SDIO_FLAG_STBITERR)))
  {
    if (SDIO_GetFlagStatus(SDIO_FLAG_RXDAVL) != RESET)
    {
      *(tempscr + index) = SDIO_ReadData();
      index++;
    }
  }

  if (SDIO_GetFlagStatus(SDIO_FLAG_DTIMEOUT) != RESET)
  {
    SDIO_ClearFlag(SDIO_FLAG_DTIMEOUT);
    errorstatus = SD_DATA_TIMEOUT;
    return(errorstatus);
  }
  else if (SDIO_GetFlagStatus(SDIO_FLAG_DCRCFAIL) != RESET)
  {
    SDIO_ClearFlag(SDIO_FLAG_DCRCFAIL);
    errorstatus = SD_DATA_CRC_FAIL;
    return(errorstatus);
  }
  else if (SDIO_GetFlagStatus(SDIO_FLAG_RXOVERR) != RESET)
  {
    SDIO_ClearFlag(SDIO_FLAG_RXOVERR);
    errorstatus = SD_RX_OVERRUN;
    return(errorstatus);
  }
  else if (SDIO_GetFlagStatus(SDIO_FLAG_STBITERR) != RESET)
  {
    SDIO_ClearFlag(SDIO_FLAG_STBITERR);
    errorstatus = SD_START_BIT_ERR;
    return(errorstatus);
  }

  /*!< Clear all the static flags */
  SDIO_ClearFlag(SDIO_STATIC_FLAGS);

  *(pscr + 1) = ((tempscr[0] & SD_0TO7BITS) << 24) | ((tempscr[0] & SD_8TO15BITS) << 8) | ((tempscr[0] & SD_16TO23BITS) >> 8) | ((tempscr[0] & SD_24TO31BITS) >> 24);

  *(pscr) = ((tempscr[1] & SD_0TO7BITS) << 24) | ((tempscr[1] & SD_8TO15BITS) << 8) | ((tempscr[1] & SD_16TO23BITS) >> 8) | ((tempscr[1] & SD_24TO31BITS) >> 24);

  return(errorstatus);
}
/********************************************************************************************************
*  Function: SD_ReadBlock						                                                           
*  Object: 	SD卡读取一个Block的函数
*  输入: *readbuff -- 存放读取数据的地址, ReadAddr 起始读取地址，BlockSize,块大小
*  输出：  SD卡状态                                    
*  备注：                 
********************************************************************************************************/
SD_Error SD_ReadBlock(u8 *readbuff, long long  ReadAddr, u16 BlockSize)
{
  SD_Error errorstatus = SD_OK;
	u8 power = 0;
	u32 timeout = SD_DATATIMEOUT;
	
  TransferError = SD_OK;
  TransferEnd = 0;
  StopCondition = 0;
  
  SDIO->DCTRL = 0x0;
  
  /*!< Clear all DPSM configuration */
  SDIO_DataInitStructure.SDIO_DataTimeOut = SD_DATATIMEOUT;
  SDIO_DataInitStructure.SDIO_DataLength = 0;
  SDIO_DataInitStructure.SDIO_DataBlockSize = SDIO_DataBlockSize_1b;
  SDIO_DataInitStructure.SDIO_TransferDir = SDIO_TransferDir_ToCard;
  SDIO_DataInitStructure.SDIO_TransferMode = SDIO_TransferMode_Block;
  SDIO_DataInitStructure.SDIO_DPSM = SDIO_DPSM_Enable;
  SDIO_DataConfig(&SDIO_DataInitStructure);
  SDIO_DMACmd(DISABLE);

  if (SDIO_GetResponse(SDIO_RESP1) & SD_CARD_LOCKED)//卡锁住了
  {
    errorstatus = SD_LOCK_UNLOCK_FAILED;
    return(errorstatus);
  }	
	
  if (CardType == SDIO_HIGH_CAPACITY_SD_CARD)
  {
    BlockSize = 512;
    ReadAddr /= 512;
  }
	//BlockSize 必须为偶数
  if ((BlockSize > 0) && (BlockSize <= 2048) && ((BlockSize & (BlockSize - 1)) == 0))
  {
    power = convert_from_bytes_to_power_of_two(BlockSize);

    /*!< Set Block Size for Card */
    SDIO_CmdInitStructure.SDIO_Argument = (u32) BlockSize;
    SDIO_CmdInitStructure.SDIO_CmdIndex = SD_CMD_SET_BLOCKLEN;
    SDIO_CmdInitStructure.SDIO_Response = SDIO_Response_Short;
    SDIO_CmdInitStructure.SDIO_Wait = SDIO_Wait_No;
    SDIO_CmdInitStructure.SDIO_CPSM = SDIO_CPSM_Enable;
    SDIO_SendCommand(&SDIO_CmdInitStructure);

    errorstatus = CmdResp1Error(SD_CMD_SET_BLOCKLEN);

    if (SD_OK != errorstatus)
    {
      return(errorstatus);
    }
  }
  else
  {
    errorstatus = SD_INVALID_PARAMETER;
    return(errorstatus);
  }
	

  SDIO_DataInitStructure.SDIO_DataTimeOut = SD_DATATIMEOUT;
  SDIO_DataInitStructure.SDIO_DataLength = BlockSize;
  SDIO_DataInitStructure.SDIO_DataBlockSize = (u32) power << 4;
  SDIO_DataInitStructure.SDIO_TransferDir = SDIO_TransferDir_ToSDIO;
  SDIO_DataInitStructure.SDIO_TransferMode = SDIO_TransferMode_Block;
  SDIO_DataInitStructure.SDIO_DPSM = SDIO_DPSM_Enable;
  SDIO_DataConfig(&SDIO_DataInitStructure);

  /*!< Send CMD17 READ_SINGLE_BLOCK */
  SDIO_CmdInitStructure.SDIO_Argument = (u32)ReadAddr;
  SDIO_CmdInitStructure.SDIO_CmdIndex = SD_CMD_READ_SINGLE_BLOCK;
  SDIO_CmdInitStructure.SDIO_Response = SDIO_Response_Short;
  SDIO_CmdInitStructure.SDIO_Wait = SDIO_Wait_No;
  SDIO_CmdInitStructure.SDIO_CPSM = SDIO_CPSM_Enable;
  SDIO_SendCommand(&SDIO_CmdInitStructure);

  errorstatus = CmdResp1Error(SD_CMD_READ_SINGLE_BLOCK);

  if (errorstatus != SD_OK)
  {
    return(errorstatus);
  }


	SDIO_ITConfig(SDIO_IT_DCRCFAIL | SDIO_IT_DTIMEOUT | SDIO_IT_DATAEND | SDIO_IT_RXOVERR | SDIO_IT_STBITERR, ENABLE);
	SD_DMA_Config((u32 *)readbuff,DMA_DIR_PeripheralToMemory);
	SDIO_DMACmd(ENABLE);	
	while ((SD_DMAEndOfTransferStatus() == RESET) && (TransferEnd == 0) && (TransferError == SD_OK)&&timeout)
	{
		timeout--;//等待传输完成 
	}
	if(timeout==0)
	{
		return SD_DATA_TIMEOUT;//超时
	}
	if (TransferError != SD_OK)
	{
		return(TransferError);
	}	
	printf("%s,errorstatus = %d\r\n",__func__,errorstatus);
  return(errorstatus);
}
/********************************************************************************************************
*  Function: SD_ReadMultiBlocks						                                                           
*  Object: 	SD卡读取多个Block的函数
*  输入: *readbuff -- 存放读取数据的地址, ReadAddr 起始读取地址，BlockSize,块大小,NumberOfBlocks 要读取的块大小
*  输出：  SD卡状态                                    
*  备注：                 
********************************************************************************************************/
SD_Error SD_ReadMultiBlocks(u8 *readbuff, long long  ReadAddr, u16 BlockSize, u32 NumberOfBlocks)
{
  SD_Error errorstatus = SD_OK;
	u8 power = 0;
	u32 timeout= SDIO_DATATIMEOUT; 
  TransferError = SD_OK;
  TransferEnd = 0;
  StopCondition = 1;
  SDIO->DCTRL = 0x0;
	
   /*!< Clear all DPSM configuration */
  SDIO_DataInitStructure.SDIO_DataTimeOut = SD_DATATIMEOUT;
  SDIO_DataInitStructure.SDIO_DataLength = 0;
  SDIO_DataInitStructure.SDIO_DataBlockSize = SDIO_DataBlockSize_1b;
  SDIO_DataInitStructure.SDIO_TransferDir = SDIO_TransferDir_ToCard;
  SDIO_DataInitStructure.SDIO_TransferMode = SDIO_TransferMode_Block;
  SDIO_DataInitStructure.SDIO_DPSM = SDIO_DPSM_Enable;
  SDIO_DataConfig(&SDIO_DataInitStructure);
  SDIO_DMACmd(DISABLE);

  if (SDIO_GetResponse(SDIO_RESP1) & SD_CARD_LOCKED)//卡锁住了
  {
    errorstatus = SD_LOCK_UNLOCK_FAILED;
    return(errorstatus);
  }	
	
  if (CardType == SDIO_HIGH_CAPACITY_SD_CARD)
  {
    BlockSize = 512;
    ReadAddr /= 512;
  }
	//BlockSize 必须为偶数
  if ((BlockSize > 0) && (BlockSize <= 2048) && ((BlockSize & (BlockSize - 1)) == 0))
  {
    power = convert_from_bytes_to_power_of_two(BlockSize);

    /*!< Set Block Size for Card */
    SDIO_CmdInitStructure.SDIO_Argument = (u32) BlockSize;
    SDIO_CmdInitStructure.SDIO_CmdIndex = SD_CMD_SET_BLOCKLEN;
    SDIO_CmdInitStructure.SDIO_Response = SDIO_Response_Short;
    SDIO_CmdInitStructure.SDIO_Wait = SDIO_Wait_No;
    SDIO_CmdInitStructure.SDIO_CPSM = SDIO_CPSM_Enable;
    SDIO_SendCommand(&SDIO_CmdInitStructure);

    errorstatus = CmdResp1Error(SD_CMD_SET_BLOCKLEN);

    if (SD_OK != errorstatus)
    {
      return(errorstatus);
    }
  }
  else
  {
    errorstatus = SD_INVALID_PARAMETER;
    return(errorstatus);
  }
	
	if(NumberOfBlocks>1)
	{	
			SDIO_DataInitStructure.SDIO_DataTimeOut = SD_DATATIMEOUT;
			SDIO_DataInitStructure.SDIO_DataLength = NumberOfBlocks * BlockSize;
			SDIO_DataInitStructure.SDIO_DataBlockSize = (u32) power << 4;
			SDIO_DataInitStructure.SDIO_TransferDir = SDIO_TransferDir_ToSDIO;
			SDIO_DataInitStructure.SDIO_TransferMode = SDIO_TransferMode_Block;
			SDIO_DataInitStructure.SDIO_DPSM = SDIO_DPSM_Enable;
			SDIO_DataConfig(&SDIO_DataInitStructure);

			/*!< Send CMD18 READ_MULT_BLOCK with argument data address */
			SDIO_CmdInitStructure.SDIO_Argument = (u32)ReadAddr;
			SDIO_CmdInitStructure.SDIO_CmdIndex = SD_CMD_READ_MULT_BLOCK;
			SDIO_CmdInitStructure.SDIO_Response = SDIO_Response_Short;
			SDIO_CmdInitStructure.SDIO_Wait = SDIO_Wait_No;
			SDIO_CmdInitStructure.SDIO_CPSM = SDIO_CPSM_Enable;
			SDIO_SendCommand(&SDIO_CmdInitStructure);

			errorstatus = CmdResp1Error(SD_CMD_READ_MULT_BLOCK);
			if (errorstatus != SD_OK)
			{
				return(errorstatus);
			}
			
			SDIO_ITConfig(SDIO_IT_DCRCFAIL | SDIO_IT_DTIMEOUT | SDIO_IT_DATAEND | SDIO_IT_RXOVERR | SDIO_IT_STBITERR, ENABLE);
			SD_DMA_Config((u32 *)readbuff,DMA_DIR_PeripheralToMemory);
			SDIO_DMACmd(ENABLE);
			while ((SD_DMAEndOfTransferStatus() == RESET) && (TransferEnd == 0) && (TransferError == SD_OK)&&timeout)
			{
				timeout--;//等待传输完成 
			}
			if(timeout==0)
			{
				return SD_DATA_TIMEOUT;//超时
			}
			if (TransferError != SD_OK)
			{
				return(TransferError);
			}						
	}
	printf("%s,errorstatus = %d\r\n",__func__,errorstatus);
  return(errorstatus);
}

/********************************************************************************************************
*  Function: SD_WriteBlock						                                                           
*  Object: 	SD卡写一个Block的函数
*  输入: *writebuff -- 写数据缓冲区, WriteAddr 写数据的地址，BlockSize,块大小
*  输出：  SD卡状态                                    
*  备注：                 
********************************************************************************************************/
SD_Error SD_WriteBlock(u8 *writebuff, long long WriteAddr, u16 BlockSize)
{
  SD_Error errorstatus = SD_OK;
	u8 power = 0;
  u8  cardstate = 0;
  u32 timeout = 0;
  u32 cardstatus = 0;

  TransferError = SD_OK;
  TransferEnd = 0;
  StopCondition = 0;

  SDIO->DCTRL = 0x0;

  /*!< Clear all DPSM configuration */
  SDIO_DataInitStructure.SDIO_DataTimeOut = SD_DATATIMEOUT;
  SDIO_DataInitStructure.SDIO_DataLength = 0;
  SDIO_DataInitStructure.SDIO_DataBlockSize = SDIO_DataBlockSize_1b;
  SDIO_DataInitStructure.SDIO_TransferDir = SDIO_TransferDir_ToCard;
  SDIO_DataInitStructure.SDIO_TransferMode = SDIO_TransferMode_Block;
  SDIO_DataInitStructure.SDIO_DPSM = SDIO_DPSM_Enable;
  SDIO_DataConfig(&SDIO_DataInitStructure);
  SDIO_DMACmd(DISABLE);

  if (SDIO_GetResponse(SDIO_RESP1) & SD_CARD_LOCKED)//卡锁住了
  {
    errorstatus = SD_LOCK_UNLOCK_FAILED;
    return(errorstatus);
  }	
	
  if (CardType == SDIO_HIGH_CAPACITY_SD_CARD)
  {
    BlockSize = 512;
    WriteAddr /= 512;
  }
	//BlockSize 必须为偶数
  if ((BlockSize > 0) && (BlockSize <= 2048) && ((BlockSize & (BlockSize - 1)) == 0))
  {
    power = convert_from_bytes_to_power_of_two(BlockSize);

    /*!< Set Block Size for Card */
    SDIO_CmdInitStructure.SDIO_Argument = (u32) BlockSize;
    SDIO_CmdInitStructure.SDIO_CmdIndex = SD_CMD_SET_BLOCKLEN;
    SDIO_CmdInitStructure.SDIO_Response = SDIO_Response_Short;
    SDIO_CmdInitStructure.SDIO_Wait = SDIO_Wait_No;
    SDIO_CmdInitStructure.SDIO_CPSM = SDIO_CPSM_Enable;
    SDIO_SendCommand(&SDIO_CmdInitStructure);

    errorstatus = CmdResp1Error(SD_CMD_SET_BLOCKLEN);

    if (SD_OK != errorstatus)
    {
      return(errorstatus);
    }
  }
  else
  {
    errorstatus = SD_INVALID_PARAMETER;
    return(errorstatus);
  }

 /*!< Wait till card is ready for data Added */
  SDIO_CmdInitStructure.SDIO_Argument = (u32) (RCA << 16);
  SDIO_CmdInitStructure.SDIO_CmdIndex = SD_CMD_SEND_STATUS;
  SDIO_CmdInitStructure.SDIO_Response = SDIO_Response_Short;
  SDIO_CmdInitStructure.SDIO_Wait = SDIO_Wait_No;
  SDIO_CmdInitStructure.SDIO_CPSM = SDIO_CPSM_Enable;
  SDIO_SendCommand(&SDIO_CmdInitStructure);

  errorstatus = CmdResp1Error(SD_CMD_SEND_STATUS);

  if (errorstatus != SD_OK)
  {
    return(errorstatus);
  }

  cardstatus = SDIO_GetResponse(SDIO_RESP1);

  timeout = SD_DATATIMEOUT;

  while (((cardstatus & 0x00000100) == 0) && (timeout > 0))
  {
    timeout--;
    SDIO_CmdInitStructure.SDIO_Argument = (u32) (RCA << 16);
    SDIO_CmdInitStructure.SDIO_CmdIndex = SD_CMD_SEND_STATUS;
    SDIO_CmdInitStructure.SDIO_Response = SDIO_Response_Short;
    SDIO_CmdInitStructure.SDIO_Wait = SDIO_Wait_No;
    SDIO_CmdInitStructure.SDIO_CPSM = SDIO_CPSM_Enable;
    SDIO_SendCommand(&SDIO_CmdInitStructure);

    errorstatus = CmdResp1Error(SD_CMD_SEND_STATUS);

    if (errorstatus != SD_OK)
    {
      return(errorstatus);
    }
    cardstatus = SDIO_GetResponse(SDIO_RESP1);
  }

  if (timeout == 0)
  {
    return(SD_ERROR);
  }

  /*!< Send CMD24 WRITE_SINGLE_BLOCK */
  SDIO_CmdInitStructure.SDIO_Argument = WriteAddr;
  SDIO_CmdInitStructure.SDIO_CmdIndex = SD_CMD_WRITE_SINGLE_BLOCK;
  SDIO_CmdInitStructure.SDIO_Response = SDIO_Response_Short;
  SDIO_CmdInitStructure.SDIO_Wait = SDIO_Wait_No;
  SDIO_CmdInitStructure.SDIO_CPSM = SDIO_CPSM_Enable;
  SDIO_SendCommand(&SDIO_CmdInitStructure);

  errorstatus = CmdResp1Error(SD_CMD_WRITE_SINGLE_BLOCK);

  if (errorstatus != SD_OK)
  {
    return(errorstatus);
  }

  SDIO_DataInitStructure.SDIO_DataTimeOut = SD_DATATIMEOUT;
  SDIO_DataInitStructure.SDIO_DataLength = BlockSize;
  SDIO_DataInitStructure.SDIO_DataBlockSize = (u32) power << 4;
  SDIO_DataInitStructure.SDIO_TransferDir = SDIO_TransferDir_ToCard;
  SDIO_DataInitStructure.SDIO_TransferMode = SDIO_TransferMode_Block;
  SDIO_DataInitStructure.SDIO_DPSM = SDIO_DPSM_Enable;
  SDIO_DataConfig(&SDIO_DataInitStructure);
	
	timeout = SD_DATATIMEOUT;

  /*!< In case of single data block transfer no need of stop command at all */
	SDIO_ITConfig(SDIO_IT_DCRCFAIL | SDIO_IT_DTIMEOUT | SDIO_IT_DATAEND | SDIO_IT_RXOVERR | SDIO_IT_STBITERR, ENABLE);
	SD_DMA_Config((u32*)writebuff, DMA_DIR_MemoryToPeripheral);
	SDIO_DMACmd(ENABLE);
	while ((SD_DMAEndOfTransferStatus() == RESET)&&timeout)
	{
		timeout--;//等待传输完成 
	}
	if(timeout==0)
	{
		return SD_DATA_TIMEOUT;//超时
	}
	if (TransferError != SD_OK)
	{
		return(TransferError);
	}	
  /*!< Clear all the static flags */
  SDIO_ClearFlag(SDIO_STATIC_FLAGS);

  /*!< Wait till the card is in programming state */
  errorstatus = IsCardProgramming(&cardstate);

  while ((errorstatus == SD_OK) && ((cardstate == SD_CARD_PROGRAMMING) || (cardstate == SD_CARD_RECEIVING)))
  {
    errorstatus = IsCardProgramming(&cardstate);
  }
	printf("%s,errorstatus = %d\r\n",__func__,errorstatus);
  return(errorstatus);
}

/********************************************************************************************************
*  Function: SD_WriteMultiBlocks						                                                           
*  Object: 	SD卡写一个Block的函数
*  输入: *writebuff -- 写数据缓冲区, WriteAddr 写数据的地址，BlockSize,块大小,NumberOfBlocks 需要写的块数
*  输出：  SD卡状态                                    
*  备注：                 
********************************************************************************************************/
SD_Error SD_WriteMultiBlocks(u8 *writebuff, long long WriteAddr, u16 BlockSize, u32 NumberOfBlocks)
{
	SD_Error errorstatus = SD_OK;
	u8 power = 0;
  u8  cardstate = 0;
  u32 timeout = 0;
	volatile u32 Cnt = 0;


  TransferError = SD_OK;
  TransferEnd = 0;
  StopCondition = 1;

  SDIO->DCTRL = 0x0;

  /*!< Clear all DPSM configuration */
  SDIO_DataInitStructure.SDIO_DataTimeOut = SD_DATATIMEOUT;
  SDIO_DataInitStructure.SDIO_DataLength = 0;
  SDIO_DataInitStructure.SDIO_DataBlockSize = SDIO_DataBlockSize_1b;
  SDIO_DataInitStructure.SDIO_TransferDir = SDIO_TransferDir_ToCard;
  SDIO_DataInitStructure.SDIO_TransferMode = SDIO_TransferMode_Block;
  SDIO_DataInitStructure.SDIO_DPSM = SDIO_DPSM_Enable;
  SDIO_DataConfig(&SDIO_DataInitStructure);
  SDIO_DMACmd(DISABLE);

  if (SDIO_GetResponse(SDIO_RESP1) & SD_CARD_LOCKED)//卡锁住了
  {
    errorstatus = SD_LOCK_UNLOCK_FAILED;
    return(errorstatus);
  }	
	
  if (CardType == SDIO_HIGH_CAPACITY_SD_CARD)
  {
    BlockSize = 512;
    WriteAddr /= 512;
  }
	//BlockSize 必须为偶数
  if ((BlockSize > 0) && (BlockSize <= 2048) && ((BlockSize & (BlockSize - 1)) == 0))
  {
    power = convert_from_bytes_to_power_of_two(BlockSize);

    /*!< Set Block Size for Card */
    SDIO_CmdInitStructure.SDIO_Argument = (u32) BlockSize;
    SDIO_CmdInitStructure.SDIO_CmdIndex = SD_CMD_SET_BLOCKLEN;
    SDIO_CmdInitStructure.SDIO_Response = SDIO_Response_Short;
    SDIO_CmdInitStructure.SDIO_Wait = SDIO_Wait_No;
    SDIO_CmdInitStructure.SDIO_CPSM = SDIO_CPSM_Enable;
    SDIO_SendCommand(&SDIO_CmdInitStructure);

    errorstatus = CmdResp1Error(SD_CMD_SET_BLOCKLEN);

    if (SD_OK != errorstatus)
    {
      return(errorstatus);
    }
  }
  else
  {
    errorstatus = SD_INVALID_PARAMETER;
    return(errorstatus);
  }

 /*!< Wait till card is ready for data Added */
  SDIO_CmdInitStructure.SDIO_Argument = (u32) (RCA << 16);
  SDIO_CmdInitStructure.SDIO_CmdIndex = SD_CMD_SEND_STATUS;
  SDIO_CmdInitStructure.SDIO_Response = SDIO_Response_Short;
  SDIO_CmdInitStructure.SDIO_Wait = SDIO_Wait_No;
  SDIO_CmdInitStructure.SDIO_CPSM = SDIO_CPSM_Enable;
  SDIO_SendCommand(&SDIO_CmdInitStructure);

  errorstatus = CmdResp1Error(SD_CMD_SEND_STATUS);

  if (errorstatus != SD_OK)
  {
    return(errorstatus);
  }

    if (NumberOfBlocks > 1)
  {
    /*!< Common to all modes */
    if (NumberOfBlocks * BlockSize > SD_MAX_DATA_LENGTH)
    {
      errorstatus = SD_INVALID_PARAMETER;
      return(errorstatus);
    }

    if ((SDIO_STD_CAPACITY_SD_CARD_V1_1 == CardType) || (SDIO_STD_CAPACITY_SD_CARD_V2_0 == CardType) || (SDIO_HIGH_CAPACITY_SD_CARD == CardType))
    {
      /*!< To improve performance */
      SDIO_CmdInitStructure.SDIO_Argument = (u32) (RCA << 16);
      SDIO_CmdInitStructure.SDIO_CmdIndex = SD_CMD_APP_CMD;
      SDIO_CmdInitStructure.SDIO_Response = SDIO_Response_Short;
      SDIO_CmdInitStructure.SDIO_Wait = SDIO_Wait_No;
      SDIO_CmdInitStructure.SDIO_CPSM = SDIO_CPSM_Enable;
      SDIO_SendCommand(&SDIO_CmdInitStructure);

      errorstatus = CmdResp1Error(SD_CMD_APP_CMD);

      if (errorstatus != SD_OK)
      {
        return(errorstatus);
      }
      /*!< To improve performance */
      SDIO_CmdInitStructure.SDIO_Argument = (u32)NumberOfBlocks;
      SDIO_CmdInitStructure.SDIO_CmdIndex = SD_CMD_SET_BLOCK_COUNT;
      SDIO_CmdInitStructure.SDIO_Response = SDIO_Response_Short;
      SDIO_CmdInitStructure.SDIO_Wait = SDIO_Wait_No;
      SDIO_CmdInitStructure.SDIO_CPSM = SDIO_CPSM_Enable;
      SDIO_SendCommand(&SDIO_CmdInitStructure);

      errorstatus = CmdResp1Error(SD_CMD_SET_BLOCK_COUNT);

      if (errorstatus != SD_OK)
      {
        return(errorstatus);
      }
    }

    /*!< Send CMD25 WRITE_MULT_BLOCK with argument data address */
    SDIO_CmdInitStructure.SDIO_Argument = (u32)WriteAddr;
    SDIO_CmdInitStructure.SDIO_CmdIndex = SD_CMD_WRITE_MULT_BLOCK;
    SDIO_CmdInitStructure.SDIO_Response = SDIO_Response_Short;
    SDIO_CmdInitStructure.SDIO_Wait = SDIO_Wait_No;
    SDIO_CmdInitStructure.SDIO_CPSM = SDIO_CPSM_Enable;
    SDIO_SendCommand(&SDIO_CmdInitStructure);

    errorstatus = CmdResp1Error(SD_CMD_WRITE_MULT_BLOCK);

    if (SD_OK != errorstatus)
    {
      return(errorstatus);
    }
    SDIO_DataInitStructure.SDIO_DataTimeOut = SD_DATATIMEOUT;
    SDIO_DataInitStructure.SDIO_DataLength = NumberOfBlocks * BlockSize;
    SDIO_DataInitStructure.SDIO_DataBlockSize = (u32) power << 4;
    SDIO_DataInitStructure.SDIO_TransferDir = SDIO_TransferDir_ToCard;
    SDIO_DataInitStructure.SDIO_TransferMode = SDIO_TransferMode_Block;
    SDIO_DataInitStructure.SDIO_DPSM = SDIO_DPSM_Enable;
    SDIO_DataConfig(&SDIO_DataInitStructure);

		SDIO_ITConfig(SDIO_IT_DCRCFAIL | SDIO_IT_DTIMEOUT | SDIO_IT_DATAEND | SDIO_IT_RXOVERR | SDIO_IT_STBITERR, ENABLE);
		SD_DMA_Config((u32 *)writebuff, DMA_DIR_MemoryToPeripheral);
		SDIO_DMACmd(ENABLE);
		timeout=SDIO_DATATIMEOUT;
		while ((SD_DMAEndOfTransferStatus() == RESET)&&timeout)
		{
			timeout--;//等待传输完成 
		}
		if(timeout==0)
		{
			return SD_DATA_TIMEOUT;//超时
		}
		if (TransferError != SD_OK)
		{
			return(TransferError);
		}	

  }
  /*!< Clear all the static flags */
  SDIO_ClearFlag(SDIO_STATIC_FLAGS);
	
  //读卡状态之前，稍作延时，不然会出现SD_ILLEGAL_CMD的错误
  for(Cnt = 0; Cnt < 0xFFFF; Cnt++);
	
  /*!< Wait till the card is in programming state */
  errorstatus = IsCardProgramming(&cardstate);

  while ((errorstatus == SD_OK) && ((cardstate == SD_CARD_PROGRAMMING) || (cardstate == SD_CARD_RECEIVING)))
  {
    errorstatus = IsCardProgramming(&cardstate);
  }
	printf("%s,errorstatus = %d\r\n",__func__,errorstatus);
  return(errorstatus);
}
/********************************************************************************************************
*  Function: IsCardProgramming						                                                           
*  Object: 	卡写状态检测
*  输入: *pstatus
*  输出： 卡错误状态                                    
*  备注：                 
********************************************************************************************************/
static SD_Error IsCardProgramming(u8 *pstatus)
{
  SD_Error errorstatus = SD_OK;
  __IO u32 respR1 = 0, status = 0;

  SDIO_CmdInitStructure.SDIO_Argument = (u32) RCA << 16;
  SDIO_CmdInitStructure.SDIO_CmdIndex = SD_CMD_SEND_STATUS;
  SDIO_CmdInitStructure.SDIO_Response = SDIO_Response_Short;
  SDIO_CmdInitStructure.SDIO_Wait = SDIO_Wait_No;
  SDIO_CmdInitStructure.SDIO_CPSM = SDIO_CPSM_Enable;
  SDIO_SendCommand(&SDIO_CmdInitStructure);

  status = SDIO->STA;
  while (!(status & (SDIO_FLAG_CCRCFAIL | SDIO_FLAG_CMDREND | SDIO_FLAG_CTIMEOUT)))
  {
    status = SDIO->STA;
  }

  if (status & SDIO_FLAG_CTIMEOUT)
  {
    errorstatus = SD_CMD_RSP_TIMEOUT;
    SDIO_ClearFlag(SDIO_FLAG_CTIMEOUT);
    return(errorstatus);
  }
  else if (status & SDIO_FLAG_CCRCFAIL)
  {
    errorstatus = SD_CMD_CRC_FAIL;
    SDIO_ClearFlag(SDIO_FLAG_CCRCFAIL);
    return(errorstatus);
  }

  status = (u32)SDIO_GetCommandResponse();

  /*!< Check response received is of desired command */
  if (status != SD_CMD_SEND_STATUS)
  {
    errorstatus = SD_ILLEGAL_CMD;
    return(errorstatus);
  }

  /*!< Clear all the static flags */
  SDIO_ClearFlag(SDIO_STATIC_FLAGS);


  /*!< We have received response, retrieve it for analysis  */
  respR1 = SDIO_GetResponse(SDIO_RESP1);

  /*!< Find out card status */
  *pstatus = (u8) ((respR1 >> 9) & 0x0000000F);

  if ((respR1 & SD_OCR_ERRORBITS) == SD_ALLZERO)
  {
    return(errorstatus);
  }

  if (respR1 & SD_OCR_ADDR_OUT_OF_RANGE)
  {
    return(SD_ADDR_OUT_OF_RANGE);
  }

  if (respR1 & SD_OCR_ADDR_MISALIGNED)
  {
    return(SD_ADDR_MISALIGNED);
  }

  if (respR1 & SD_OCR_BLOCK_LEN_ERR)
  {
    return(SD_BLOCK_LEN_ERR);
  }

  if (respR1 & SD_OCR_ERASE_SEQ_ERR)
  {
    return(SD_ERASE_SEQ_ERR);
  }

  if (respR1 & SD_OCR_BAD_ERASE_PARAM)
  {
    return(SD_BAD_ERASE_PARAM);
  }

  if (respR1 & SD_OCR_WRITE_PROT_VIOLATION)
  {
    return(SD_WRITE_PROT_VIOLATION);
  }

  if (respR1 & SD_OCR_LOCK_UNLOCK_FAILED)
  {
    return(SD_LOCK_UNLOCK_FAILED);
  }

  if (respR1 & SD_OCR_COM_CRC_FAILED)
  {
    return(SD_COM_CRC_FAILED);
  }

  if (respR1 & SD_OCR_ILLEGAL_CMD)
  {

    return(SD_ILLEGAL_CMD);
  }

  if (respR1 & SD_OCR_CARD_ECC_FAILED)
  {
    return(SD_CARD_ECC_FAILED);
  }

  if (respR1 & SD_OCR_CC_ERROR)
  {
    return(SD_CC_ERROR);
  }

  if (respR1 & SD_OCR_GENERAL_UNKNOWN_ERROR)
  {
    return(SD_GENERAL_UNKNOWN_ERROR);
  }

  if (respR1 & SD_OCR_STREAM_READ_UNDERRUN)
  {
    return(SD_STREAM_READ_UNDERRUN);
  }

  if (respR1 & SD_OCR_STREAM_WRITE_OVERRUN)
  {
    return(SD_STREAM_WRITE_OVERRUN);
  }

  if (respR1 & SD_OCR_CID_CSD_OVERWRIETE)
  {
    return(SD_CID_CSD_OVERWRITE);
  }

  if (respR1 & SD_OCR_WP_ERASE_SKIP)
  {
    return(SD_WP_ERASE_SKIP);
  }

  if (respR1 & SD_OCR_CARD_ECC_DISABLED)
  {
    return(SD_CARD_ECC_DISABLED);
  }

  if (respR1 & SD_OCR_ERASE_RESET)
  {
    return(SD_ERASE_RESET);
  }

  if (respR1 & SD_OCR_AKE_SEQ_ERROR)
  {
    return(SD_AKE_SEQ_ERROR);
  }
	printf("%s,errorstatus = %d\r\n",__func__,errorstatus);
  return(errorstatus);
}
/********************************************************************************************************
*  Function: convert_from_bytes_to_power_of_two						                                                           
*  Object: 	查找SCR寄存器
*  输入: NumberOfBytes
*  输出： 2为底的指数值	                                     
*  备注：                 
********************************************************************************************************/
u8 convert_from_bytes_to_power_of_two(u16 NumberOfBytes)
{
  u8 count = 0;

  while (NumberOfBytes != 1)
  {
    NumberOfBytes >>= 1;
    count++;
  }
  return(count);
}

/********************************************************************************************************
*  Function: SD_DMA_Config						                                                           
*  Object: 	SDIO接收DMA的数据传输
*  输入: *BufferDST, BufferSize
*  输出： 无                                  
*  备注：                 
********************************************************************************************************/
void SD_DMA_Config(u32 *BufferDST,u32 DMA_Dir)
{
		DMA_InitTypeDef DMA_InitStructure;	
		//DMA通道配置 
		while (DMA_GetCmdStatus(DMA2_Stream3) != DISABLE);//等待DMA可配置 		
		DMA_DeInit(DMA2_Stream3);	
		DMA_InitStructure.DMA_Channel = DMA_Channel_4;	
		DMA_InitStructure.DMA_PeripheralBaseAddr = (u32)&SDIO->FIFO;; 
		DMA_InitStructure.DMA_Memory0BaseAddr = (u32)BufferDST;
		DMA_InitStructure.DMA_DIR = DMA_Dir;
		DMA_InitStructure.DMA_BufferSize = 0;
		DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
		DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
		DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Word;
		DMA_InitStructure.DMA_MemoryDataSize = DMA_PeripheralDataSize_Word ;
		DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;
		DMA_InitStructure.DMA_Priority = DMA_Priority_VeryHigh;
		DMA_InitStructure.DMA_FIFOMode = DMA_FIFOMode_Enable;
		DMA_InitStructure.DMA_FIFOThreshold = DMA_FIFOThreshold_Full;
		DMA_InitStructure.DMA_MemoryBurst = DMA_MemoryBurst_INC4;
		DMA_InitStructure.DMA_PeripheralBurst = DMA_PeripheralBurst_INC4;
		DMA_Init(DMA2_Stream3, &DMA_InitStructure); 			
		DMA_FlowControllerConfig(DMA2_Stream3,DMA_FlowCtrl_Peripheral);
		DMA_Cmd(DMA2_Stream3, ENABLE);		
}

/********************************************************************************************************
*  Function: SD_DMAEndOfTransferStatus						                                                           
*  Object: 	DMA的数据传输结束检测
*  输入: *BufferDST, BufferSize
*  输出： 无                                  
*  备注：                 
********************************************************************************************************/
u32 SD_DMAEndOfTransferStatus(void)
{
  return (u32)DMA_GetFlagStatus(DMA2_Stream3,DMA_FLAG_TCIF3);
}
/********************************************************************************************************
*  Function: SD_ProcessIRQSrc						                                                           
*  Object: 	SDIO中断处理函数
*  输入:  
*  输出： 卡错误状态                                  
*  备注：                 
********************************************************************************************************/
SD_Error SD_ProcessIRQSrc(void)
{ 
  if (SDIO_GetITStatus(SDIO_IT_DATAEND) != RESET)
  {
		if (StopCondition==1)
		{  
				SDIO_CmdInitStructure.SDIO_Argument =0;//发送CMD12,结束传输 	  
				SDIO_CmdInitStructure.SDIO_CmdIndex = SD_CMD_STOP_TRANSMISSION;
				SDIO_CmdInitStructure.SDIO_Response = SDIO_Response_Short;
				SDIO_CmdInitStructure.SDIO_Wait = SDIO_Wait_No;
				SDIO_CmdInitStructure.SDIO_CPSM = SDIO_CPSM_Enable;
				SDIO_SendCommand(&SDIO_CmdInitStructure);	
					
			TransferError=CmdResp1Error(SD_CMD_STOP_TRANSMISSION);
		}
		else
		{
			TransferError = SD_OK;	
		}
    SDIO_ClearITPendingBit(SDIO_IT_DATAEND);
    TransferEnd = 1;
  }  
  else if (SDIO_GetITStatus(SDIO_IT_DCRCFAIL) != RESET)
  {
    SDIO_ClearITPendingBit(SDIO_IT_DCRCFAIL);
    TransferError = SD_DATA_CRC_FAIL;
  }
  else if (SDIO_GetITStatus(SDIO_IT_DTIMEOUT) != RESET)
  {
    SDIO_ClearITPendingBit(SDIO_IT_DTIMEOUT);
    TransferError = SD_DATA_TIMEOUT;
  }
  else if (SDIO_GetITStatus(SDIO_IT_RXOVERR) != RESET)
  {
    SDIO_ClearITPendingBit(SDIO_IT_RXOVERR);
    TransferError = SD_RX_OVERRUN;
  }
  else if (SDIO_GetITStatus(SDIO_IT_TXUNDERR) != RESET)
  {
    SDIO_ClearITPendingBit(SDIO_IT_TXUNDERR);
    TransferError = SD_TX_UNDERRUN;
  }
  else if (SDIO_GetITStatus(SDIO_IT_STBITERR) != RESET)
  {
    SDIO_ClearITPendingBit(SDIO_IT_STBITERR);
    TransferError = SD_START_BIT_ERR;
  }

  SDIO_ITConfig(SDIO_IT_DCRCFAIL | SDIO_IT_DTIMEOUT | SDIO_IT_DATAEND |
                SDIO_IT_TXFIFOHE | SDIO_IT_RXFIFOHF | SDIO_IT_TXUNDERR |
                SDIO_IT_RXOVERR | SDIO_IT_STBITERR, DISABLE);

  return(TransferError);
}
