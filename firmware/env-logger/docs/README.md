# MPU6050 资料目录

## 核心文档

| 文件 | 内容 |
|---|---|
| `MPU-6000-Datasheet1_rev3.4.pdf` | 英文数据手册，PS-MPU-6000A-00 |
| `MPU-6000-Register-Map1_rev4.2.pdf` | 英文寄存器手册，RM-MPU-6000A-00 |
| `MPU-6050-中文资料.pdf` | 中文资料 |

## 补充资料

| 文件 | 内容 |
|---|---|
| `MPU6050-Sensor-Introduction.pdf` | 传感器介绍 |
| `MPU6050-DMP-Filter.pdf` | DMP / 滤波相关 |
| `MPU6050-Hardware-Offset-Registers.pdf` | 硬件偏移寄存器 |

## 速查

| 文件 | 内容 |
|---|---|
| `MPU6050-cheatsheet.md` | 本工程用到的寄存器、位定义、换算公式、burst read、10 道自测题 |

## 建议阅读顺序

1. **中文资料**：先建立整体印象。
2. **Datasheet**：Features → Block Diagram → Pinout → I2C → 电气特性。
3. **Register Map**：`WHO_AM_I` → `PWR_MGMT_1` → `CONFIG` → `GYRO_CONFIG` → `ACCEL_CONFIG` → `0x3B–0x48`。
4. **对照代码**：`MPU6050_Init()` 和 `MPU6050_GetData()` 每一行对应哪个寄存器。

## 注意

这些 PDF 是学习参考资料，体积较大。如果仓库要公开，建议不要提交 PDF：

```gitignore
docs/*.pdf
```

只提交 `MPU6050-cheatsheet.md` 和 `README.md` 即可。
