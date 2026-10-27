#ifndef __MYSPI_H
#define __MYSPI_H

#include "stm32f10x.h"

void MySPI_Init(void);
void MySPI_Start(void);//起始条件
void MySPI_Stop(void);//终止条件
uint8_t MySPI_SwapByte(uint8_t ByteSend);//交换一个字节

#endif
