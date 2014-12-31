#include "stm32f10x.h"
#include "TFT_Driver.h"
#include "delay.h"
#include "font.h"

//spi 写一个字节
u8 SPI_WriteByte(SPI_TypeDef* SPIx,u8 Byte)
{
	while((SPIx->SR&SPI_I2S_FLAG_TXE)==RESET);		//等待发送区空	  
	SPIx->DR=Byte;	 	//发送一个byte   
	while((SPIx->SR&SPI_I2S_FLAG_RXNE)==RESET);//等待接收完一个byte  
	return SPIx->DR;          	     //返回收到的数据			
} 

//设置SPI的速度
//SpeedSet:1,高速;0,低速;
void SPI_SetSpeed(SPI_TypeDef* SPIx,u8 SpeedSet)
{
	SPIx->CR1&=0XFFC7;
	if(SpeedSet==1)//高速
	{
		SPIx->CR1|=SPI_BaudRatePrescaler_2;//Fsck=Fpclk/2	
	}
	else//低速
	{
		SPIx->CR1|=SPI_BaudRatePrescaler_32; //Fsck=Fpclk/32
	}
	SPIx->CR1|=1<<6; //SPI设备使能
} 

//硬件SPI配置
void SPI2_Init(void)	
{
	SPI_InitTypeDef  SPI_InitStructure;
	GPIO_InitTypeDef GPIO_InitStructure;
	 
	//配置SPI2管脚
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO|RCC_APB2Periph_GPIOB, ENABLE);
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13 | GPIO_Pin_15;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_Init(GPIOB, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_14;    
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU; 
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;  
	GPIO_Init(GPIOB, &GPIO_InitStructure);  

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9|GPIO_Pin_10| GPIO_Pin_11| GPIO_Pin_12;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	
	//SPI2配置选项
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_SPI2 ,ENABLE);
	   
	SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
	SPI_InitStructure.SPI_Mode = SPI_Mode_Master;
	SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;
	SPI_InitStructure.SPI_CPOL = SPI_CPOL_High;
	SPI_InitStructure.SPI_CPHA = SPI_CPHA_2Edge;
	SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;
	SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_2;
	SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;
	SPI_InitStructure.SPI_CRCPolynomial = 7;
	SPI_Init(SPI2, &SPI_InitStructure);

	//使能SPI2
	SPI_Cmd(SPI2, ENABLE);   
}

void TFT_WriteIndex(u8 Index)
{
   TFT_RS_CLR;
   SPI_WriteByte(SPI2,Index);
}

void TFT_WriteData(u8 Data)
{
   TFT_RS_SET;
   SPI_WriteByte(SPI2,Data);
}

void TFT_WriteData16Bit(u8 DataH,u8 DataL)
{
	TFT_WriteData(DataH);
	TFT_WriteData(DataL);
}

void TFT_WriteIndex16Bit(u8 DataH,u8 DataL)
{
	TFT_WriteIndex(DataH);
	TFT_WriteIndex(DataL);
}

void TFT_Init(void)
{
	SPI2_Init();
	TFT_RST_CLR;
	delay_ms(100);
	TFT_RST_SET;
	delay_ms(50);
	TFT_WriteIndex(0xCB);  
  TFT_WriteData(0x39); 
  TFT_WriteData(0x2C); 
  TFT_WriteData(0x00); 
  TFT_WriteData(0x34); 
  TFT_WriteData(0x02); 
  TFT_WriteIndex(0xCF);  
  TFT_WriteData(0x00); 
  TFT_WriteData(0XC1); 
  TFT_WriteData(0X30); 
  TFT_WriteIndex(0xE8);  
  TFT_WriteData(0x85); 
  TFT_WriteData(0x00); 
  TFT_WriteData(0x78); 
  TFT_WriteIndex(0xEA);  
  TFT_WriteData(0x00); 
  TFT_WriteData(0x00); 
  TFT_WriteIndex(0xED);  
  TFT_WriteData(0x64); 
  TFT_WriteData(0x03); 
  TFT_WriteData(0X12); 
  TFT_WriteData(0X81); 
  TFT_WriteIndex(0xF7);  
  TFT_WriteData(0x20); 
  TFT_WriteIndex(0xC0);    //Power control 
  TFT_WriteData(0x23);   //VRH[5:0] 
  TFT_WriteIndex(0xC1);    //Power control 
  TFT_WriteData(0x10);   //SAP[2:0];BT[3:0] 
  TFT_WriteIndex(0xC5);    //VCM control 
  TFT_WriteData(0x3e); //对比度调节
  TFT_WriteData(0x28); 
  TFT_WriteIndex(0xC7);    //VCM control2 
  TFT_WriteData(0x86);  //--
  TFT_WriteIndex(0x36);    // Memory Access Control 
#ifdef H_VIEW
        TFT_WriteData(0xE8); //C8	   //48 68竖屏//28 E8 横屏
#else
				TFT_WriteData(0x48); 
#endif
        TFT_WriteIndex(0x3A);    
        TFT_WriteData(0x55); 
        TFT_WriteIndex(0xB1);    
        TFT_WriteData(0x00);  
        TFT_WriteData(0x18); 
        TFT_WriteIndex(0xB6);    // Display Function Control 
        TFT_WriteData(0x08); 
        TFT_WriteData(0x82);
        TFT_WriteData(0x27);  
        TFT_WriteIndex(0xF2);    // 3Gamma Function Disable 
        TFT_WriteData(0x00); 
        TFT_WriteIndex(0x26);    //Gamma curve selected 
        TFT_WriteData(0x01); 
        TFT_WriteIndex(0xE0);    //Set Gamma 
        TFT_WriteData(0x0F); 
        TFT_WriteData(0x31); 
        TFT_WriteData(0x2B); 
        TFT_WriteData(0x0C); 
        TFT_WriteData(0x0E); 
        TFT_WriteData(0x08); 
        TFT_WriteData(0x4E); 
        TFT_WriteData(0xF1); 
        TFT_WriteData(0x37); 
        TFT_WriteData(0x07); 
        TFT_WriteData(0x10); 
        TFT_WriteData(0x03); 
        TFT_WriteData(0x0E); 
        TFT_WriteData(0x09); 
        TFT_WriteData(0x00); 
        TFT_WriteIndex(0XE1);    //Set Gamma 
        TFT_WriteData(0x00); 
        TFT_WriteData(0x0E); 
        TFT_WriteData(0x14); 
        TFT_WriteData(0x03); 
        TFT_WriteData(0x11); 
        TFT_WriteData(0x07); 
        TFT_WriteData(0x31); 
        TFT_WriteData(0xC1); 
        TFT_WriteData(0x48); 
        TFT_WriteData(0x08); 
        TFT_WriteData(0x0F); 
        TFT_WriteData(0x0C); 
        TFT_WriteData(0x31); 
        TFT_WriteData(0x36); 
        TFT_WriteData(0x0F); 
        TFT_WriteIndex(0x11);    //Exit Sleep 
        delay_ms(120); 
			  TFT_WriteIndex(0x29);    //Display on 
        TFT_WriteIndex(0x2c); 
}


/*************************************************
函数名：TFT_Set_Region
功能：设置lcd显示区域，在此区域写点数据自动换行
入口参数：xy起点和终点,Y_IncMode表示先自增y再自增x
返回值：无
*************************************************/
void TFT_SetRegion(u16 x_start,u16 y_start,u16 x_end,u16 y_end)
{	
	TFT_WriteIndex(0x2a);
	TFT_WriteData16Bit(x_start>>8,x_start);
	TFT_WriteData16Bit(x_end>>8,x_end);
	TFT_WriteIndex(0x2b);
	TFT_WriteData16Bit(y_start>>8,y_start);
	TFT_WriteData16Bit(y_end>>8,y_end);
	TFT_WriteIndex(0x2c);

}

/*************************************************
函数名：TFT_Set_XY
功能：设置lcd显示起始点
入口参数：xy坐标
返回值：无
*************************************************/
void TFT_SetXY(u16 x,u16 y)
{
  TFT_WriteIndex(0x2a);
	TFT_WriteData16Bit(x>>8,x);
	TFT_WriteIndex(0x2b);
	TFT_WriteData16Bit(y>>8,y);
	TFT_WriteIndex(0x2c);
}

	
/*************************************************
函数名：TFT_DrawPoint
功能：画一个点
入口参数：无
返回值：无
*************************************************/
void TFT_DrawPoint(u16 x,u16 y,u16 Data)
{
	TFT_SetXY(x,y);
	TFT_WriteData(Data>>8);
	TFT_WriteData(Data);
}    

/*****************************************
 函数功能：读TFT某一点的颜色                          
 出口参数：color  点颜色值                                 
******************************************/
u16 TFT_ReadPoint(u16 x,u16 y)
{
  u8 r,g,b;
	u16 R,G,B,Data;
	TFT_SetXY(x,y);  
	TFT_CS_CLR;
	TFT_WriteIndex(0X2E); //第一次写入控制命令
  TFT_RS_SET;
	TFT_RS_CLR; //产生脉冲
  SPI_WriteByte(SPI2,0xff);//第二次空读写DUMMY CLOCK
	r=SPI_WriteByte(SPI2,0xff); //高六位有效
	g=SPI_WriteByte(SPI2,0xff); //高六位有效
	b=SPI_WriteByte(SPI2,0xff); //高六位有效
	TFT_CS_SET;
	R = (r<<1)&0x00FF;
	G = g&0x00FF;
	B = (b<<1)&0x00FF;
	Data = 	(R<<8)|(G<<5)|(B>>3);
	return Data;
}
/*
		r=LCD->LCD_RAM;  		  						
		delay_us(2);	  
		b=LCD->LCD_RAM; 
		g=r&0XFF;//9341要分两次读出
		g<<=8;
		return (((r>>11)<<11)|((g>>10)<<5)|(b>>11));
*/

/*************************************************
函数名：TFT_Clear
功能：全屏清屏函数
入口参数：填充颜色COLOR
返回值：无
*************************************************/
void TFT_Clear(u16 Color)               
{	
   unsigned int i,m;
   TFT_SetRegion(0,0,X_MAX_PIXEL-1,Y_MAX_PIXEL-1);
   TFT_RS_SET;
   
   for(i=0;i<Y_MAX_PIXEL;i++)
   {
    for(m=0;m<X_MAX_PIXEL;m++)
      {	 
		SPI_WriteByte(SPI2,Color>>8);
		SPI_WriteByte(SPI2,Color);
      }   
	}
}


void TFT_Circle(u16 X,u16 Y,u16 R,u16 fc) 
{//Bresenham算法 
    unsigned short  a,b; 
    int c; 
    a=0; 
    b=R; 
    c=3-2*R; 
    while (a<b) 
    { 
        TFT_DrawPoint(X+a,Y+b,fc);     //        7 
        TFT_DrawPoint(X-a,Y+b,fc);     //        6 
        TFT_DrawPoint(X+a,Y-b,fc);     //        2 
        TFT_DrawPoint(X-a,Y-b,fc);     //        3 
        TFT_DrawPoint(X+b,Y+a,fc);     //        8 
        TFT_DrawPoint(X-b,Y+a,fc);     //        5 
        TFT_DrawPoint(X+b,Y-a,fc);     //        1 
        TFT_DrawPoint(X-b,Y-a,fc);     //        4 

        if(c<0) c=c+4*a+6; 
        else 
        { 
            c=c+4*(a-b)+10; 
            b-=1; 
        } 
       a+=1; 
    } 
    if (a==b) 
    { 
        TFT_DrawPoint(X+a,Y+b,fc); 
        TFT_DrawPoint(X+a,Y+b,fc); 
        TFT_DrawPoint(X+a,Y-b,fc); 
        TFT_DrawPoint(X-a,Y-b,fc); 
        TFT_DrawPoint(X+b,Y+a,fc); 
        TFT_DrawPoint(X-b,Y+a,fc); 
        TFT_DrawPoint(X+b,Y-a,fc); 
        TFT_DrawPoint(X-b,Y-a,fc); 
    } 
	
} 
//画线函数，使用Bresenham 画线算法
void TFT_DrawLine(u16 x0, u16 y0,u16 x1, u16 y1,u16 Color)   
{
int dx,             // difference in x's
    dy,             // difference in y's
    dx2,            // dx,dy * 2
    dy2, 
    x_inc,          // amount in pixel space to move during drawing
    y_inc,          // amount in pixel space to move during drawing
    error,          // the discriminant i.e. error i.e. decision variable
    index;          // used for looping	

	TFT_SetXY(x0,y0);
	dx = x1-x0;//计算x距离
	dy = y1-y0;//计算y距离

	if (dx>=0)
	{
		x_inc = 1;
	}
	else
	{
		x_inc = -1;
		dx    = -dx;  
	} 
	
	if (dy>=0)
	{
		y_inc = 1;
	} 
	else
	{
		y_inc = -1;
		dy    = -dy; 
	} 

	dx2 = dx << 1;
	dy2 = dy << 1;

	if (dx > dy)//x距离大于y距离，那么每个x轴上只有一个点，每个y轴上有若干个点
	{//且线的点数等于x距离，以x轴递增画点
		// initialize error term
		error = dy2 - dx; 

		// draw the line
		for (index=0; index <= dx; index++)//要画的点数不会超过x距离
		{
			//画点
			TFT_DrawPoint(x0,y0,Color);
			
			// test if error has overflowed
			if (error >= 0) //是否需要增加y坐标值
			{
				error-=dx2;

				// move to next line
				y0+=y_inc;//增加y坐标值
			} // end if error overflowed

			// adjust the error term
			error+=dy2;

			// move to the next pixel
			x0+=x_inc;//x坐标值每次画点后都递增1
		} // end for
	} // end if |slope| <= 1
	else//y轴大于x轴，则每个y轴上只有一个点，x轴若干个点
	{//以y轴为递增画点
		// initialize error term
		error = dx2 - dy; 

		// draw the line
		for (index=0; index <= dy; index++)
		{
			// set the pixel
			TFT_DrawPoint(x0,y0,Color);

			// test if error overflowed
			if (error >= 0)
			{
				error-=dy2;

				// move to next line
				x0+=x_inc;
			} // end if error overflowed

			// adjust the error term
			error+=dx2;

			// move to the next pixel
			y0+=y_inc;
		} // end for
	} // end else |slope| > 1
}

void TFT_box(u16 x, u16 y, u16 w, u16 h,u16 bc)
{
	TFT_DrawLine(x,y,x+w,y,0xEF7D);
	TFT_DrawLine(x+w-1,y+1,x+w-1,y+1+h,0x2965);
	TFT_DrawLine(x,y+h,x+w,y+h,0x2965);
	TFT_DrawLine(x,y,x,y+h,0xEF7D);
    TFT_DrawLine(x+1,y+1,x+1+w-2,y+1+h-2,bc);
}
void TFT_box2(u16 x,u16 y,u16 w,u16 h, u8 mode)
{
	if (mode==0)	{
		TFT_DrawLine(x,y,x+w,y,0xEF7D);
		TFT_DrawLine(x+w-1,y+1,x+w-1,y+1+h,0x2965);
		TFT_DrawLine(x,y+h,x+w,y+h,0x2965);
		TFT_DrawLine(x,y,x,y+h,0xEF7D);
		}
	if (mode==1)	{
		TFT_DrawLine(x,y,x+w,y,0x2965);
		TFT_DrawLine(x+w-1,y+1,x+w-1,y+1+h,0xEF7D);
		TFT_DrawLine(x,y+h,x+w,y+h,0xEF7D);
		TFT_DrawLine(x,y,x,y+h,0x2965);
	}
	if (mode==2)	{
		TFT_DrawLine(x,y,x+w,y,0xffff);
		TFT_DrawLine(x+w-1,y+1,x+w-1,y+1+h,0xffff);
		TFT_DrawLine(x,y+h,x+w,y+h,0xffff);
		TFT_DrawLine(x,y,x,y+h,0xffff);
	}
}


/**************************************************************************************
功能描述: 在屏幕显示一凸起的按钮框
输    入: u16 x1,y1,x2,y2 按钮框左上角和右下角坐标
输    出: 无
**************************************************************************************/
void DisplayButtonUp(u16 x1,u16 y1,u16 x2,u16 y2)
{
	TFT_DrawLine(x1,  y1,  x2,y1, WHITE); //H
	TFT_DrawLine(x1,  y1,  x1,y2, WHITE); //V
	
	TFT_DrawLine(x1+1,y2-1,x2,y2-1, GRAY1);  //H
	TFT_DrawLine(x1,  y2,  x2,y2, GRAY2);  //H
	TFT_DrawLine(x2-1,y1+1,x2-1,y2, GRAY1);  //V
  TFT_DrawLine(x2  ,y1  ,x2,y2, GRAY2); //V
}

/**************************************************************************************
功能描述: 在屏幕显示一凹下的按钮框
输    入: u16 x1,y1,x2,y2 按钮框左上角和右下角坐标
输    出: 无
**************************************************************************************/
void DisplayButtonDown(u16 x1,u16 y1,u16 x2,u16 y2)
{
	TFT_DrawLine(x1,  y1,  x2,y1, GRAY2);  //H
	TFT_DrawLine(x1+1,y1+1,x2,y1+1, GRAY1);  //H
	TFT_DrawLine(x1,  y1,  x1,y2, GRAY2);  //V
	TFT_DrawLine(x1+1,y1+1,x1+1,y2, GRAY1);  //V
	TFT_DrawLine(x1,  y2,  x2,y2, WHITE);  //H
	TFT_DrawLine(x2,  y1,  x2,y2, WHITE);  //V
}

void TFT_DrawFont_GBK16(u16 x, u16 y, u16 fc, u16 bc, u8 *s)
{
	unsigned char i,j;
	unsigned short k,x0;
	x0=x;

	while(*s) 
	{	
		if((*s) < 128) 
		{
			k=*s;
			if (k==13) 
			{
				x=x0;
				y+=16;
			}
			else 
			{
				if (k>32) k-=32; else k=0;
	
			    for(i=0;i<16;i++)
				for(j=0;j<8;j++) 
					{
				    	if(asc16[k*16+i]&(0x80>>j))	TFT_DrawPoint(x+j,y+i,fc);
						else 
						{
							if (fc!=bc) TFT_DrawPoint(x+j,y+i,bc);
						}
					}
				x+=8;
			}
			s++;
		}
			
		else 
		{
		

			for (k=0;k<hz16_num;k++) 
			{
			  if ((hz16[k].Index[0]==*(s))&&(hz16[k].Index[1]==*(s+1)))
			  { 
				    for(i=0;i<16;i++)
				    {
						for(j=0;j<8;j++) 
							{
						    	if(hz16[k].Msk[i*2]&(0x80>>j))	TFT_DrawPoint(x+j,y+i,fc);
								else {
									if (fc!=bc) TFT_DrawPoint(x+j,y+i,bc);
								}
							}
						for(j=0;j<8;j++) 
							{
						    	if(hz16[k].Msk[i*2+1]&(0x80>>j))	TFT_DrawPoint(x+j+8,y+i,fc);
								else 
								{
									if (fc!=bc) TFT_DrawPoint(x+j+8,y+i,bc);
								}
							}
				    }
				}
			  }
			s+=2;x+=16;
		} 
		
	}
}

void TFT_DrawFont_GBK24(u16 x, u16 y, u16 fc, u16 bc, u8 *s)
{
	unsigned char i,j;
	unsigned short k;

	while(*s) 
	{
		if( *s < 0x80 ) 
		{
			k=*s;
			if (k>32) k-=32; else k=0;

		    for(i=0;i<16;i++)
			for(j=0;j<8;j++) 
				{
			    	if(asc16[k*16+i]&(0x80>>j))	
					TFT_DrawPoint(x+j,y+i,fc);
					else 
					{
						if (fc!=bc) TFT_DrawPoint(x+j,y+i,bc);
					}
				}
			s++;x+=8;
		}
		else 
		{

			for (k=0;k<hz24_num;k++) 
			{
			  if ((hz24[k].Index[0]==*(s))&&(hz24[k].Index[1]==*(s+1)))
			  { 
				    for(i=0;i<24;i++)
				    {
						for(j=0;j<8;j++) 
							{
						    	if(hz24[k].Msk[i*3]&(0x80>>j))
								TFT_DrawPoint(x+j,y+i,fc);
								else 
								{
									if (fc!=bc) TFT_DrawPoint(x+j,y+i,bc);
								}
							}
						for(j=0;j<8;j++) 
							{
						    	if(hz24[k].Msk[i*3+1]&(0x80>>j))	TFT_DrawPoint(x+j+8,y+i,fc);
								else {
									if (fc!=bc) TFT_DrawPoint(x+j+8,y+i,bc);
								}
							}
						for(j=0;j<8;j++) 
							{
						    	if(hz24[k].Msk[i*3+2]&(0x80>>j))	
								TFT_DrawPoint(x+j+16,y+i,fc);
								else 
								{
									if (fc!=bc) TFT_DrawPoint(x+j+16,y+i,bc);
								}
							}
				    }
			  }
			}
			s+=2;x+=24;
		}
	}
}
void TFT_DrawFont_Num32(u16 x, u16 y, u16 fc, u16 bc, u16 num)
{
	unsigned char i,j,k,c;
	//TFT_text_any(x+94+i*42,y+34,32,32,0x7E8,0x0,sz32,knum[i]);
//	w=w/8;

    for(i=0;i<32;i++)
	{
		for(j=0;j<4;j++) 
		{
			c=*(sz32+num*32*4+i*4+j);
			for (k=0;k<8;k++)	
			{
	
		    	if(c&(0x80>>k))	TFT_DrawPoint(x+j*8+k,y+i,fc);
				else {
					if (fc!=bc) TFT_DrawPoint(x+j*8+k,y+i,bc);
				}
			}
		}
	}
}


