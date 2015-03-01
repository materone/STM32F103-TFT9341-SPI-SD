#ifndef __ESP8266_DRIVER_H
#define __ESP8266_DRIVER_H
#include "stm32f10x.h"

extern u8 ESP8266_RX_BUF[255];     //接收缓冲,最255个字节.末字节为换行符 
extern u8 ESP8266_RX_STA;         //接收状态标记	

void wifi_init(u32 bound);
int esp_write(u8 *str);

#endif
