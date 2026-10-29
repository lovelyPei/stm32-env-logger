#include "stm32f10x.h"                  // Device header
#include "Delay.h"
#include "OLED.h"
#include "MPU6050.h"
#include "W25Q64.h"

uint8_t ID;								//定义用于存放ID号的变量
int16_t AX, AY, AZ, GX, GY, GZ;			//定义用于存放各个数据的变量
uint8_t MID;						//定义用于存放W25Q64的ID号的变量
uint16_t DID;
uint8_t test_w[4] = {0x00, 0x01, 0x02, 0x03};
uint8_t test_r[4];
	uint8_t i,ok = 1;

int main(void)
{
	/*模块初始化*/
	OLED_Init();		//OLED初始化
	MPU6050_Init();		//MPU6050初始化
	W25Q64_Init();		//W25Q64初始化
	
	/*显示ID号*/
	OLED_ShowString(1, 1, "W25Q64:");		//显示静态字符串
	W25Q64_ReadID(&MID, &DID);
	OLED_ShowHexNum(1, 8, MID, 2);
	OLED_ShowHexNum(1, 11, DID, 4);
	W25Q64_SectorErase(0x000000);
	W25Q64_PageProgram(0x000000, test_w, 4);
	W25Q64_ReadData(0x000000, test_r, 4);
	for(i = 0; i < 4; i++)
	{
		if(test_r[i] != test_w[i])
		{
			ok = 0;
			break;
		}
	}
	OLED_ShowString(2, 1, ok ? "RW: OK":"RW: FAIL");
	ID = MPU6050_GetID();				//获取MPU6050的ID号
	OLED_ShowString(3, 1, "MPUID:");		//显示静态字符串
	OLED_ShowHexNum(3, 7, ID, 2);		//OLED显示ID号

	Delay_ms(2000);
	OLED_Clear();
	
	while (1)
	{
		MPU6050_GetData(&AX, &AY, &AZ, &GX, &GY, &GZ);		//获取MPU6050的数据
		OLED_ShowSignedNum(2, 1, AX, 5);					//OLED显示数据
		OLED_ShowSignedNum(3, 1, AY, 5);
		OLED_ShowSignedNum(4, 1, AZ, 5);
		OLED_ShowSignedNum(2, 8, GX, 5);
		OLED_ShowSignedNum(3, 8, GY, 5);
		OLED_ShowSignedNum(4, 8, GZ, 5);

		Delay_ms(100);
	}
}
