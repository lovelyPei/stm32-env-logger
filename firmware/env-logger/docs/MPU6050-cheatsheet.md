# MPU6050 参考资料与寄存器速查

## 1. 官方文档名称

| 文档 | 编号 | 内容 |
|---|---|---|
| MPU-6000/MPU-6050 Product Specification | PS-MPU-6000A-00 | 电气特性、引脚、I2C 时序、封装 |
| MPU-6000/MPU-6050 Register Map and Descriptions | RM-MPU-6000A-00 | 所有寄存器地址、位定义、复位值 |

## 2. 下载链接

优先看 GitHub 镜像里的原始 PDF（需要时开 Watt Toolkit 加速）：

- GitHub 镜像（datasheet）：  
  https://github.com/domede/CodeDroneDIY/tree/efa3c50eb8035c3e957722b09875f8045ff6eec0/MPU6050_datasheet
- 中文资料合集：  
  https://atomgit.com/Open-source-documentation-tutorial/58e4e
- 寄存器手册节选（第三方论文，含 RM-MPU-6000A-00）：  
  https://core.ac.uk/download/288501979.pdf
- Datasheet 节选（第三方论文，含 PS-MPU-6000A-00）：  
  https://upcommons.upc.edu/bitstreams/a029dd87-ff5b-4fa1-8abc-3a80d81cebdb/download

官方旧链接（现在可能重定向到搜索页，打不开就用上面的镜像）：

- https://invensense.tdk.com/wp-content/uploads/2015/02/MPU-6000-Datasheet1.pdf
- https://invensense.tdk.com/wp-content/uploads/2015/02/MPU-6000-Register-Map1.pdf
- TDK 产品页：https://product.tdk.com/en/search/sensor/mortion-inertial/imu/info?part_no=MPU-6050

> 你本地 `stm32入门教程\参考文档` 里只有 STM32 的文档，没有 MPU6050 的，所以需要外找。

## 3. 本工程用到的寄存器

| 寄存器 | 地址 | 本工程操作 | 作用 |
|---|---|---|---|
| `WHO_AM_I` | 0x75 | 读 | 默认 0x68，确认芯片在线 |
| `PWR_MGMT_1` | 0x6B | 写 0x01 | 退出睡眠；时钟源 = PLL X 轴陀螺仪 |
| `PWR_MGMT_2` | 0x6C | 写 0x00 | 六轴都不待机 |
| `SMPLRT_DIV` | 0x19 | 写 0x09 | 采样率 = 1kHz / (1+9) = 100Hz |
| `CONFIG` | 0x1A | 写 0x06 | DLPF_CFG=6，带宽约 5Hz，延迟约 19ms |
| `GYRO_CONFIG` | 0x1B | 写 0x18 | FS_SEL=3，量程 ±2000°/s |
| `ACCEL_CONFIG` | 0x1C | 写 0x18 | AFS_SEL=3，量程 ±16g |
| `ACCEL_XOUT_H`…`GYRO_ZOUT_L` | 0x3B–0x48 | 读 14 字节 | 加速度 6 + 温度 2 + 陀螺仪 6 |
| `TEMP_OUT_H/L` | 0x41/0x42 | 读 | 温度 |

### 关键位定义

- `PWR_MGMT_1` (0x6B)：bit7 DEVICE_RESET，bit6 SLEEP，bit5 CYCLE，bit4:3 STANDBY，bit2:0 CLKSEL。
  - 写 `0x01`：SLEEP=0，CLKSEL=1（PLL with X gyro reference）。
- `CONFIG` (0x1A)：bit5:3 EXT_SYNC_SET，bit2:0 DLPF_CFG。
  - `0x06`：DLPF_CFG=6 → 加速度/陀螺仪带宽 5Hz，延迟约 19ms。
- `GYRO_CONFIG` (0x1B)：bit4:3 FS_SEL。
  - `0x18`：FS_SEL=3 → ±2000°/s，灵敏度 16.4 LSB/(°/s)。
- `ACCEL_CONFIG` (0x1C)：bit4:3 AFS_SEL。
  - `0x18`：AFS_SEL=3 → ±16g，灵敏度 2048 LSB/g。

## 4. 数据换算

```c
// 加速度：±16g
float ax_g = ax_raw / 2048.0f;

// 陀螺仪：±2000°/s
float gx_dps = gx_raw / 16.4f;

// 温度
float temp_c = temp_raw / 340.0f + 36.53f;
```

> 原始数据是 **有符号 16 位、大端**：高字节在前，低字节在后。

## 5. I2C 地址

- 7 位地址：`0x68`（AD0 接 GND）或 `0x69`（AD0 接 VCC）。
- 8 位写地址：`0xD0` / `0xD2`；读地址：`0xD1` / `0xD3`。
- 本工程 `#define MPU6050_ADDRESS 0xD0`，所以 **AD0 必须接 GND**。

## 6. 读数据的两种方式

### 当前代码：逐寄存器读

`MPU6050_GetData` 对 0x3B–0x48 每个寄存器各发一次 I2C 读事务，共 14 次。  
优点：简单、好调试；缺点：慢。

### 优化：burst read（一次读 14 字节）

```
START
0xD0          // 写地址
0x3B          // 起始寄存器 ACCEL_XOUT_H
Repeated START
0xD1          // 读地址
读 14 字节：前 13 个回 ACK，最后一个回 NACK
STOP
```

这是项目 v2 可以做的优化，也是面试能讲的点：  
“为什么改 burst read？省了多少次起始/地址/应答？对 100Hz 采样有什么影响？”

## 7. 读文档的顺序

1. **Datasheet**：Features → Block Diagram → Pinout → I2C → Electrical Characteristics。
2. **Register Map**：WHO_AM_I → PWR_MGMT_1 → CONFIG → SMPLRT_DIV → GYRO_CONFIG → ACCEL_CONFIG → ACCEL_XOUT_H… → TEMP_OUT。
3. **对照代码**：
   - `MPU6050_Init()` 写了哪几个寄存器？为什么是这些值？
   - `MPU6050_GetData()` 读了哪几个地址？为什么先高字节后低字节？
   - `MPU6050_GetID()` 读 0x75，返回值应该是多少？

## 8. 自测题

1. 为什么 `PWR_MGMT_1` 写 `0x01` 而不是 `0x00`？
2. `SMPLRT_DIV = 0x09` 时采样率是多少？怎么算的？
3. `CONFIG = 0x06` 的 DLPF 带宽和延迟是多少？
4. `GYRO_CONFIG = 0x18` 和 `ACCEL_CONFIG = 0x18` 分别是什么量程？
5. 原始加速度怎么换算成 `g`？陀螺仪怎么换算成 `°/s`？
6. 温度换算公式是什么？
7. 为什么数据是大端？代码里怎么拼接高/低字节？
8. AD0 接 VCC 时，7 位地址和 8 位写地址分别是多少？
9. `WHO_AM_I` 返回 `0x00` 或 `0xFF` 通常说明什么？
10. 怎么把 14 次单字节读改成一次 burst read？
