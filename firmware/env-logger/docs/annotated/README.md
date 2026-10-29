# MPU6050 荧光笔标注版

原 PDF 保留在 `docs/`，这里是加了高亮的副本，原文件未被修改。

| 文件 | 高亮数 | 内容 |
|---|---|---|
| `MPU-6000-Datasheet1_rev3.4.pdf` | 120 | 英文数据手册 |
| `MPU-6000-Register-Map1_rev4.2.pdf` | 85 | 英文寄存器手册 |
| `MPU-6050-中文资料.pdf` | 85 | 中文资料 |

## 颜色说明

| 颜色 | 含义 |
|---|---|
| 黄色 | 章节标题、核心概念 |
| 绿色 | 引脚 / 接口 / 关键位（SCL、SDA、AD0、INT、SLEEP、CLKSEL 等） |
| 粉色 | 寄存器名（WHO_AM_I、PWR_MGMT_1、CONFIG、GYRO_CONFIG 等） |

## 各文档标注位置

### 英文 Datasheet

- p7：Product Overview / MPU-60X0 Overview
- p9：Applications
- p10：Features / Gyroscope / Accelerometer / Additional Features
- p11：MotionProcessing / Clocking
- p12：Electrical Characteristics
- p18：I2C Timing
- p20：Absolute Maximum Ratings
- p21：Pin Out and Signal Description（SCL/SDA/AD0/INT/VDD）
- p22：Typical Operating Circuit
- p23：Recommended Power-On Procedure
- p24：Block Diagram
- p25：Primary I2C and SPI Interfaces
- p31：Digital-Output Temperature Sensor
- p33：Digital Interface / I2C Interface / I2C Protocol
- p36：I2C Terms
- p38：Logic Levels

### 英文 Register Map

- p6–8：Register Map 总表（关键寄存器名）
- p11：Sample Rate Divider
- p13：Configuration（DLPF）
- p14：Gyroscope Configuration（FS_SEL）
- p15：Accelerometer Configuration（AFS_SEL）
- p29：Accelerometer Measurements
- p30：Temperature Measurement
- p31：Gyroscope Measurements
- p40：Power Management 1（SLEEP、CLKSEL）
- p42：Power Management 2
- p45：WHO AM I

### 中文资料

- p1–2：特征、陀螺仪、加速度、引脚、温度
- p5：寄存器、陀螺仪、加速度、I2C
- p7–11：寄存器、加速度、温度、I2C

## 怎么用

- 用 Edge / Chrome / Adobe Acrobat / SumatraPDF 打开。
- 高亮是标准 PDF 注释，可以删除、改色、加备注。
- 打印时如果不想带高亮，在打印设置里关闭“注释/Annotations”。

> 补充资料 `MPU6050-Sensor-Introduction.pdf`、`MPU6050-DMP-Filter.pdf`、`MPU6050-Hardware-Offset-Registers.pdf` 没有标注，保持原样。
