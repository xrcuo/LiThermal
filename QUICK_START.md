# 🚀 LiThermal ST7789屏幕 - 快速启动指南

## 📊 项目完整性确认

✅ **所有必需文件已准备完毕**

```
✓ 驱动程序        ST7789.h/c       (LVGL驱动)
✓ 接口层          st7789_spi.h/c   (GPIO/SPI + T113配置)
✓ 设备树          t113-st7789-overlay.dts
✓ 配置文件        lv_drv_conf.h    (USE_ST7789启用)
✓ 构建系统        CMakeLists.txt   (已修改)
✓ 应用集成        myHAL.cpp        (已修改初始化)
✓ 文档            BUILD_DEVICETREE.md + 完整性清单
```

## 🔌 硬件连接确认

**确认以下IO配置已应用到st7789_spi.h**:

| 功能 | T113引脚 | GPIO编号 | 当前状态 |
|------|----------|----------|--------|
| SPI MOSI | PD0 | 98 | ✅ |
| SPI CLK | PD1 | 99 | ✅ |
| SPI CS | PD3 | 101 | ✅ |
| 复位RST | PB4 | 36 | ✅ |
| 命令DC | PB5 | 37 | ✅ |
| 背光BL | PB2 (PWM0) | 34 | ✅ |

**硬件接线 (连接ST7789屏幕)**:

```
T113并行          ST7789屏幕
══════════════════════════════════
3.3V      ─────→  VCC
GND       ─────→  GND
PD0(MOSI) ─────→  SDA/MOSI
PD1(CLK)  ─────→  SCL/CLK
PD3(CS)   ─────→  CS
PB4(RST)  ─────→  RES/RST
PB5(DC)   ─────→  DC/RS
PB2(PWM0) ─────→  BL/LED(背光)
```

## ⚡ 编译 & 部署 (5步，≈60分钟)

### 第1步: 编译设备树 (10分钟)

```bash
# 在项目目录执行
cd ~/LiThermal

# 编译设备树为DTBO
dtc -I dts -O dtb hardware/t113-st7789-overlay.dts -o st7789.dtbo

# 验证
file st7789.dtbo
```
✅ 预期: `Device Tree Blob`

### 第2步: 更新CMakeLists.txt (5分钟) [如果还未做]

```bash
# 确保CMakeLists.txt中有:
add_compile_definitions(USE_ST7789)

# 移除旧的SUNXIFB相关定义
```

### 第3步: 编译LiThermal应用 (20分钟)

```bash
cd ~/LiThermal/build
cmake ..
make -j32

# 验证编译成功
file build/thermalcamera
ls -lh build/thermalcamera
```

### 第4步: 部署到设备 (10分钟)

```bash
# 推送DTBO文件
adb push st7789.dtbo /boot/overlays/
adb shell chmod 644 /boot/overlays/st7789.dtbo

# 推送应用
adb push build/thermalcamera /mnt/UDISK/
adb shell chmod +x /mnt/UDISK/thermalcamera
```

### 第5步: 启用并运行 (15分钟)

```bash
# SSH到设备
adb shell

# 启用设备树overlay
su root
echo "dtoverlay=st7789" >> /boot/config.txt

# 重启设备
reboot

# 设备重启后验证
sleep 30
adb shell

# 检查关键设备/文件
ls /dev/spidev1.0                      # SPI设备
ls /sys/class/gpio/gpio36 /sys/class/gpio/gpio37  # GPIO
dmesg | tail -20                       # 查看初始化日志

# 运行应用
/mnt/UDISK/thermalcamera &

# 观察屏幕 - 应该显示热成像摄像头的界面
```

## ✅ 验证清单 (启动前确认)

在运行应用前，执行以下检查:

```bash
# 1️⃣ 硬件电源检查 (使用万用表)
□ VCC: 3.3V ± 0.1V
□ GND: 0V (与T113相同)
□ 接线牢固，无松动

# 2️⃣ 设备文件检查
adb shell "ls /dev/spidev1.0" && echo ✅ SPI就绪 || echo ❌ SPI缺失
adb shell "ls /sys/class/gpio/gpio36" && echo ✅ GPIO36就绪 || echo ❌ GPIO36缺失
adb shell "ls /sys/class/gpio/gpio37" && echo ✅ GPIO37就绪 || echo ❌ GPIO37缺失

# 3️⃣ 内核日志检查
adb shell "dmesg | grep -i st7789" && echo ✅ 驱动加载 || echo ⚠️   可能未加载

# 4️⃣ GPIO手动测试
adb shell "echo 1 > /sys/class/gpio/gpio36/value"  # RST拉高
adb shell "echo 0 > /sys/class/gpio/gpio36/value"  # RST拉低
# 屏幕应该有反应(闪烁或显示/停止显示)
```

## 🐛 常见问题速查

### ❌ 编译错误: `找不到st7789_spi.h`

**原因**: 没有正确配置包含路径

**解决**:
```bash
# 检查lv_drv_conf.h第33行是否有:
grep "st7789_spi.h" include/lv_drv_conf.h

# 如果没有，手动添加:
vim include/lv_drv_conf.h
# 在 #if USE_ST7789 块中确保有:
#include "lv_drivers/display/st7789_spi.h"
```

### ❌ 没有 /dev/spidev1.0

**原因**: 设备树未正确加载

**解决**:
```bash
# 检查overlay是否加载
cat /boot/config.txt | grep dtoverlay=st7789

# 如果没有，手动添加
echo "dtoverlay=st7789" >> /boot/config.txt

# 重新编译和推送DTBO
dtc -I dts -O dtb hardware/t113-st7789-overlay.dts -o st7789.dtbo
adb push st7789.dtbo /boot/overlays/

# 重启
adb reboot
```

### ❌ GPIO导出失败

**原因**: GPIO已被占用或权限不足

**解决**:
```bash
# 检查GPIO被谁占用
cat /sys/kernel/debug/gpio | grep 36
cat /sys/kernel/debug/gpio | grep 37

# 手动导出GPU (如果不在设备树中)
echo 36 > /sys/class/gpio/export
echo 37 > /sys/class/gpio/export
echo 101 > /sys/class/gpio/export
```

### ❌ 屏幕无显示

**检查流程** (参照 [PROJECT_COMPLETION_CHECKLIST.md](./PROJECT_COMPLETION_CHECKLIST.md)):

```
1. ✓ 电源: 万用表测量3.3V
2. ✓ 地线: 确认T113和屏幕GND连接
3. ✓ 复位: 观察RST脚是否收到脉冲
4. ✓ SPI: 使用示波器观察CLK和MOSI
5. ✓ DC脚: 确认DC脚有高低变化
6. ✓ 日志: 查看 dmesg | tail -50
```

## 📚 关键文件位置

```
LiThermal/
├── libs/lv_drivers/display/
│   ├── ST7789.h/c           # 显示驱动
│   └── st7789_spi.h/c       # ← T113配置在这里!
│
├── include/
│   └── lv_drv_conf.h        # USE_ST7789=1
│
├── src/
│   └── myHAL.cpp            # 初始化调用点
│
├── hardware/
│   ├── t113-st7789-overlay.dts  # 设备树
│   └── BUILD_DEVICETREE.md      # 编译指南
│
└── PROJECT_COMPLETION_CHECKLIST.md  # 完整清单
```

## 🔍 I/O 配置验证 (快速检查)

打开 `libs/lv_drivers/display/st7789_spi.h`，确认以下行存在:

```c
#define ST7789_SPI_CS_PIN    101   /* PD3 = GPIO101 */
#define ST7789_SPI_DC_PIN    37    /* PB5 = GPIO37 */
#define ST7789_SPI_RST_PIN   36    /* PB4 = GPIO36 */
#define ST7789_SPI_BL_PIN    34    /* PB2 = GPIO34 */
#define ST7789_SPI_BUS       "/dev/spidev1.0"
#define ST7789_SPI_SPEED_HZ  80000000
```

✅ 如果这些都存在，IO配置已完全完成！

## 📊 性能指标 (目标)

| 指标 | 目标值 | 实际值 |
|------|--------|--------|
| 刷新率 | 28-30 FPS | - |
| 响应延迟 | ~33ms | - |
| CPU占用 | < 50% | - |
| 内存占用 | < 20MB | - |
| 功耗 | < 1W @ 3.3V | - |

## 📞 技术支持流程

如遇问题，按以下顺序排查:

1. **查看日志** → `dmesg | tail -50`
2. **查看文档** → [PROJECT_COMPLETION_CHECKLIST.md](./PROJECT_COMPLETION_CHECKLIST.md)
3. **验证硬件** → 万用表测量电压和信号
4. **测试隔离** → 手动控制GPIO: `echo 0/1 > /sys/class/gpio/gpio36/value`

## ✨ 总结

| 步骤 | 耗时 | 状态 |
|------|------|------|
| ✅ 编译设备树 | 10分钟 | 准备就绪 |
| ✅ 编译应用 | 20分钟 | 准备就绪 |
| ✅ 部署 | 10分钟 | 准备就绪 |
| ✅ 验证 | 15分钟 | 准备就绪 |
| **总计** | **≈60分钟** | **✅ 就绪** |

## 🎯 下一步

```bash
# 1. 编译设备树
dtc -I dts -O dtb hardware/t113-st7789-overlay.dts -o st7789.dtbo

# 2. 编译应用
cd build && make

# 3. 部署
adb push st7789.dtbo /boot/overlays/
adb push build/thermalcamera /mnt/UDISK/

# 4. 启动
adb reboot
adb shell /mnt/UDISK/thermalcamera
```

---

**项目完成日期**: 2026年4月14日  
**平台**: Allwinner T113 + Tina Linux + LVGL 8.4.0  
**屏幕**: 3.2英寸 ST7789 320×240 TFT LCD  
**完成度**: 100% ✅
