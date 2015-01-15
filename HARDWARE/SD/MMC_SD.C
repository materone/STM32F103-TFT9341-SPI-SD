#include "sys.h"
#include "MMC_SD.h"
#include "spi.h"
#include "usart.h"
#include "delay.h" 							   
u8  SD_Type=0;//SD卡的类型
//////////////////////////////////////////////////////////////////////////////////	 
//本程序只供学习使用，未经作者许可，不得用于其它任何用途
//Mini STM32开发板
//SD卡 驱动代码		   
//正点原子@ALIENTEK
//技术论坛:www.openedv.com
//修改日期:2010/11/28 
//版本：V1.1
//版权所有，盗版必究。
//Copyright(C) 正点原子 2009-2019
//All rights reserved
//********************************************************************************
//V1.1修改说明
//2010/5/13									   
//增加了一些延时,实测可以支持TF卡(1G/2G),金士顿2G,4G 16G SD卡
//2010/6/24
//加入了u8 SD_GetResponse(u8 Response)函数
//修改了u8 SD_WaitDataReady(void)函数
//增加了USB读卡器支持的u8 MSD_ReadBuffer(u8* pBuffer, u32 ReadAddr, u32 NumByteToRead);
//和u8 MSD_WriteBuffer(u8* pBuffer, u32 WriteAddr, u32 NumByteToWrite);两个函数
////////////////////////////////////////////////////////////////////////////////// 	  
/*******************************************************************************
* Function Name  : SPI_ReadWriteByte
* Description    : SPI读写一个字节（发送完成后返回本次通讯读取的数据）
* Input          : u8 TxData 待发送的数
* Output         : None
* Return         : u8 RxData 收到的数
*******************************************************************************/
u8 SPI_ReadWriteByte(u8 TxData)
{
    u8 RxData = 0;
    
    //等待发送缓冲区空
    while(SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE) == RESET);
    //发一个字节
    SPI_I2S_SendData(SPI1, TxData);

    //等待数据接收
    while(SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_RXNE) == RESET);
    //取数据
    RxData = SPI_I2S_ReceiveData(SPI1);

    return (u8)RxData;
}

/*******************************************************************************
* Function Name  : SPI_SetSpeed
* Description    : SPI设置速度为高速
* Input          : u8 SpeedSet 
*                  如果速度设置输入0，则低速模式，非0则高速模式
*                  SPI_SPEED_HIGH   1
*                  SPI_SPEED_LOW    0
* Output         : None
* Return         : None
*******************************************************************************/
void SPI_SetSpeed1(u8 SpeedSet)
{
    SPI_InitTypeDef SPI_InitStructure;

    SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
	SPI_InitStructure.SPI_Mode = SPI_Mode_Master;
	SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;
	SPI_InitStructure.SPI_CPOL = SPI_CPOL_High;
	SPI_InitStructure.SPI_CPHA = SPI_CPHA_2Edge;
	SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;
    //如果速度设置输入0，则低速模式，非0则高速模式
    if(SpeedSet==0)
    {
        SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_256;
    }
    else
    {
        SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_4;
    }
	SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;
	SPI_InitStructure.SPI_CRCPolynomial = 7;
	SPI_Init(SPI1, &SPI_InitStructure);
    return;
}


/*******************************************************************************
* Function Name  : SD_WaitReady
* Description    : 等待SD卡Ready
* Input          : None
* Output         : None
* Return         : u8 
*                   0： 成功
*                   other：失败
*******************************************************************************/
u8 SD_WaitReady(void)
{
    u8 r1;
    u16 retry;
    retry = 0;
    do
    {
        r1 = SPI_ReadWriteByte(0xFF);
        if(retry==0xfffe)
        {
            return 1;
        }
    }while(r1!=0xFF);

    return 0;
} 

//等待SD卡回应
//Response:要得到的回应值
//返回值:0,成功得到了该回应值
//    其他,得到回应值失败
u8 SD_GetResponse(u8 Response)
{
	u16 Count=0xFFF;//等待次数	   						  
	while ((SPIx_ReadWriteByte(0XFF)!=Response)&&Count)Count--;//等待得到准确的回应  	  
	if (Count==0)return MSD_RESPONSE_FAILURE;//得到回应失败   
	else return MSD_RESPONSE_NO_ERROR;//正确回应
}
//等待SD卡写入完成
//返回值:0,成功;   
//    其他,错误代码;
u8 SD_WaitDataReady(void)
{
    u8 r1=MSD_DATA_OTHER_ERROR;
    u32 retry;
    retry=0;
    do
    {
        r1=SPIx_ReadWriteByte(0xFF)&0X1F;//读到回应
        if(retry==0xfffe)return 1; 
		retry++;
		switch (r1)
		{					   
			case MSD_DATA_OK://数据接收正确了	 
				r1=MSD_DATA_OK;
				break;  
			case MSD_DATA_CRC_ERROR:  //CRC校验错误
				return MSD_DATA_CRC_ERROR;  
			case MSD_DATA_WRITE_ERROR://数据写入错误
				return MSD_DATA_WRITE_ERROR;  
			default://未知错误    
				r1=MSD_DATA_OTHER_ERROR;
				break;	 
		}   
    }while(r1==MSD_DATA_OTHER_ERROR); //数据错误时一直等待
	retry=0;
	while(SPIx_ReadWriteByte(0XFF)==0)//读到数据为0,则数据还未写完成
	{
		retry++;
		//delay_us(10);//SD卡写等待需要较长的时间
		if(retry>=0XFFFFFFFE)return 0XFF;//等待失败了
	};	    
    return 0;//成功了
}	 
//向SD卡发送一个命令
//输入: u8 cmd   命令 
//      u32 arg  命令参数
//      u8 crc   crc校验值	   
//返回值:SD卡返回的响应															  
u8 SD_SendCommand(u8 cmd, u32 arg, u8 crc)
{
    u8 r1;	
	u8 Retry=0;	         
	SD_CS=1;
    SPIx_ReadWriteByte(0xff);//高速写命令延时
	SPIx_ReadWriteByte(0xff);     
 	SPIx_ReadWriteByte(0xff);  	 
    //片选端置低，选中SD卡
    SD_CS=0; 
    //发送
    SPIx_ReadWriteByte(cmd | 0x40);//分别写入命令
    SPIx_ReadWriteByte(arg >> 24);
    SPIx_ReadWriteByte(arg >> 16);
    SPIx_ReadWriteByte(arg >> 8);
    SPIx_ReadWriteByte(arg);
    SPIx_ReadWriteByte(crc); 
    //等待响应，或超时退出
    while((r1=SPIx_ReadWriteByte(0xFF))==0xFF)
    {
        Retry++;	    
        if(Retry>200)break; 
    }   
    //关闭片选
    SD_CS=1;
    //在总线上额外增加8个时钟，让SD卡完成剩下的工作
    SPIx_ReadWriteByte(0xFF);
    //返回状态值
    return r1;
}		  																				 
//向SD卡发送一个命令(结束是不失能片选，还有后续数据传来）
//输入:u8 cmd   命令 
//     u32 arg  命令参数
//     u8 crc   crc校验值	 
//返回值:SD卡返回的响应															  
u8 SD_SendCommand_NoDeassert(u8 cmd, u32 arg, u8 crc)
{
	u8 Retry=0;	         
	u8 r1;			   
    SPIx_ReadWriteByte(0xff);//高速写命令延时
	SPIx_ReadWriteByte(0xff);  	 	 
    SD_CS=0;//片选端置低，选中SD卡	   
    //发送
    SPIx_ReadWriteByte(cmd | 0x40); //分别写入命令
    SPIx_ReadWriteByte(arg >> 24);
    SPIx_ReadWriteByte(arg >> 16);
    SPIx_ReadWriteByte(arg >> 8);
    SPIx_ReadWriteByte(arg);
    SPIx_ReadWriteByte(crc);   
    //等待响应，或超时退出
    while((r1=SPIx_ReadWriteByte(0xFF))==0xFF)
    {
        Retry++;	    
        if(Retry>200)break; 
    }  	  
    //返回响应值
    return r1;
}
//把SD卡设置到挂起模式
//返回值:0,成功设置
//       1,设置失败
u8 SD_Idle_Sta(void)
{
	u16 i;
	u8 retry;	
    //for(i=0;i<0xf00;i++);//纯延时，等待SD卡上电完成	 
    //先产生>74个脉冲，让SD卡自己初始化完成
	//delay_ms(2000);
    for(i=0;i<10;i++)SPIx_ReadWriteByte(0xFF); 
    //-----------------SD卡复位到idle开始-----------------
    //循环连续发送CMD0，直到SD卡返回0x01,进入IDLE状态
    //超时则直接退出
    retry = 0;
    do
    {	   
        //发送CMD0，让SD卡进入IDLE状态
        i = SD_SendCommand(CMD0, 0, 0x95);
        retry++;
    }while((i!=0x01)&&(retry<200));
	printf("The SD init ret %d\r\n",i);
    //跳出循环后，检查原因：初始化成功？or 重试超时？
    if(retry==200)return 1; //失败
	return 0;//成功	 						  
}														    
//初始化SD卡
//如果成功返回,则会自动设置SPI速度为18Mhz
//返回值:0：NO_ERR
//       1：TIME_OUT
//      99：NO_CARD																 
u8 SD_Init(void)
{								 
    u8 r1;      // 存放SD卡的返回值
    u16 retry;  // 用来进行超时计数
    u8 buff[6];
    //设置硬件上与SD卡相关联的控制引脚输出
	//避免NRF24L01/W25X16等的影响
	RCC->APB2ENR|=1<<2;       //PORTA时钟使能 
	GPIOA->CRL&=0XFFF000FF; 
	GPIOA->CRL|=0X00033300;//PA2.3.4 推挽 	    
	GPIOA->ODR|=0X7<<2;    //PA2.3.4上拉 
	SPIx_Init();
 	//SPIx_SetSpeed(SPI_SPEED_256);//设置到低速模式		 
	SD_CS=1;	
	//GPIOA->BSRR=GPIO_Pin_3;
    if(SD_Idle_Sta()) return 1;//超时返回1 设置到idle 模式失败	  
    //-----------------SD卡复位到idle结束-----------------	 
    //获取卡片的SD版本信息
 	SD_CS=0;	
	r1 = SD_SendCommand_NoDeassert(8, 0x1aa,0x87);	     
    //如果卡片版本信息是v1.0版本的，即r1=0x05，则进行以下初始化
    if(r1 == 0x05)
    {
        //设置卡类型为SDV1.0，如果后面检测到为MMC卡，再修改为MMC
        SD_Type = SD_TYPE_V1;	   
        //如果是V1.0卡，CMD8指令后没有后续数据
        //片选置高，结束本次命令
        SD_CS=1;
        //多发8个CLK，让SD结束后续操作
        SPIx_ReadWriteByte(0xFF);	  
        //-----------------SD卡、MMC卡初始化开始-----------------	 
        //发卡初始化指令CMD55+ACMD41
        // 如果有应答，说明是SD卡，且初始化完成
        // 没有回应，说明是MMC卡，额外进行相应初始化
        retry = 0;
        do
        {
            //先发CMD55，应返回0x01；否则出错
            r1 = SD_SendCommand(CMD55, 0, 0);
            if(r1 == 0XFF)return r1;//只要不是0xff,就接着发送	  
            //得到正确响应后，发ACMD41，应得到返回值0x00，否则重试200次
            r1 = SD_SendCommand(ACMD41, 0, 0);
            retry++;
        }while((r1!=0x00) && (retry<400));
        // 判断是超时还是得到正确回应
        // 若有回应：是SD卡；没有回应：是MMC卡	  
        //----------MMC卡额外初始化操作开始------------
        if(retry==400)
        {
            retry = 0;
            //发送MMC卡初始化命令（没有测试）
            do
            {
                r1 = SD_SendCommand(1,0,0);
                retry++;
            }while((r1!=0x00)&& (retry<400));
            if(retry==400)return 1;   //MMC卡初始化超时		    
            //写入卡类型
            SD_Type = SD_TYPE_MMC;
        }
        //----------MMC卡额外初始化操作结束------------	    
        //设置SPI为高速模式
        SPIx_SetSpeed(SPI_SPEED_4);   
		SPIx_ReadWriteByte(0xFF);	 
        //禁止CRC校验	   
		r1 = SD_SendCommand(CMD59, 0, 0x95);
        if(r1 != 0x00)return r1;  //命令错误，返回r1   	   
        //设置Sector Size
        r1 = SD_SendCommand(CMD16, 512, 0x95);
        if(r1 != 0x00)return r1;//命令错误，返回r1		 
        //-----------------SD卡、MMC卡初始化结束-----------------

    }//SD卡为V1.0版本的初始化结束	 
    //下面是V2.0卡的初始化
    //其中需要读取OCR数据，判断是SD2.0还是SD2.0HC卡
    else if(r1 == 0x01)
    {
        //V2.0的卡，CMD8命令后会传回4字节的数据，要跳过再结束本命令
        buff[0] = SPIx_ReadWriteByte(0xFF);  //should be 0x00
        buff[1] = SPIx_ReadWriteByte(0xFF);  //should be 0x00
        buff[2] = SPIx_ReadWriteByte(0xFF);  //should be 0x01
        buff[3] = SPIx_ReadWriteByte(0xFF);  //should be 0xAA	    
        SD_CS=1;	  
        SPIx_ReadWriteByte(0xFF);//the next 8 clocks			 
        //判断该卡是否支持2.7V-3.6V的电压范围
        //if(buff[2]==0x01 && buff[3]==0xAA) //不判断，让其支持的卡更多
        {	  
            retry = 0;
            //发卡初始化指令CMD55+ACMD41
    		do
    		{
    			r1 = SD_SendCommand(CMD55, 0, 0);
    			if(r1!=0x01)return r1;	   
    			r1 = SD_SendCommand(ACMD41, 0x40000000, 0);
                if(retry>200)return r1;  //超时则返回r1状态  
            }while(r1!=0);		  
            //初始化指令发送完成，接下来获取OCR信息		   
            //-----------鉴别SD2.0卡版本开始-----------
            r1 = SD_SendCommand_NoDeassert(CMD58, 0, 0);
            if(r1!=0x00)
			{
				SD_CS=1;//释放SD片选信号
				return r1;  //如果命令没有返回正确应答，直接退出，返回应答	 
			}//读OCR指令发出后，紧接着是4字节的OCR信息
            buff[0] = SPIx_ReadWriteByte(0xFF);
            buff[1] = SPIx_ReadWriteByte(0xFF); 
            buff[2] = SPIx_ReadWriteByte(0xFF);
            buff[3] = SPIx_ReadWriteByte(0xFF);		 
            //OCR接收完成，片选置高
            SD_CS=1;
            SPIx_ReadWriteByte(0xFF);	   
            //检查接收到的OCR中的bit30位（CCS），确定其为SD2.0还是SDHC
            //如果CCS=1：SDHC   CCS=0：SD2.0
            if(buff[0]&0x40)SD_Type = SD_TYPE_V2HC;    //检查CCS	 
            else SD_Type = SD_TYPE_V2;	    
            //-----------鉴别SD2.0卡版本结束----------- 
            //设置SPI为高速模式
            SPIx_SetSpeed(SPI_SPEED_4);  
        }	    
    }
    return r1;
}	

u8 SD_Init1(void)
{
    u16 i;      // 用来循环计数
    u8 r1;      // 存放SD卡的返回值
    u16 retry;  // 用来进行超时计数
    u8 buff[6];

   
    // 纯延时，等待SD卡上电完成
   // for(i=0;i<0xf00;i++);
	delay_ms(2000);
    //先产生>74个脉冲，让SD卡自己初始化完成
    for(i=0;i<10;i++)
    {
        SPI_ReadWriteByte(0xFF);
    }

    //-----------------SD卡复位到idle开始-----------------
    //循环连续发送CMD0，直到SD卡返回0x01,进入IDLE状态
    //超时则直接退出
    retry = 0;
    do
    {
        //发送CMD0，让SD卡进入IDLE状态
        r1 = SD_SendCommand(CMD0, 0, 0x95);
        retry++;
    }while((r1 != 0x01) && (retry<200));
    //跳出循环后，检查原因：初始化成功？or 重试超时？
    if(retry==200)
    {
        return 1;   //超时返回1
    }
    //-----------------SD卡复位到idle结束-----------------



    //获取卡片的SD版本信息
    r1 = SD_SendCommand_NoDeassert(8, 0x1aa, 0x87);

    //如果卡片版本信息是v1.0版本的，即r1=0x05，则进行以下初始化
    if(r1 == 0x05)
    {
        //设置卡类型为SDV1.0，如果后面检测到为MMC卡，再修改为MMC
        SD_Type = SD_TYPE_V1;

        //如果是V1.0卡，CMD8指令后没有后续数据
        //片选置高，结束本次命令
        SD_CS = 1;
        //多发8个CLK，让SD结束后续操作
        SPI_ReadWriteByte(0xFF);

        //-----------------SD卡、MMC卡初始化开始-----------------

        //发卡初始化指令CMD55+ACMD41
        // 如果有应答，说明是SD卡，且初始化完成
        // 没有回应，说明是MMC卡，额外进行相应初始化
        retry = 0;
        do
        {
            //先发CMD55，应返回0x01；否则出错
            r1 = SD_SendCommand(CMD55, 0, 0);
            if(r1 != 0x01)
            {
                return r1;  
            }
            //得到正确响应后，发ACMD41，应得到返回值0x00，否则重试200次
            r1 = SD_SendCommand(ACMD41, 0, 0);
            retry++;
        }while((r1!=0x00) && (retry<400));
        // 判断是超时还是得到正确回应
        // 若有回应：是SD卡；没有回应：是MMC卡
        
        //----------MMC卡额外初始化操作开始------------
        if(retry==400)
        {
            retry = 0;
            //发送MMC卡初始化命令（没有测试）
            do
            {
                r1 = SD_SendCommand(1, 0, 0);
                retry++;
            }while((r1!=0x00)&& (retry<400));
            if(retry==400)
            {
                return 1;   //MMC卡初始化超时
            }
            //写入卡类型
            SD_Type = SD_TYPE_MMC;
        }
        //----------MMC卡额外初始化操作结束------------
        
        //设置SPI为高速模式
        SPI_SetSpeed1(1);

		SPI_ReadWriteByte(0xFF);
        
        //禁止CRC校验
        /*
		r1 = SD_SendCommand(CMD59, 0, 0x01);
        if(r1 != 0x00)
        {
            return r1;  //命令错误，返回r1
        }
        */    
        //设置Sector Size
        r1 = SD_SendCommand(CMD16, 512, 0xff);
        if(r1 != 0x00)
        {
            return r1;  //命令错误，返回r1
        }
        //-----------------SD卡、MMC卡初始化结束-----------------

    }//SD卡为V1.0版本的初始化结束
    

    //下面是V2.0卡的初始化
    //其中需要读取OCR数据，判断是SD2.0还是SD2.0HC卡
    else if(r1 == 0x01)
    {
        //V2.0的卡，CMD8命令后会传回4字节的数据，要跳过再结束本命令
        buff[0] = SPI_ReadWriteByte(0xFF);  //should be 0x00
        buff[1] = SPI_ReadWriteByte(0xFF);  //should be 0x00
        buff[2] = SPI_ReadWriteByte(0xFF);  //should be 0x01
        buff[3] = SPI_ReadWriteByte(0xFF);  //should be 0xAA
     
        SD_CS=1;
        //the next 8 clocks
        SPI_ReadWriteByte(0xFF);
        
        //判断该卡是否支持2.7V-3.6V的电压范围
        if(buff[2]==0x01 && buff[3]==0xAA)
        {
            //支持电压范围，可以操作
            retry = 0;
            //发卡初始化指令CMD55+ACMD41
    		do
    		{
    			r1 = SD_SendCommand(CMD55, 0, 0);
    			if(r1!=0x01)
    			{
    				return r1;
    			}
    			r1 = SD_SendCommand(ACMD41, 0x40000000, 0);
                if(retry>200)   
                {
                    return r1;  //超时则返回r1状态
                }
            }while(r1!=0);
          
            //初始化指令发送完成，接下来获取OCR信息

            //-----------鉴别SD2.0卡版本开始-----------
            r1 = SD_SendCommand_NoDeassert(CMD58, 0, 0);
            if(r1!=0x00)
            {
                return r1;  //如果命令没有返回正确应答，直接退出，返回应答
            }
            //读OCR指令发出后，紧接着是4字节的OCR信息
            buff[0] = SPI_ReadWriteByte(0xFF);
            buff[1] = SPI_ReadWriteByte(0xFF); 
            buff[2] = SPI_ReadWriteByte(0xFF);
            buff[3] = SPI_ReadWriteByte(0xFF);

            //OCR接收完成，片选置高
            SD_CS = 1;
            SPI_ReadWriteByte(0xFF);

            //检查接收到的OCR中的bit30位（CCS），确定其为SD2.0还是SDHC
            //如果CCS=1：SDHC   CCS=0：SD2.0
            if(buff[0]&0x40)    //检查CCS
            {
                SD_Type = SD_TYPE_V2HC;
            }
            else
            {
                SD_Type = SD_TYPE_V2;
            }
            //-----------鉴别SD2.0卡版本结束-----------

            
            //设置SPI为高速模式
            SPI_SetSpeed1(1);  
        }

    }
    return r1;
}
//从SD卡中读回指定长度的数据，放置在给定位置
//输入: u8 *data(存放读回数据的内存>len)
//      u16 len(数据长度）
//      u8 release(传输完成后是否释放总线CS置高 0：不释放 1：释放）	 
//返回值:0：NO_ERR
//  	 other：错误信息														  
u8 SD_ReceiveData(u8 *data, u16 len, u8 release)
{
    // 启动一次传输
    SD_CS=0;				  	  
	if(SD_GetResponse(0xFE))//等待SD卡发回数据起始令牌0xFE
	{	  
		SD_CS=1;
		return 1;
	}
    while(len--)//开始接收数据
    {
        *data=SPIx_ReadWriteByte(0xFF);
        data++;
    }
    //下面是2个伪CRC（dummy CRC）
    SPIx_ReadWriteByte(0xFF);
    SPIx_ReadWriteByte(0xFF);
    if(release==RELEASE)//按需释放总线，将CS置高
    {
        SD_CS=1;//传输结束
        SPIx_ReadWriteByte(0xFF);
    }											  					    
    return 0;
}																				  
//获取SD卡的CID信息，包括制造商信息
//输入: u8 *cid_data(存放CID的内存，至少16Byte）	  
//返回值:0：NO_ERR
//		 1：TIME_OUT
//       other：错误信息														   
u8 SD_GetCID(u8 *cid_data)
{
    u8 r1;	   
    //发CMD10命令，读CID
    r1 = SD_SendCommand(CMD10,0,0xFF);
    if(r1 != 0x00)return r1;  //没返回正确应答，则退出，报错  
    SD_ReceiveData(cid_data,16,RELEASE);//接收16个字节的数据	 
    return 0;
}																				  
//获取SD卡的CSD信息，包括容量和速度信息
//输入:u8 *cid_data(存放CID的内存，至少16Byte）	    
//返回值:0：NO_ERR
//       1：TIME_OUT
//       other：错误信息														   
u8 SD_GetCSD(u8 *csd_data)
{
    u8 r1;	 
    r1=SD_SendCommand(CMD9,0,0xFF);//发CMD9命令，读CSD
    if(r1)return r1;  //没返回正确应答，则退出，报错  
    SD_ReceiveData(csd_data, 16, RELEASE);//接收16个字节的数据 
    return 0;
}  
//获取SD卡的容量（字节）   
//返回值:0： 取容量出错 
//       其他:SD卡的容量(字节)														  
u32 SD_GetCapacity(void)
{
    u8 csd[16];
    u32 Capacity;
    u8 r1;
    u16 i;
	u16 temp;  					    
	//取CSD信息，如果期间出错，返回0
    if(SD_GetCSD(csd)!=0) return 0;	    
    //如果为SDHC卡，按照下面方式计算
    if((csd[0]&0xC0)==0x40)
    {									  
	    Capacity=((u32)csd[8])<<8;
		Capacity+=(u32)csd[9]+1;	 
        Capacity = (Capacity)*1024;//得到扇区数
		Capacity*=512;//得到字节数			   
    }
    else
    {		    
    	i = csd[6]&0x03;
    	i<<=8;
    	i += csd[7];
    	i<<=2;
    	i += ((csd[8]&0xc0)>>6);
        //C_SIZE_MULT
    	r1 = csd[9]&0x03;
    	r1<<=1;
    	r1 += ((csd[10]&0x80)>>7);	 
    	r1+=2;//BLOCKNR
    	temp = 1;
    	while(r1)
    	{
    		temp*=2;
    		r1--;
    	}
    	Capacity = ((u32)(i+1))*((u32)temp);	 
        // READ_BL_LEN
    	i = csd[5]&0x0f;
        //BLOCK_LEN
    	temp = 1;
    	while(i)
    	{
    		temp*=2;
    		i--;
    	}
        //The final result
    	Capacity *= (u32)temp;//字节为单位 	  
    }
    return (u32)Capacity;
}	    																			    
//读SD卡的一个block
//输入:u32 sector 取地址（sector值，非物理地址） 
//     u8 *buffer 数据存储地址（大小至少512byte） 		   
//返回值:0： 成功
//       other：失败															  
u8 SD_ReadSingleBlock(u32 sector, u8 *buffer)
{
	u8 r1;	    
    //设置为高速模式
    SPIx_SetSpeed(SPI_SPEED_4);  		   
    //如果不是SDHC，给定的是sector地址，将其转换成byte地址
    if(SD_Type!=SD_TYPE_V2HC)
    {
        sector = sector<<9;
    } 
	r1 = SD_SendCommand(CMD17, sector, 0);//读命令	 		    
	if(r1 != 0x00)return r1; 		   							  
	r1 = SD_ReceiveData(buffer, 512, RELEASE);		 
	if(r1 != 0)return r1;   //读数据出错！
    else return 0; 
}
////////////////////////////下面2个函数为USB读写所需要的/////////////////////////
//定义SD卡的块大小	 				   
#define BLOCK_SIZE 512 
//写入MSD/SD数据 
//pBuffer:数据存放区
//ReadAddr:写入的首地址
//NumByteToRead:要写入的字节数
//返回值:0,写入完成
//    其他,写入失败
u8 MSD_WriteBuffer(u8* pBuffer, u32 WriteAddr, u32 NumByteToWrite)
{
	u32 i,NbrOfBlock = 0, Offset = 0;
	u32 sector;
	u8 r1;
   	NbrOfBlock = NumByteToWrite / BLOCK_SIZE;//得到要写入的块的数目	    
    SD_CS=0;	  		   
	while (NbrOfBlock--)//写入一个扇区
	{
		sector=WriteAddr+Offset;
		if(SD_Type==SD_TYPE_V2HC)sector>>=9;//执行与普通操作相反的操作					  			 
		r1=SD_SendCommand_NoDeassert(CMD24,sector,0xff);//写命令   
 		if(r1)
		{
			SD_CS=1;
			return 1;//应答不正确，直接返回 	   
	    }
	    SPIx_ReadWriteByte(0xFE);//放起始令牌0xFE   
	    //放一个sector的数据
	    for(i=0;i<512;i++)SPIx_ReadWriteByte(*pBuffer++);  
	    //发2个Byte的dummy CRC
	    SPIx_ReadWriteByte(0xff);
	    SPIx_ReadWriteByte(0xff); 
 		if(SD_WaitDataReady())//等待SD卡数据写入完成
		{
			SD_CS=1;
			return 2;    
		}
		Offset += 512;	   
	}	    
    //写入完成，片选置1
    SD_CS=1;
    SPIx_ReadWriteByte(0xff);	 
    return 0;
}
//读取MSD/SD数据 
//pBuffer:数据存放区
//ReadAddr:读取的首地址
//NumByteToRead:要读出的字节数
//返回值:0,读出完成
//    其他,读出失败
u8 MSD_ReadBuffer(u8* pBuffer, u32 ReadAddr, u32 NumByteToRead)
{
	u32 NbrOfBlock=0,Offset=0;
	u32 sector=0;
	u8 r1=0;   	 
  	NbrOfBlock=NumByteToRead/BLOCK_SIZE;	  
    SD_CS=0;
	while (NbrOfBlock --)
	{	
		sector=ReadAddr+Offset;
		if(SD_Type==SD_TYPE_V2HC)sector>>=9;//执行与普通操作相反的操作					  			 
		r1=SD_SendCommand_NoDeassert(CMD17,sector,0xff);//读命令	 		    
		if(r1)//命令发送错误
		{
    		SD_CS=1;
			return r1;
		}	   							  
		r1=SD_ReceiveData(pBuffer,512,RELEASE);		 
		if(r1)//读数错误
		{
    		SD_CS=1;
			return r1;
		}
		pBuffer+=512;	 					    
	  	Offset+=512;				 	 
	}	 	 
    SD_CS=1;
    SPIx_ReadWriteByte(0xff);	 
    return 0;
}
//////////////////////////////////////////////////////////////////////////
//写入SD卡的一个block(未实际测试过)										    
//输入:u32 sector 扇区地址（sector值，非物理地址） 
//     u8 *buffer 数据存储地址（大小至少512byte） 		   
//返回值:0： 成功
//       other：失败															  
u8 SD_WriteSingleBlock(u32 sector, const u8 *data)
{
    u8 r1;
    u16 i;
    u16 retry;

    //设置为高速模式
    //SPIx_SetSpeed(SPI_SPEED_HIGH);	   
    //如果不是SDHC，给定的是sector地址，将其转换成byte地址
    if(SD_Type!=SD_TYPE_V2HC)
    {
        sector = sector<<9;
    }   
    r1 = SD_SendCommand(CMD24, sector, 0x00);
    if(r1 != 0x00)
    {
        return r1;  //应答不正确，直接返回
    }
    
    //开始准备数据传输
    SD_CS=0;
    //先放3个空数据，等待SD卡准备好
    SPIx_ReadWriteByte(0xff);
    SPIx_ReadWriteByte(0xff);
    SPIx_ReadWriteByte(0xff);
    //放起始令牌0xFE
    SPIx_ReadWriteByte(0xFE);

    //放一个sector的数据
    for(i=0;i<512;i++)
    {
        SPIx_ReadWriteByte(*data++);
    }
    //发2个Byte的dummy CRC
    SPIx_ReadWriteByte(0xff);
    SPIx_ReadWriteByte(0xff);
    
    //等待SD卡应答
    r1 = SPIx_ReadWriteByte(0xff);
    if((r1&0x1F)!=0x05)
    {
        SD_CS=1;
        return r1;
    }
    
    //等待操作完成
    retry = 0;
    while(!SPIx_ReadWriteByte(0xff))
    {
        retry++;
        if(retry>0xfffe)        //如果长时间写入没有完成，报错退出
        {
            SD_CS=1;
            return 1;           //写入超时返回1
        }
    }	    
    //写入完成，片选置1
    SD_CS=1;
    SPIx_ReadWriteByte(0xff);

    return 0;
}				           
//读SD卡的多个block(实际测试过)										    
//输入:u32 sector 扇区地址（sector值，非物理地址） 
//     u8 *buffer 数据存储地址（大小至少512byte）
//     u8 count 连续读count个block 		   
//返回值:0： 成功
//       other：失败															  
u8 SD_ReadMultiBlock(u32 sector, u8 *buffer, u8 count)
{
    u8 r1;	 			 
    //SPIx_SetSpeed(SPI_SPEED_HIGH);//设置为高速模式  
 	//如果不是SDHC，将sector地址转成byte地址
    if(SD_Type!=SD_TYPE_V2HC)sector = sector<<9;  
    //SD_WaitDataReady();
    //发读多块命令
	r1 = SD_SendCommand(CMD18, sector, 0);//读命令
	if(r1 != 0x00)return r1;	 
    do//开始接收数据
    {
        if(SD_ReceiveData(buffer, 512, NO_RELEASE) != 0x00)break; 
        buffer += 512;
    } while(--count);		 
    //全部传输完毕，发送停止命令
    SD_SendCommand(CMD12, 0, 0);
    //释放总线
    SD_CS=1;
    SPIx_ReadWriteByte(0xFF);    
    if(count != 0)return count;   //如果没有传完，返回剩余个数	 
    else return 0;	 
}											  
//写入SD卡的N个block(未实际测试过)									    
//输入:u32 sector 扇区地址（sector值，非物理地址） 
//     u8 *buffer 数据存储地址（大小至少512byte）
//     u8 count 写入的block数目		   
//返回值:0： 成功
//       other：失败															   
u8 SD_WriteMultiBlock(u32 sector, const u8 *data, u8 count)
{
    u8 r1;
    u16 i;	 		 
    //SPIx_SetSpeed(SPI_SPEED_HIGH);//设置为高速模式	 
    if(SD_Type != SD_TYPE_V2HC)sector = sector<<9;//如果不是SDHC，给定的是sector地址，将其转换成byte地址  
    if(SD_Type != SD_TYPE_MMC) r1 = SD_SendCommand(ACMD23, count, 0x00);//如果目标卡不是MMC卡，启用ACMD23指令使能预擦除   
    r1 = SD_SendCommand(CMD25, sector, 0x00);//发多块写入指令
    if(r1 != 0x00)return r1;  //应答不正确，直接返回	 
    SD_CS=0;//开始准备数据传输   
    SPIx_ReadWriteByte(0xff);//先放3个空数据，等待SD卡准备好
    SPIx_ReadWriteByte(0xff);   
    //--------下面是N个sector写入的循环部分
    do
    {
        //放起始令牌0xFC 表明是多块写入
        SPIx_ReadWriteByte(0xFC);	  
        //放一个sector的数据
        for(i=0;i<512;i++)
        {
            SPIx_ReadWriteByte(*data++);
        }
        //发2个Byte的dummy CRC
        SPIx_ReadWriteByte(0xff);
        SPIx_ReadWriteByte(0xff);
        
        //等待SD卡应答
        r1 = SPIx_ReadWriteByte(0xff);
        if((r1&0x1F)!=0x05)
        {
            SD_CS=1;    //如果应答为报错，则带错误代码直接退出
            return r1;
        }		   
        //等待SD卡写入完成
        if(SD_WaitDataReady()==1)
        {
            SD_CS=1;    //等待SD卡写入完成超时，直接退出报错
            return 1;
        }	   
    }while(--count);//本sector数据传输完成  
    //发结束传输令牌0xFD
    r1 = SPIx_ReadWriteByte(0xFD);
    if(r1==0x00)
    {
        count =  0xfe;
    }		   
    if(SD_WaitDataReady()) //等待准备好
	{
		SD_CS=1;
		return 1;  
	}
    //写入完成，片选置1
    SD_CS=1;
    SPIx_ReadWriteByte(0xff);  
    return count;   //返回count值，如果写完则count=0，否则count=1
}						  					  
//在指定扇区,从offset开始读出bytes个字节								    
//输入:u32 sector 扇区地址（sector值，非物理地址） 
//     u8 *buf     数据存储地址（大小<=512byte）
//     u16 offset  在扇区里面的偏移量
//     u16 bytes   要读出的字节数	   
//返回值:0： 成功
//       other：失败															   
u8 SD_Read_Bytes(unsigned long address,unsigned char *buf,unsigned int offset,unsigned int bytes)
{
    u8 r1;u16 i=0;  
    r1=SD_SendCommand(CMD17,address<<9,0);//发送读扇区命令      
    if(r1)return r1;  //应答不正确，直接返回
	SD_CS=0;//选中SD卡
	if(SD_GetResponse(0xFE))//等待SD卡发回数据起始令牌0xFE
	{
		SD_CS=1; //关闭SD卡
		return 1;//读取失败
	}	 
	for(i=0;i<offset;i++)SPIx_ReadWriteByte(0xff);//跳过offset位 
    for(;i<offset+bytes;i++)*buf++=SPIx_ReadWriteByte(0xff);//读取有用数据	
    for(;i<512;i++) SPIx_ReadWriteByte(0xff); 	 //读出剩余字节
    SPIx_ReadWriteByte(0xff);//发送伪CRC码
    SPIx_ReadWriteByte(0xff);  
    SD_CS=1;//关闭SD卡
	return 0;
}














