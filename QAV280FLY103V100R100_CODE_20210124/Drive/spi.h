#ifndef _spi_H
#define _spi_H
#include "stm32f10x.h"


void SPI2_Init(void);  //SPI初始化
u8 SPI2_ReadWriteByte(u8 dat);		 //SPI2读写一个字节
void SPI2_SetSpeed(u8 SPI_BaudRatePrescaler);	//设置SPI2的速度


#endif
