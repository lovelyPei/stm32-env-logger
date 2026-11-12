#include "stm32f10x.h"                  // Device header
#include <string.h>
#include <math.h>
#include "Delay.h"
#include "OLED.h"
#include "MPU6050.h"
#include "W25Q64.h"
#include "Serial.h"
#include "AD.h"
#include "Motor.h"

/* 一条记录：32 字节，正好 128 条一个 4KB 扇区 */
typedef struct
{
    uint32_t time_s;    // 第几秒
    int16_t ax;         // 加速度 X
    int16_t ay;         // 加速度 Y
    int16_t az;         // 加速度 Z
    int16_t gx;         // 陀螺仪 X
    int16_t gy;         // 陀螺仪 Y
    int16_t gz;         // 陀螺仪 Z
    uint16_t temp_raw;  // 热敏 ADC
    uint16_t light_raw; // 光敏 ADC
    uint16_t pot_raw;   // 电位器 ADC
    uint8_t fan_on;     // 风扇状态
    uint8_t reserved[9];// 填充到 32 字节
} Record_t;

#define RECORDS_PER_SECTOR (4096 / sizeof(Record_t))

uint8_t ID;                             // MPU6050 ID
int16_t AX, AY, AZ, GX, GY, GZ;         // MPU6050 原始数据
uint8_t MID;                            // W25Q64 制造商 ID
uint16_t DID;                           // W25Q64 设备 ID
uint8_t test_w[4] = {0x00, 0x01, 0x02, 0x03};  // 自检写入数据
uint8_t test_r[4];                      // 自检读回数据

uint32_t record_count = 0;              // 已写记录数
uint32_t time_s = 0;                    // 秒计数

uint16_t ADC_T, ADC_L, ADC_P;           // 热敏、光敏、电位器原始值
float temperature;                      // 热敏换算后的温度（°C）
float set_temp;                         // 设定温度（°C）
uint8_t fan_on;                         // 风扇状态

/* NTC 接 GND，固定 10k 接 VCC；Beta 公式换算温度 */
float ADC_ToTemp(uint16_t adc)
{
    float r_ntc;
    float temp_k;

    if (adc == 0 || adc >= 4095)
    {
        return -99.0f;
    }

    r_ntc = 10000.0f * (float)adc / (4095.0f - (float)adc);
    temp_k = 1.0f / (1.0f / 298.15f + (1.0f / 3950.0f) * logf(r_ntc / 10000.0f));
    return temp_k - 273.15f;
}

void DumpRecords(void)
{
    uint32_t i;
	Record_t rec;

	Serial_Printf("index,time_s,ax,ay,az,gx,gy,gz,temp,light,pot,fan\r\n");

	for(i = 0; i < record_count; i ++)
	{
		W25Q64_ReadData(i * sizeof(Record_t), (uint8_t *)&rec, sizeof(Record_t));
		Serial_Printf("%u,%u,%d,%d,%d,%d,%d,%d,%u,%u,%u,%u\r\n",
						i, rec.time_s, rec.ax, rec.ay, rec.az, rec.gx, rec.gy, rec.gz,
              (unsigned int)rec.temp_raw, (unsigned int)rec.light_raw,
              (unsigned int)rec.pot_raw, (unsigned int)rec.fan_on);
	}

	Serial_Printf("END\r\n");
}

int main(void)
{
    uint32_t i;
    uint8_t ok;
    Record_t rec;
    Record_t check;
    uint32_t addr;
    uint8_t *p1, *p2;

    /* 模块初始化 */
    OLED_Init();                        // OLED 初始化
    MPU6050_Init();                     // MPU6050 初始化
    W25Q64_Init();                      // W25Q64 初始化
    Serial_Init();                      // 串口初始化（9600）
    AD_Init();                          // ADC 初始化（PA0/PA1/PA2）
    Motor_Init();                       // 电机 PWM 初始化

    /* 1. 读 W25Q64 ID 并显示 */
    OLED_ShowString(1, 1, "W25Q64:");
    W25Q64_ReadID(&MID, &DID);
    OLED_ShowHexNum(1, 8, MID, 2);      // 制造商 ID，预期 EF
    OLED_ShowHexNum(1, 11, DID, 4);     // 设备 ID，预期 4017

    /* 2. W25Q64 读写自检：擦除、写 4 字节、读回比较 */
    W25Q64_SectorErase(0x000000);
    W25Q64_PageProgram(0x000000, test_w, 4);
    W25Q64_ReadData(0x000000, test_r, 4);

    ok = 1;
    for (i = 0; i < 4; i++)
    {
        if (test_r[i] != test_w[i])
        {
            ok = 0;
            break;
        }
    }
    OLED_ShowString(2, 1, ok ? "RW: OK" : "RW: FAIL");

    /* 3. 读 MPU6050 ID 并显示 */
    ID = MPU6050_GetID();
    OLED_ShowString(3, 1, "MPUID:");
    OLED_ShowHexNum(3, 7, ID, 2);       // 预期 68

    /* 自检结果显示 2 秒 */
    Delay_ms(2000);
    OLED_Clear();

    /* 串口 CSV 表头 */
    Serial_Printf("index,time_s,ax,ay,az,gx,gy,gz,temp,light,pot,fan\r\n");

    /* 4. 数据记录主循环：每秒一条 */
    while (1)
    {
		if(Serial_RxFlag == 1)
		{
			if(strcmp(Serial_RxPacket, "read") == 0)
			{
				DumpRecords();
			}
			else if(strcmp(Serial_RxPacket, "count") == 0)
			{
				Serial_Printf("Count = %u\r\n",record_count);
			}

						else if(strcmp(Serial_RxPacket, "adc") == 0)
			{
				Serial_Printf("ADC:%u,%u,%u,T=%dC,S=%dC,F=%d\r\n",
                              ADC_T, ADC_L, ADC_P, (int)temperature, (int)set_temp, fan_on);
			}

			Serial_RxFlag = 0;
		}


        MPU6050_GetData(&AX, &AY, &AZ, &GX, &GY, &GZ);

        /* 读三路 ADC 原始值 */
        ADC_T = AD_GetValue(ADC_Channel_0);   // 热敏 PA0
        ADC_L = AD_GetValue(ADC_Channel_1);   // 光敏 PA1
        ADC_P = AD_GetValue(ADC_Channel_2);   // 电位器 PA2
        temperature = ADC_ToTemp(ADC_T);

        /* 电位器映射成 20~40°C 设定温度 */
        set_temp = 20.0f + (ADC_P / 4095.0f) * 20.0f;

        /* 滞回控制：高于设定 +0.5 开，低于设定 -0.5 关 */
        if (temperature > set_temp + 0.5f)
        {
            fan_on = 1;
        }
        else if (temperature < set_temp - 0.5f)
        {
            fan_on = 0;
        }

        Motor_SetSpeed(fan_on ? 100 : 0);

		rec.time_s = time_s;
		rec.ax = AX;
		rec.ay = AY;
		rec.az = AZ;
		rec.gx = GX;
		rec.gy = GY;
		rec.gz = GZ;
        rec.temp_raw = ADC_T;
        rec.light_raw = ADC_L;
        rec.pot_raw = ADC_P;
        rec.fan_on = fan_on;
        memset(rec.reserved, 0, sizeof(rec.reserved));

		addr = record_count * sizeof(Record_t);

		if(record_count % RECORDS_PER_SECTOR == 0)
		{
			W25Q64_SectorErase(addr);
		}

		W25Q64_PageProgram(addr, (uint8_t *)&rec, (uint16_t)sizeof(Record_t));

		W25Q64_ReadData(addr, (uint8_t *)&check, sizeof(Record_t));

		ok = 1;
		p1 = (uint8_t *)&rec;
		p2 = (uint8_t *)&check;

		for(i = 0; i < sizeof(Record_t); i ++)
		{
			if(p1[i] != p2[i])
			{
				ok = 0;
				break;
			}
		}

        /* OLED 显示：记录数、三路 ADC、校验结果 */
        OLED_ShowString(1, 1, "REC:");
        OLED_ShowNum(1, 5, record_count, 6);

        OLED_ShowString(2, 1, "T:");
        OLED_ShowNum(2, 3, ADC_T, 4);
        OLED_ShowString(2, 8, "L:");
        OLED_ShowNum(2, 10, ADC_L, 4);

        OLED_ShowString(3, 1, "C:");
        OLED_ShowSignedNum(3, 3, (int32_t)temperature, 3);
        OLED_ShowString(3, 8, "P:");
        OLED_ShowNum(3, 10, ADC_P, 4);

        OLED_ShowString(4, 1, fan_on ? "F:ON  " : "F:OFF ");
        OLED_ShowString(4, 7, ok ? "LOG:OK" : "LOG:FAIL");

        /* 串口打印一行 CSV：索引,秒,ax,ay,az,gx,gy,gz */
        Serial_Printf("%u,%u,%d,%d,%d,%d,%d,%d,%u,%u,%u,%u\r\n",
                      record_count, time_s, AX, AY, AZ, GX, GY, GZ,
                      (unsigned int)ADC_T, (unsigned int)ADC_L,
                      (unsigned int)ADC_P, (unsigned int)fan_on);

        /* 下一条 */
        record_count++;
        time_s++;

        Delay_ms(1000);
    }
}
