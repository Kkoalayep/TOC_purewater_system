/*-----------------------------------------------------------------------*/
/* Low level disk I/O module skeleton for FatFs     (C)ChaN, 2014        */
/*-----------------------------------------------------------------------*/
/* If a working storage control module is available, it should be        */
/* attached to the FatFs via a glue function rather than modifying it.   */
/* This is an example of glue functions to attach various exsisting      */
/* storage control modules to the FatFs module with a defined API.       */
/*-----------------------------------------------------------------------*/

#include "diskio.h"		/* FatFs lower layer API */
//#include "usbdisk.h"	/* Example: Header file of existing USB MSD control module */
//#include "atadrive.h"	/* Example: Header file of existing SDCARD harddisk control module */
#include "SDcard.h"		/* Example: Header file of existing MMC/SDC contorl module */
#include "stdio.h"
/* Definitions of physical drive number for each drive */
#define SDCARD		0	 //SD卡 设备号
#define SD_SECTOR_SIZE    512

/*-----------------------------------------------------------------------*/
/* Get Drive Status                                                      */
/*-----------------------------------------------------------------------*/

DSTATUS disk_status (
	BYTE pdrv		/* Physical drive nmuber to identify the drive */
)
{
	DSTATUS stat = RES_OK;
	//int result;

	switch (pdrv) {
	case SDCARD :
		//result = ATA_disk_status();

		// translate the reslut code here

		return stat;
	}
	return STA_NOINIT;
}



/*-----------------------------------------------------------------------*/
/* Inidialize a Drive                                                    */
/*-----------------------------------------------------------------------*/

DSTATUS disk_initialize (
	BYTE pdrv				/* Physical drive nmuber to identify the drive */
)
{
	DSTATUS stat = RES_OK;
	int result;

	switch (pdrv) {
	case SDCARD :
		result = SD_Init();
		if(result == SD_OK)			// translate the reslut code here
		{
			//printf(" SD Card Initial OK \r\n");
			stat = STA_OK; //respond the sucessfuly result 
		}
		else
		{
			//printf(" SD Card Initial Fail \r\n");
			stat = STA_NOINIT;
		}
		return stat;
	}
	return STA_NOINIT;
}



/*-----------------------------------------------------------------------*/
/* Read Sector(s)                                                        */
/*-----------------------------------------------------------------------*/

DRESULT disk_read (
	BYTE pdrv,		/* Physical drive nmuber to identify the drive */
	BYTE *buff,		/* Data buffer to store read data */
	DWORD sector,	/* Sector address in LBA */
	UINT count		/* Number of sectors to read */
)
{
	//DRESULT res;

	switch (pdrv) {
	case SDCARD :
		
		{
			SD_Error res = SD_OK;

			if (!count)
			{    
						return RES_PARERR;  //仅支持单磁盘操作，count不能等于0，否则返回参数错误
			}
				
			if(count==1)
			{
				res = SD_ReadBlock(buff,sector << 9  ,SD_SECTOR_SIZE);        
			}
			else
			{
				res =  SD_ReadMultiBlocks(buff,sector << 9 ,SD_SECTOR_SIZE,count);        
			} 
			
			if(res == SD_OK)
			{
						return RES_OK;
			}
			else
			{					
						return RES_ERROR;
			} 
		}

	}

	return RES_PARERR;
}



/*-----------------------------------------------------------------------*/
/* Write Sector(s)                                                       */
/*-----------------------------------------------------------------------*/

#if _USE_WRITE
DRESULT disk_write (
	BYTE pdrv,			/* Physical drive nmuber to identify the drive */
	const BYTE *buff,	/* Data to be written */
	DWORD sector,		/* Sector address in LBA */
	UINT count			/* Number of sectors to write */
)
{
	//DRESULT res;
	//int result;

	switch (pdrv) {
	case SDCARD :
		{
			SD_Error res = SD_OK;

			if (!count)
			{    
						return RES_PARERR;  //仅支持单磁盘操作，count不能等于0，否则返回参数错误
			}
				
			if(count==1)
			{
				res = SD_WriteBlock((u8*)buff,sector << 9  ,SD_SECTOR_SIZE);        
			}
			else
			{
				res =  SD_WriteMultiBlocks((u8*)buff,sector << 9 ,SD_SECTOR_SIZE,count);        
			} 
			
			if(res == SD_OK)
			{
						return RES_OK;
			}
			else
			{			
						return RES_ERROR;
			}
		}
	}

	return RES_PARERR;
}
#endif


/*-----------------------------------------------------------------------*/
/* Miscellaneous Functions                                               */
/*-----------------------------------------------------------------------*/

#if _USE_IOCTL
DRESULT disk_ioctl (
	BYTE pdrv,		/* Physical drive nmuber (0..) */
	BYTE cmd,		/* Control code */
	void *buff		/* Buffer to send/receive control data */
)
{
	DRESULT res = RES_OK;
	//int result;

	switch (pdrv) {
	case SDCARD :

		// Process of the command for the SDCARD drive

		return res;

	}

	return RES_PARERR;
}
#endif


DWORD get_fattime (void)
{   
	return 0;
}
