#include "stm32f10x.h"
#include "usart.h"
#include "ESP8266.h"

#define BUFSIZE 255
u8 ESP8266_RX_BUF[BUFSIZE];     //接收缓冲,最大64个字节.
//接收状态
//bit7，接收完成标志
//bit6，接收到0x0d
//bit5~0，接收到的有效字节数目
u8 ESP8266_RX_STA=0;       //接收状态标记

void uart2_init(u32 bound){
	//GPIO端口设置
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	 
	RCC_APB2PeriphClockCmd( RCC_APB2Periph_GPIOA|RCC_APB2Periph_AFIO, ENABLE);
	RCC_APB1PeriphClockCmd( RCC_APB1Periph_USART2, ENABLE);  
  //USART2_TX   PA.2
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
 
	//USART2_RX	  PA.3
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(GPIOA, &GPIO_InitStructure);  

  //Usart2 NVIC 配置

	NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=3 ;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;		//

	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//IRQ通道使能
	NVIC_Init(&NVIC_InitStructure);	//根据NVIC_InitStruct中指定的参数初始化外设NVIC寄存器USART1
  
   //USART 初始化设置
   
	USART_InitStructure.USART_BaudRate = bound;//一般设置为9600;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;

	USART_Init(USART2, &USART_InitStructure);
	
	USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);//开启中断
 
	USART_Cmd(USART2, ENABLE);                    //使能串口 

}

void USART2_IRQHandler(void)                	//串口2中断服务程序
	{
	u8 Res;
	if(USART_GetITStatus(USART2, USART_IT_RXNE) != RESET)  //接收中断(接收到的数据必须是0x0d 0x0a结尾)
		{
		Res =USART_ReceiveData(USART2);//(USART2->DR);	//读取接收到的数据
		printf("%c --> %x",Res,Res);
		
//		if((USART_RX_STA&0x80)==0)//接收未完成
//			{
//			if(USART_RX_STA&0x40)//接收到了0x0d
//				{
//				if(Res!=0x0a)USART_RX_STA=0;//接收错误,重新开始
//				else USART_RX_STA|=0x80;	//接收完成了 
//				}
//			else //还没收到0X0D
//				{	
//				if(Res==0x0d)USART_RX_STA|=0x40;
//				else
//					{
//					USART_RX_BUF[USART_RX_STA&0X3F]=Res ;
//					USART_RX_STA++;
//					if(USART_RX_STA>BUFSIZE)USART_RX_STA=0;//接收数据错误,重新开始接收	  
//					}		 
//				}
//			}   		 
     } 
		printf("  In U2 \n");
} 
	
void wifi_init(u32 band){
	// init usart2 PA2,PA3
	uart2_init(band);
	esp_write("\r\n");
	esp_write("AT+GMR\r\n");
}

int esp_write(u8 *str)
{
	while(*str!='\0'){
		while(USART_GetFlagStatus(USART2,USART_FLAG_TC)!=SET);
		USART_SendData(USART2, *str);
//		USART1->DR = (u8) *str;  
//		while((USART2->SR&0X40)==0);//循环发送,直到发送完毕   
		str++;
	}
	return 1;
}

