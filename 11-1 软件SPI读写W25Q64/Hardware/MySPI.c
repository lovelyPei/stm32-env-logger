#include "stm32f10x.h"

void MySPI_W_SS(uint8_t BitValue)//主机发送一个字�?
{
    GPIO_WriteBit(GPIOA, GPIO_Pin_4, (BitAction)BitValue);
}

void MySPI_W_SCK(uint8_t BitValue)//主机发送一个字�?
{
    GPIO_WriteBit(GPIOA, GPIO_Pin_5, (BitAction)BitValue);
}

void MySPI_W_MOSI(uint8_t BitValue)//主机发送一个字�?
{
    GPIO_WriteBit(GPIOA, GPIO_Pin_7, (BitAction)BitValue);
}

uint8_t MySPI_R_MISO(void)
{
    return GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_6);
}

void MySPI_Init(void)
{
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
	
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;            //输出引脚配置为推挽输�?
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4 | GPIO_Pin_5 | GPIO_Pin_7;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;              //输入引脚配置为浮空或上拉输入    
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

    MySPI_W_SS(1);
    MySPI_W_SCK(0);
}

void MySPI_Start(void)//起始条件
{
    MySPI_W_SS(0);
}

void MySPI_Stop(void)//终止条件
{
    MySPI_W_SS(1);
}

uint8_t MySPI_SwapByte(uint8_t ByteSend)//交换一个字�?
{
    uint8_t i;  
    for(i = 0; i < 8; i++)
    {
        MySPI_W_MOSI(ByteSend & 0x80);      //主机发送按高到低逐一发送字�?
        ByteSend <<= 1;
        MySPI_W_SCK(1);
        if(MySPI_R_MISO() == 1) {ByteSend |= 0x01;}//主机接收字节
        MySPI_W_SCK(0);
    }      
    
    return ByteSend;
}
