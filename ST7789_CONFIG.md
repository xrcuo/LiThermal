# ST7789 3.2寸屏幕配置指南

## ✅ 已完成的配置修改

### 1. 新增的驱动文件
- `libs/lv_drivers/display/ST7789.c/h` - ST7789 LCD驱动
- `libs/lv_drivers/display/st7789_spi.c/h` - SPI接口实现

### 2. 配置文件更新
- **CMakeLists.txt** - 移除SUNXIFB定义，添加USE_ST7789
- **include/lv_drv_conf.h** - 启用ST7789驱动，配置分辨率320x240
- **src/myHAL.cpp** - 改用st7789驱动初始化

### 3. 分辨率配置
- 水平分辨率: 320像素
- 竖直分辨率: 240像素 (3.2寸标准分辨率)

## 🔧 需要配置的硬件接口

ST7789需要SPI硬件访问。根据你的Allwinner SoC平台，需要配置以下GPIO脚位：

### 在 `libs/lv_drivers/display/st7789_spi.h` 中修改GPIO编号

```c
/* 根据实际硬件电路修改这些GPIO编号 */
#define ST7789_SPI_CS_PIN    XX    /* 芯片选择引脚 */
#define ST7789_SPI_DC_PIN    YY    /* Data/Command引脚 */
#define ST7789_SPI_RST_PIN   ZZ    /* 复位引脚 */

#define ST7789_SPI_BUS       "/dev/spidev0.0"  /* SPI总线 */
```

### GPIO接线示意图
```
硬件引脚          SPI信号          Arduino引脚
-----------      -----------       -----------
VCC              3.3V
GND              地
MOSI     ------>  SPI_MOSI
MISO     <------  SPI_MISO  
SCK      ------>  SPI_CLK
CS       ------>  GPIO (ST7789_SPI_CS_PIN)
DC/RS    ------>  GPIO (ST7789_SPI_DC_PIN)
RES/RST  ------>  GPIO (ST7789_SPI_RST_PIN)
BL/LED   ------>  3.3V 或 PWM (背光控制，可选)
```

## 🛠️ 完整的SPI GPIO映射实现

在 `libs/lv_drivers/display/st7789_spi.c` 中的 `st7789_spi_set_dc()`, `st7789_spi_set_rst()`, `st7789_spi_set_cs()` 函数中，需要实现GPIO控制：

### 方法1：使用Linux GPIO sysfs (最简单)

```c
#include <stdio.h>
#include <stdlib.h>

static void gpio_export(int pin)
{
    FILE *f = fopen("/sys/class/gpio/export", "w");
    fprintf(f, "%d\n", pin);
    fclose(f);
}

static void gpio_set_direction(int pin, const char *dir)
{
    char path[64];
    FILE *f;
    snprintf(path, sizeof(path), "/sys/class/gpio/gpio%d/direction", pin);
    f = fopen(path, "w");
    fprintf(f, "%s\n", dir);
    fclose(f);
}

static void gpio_set_value(int pin, int value)
{
    char path[64];
    FILE *f;
    snprintf(path, sizeof(path), "/sys/class/gpio/gpio%d/value", pin);
    f = fopen(path, "w");
    fprintf(f, "%d\n", value);
    fclose(f);
}

/* 在st7789_spi_init()中初始化GPIO */
void st7789_spi_init(void)
{
    /* ... 现有SPI初始化代码 ... */
    
    /* 初始化GPIO */
    gpio_export(ST7789_SPI_CS_PIN);
    gpio_export(ST7789_SPI_DC_PIN);
    gpio_export(ST7789_SPI_RST_PIN);
    
    gpio_set_direction(ST7789_SPI_CS_PIN, "out");
    gpio_set_direction(ST7789_SPI_DC_PIN, "out");
    gpio_set_direction(ST7789_SPI_RST_PIN, "out");
    
    gpio_set_value(ST7789_SPI_CS_PIN, 1);    /* CS默认高 */
}

/* 实现GPIO控制函数 */
void st7789_spi_set_dc(int level)
{
    gpio_set_value(ST7789_SPI_DC_PIN, level);
}

void st7789_spi_set_rst(int level)
{
    gpio_set_value(ST7789_SPI_RST_PIN, level);
}

void st7789_spi_set_cs(int level)
{
    gpio_set_value(ST7789_SPI_CS_PIN, level);
}
```

### 方法2：使用WiringPi库 (如已安装)

```c
#include <wiringPi.h>

void st7789_spi_init(void)
{
    wiringPiSetup();
    
    pinMode(ST7789_SPI_CS_PIN, OUTPUT);
    pinMode(ST7789_SPI_DC_PIN, OUTPUT);
    pinMode(ST7789_SPI_RST_PIN, OUTPUT);
    
    digitalWrite(ST7789_SPI_CS_PIN, HIGH);
}

void st7789_spi_set_dc(int level)
{
    digitalWrite(ST7789_SPI_DC_PIN, level ? HIGH : LOW);
}

void st7789_spi_set_rst(int level)
{
    digitalWrite(ST7789_SPI_RST_PIN, level ? HIGH : LOW);
}

void st7789_spi_set_cs(int level)
{
    digitalWrite(ST7789_SPI_CS_PIN, level ? HIGH : LOW);
}
```

## 📝 编译步骤

1. **修改GPIO引脚编号**
   ```bash
   # 编辑 libs/lv_drivers/display/st7789_spi.h
   # 将 ST7789_SPI_CS_PIN, ST7789_SPI_DC_PIN, ST7789_SPI_RST_PIN 改为实际GPIO编号
   ```

2. **实现GPIO控制函数**
   ```bash
   # 编辑 libs/lv_drivers/display/st7789_spi.c
   # 选择方法1 (sysfs) 或方法2 (WiringPi) 实现GPIO函数
   ```

3. **编译项目**
   ```bash
   cd build
   cmake ..
   make -j32
   ```

## 🐛 常见问题排查

### 编译错误：找不到st7789_spi.h
→ 确保包含路径正确，检查 `lv_drv_conf.h` 中的 #include 语句

### 编译错误：未定义的引用 'st7789_spi_write_data'
→ 确保 st7789_spi.c 被编译，检查CMakeLists.txt中的源文件列表

### 屏幕不显示
1. 检查GPIO接线是否正确
2. 检查SPI总线是否正确 (/dev/spidev0.0)
3. 验证DC和RST信号是否正常
4. 使用逻辑分析仪或示波器观察SPI信号

### SPI通信错误
→ 检查SPI频率设置 (当前40MHz)，某些IC可能需要降低速率
→ 确认SPI总线是否存在

## 📚 示例GPIO编号 (Allwinner T113)
```
GPIO组：PA, PB, PC, PD, PE, PF, PG, PH, PI, PJ, PK...
编号: PA0=0, PA1=1, ..., PB0=32, PB1=33, ...
```

获取GPIO编号: `GPIO编号 = 组号*32 + 脚号`
例如: PF2 = 5*32 + 2 = 162

## 🔍 验证ST7789通信

可以使用以下命令测试SPI总线：

```bash
# 检查SPI设备是否存在
ls -la /dev/spidev*

# 检查GPIO sysfs是否可用
ls -la /sys/class/gpio/

# 手动控制GPIO进行测试
echo 162 > /sys/class/gpio/export
echo out > /sys/class/gpio/gpio162/direction
echo 1 > /sys/class/gpio/gpio162/value
```

---
**注意**：完成以上配置后，请进行完整的编译测试，确保没有编译错误。
