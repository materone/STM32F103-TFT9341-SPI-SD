#include "stm32f10x.h"
#include "TFT_Driver.h"
#include "delay.h"
#include "usart.h"
#include "MMC_SD.h"
#include "spi.h"
#include "timer.h"
#include "rtc.h"
#include "ff.h"  
#include "string.h"

//define
#define BUFFPIXEL 3*100
#define PICFILES	140;
uint8_t color18 = 1;
uint8_t feed = 1;

//var
FATFS fs;		 /* Work area (file system object) for logical drive */
FIL fsrc, fdst;	  /* file objects */
FRESULT res;
UINT br,bw;
uint16_t bmpWidth, bmpHeight;
uint8_t bmpDepth, bmpImageoffset;


char path0[20]="0:";
//char path0[512]="0:";
//char buffer[4096];   /* file copy buffer */
char buffer[10];   /* file copy buffer */
uint8_t textFileBuffer[] = "ABCDE";

GPIO_InitTypeDef GPIO_InitStructure;

//void Delayms(__IO uint32_t nCount);
int SDTest(void);
int FTTest(void);
int testRTC(void);
int bmpReadHeader(FIL *);
void bmpdraw(FIL *, uint8_t , uint8_t);
unsigned char Num[10]={0,1,2,3,4,5,6,7,8,9};
void Redraw_Mainmenu(void)
{
	TFT_Clear(GRAY0);
	TFT_DrawFont_GBK16(16,0,BLUE,GRAY0,"全动电子技术");
	TFT_DrawFont_GBK16(16,20,RED,GRAY0,"液晶测试程序");
	DisplayButtonDown(15,38,113,58); //x1,y1,x2,y2
	TFT_DrawFont_GBK16(16,40,YELLOW,GRAY0,"颜色填充测试");
	DisplayButtonUp(15,68,113,88); //x1,y1,x2,y2
	TFT_DrawFont_GBK16(16,70,BLUE,GRAY0,"文字显示测试");
	DisplayButtonUp(15,98,113,118); //x1,y1,x2,y2
	TFT_DrawFont_GBK16(16,100,RED,GRAY0,"图片显示测试");

	TFT_DrawFont_GBK16(16,120,BLUE,GRAY0,"S1:Move.	");
	TFT_DrawFont_GBK16(16,140,RED,GRAY0, "S2:Sellect  ");
	//delay_ms(1000);
	//delay_ms(1000);

	TFT_DrawFont_Num32(100,125,RED,GRAY0,Num[5]);
	delay_ms(1000);
	TFT_DrawFont_Num32(100,125,RED,GRAY0,Num[4]);
	delay_ms(1000);
	TFT_DrawFont_Num32(100,125,RED,GRAY0,Num[3]);
	delay_ms(1000);
	TFT_DrawFont_Num32(100,125,RED,GRAY0,Num[2]);
	delay_ms(1000);
	TFT_DrawFont_Num32(100,125,RED,GRAY0,Num[1]);
	delay_ms(1000);
	TFT_DrawFont_Num32(100,125,RED,GRAY0,Num[0]);	
}

void Num_Test(void)
{
	u8 i=0;
	TFT_Clear(GRAY0);
	TFT_DrawFont_GBK16(16,20,RED,GRAY0,"Num Test");
	delay_ms(1000);
	TFT_Clear(GRAY0);

	for(i=0;i<10;i++)
	{
	TFT_DrawFont_Num32((i%3)*40,32*(i/3)+30,RED,GRAY0,Num[i+1]);
	delay_ms(100);
	}
	delay_ms(1000);
	
}
//文字显示测试全动电子技术商城因为专注所以专业
void Font_Test(void)
{
	u8 x_offset;
#ifdef H_VIEW  //如果定义了横屏显示
	x_offset=40;
#else
	x_offset=0;
#endif
	TFT_Clear(GRAY0);
	TFT_DrawFont_GBK16(16,10,BLUE,GRAY0,  "   文字显示测试   ");

	delay_ms(1000);
	TFT_Clear(WHITE);
	TFT_DrawFont_GBK16(x_offset,10,RED,WHITE,  "	 全动电子技术有限公司	  ");
	TFT_DrawFont_GBK16(x_offset,30,RED,WHITE,  "		 QDtech .,LTD		 ");
	TFT_DrawFont_GBK24(x_offset,50,BLUE,WHITE, "		   欢迎您			");
	TFT_DrawFont_GBK16(x_offset,100,GREEN,WHITE,   "	 全程技术支持	 ");	
	TFT_DrawFont_GBK16(x_offset,120,RED,YELLOW,   "http://qdtech.taobao.com");
	TFT_DrawFont_GBK16(x_offset,140,RED,YELLOW,   "E-Mail:QDtech2008@gmail.com");
	TFT_DrawFont_GBK16(x_offset,160,RED,YELLOW,   "技术交流群:324828016");	
	TFT_DrawFont_GBK16(x_offset,180,BLUE,WHITE,   "Tel:15989313508 ");
	delay_ms(3000);	
	

}

void Color_Test(void)
{
	//u8 i=2;
	TFT_Clear(GRAY0);
	
	TFT_DrawFont_GBK16(20,10,BLUE,GRAY0,"Color Test");
	delay_ms(1000);

	//while(i--)
	//{
	  TFT_Clear(WHITE);
	  TFT_Clear(BLACK);
	  TFT_Clear(RED);
  	TFT_Clear(GREEN);
  	TFT_Clear(BLUE);
	//}		
}

//u16 ID=0;
u8 rdata = 255;
int main(void)
{
  //uint16_t a;
  /* System Clocks Configuration **********************************************/
  SystemInit();
  delay_init(72);//延时初始化 
  uart_init(9600);
	TFT_Init();			
	TFT_Clear(BLACK);
	TFT_LED_SET;
	//printf("Test RTC\r\n");		
	//testRTC();
	printf("Test SD\r\n");
	TFT_CS_SET;
	SDTest();
	TFT_CS_CLR;
  while(1) 
  {
		FTTest();
		TFT_Clear(BLUE);  
		//delay_ms(1000);
		delay_ms(1000);
		//TFT_Clear(BLACK); 
		TFT_Clear(WHITE);	

		Redraw_Mainmenu();//简单菜单测试
		Color_Test();//简单刷屏测试
		Num_Test();//数码管字体显示
		Font_Test();//中英文测试  
			
		TFT_Clear(BLACK);  
		//delay_ms(1000);
		delay_ms(1000);
		TFT_Clear(BLACK); 
		//TFT_Clear(WHITE);		
		TFT_DrawFont_GBK16(10,50,BLUE,GRAY0,"测试成功.");
		delay_ms(1000);
  }
 
}

u8 buf[512];//SD卡数据缓存区
int SDTest(void)
{	
	u32 sd_size;
//	u8 t=0;						
//  	Stm32_Clock_Init(9);//系统时钟设置
//	delay_init(72);		//延时初始化
//	uart_init(72,9600); //串口1初始化  	  
//	LCD_Init();			//初始化液晶 
//	LED_Init();		 //LED初始化		  														
//					   
// 	POINT_COLOR=RED;//设置字体为红色	   
//	LCD_ShowString(60,50,"Mini STM32");	
//	LCD_ShowString(60,70,"SD Card TEST");	
//	LCD_ShowString(60,90,"ATOM@ALIENTEK");
//	LCD_ShowString(60,110,"2011/1/1");
	u8 retry = 3;	
	while(SD_Init()!=0 && retry-->0) //检测不到SD卡
	{
		printf("SD Card Failed!\r\n");
		delay_ms(500);
		printf("Please Check!	  \r\n");
		delay_ms(500);
//		LED0=!LED0;//DS0闪烁
	}
	if(retry == 255)return 1;
	//检测SD卡成功 	
	sd_size=SD_GetCapacity();											
	printf("SD Card Checked OK \r\n");
	printf("SD Card Size:  %d Byte\r\n",sd_size);	
//	while(1)
//	{
//		if(t==30)//每6s钟执行一次
//		{
//			if(SD_ReadSingleBlock(0,buf)==0)//读取MBR扇区
//			{	
//				printf("USART1 Sending Data...");
//				printf("SECTOR 0 DATA:\n");
//				for(sd_size=0;sd_size<512;sd_size++)printf("%x ",buf[sd_size]);//打印MBR扇区数据		   
//				printf("\nDATA ENDED\n");
//				printf("USART1 Send Data Over!");
//			}
//			t=0;
//			break;
//		}   
//		t++;
//		delay_ms(200);
//		//LED0=!LED0;
//	}
	return 0;
}

//ALIENTEK Mini STM32开发板范例代码11
//RTC实时时钟 实验
//正点原子@ALIENTEK
//技术论坛:www.openedv.com	  
 	
const u8 *COMPILED_DATE=__DATE__;//获得编译日期
const u8 *COMPILED_TIME=__TIME__;//获得编译时间

const u8* Week[7]={"Sunday","Monday","Tuesday","Wednesday","Thursday","Friday","Saturday"};

 int testRTC(void)
 {
	u8 t=0;	
	RTC_Init();
	if(t!=timer.sec)
	{
		t=timer.sec;
		printf("%d-",timer.w_year);		
		printf("%d-",timer.w_month);	
		printf("%d  ",timer.w_date);	
		printf("%d:",timer.hour);	
		printf("%d:",timer.min);	
		printf("%d",timer.sec);	
		switch(timer.week)
		{
			case 0:
				printf("%s\n","Sunday   ");
				break;
			case 1:
				printf("%s\n","Monday   ");
				break;
			case 2:
				printf("%s\n","Tuesday  ");
				break;
			case 3:
				printf("%s\n","Wednesday");
				break;
			case 4:
				printf("%s\n","Thursday ");
				break;
			case 5:
				printf("%s\n","Friday   ");
				break;
			case 6:
				printf("%s\n","Saturday ");
				break;  
		}
	} 
	return 0;
 }
FRESULT scan_files (
	char* path		/* Start node to be scanned (also used as work area) */
)
{
	FRESULT res;
	FILINFO fno;
	DIR dir;
	int i;
	char *fn;   /* This function is assuming non-Unicode cfg. */
#if _USE_LFN
	static char lfn[_MAX_LFN + 1];
	fno.lfname = lfn;
	fno.lfsize = sizeof(lfn);
#endif


	res = f_opendir(&dir, path);					   /* Open the directory */
	if (res == FR_OK) {
		i = strlen(path);
		for (;;) {
			res = f_readdir(&dir, &fno);				   /* Read a directory item */
			if (res != FR_OK || fno.fname[0] == 0) break;  /* Break on error or end of dir */
			if (fno.fname[0] == '.') continue;			 /* Ignore dot entry */
#if _USE_LFN
			fn = *fno.lfname ? fno.lfname : fno.fname;
#else
			fn = fno.fname;
#endif
			if (fno.fattrib & AM_DIR) {					/* It is a directory */
				sprintf(&path[i], "/%s", fn);
				printf("scan file - %s\n\r",path);
				res = scan_files(path);
				if (res != FR_OK) break;
				path[i] = 0;
			} else {									   /* It is a file. */
				printf("scan file - %s/%s\n\r", path, fn);
			}
		}
	}else{
		printf("scan files error : %d\n\r",res);
	}

	return res;
}
/*******************************************************************************
  * @函数名称	SD_TotalSize
  * @函数说明   文件空间占用情况 
  * @输入参数   无 
  * @输出参数   无
  * @返回参数   1: 成功 
  				0: 失败
  * @注意事项	无
  *****************************************************************************/
int SD_TotalSize(char *path)
{
	FATFS *fs;
	DWORD fre_clust;		

	res = f_getfree(path, &fre_clust, &fs);  /* 必须是根目录，选择磁盘0 */
	if ( res==FR_OK ) 
	{
	  printf("\n\rget %s drive space.\n\r",path);
	  /* Print free space in unit of MB (assuming 512 bytes/sector) */
	  printf("%d MB total drive space.\r\n"
		   "%d MB available.\r\n",
		   ( (fs->n_fatent - 2) * fs->csize ) / 2 /1024 , (fre_clust * fs->csize) / 2 /1024 );
		
	  return 1;
	}
	else
	{ 
	  printf("\n\rGet total drive space faild!\n\r");
	  return 0;   
	}
}

/**
  * @brief  串口打印输出
  * @param  None
  * @retval None
  */
int FTTest(void)
{
	u8 res;
	uint16_t x = 0, y = 0;
	u16 cnt = 1;
	printf("Begin BMP File display test\r\n");
	TFT_CS_SET;
	//挂载文件系统
	res = f_mount(&fs,"0:",1);
	if(res != FR_OK){
		printf("mount filesystem 0 failed : %d\n\r",res);
	}

	//读文件测试
//	printf("read file test......\n\r");
//	res = f_open(&fsrc, "0:/1.TXT", FA_OPEN_EXISTING | FA_READ);
//	if(res != FR_OK){
//		printf("open file error : %d\n\r",res);
//	}else{
//		res = f_read(&fsrc, buffer, sizeof(textFileBuffer), &br);	 /* Read a chunk of src file */
//		if(res==FR_OK){
//			printf("read data num : %d\n\r",br);
//			printf("Cont:%s\n\r",buffer);
//		}else{
//			printf("read file error : %d\n\r",res);
//		}
//		/*close file */
//		f_close(&fsrc);
//	}
	
		
	//扫描已经存在的文件
//	printf("\n\rbegin scan files path0......\n\r");
	//scan_files(path0);

	//SD_TotalSize(path0);//获取SD容量
	
	//读BMP文件测试
	printf("read bmp file test......\n\r");
	
	for ( ;cnt < 140; cnt++){
		sprintf(path0,"0:/%d.BMP",cnt);
		printf(path0);
		res = f_open(&fsrc, path0, FA_OPEN_EXISTING | FA_READ);
		if(res != FR_OK){
			printf("open file error : %d\n\r",res);
		}else{
			bmpReadHeader(&fsrc);
			x = 0;
			y = 0;
			if (bmpWidth < 240);
			x = (240 - bmpWidth) / 2;
			if (bmpHeight < 320)
			  y = (320 - bmpHeight) / 2;
			if (bmpWidth % 4 == 0) {
			  feed = 0;
			} else {
			  feed = 1;
			}
			//clear screen
			TFT_CS_CLR;
			TFT_Clear(BLACK); 
			bmpdraw(&fsrc, x, y);
			/*close file */
			f_close(&fsrc);
		}
	}
	return 0;
}

/*********************************************/
// These read data from the SD card file and convert them
// to big endian(the data is stored in little endian format!)

// LITTLE ENDIAN!
uint16_t read16(FIL *f)
{
  uint16_t d;
	uint32_t rc;	
  uint8_t b,buff[1];
  f_read(f,buff,1,&rc);
	b = buff[0];
  f_read(f,buff,1,&rc);
	d=buff[0];
  d <<= 8;
  d |= b;
  return d;
}

// LITTLE ENDIAN!
uint32_t read32(FIL *f)
{
  uint32_t d;
  uint16_t b;

  b = read16(f);
  d = read16(f);
  d <<= 16;
  d |= b;
  return d;
}

int  bmpReadHeader(FIL *f) {
  // read header
  uint32_t tmp;

  if (read16(f) != 0x4D42) {
	// magic bytes missing
	return -1;
  }

  // read file size
  tmp = read32(f);
  printf("size %x\r\n",tmp);

  // read and ignore creator bytes
  read32(f);

  bmpImageoffset = read32(f);
  printf("offset %d\r\n",bmpImageoffset);

  // read DIB header
  tmp = read32(f);
  printf("header size %d\r\n",tmp);
  bmpWidth = read32(f);
  bmpHeight = read32(f);


  if (read16(f) != 1)
	return -1;

  bmpDepth = read16(f);
 printf("bitdepth %d\r\n",bmpDepth);

  if (read32(f) != 0) {
	// compression not supported!
	return -1;
  }

   printf("compression %d\r\n",tmp);

	printf("W:H %d:%d\r\n",bmpWidth,bmpHeight);
  return 1;
}

void bmpdraw(FIL *f, uint8_t x, uint8_t y)
{
  uint32_t rc;
  uint8_t  p, g, b;
  uint16_t i, j;
//  uint8_t temp[4];

  uint8_t sdbuffer[BUFFPIXEL];  // 3 * pixels to buffer
  uint16_t buffidx = BUFFPIXEL;
	TFT_WriteIndex(0x3A);   //set color 18 bit or 16bit 
  TFT_WriteData(0x66);	//55 -> 16 66->18
  f_lseek(f,bmpImageoffset);
//  Tft.setCol(x, bmpWidth + x - 1);
//  Tft.setPage(y, bmpHeight + y - 1);
	
	TFT_SetRegion(x,y,bmpWidth + x - 1,bmpHeight + y - 1);

  //  uint8_t buf[4];
  //  Tft.rcvData(0x09, buf, 4);
  //  for (uint8_t idx = 0; idx < 4; idx++) {
  //	log(idx);
  //	Serial.println(buf[idx], HEX);
  //  }
  //Tft.sendCMD(0x2c);
  TFT_WriteIndex(0x2c);
//  TFT_DC_HIGH;
//  TFT_CS_LOW;
	TFT_CS_CLR;
	TFT_RS_SET;
  for (i = 0; i < bmpHeight; i++)
  {
		for (j = 0; j < bmpWidth; j++)
		{
			// read more pixels
			if (buffidx >= BUFFPIXEL)
			{
			//TFT_CS_HIGH;
				TFT_CS_SET;
			 // bmpFile.read(sdbuffer, BUFFPIXEL);
				f_read(f,sdbuffer,BUFFPIXEL,&rc);
				buffidx = 0;
				//TFT_CS_LOW;		  
				TFT_CS_CLR;
			}
			b = sdbuffer[buffidx++];	 // blue
			g = sdbuffer[buffidx++];	 // green
			p = sdbuffer[buffidx++];	 // red
			
	//	  SPI.transfer(p );
	//	  SPI.transfer(g ); //&0xFC
	//	  SPI.transfer(b );
			SPI_WriteByte(SPI2,p);
			SPI_WriteByte(SPI2,g);
			SPI_WriteByte(SPI2,b);
		}
		//pad last bit,for bmp must 4 * byte per line
		if (feed) {
			uint8_t pad = bmpWidth % 4;
			if (buffidx >=  BUFFPIXEL) {
				//TFT_CS_HIGH;		  
				TFT_CS_SET;
				//bmpFile.seek(bmpFile.position() + pad);
				f_lseek(f,f_tell(f)+pad);
				//f_read(f,temp,pad,&rc);
				//TFT_CS_LOW;		  		  
				TFT_CS_CLR;
			} else if (pad == 3) {
				buffidx += 3;
			} else {
				memmove(sdbuffer + buffidx, sdbuffer + buffidx + pad, BUFFPIXEL - pad - buffidx);
	//		TFT_CS_HIGH;
	//		bmpFile.read(sdbuffer + BUFFPIXEL - pad, pad);
	//		TFT_CS_LOW;
				TFT_CS_SET;
				f_read(f,sdbuffer + BUFFPIXEL - pad,pad,&rc);		  		  
				TFT_CS_CLR;
			}
		}
  }
  //TFT_CS_HIGH;
//	TFT_CS_SET;
//  char s1[14 + sizeof(bmpfchar)];
//  sprintf(s1, "%s %i * %i", bmpfchar, bmpWidth, bmpHeight);
//  Tft.drawString(s1, 0, 0, 2, 0xFF00);
//  delay(100);
//  scrollV();
	TFT_CS_CLR;
	TFT_WriteIndex(0x3A);   //set color 18 bit or 16bit 
	TFT_WriteData(0x55);	//55 -> 16 66->18
	TFT_DrawFont_GBK16(0,0,BLUE,YELLOW,(u8 *)path0);
	delay_ms(5000);
// printf("Use %d ms",millis() - time);
//  Serial.println(" ms");
}


