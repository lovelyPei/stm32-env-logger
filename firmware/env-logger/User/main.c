#include "stm32f10x.h"                  // Device header
#include "Delay.h"
#include "OLED.h"
#include "MPU6050.h"
#include "W25Q64.h"

/* 一条记录：4 + 6*2 = 16 字节 */
typedef struct
{
    uint32_t time_s;    // 第几秒
    int16_t ax;         // 加速度 X
    int16_t ay;         // 加速度 Y
    int16_t az;         // 加速度 Z
    int16_t gx;         // 陀螺仪 X
    int16_t gy;         // 陀螺仪 Y
    int16_t gz;         // 陀螺仪 Z
} Record_t;

uint8_t ID;                             // MPU6050 ID
int16_t AX, AY, AZ, GX, GY, GZ;         // MPU6050 原始数据
uint8_t MID;                            // W25Q64 制造商 ID
uint16_t DID;                           // W25Q64 设备 ID
uint8_t test_w[4] = {0x00, 0x01, 0x02, 0x03};  // 自检写入数据
uint8_t test_r[4];                      // 自检读回数据

uint32_t record_count = 0;              // 已写记录数
uint32_t time_s = 0;                    // 秒计数

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

    /* 4. 数据记录主循环：每秒一条 */
    while (1)
    {
        MPU6050_GetData(&AX, &AY, &AZ, &GX, &GY, &GZ);

		rec.time_s = time_s;
		rec.ax = AX;
		rec.ay = AY;
		rec.az = AZ;
		rec.gx = GX;
		rec.gy = GY;
		rec.gz = GZ;

		addr = record_count * sizeof(Record_t);

		if(record_count % 256 == 0)
		{
			W25Q64_SectorErase(addr);
		}

		W25Q64_PageProgram(addr, (uint8_t *)&rec, sizeof(Record_t));

		W25Q64_ReadData(addr, (uint8_t *)&check, sizeof(Record_t));

		ok = 1;
		p1 = (uint8_t *)&rec;
		p2 = (uint8_t* )&check;

		for(i = 0; i < sizeof(Record_t); i ++)
		{
			if(p1[i] != p2[i])
			{
				ok = 0;
				break;
			}
		}

        /* OLED 显示：记录数、校验结果、AX、AY */
        OLED_ShowString(1, 1, "REC:");
        OLED_ShowNum(1, 5, record_count, 6);
        OLED_ShowString(2, 1, ok ? "LOG:OK" : "LOG:FAIL");
        OLED_ShowString(3, 1, "AX:");
        OLED_ShowSignedNum(3, 4, AX, 5);
        OLED_ShowString(4, 1, "AY:");
        OLED_ShowSignedNum(4, 4, AY, 5);

        /* 下一条 */
        record_count++;
        time_s++;

        Delay_ms(1000);
    }
}
