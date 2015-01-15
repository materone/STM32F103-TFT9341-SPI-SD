/*-----------------------------------------------------------------------*/
/* Low level disk I/O module skeleton for FatFs     (C)ChaN, 2014        */
/*-----------------------------------------------------------------------*/
/* If a working storage control module is available, it should be        */
/* attached to the FatFs via a glue function rather than modifying it.   */
/* This is an example of glue functions to attach various exsisting      */
/* storage control modules to the FatFs module with a defined API.       */
/*-----------------------------------------------------------------------*/

#include "diskio.h"		/* FatFs lower layer API */
//#include "usbdisk.h"	/* Example: USB drive control */
//#include "atadrive.h"	/* Example: ATA drive control */
#include "MMC_SD.h"		/* Example: MMC/SDC contorl */
#include "spi.h"

/* Definitions of physical drive number for each drive */
#define ATA		1	/* Example: Map ATA drive to drive number 0 */
#define MMC		0	/* Example: Map MMC/SD card to drive number 1 */
#define USB		2	/* Example: Map USB drive to drive number 2 */
#define SPI_SPEED_HIGH 1
#define SPI_SPEED_LOW  0

DWORD get_fattime (void)
{				 
	return 0;
}

/*-----------------------------------------------------------------------*/
/* Get Drive Status                                                      */
/*-----------------------------------------------------------------------*/

DSTATUS disk_status (
	BYTE pdrv		/* Physical drive nmuber to identify the drive */
)
{
//	DSTATUS stat;
//	int result;

//	switch (pdrv) {
//	case ATA :
//		result = ATA_disk_status();

//		// translate the reslut code here

//		return stat;

//	case MMC :
//		result = MMC_disk_status();

//		// translate the reslut code here

//		return 0;

//	case USB :
//		result = USB_disk_status();

//		// translate the reslut code here

//		return stat;
//	}
//	return STA_NOINIT;
	return 0;
}



/*-----------------------------------------------------------------------*/
/* Inidialize a Drive                                                    */
/*-----------------------------------------------------------------------*/

DSTATUS disk_initialize (
	BYTE pdrv				/* Physical drive nmuber to identify the drive */
)
{
//	DSTATUS stat;
//	int result;

//	switch (pdrv) {
//	case ATA :
//		result = ATA_disk_initialize();

//		// translate the reslut code here

//		return stat;

//	case MMC :
//		result = MMC_disk_initialize();

//		// translate the reslut code here

//		return stat;

//	case USB :
//		result = USB_disk_initialize();

//		// translate the reslut code here

//		return stat;
//	}
//	return STA_NOINIT;

	u8 res=0;	    
	switch(pdrv)
	{
		case MMC://SD卡
			res = SD_Init();//SD_Initialize() 
		 	if(res)//STM32 SPI的bug,在sd卡操作失败的时候如果不执行下面的语句,可能导致SPI读写异常
			{
				SPI_SetSpeed1(SPI_SPEED_HIGH);
				SPIx_ReadWriteByte(0xff);//提供额外的8个时钟
				SPI_SetSpeed1(SPI_SPEED_LOW);
			}
  			break;
		default:
			res=1; 
	}		 
	if(res)return  STA_NOINIT;
	else return 0; //初始化成功
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
//	DRESULT res;
//	int result;

//	switch (pdrv) {
//	case ATA :
//		// translate the arguments here

//		result = ATA_disk_read(buff, sector, count);

//		// translate the reslut code here

//		return res;

//	case MMC :
//		// translate the arguments here

//		result = MMC_disk_read(buff, sector, count);

//		// translate the reslut code here

//		return res;

//	case USB :
//		// translate the arguments here

//		result = USB_disk_read(buff, sector, count);

//		// translate the reslut code here

//		return res;
//	}

//	return RES_PARERR;
	u8 res=0; 
    if (!count)return RES_PARERR;//count不能等于0，否则返回参数错误		 	 
	switch(pdrv)
	{
		case MMC://SD卡
			res=SD_ReadMultiBlock(sector,buff,count);	 
		 	if(res)//STM32 SPI的bug,在sd卡操作失败的时候如果不执行下面的语句,可能导致SPI读写异常
			{
				SPI_SetSpeed1(SPI_SPEED_HIGH);
				SPIx_ReadWriteByte(0xff);//提供额外的8个时钟
				SPI_SetSpeed1(SPI_SPEED_LOW);
			}
			break;		
		default:
			res=1; 
	}
   //处理返回值，将SPI_SD_driver.c的返回值转成ff.c的返回值
    if(res==0x00)return RES_OK;	 
    else return RES_ERROR;	
		
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
//	DRESULT res;
//	int result;

//	switch (pdrv) {
//	case ATA :
//		// translate the arguments here

//		result = ATA_disk_write(buff, sector, count);

//		// translate the reslut code here

//		return res;

//	case MMC :
//		// translate the arguments here

//		result = MMC_disk_write(buff, sector, count);

//		// translate the reslut code here

//		return res;

//	case USB :
//		// translate the arguments here

//		result = USB_disk_write(buff, sector, count);

//		// translate the reslut code here

//		return res;
//	}

//	return RES_PARERR;

	u8 res=0;  
    if (!count)return RES_PARERR;//count不能等于0，否则返回参数错误		 	 
	switch(pdrv)
	{
		case MMC://SD卡
			res=SD_WriteMultiBlock(sector,(u8*)buff,count);
			break;
		default:
			res=1; 
	}
    //处理返回值，将SPI_SD_driver.c的返回值转成ff.c的返回值
    if(res == 0x00)return RES_OK;	 
    else return RES_ERROR;	
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
DRESULT res;						  			     
	if(pdrv==MMC)//SD卡
	{
	    switch(cmd)
	    {
		    case CTRL_SYNC:
				SD_CS=0;
		        if(SD_WaitReady()==0)res = RES_OK; 
		        else res = RES_ERROR;	  
				SD_CS=1;
		        break;	 
		    case GET_SECTOR_SIZE:
		        *(WORD*)buff = 512;
		        res = RES_OK;
		        break;	 
		    case GET_BLOCK_SIZE:
		        *(WORD*)buff = 8;
		        res = RES_OK;
		        break;	 
		    case GET_SECTOR_COUNT:
		        *(DWORD*)buff = SD_GetCapacity()/512;
		        res = RES_OK;
		        break;
		    default:
		        res = RES_PARERR;
		        break;
	    }
	}else res=RES_ERROR;//其他的不支持
    return res;	
//	DRESULT res;
//	int result;

//	switch (pdrv) {
//	case ATA :

//		// Process of the command for the ATA drive

//		return res;

//	case MMC :

//		// Process of the command for the MMC/SD card

//		return res;

//	case USB :

//		// Process of the command the USB drive

//		return res;
//	}

//	return RES_PARERR;
}
#endif
