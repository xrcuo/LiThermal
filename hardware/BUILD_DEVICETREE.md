# ST7789 设备树编译指南 (Allwinner T113)

## 📋 概述

设备树用于在Linux内核启动时配置ST7789 LCD驱动的硬件资源（GPIO、SPI等）。

## 🔧 编译设备树

### 方法1: 使用DTC工具编译为DTBO (推荐)

```bash
# 安装device-tree-compiler
sudo apt-get install device-tree-compiler

# 编译DTS为DTBO
dtc -I dts -O dtb hardware/t113-st7789-overlay.dts -o st7789.dtbo

# 验证编译结果
file st7789.dtbo
# 预期输出: st7789.dtbo: Device Tree Blob

# 检查文件大小 (通常200-500字节)
ls -lh st7789.dtbo
```

### 方法2: 使用Tina SDK编译

如果使用Allwinner Tina Linux SDK:

```bash
# 从SDK根目录编译
cd tina-sdk
./build/scripts/compile_overlays.sh device/config/boards/t113/overlays/

# 输出文件通常在:
ls -la device/config/boards/t113/overlays/st7789.dtbo
```

## 📦 部署到T113设备

### 步骤1: 将DTBO文件传到设备

```bash
# 使用ADB
adb push st7789.dtbo /boot/overlays/

# 或使用SCP
scp st7789.dtbo root@<设备IP>:/boot/overlays/
```

### 步骤2: 在设备上启用overlay

```bash
# SSH到设备
adb shell
# 或
ssh root@<设备IP>

# 编辑启动配置
vi /boot/config.txt

# 添加以下行
dtoverlay=st7789

# 保存并退出 (:wq)
```

### 步骤3: 重启设备

```bash
# 方法1: 使用ADB
adb reboot

# 方法2: SSH
reboot

# 方法3: 手动重启
shutdown -r now
```

## ✅ 验证设备树加载

重启后验证设备树是否正确加载:

```bash
# 检查内核日志
dmesg | grep -i st7789
dmesg | grep -i spi1

# 检查设备是否存在
ls -la /dev/spidev1.0          # SPI1设备
cat /proc/device-tree/.../compatible  # 查看兼容性

# 检查GPIO导出
ls -la /sys/class/gpio/gpio36  # RST (PB4)
ls -la /sys/class/gpio/gpio37  # DC (PB5)
ls -la /sys/class/gpio/gpio101 # CS (PD3)
```

## 🐛 故障排查

### 问题1: DTBO文件编译失败

```bash
# 检查DTS语法
dtc -I dts -O dtb t113-st7789-overlay.dts 2>&1 | head -20

# 常见错误:
# - 缺少 /dts-v1/; 声明
# - 缺少 /plugin/; 声明
# - 括号或分号不匹配
```

**解决方法**:
- 检查DTS文件的第1-2行
- 验证所有的 `{` 都有对应的 `}`
- 检查所有的 `;` 是否正确放置

### 问题2: 设备树加载失败

```bash
# 查看详细错误信息
dmesg | tail -50

# 检查overlay路径
ls -la /boot/overlays/
ls -la /boot/dtbs/overlays/
```

**可能的原因**:
- DTBO文件路径错误
- 设备树格式不兼容
- kernel版本不支持device tree overlay

### 问题3: SPI设备不存在

```bash
# 检查SPI总线是否启用
dmesg | grep -i "spi1"
cat /proc/device-tree/soc/spi@*/status

# 检查设备树中的SPI节点
find /proc/device-tree -name "*spi*" -type d
```

**解决方法**:
- 确保DTS中的target正确指向SPI1
- 检查SPI1在基础设备树中是否存在
- 验证脚位配置是否正确

## 📊 DTS文件结构说明

### Fragment 0: 系统配置
```dts
chosen {
    bootargs = "earlyprintk";
}
```
- 配置内核参数和启动选项

### Fragment 1: SPI1总线 + ST7789设备
```dts
target = <&spi1>;
st7789@0 { ... }
```
- 启用SPI1总线
- 定义ST7789设备属性

### Fragment 2: GPIO脚配置
```dts
allwinner,pins = "PB4", "PB5", "PD0", "PD1", "PD3";
```
- 配置复位、DC、SPI信号脚

### Fragment 3-4: PWM背光
```dts
target = <&pwm0>;
pwm0_pins: pwm0_pins { ... }
```
- 启用PWM0控制背光

## 🔍 验证GPIO和SPI配置

```bash
# 1. 验证GPIO是否可控制
echo 36 > /sys/class/gpio/export           # 导出RST脚
echo out > /sys/class/gpio/gpio36/direction
echo 1 > /sys/class/gpio/gpio36/value      # 设置高电平
echo 0 > /sys/class/gpio/gpio36/value      # 设置低电平

# 2. 测试SPI通信
echo -ne "\x01\x02\x03" > /dev/spidev1.0   # 发送3字节
od -x < /dev/spidev1.0                     # 读取响应

# 3. 查看实时设备树
find /proc/device-tree -type f | head -20
```

## 📝 GPIO编号对照表 (Allwinner T113)

```
GPIO组: PA(0-31), PB(32-63), PC(64-95), PD(96-127), PE(128-159), PF(160-191)...

本项目配置:
- PB4 (GPIO36)  = 4 + 32 = 36  → RST
- PB5 (GPIO37)  = 5 + 32 = 37  → DC
- PD0 (GPIO98)  = 0 + 96 = 98  → SPI MOSI
- PD1 (GPIO99)  = 1 + 96 = 99  → SPI CLK
- PD3 (GPIO101) = 3 + 96 = 101 → SPI CS
- PB2 (GPIO34)  = 2 + 32 = 34  → PWM0 (背光)
```

## 🔗 相关资源

- Allwinner T113规格书: Device Tree Overlay章节
- Linux设备树文档: `/usr/share/doc/linux-doc*/device-trees/`
- DTC文档: `man dtc`

---

**下一步**: 完成设备树编译和部署后，返回主项目进行LiThermal应用的编译和测试。
