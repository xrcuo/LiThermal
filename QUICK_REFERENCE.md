# 快速参考卡 - 3.2寸 ST7789 屏幕适配

## ⚡ 核心改动（3处）

### ✅ 改动1：启用 ST7789 驱动
**文件**: `include/lv_drv_conf.h` 第 19 行
```c
-#  define USE_ST7789            0
+#  define USE_ST7789            1
```

### ✅ 改动2：禁用 GC9A01 驱动
**文件**: `include/lv_drv_conf.h` 第 274 行
```c
-#  define USE_GC9A01          1
+#  define USE_GC9A01          0
```

### ✅ 改动3：LCD 初始化日志
**文件**: `src/myHAL.cpp` 第 211 行
```cpp
  st7789_spi_init();
  st7789_init();
+ printf("ST7789 LCD initialized at 320x240\n");
```

---

## 📊 屏幕规格

| 规格 | 原 (GC9A01) | 新 (ST7789) |
|------|-----------|-----------|
| **分辨率** | 240×240 | 320×240 |
| **屏幕类型** | 圆形 | 矩形 |
| **驱动芯片** | GC9A01 | ST7789 |
| **显示面积** | 57,600px | 76,800px (+33%) |
| **总线** | SPI1 | SPI1 |
| **GPIO位置** | GPIO36/37/101 | GPIO36/37/101 |

---

## 🔗 硬件连接（T113）

```
ST7789 Display          Allwinner T113
┌─────────────────┐    ┌──────────────┐
│ VCC    ─────────┼───┤ 3.3V         │
│ GND    ─────────┼───┤ GND          │
│ MOSI   ─────────┼───┤ PD0 (GPIO98) │
│ CLK    ─────────┼───┤ PD1 (GPIO99) │
│ CS     ─────────┼───┤ PD3 (GPIO101)│
│ DC     ─────────┼───┤ PB5 (GPIO37) │
│ RST    ─────────┼───┤ PB4 (GPIO36) │
│ BL     ─────────┼───┤ PB2 (GPIO34) │
└─────────────────┘    └──────────────┘
```

---

## 🚀 编译步骤

### 第1步：同步文件到 Linux
```bash
# 将以下路径复制到 Linux 编译环境
src/myHAL.cpp
include/lv_drv_conf.h
libs/lv_drivers/display/ST7789.*
libs/lv_drivers/display/st7789_spi.*
```

### 第2步：清理并重新编译
```bash
cd /OW/LiThermal_Compiler-master/build
rm -rf *              # ⚠️ 必须清空缓存
cmake ..
make -j4
```

### 第3步：验证编译
```bash
# 看到以下输出代表成功
[100%] Built target thermalcamera
ST7789 LCD initialized at 320x240
```

---

## 📲 部署命令

```bash
adb push /build/thermalcamera /mnt/UDISK/
adb shell chmod +x /mnt/UDISK/thermalcamera
adb shell /mnt/UDISK/thermalcamera
```

---

## 🔍 常见问题

### Q: 编译出现 `st7789_spi.h: No such file or directory`
**A**: 文件未同步到 Linux。执行：
```bash
scp f:\RE-NOW\LiThermal\libs\lv_drivers\display\st7789_spi.* \
    [用户@IP]:/OW/LiThermal_Compiler-master/LiThermal/libs/lv_drivers/display/
```

### Q: 屏幕显示全白或全黑
**A**: 检查初始化：
```bash
adb shell "cat /sys/class/gpio/gpio36/value"   # RST 应为 1
adb shell "cat /sys/class/gpio/gpio37/value"   # DC 应为 1
```

### Q: 需要快速切换回 GC9A01？
**A**: 改回配置：
```c
#define USE_ST7789    0    // 关闭 ST7789
#define USE_GC9A01    1    // 开启 GC9A01
```
然后重新编译。

---

## 📈 性能指标

```
分辨率:    320×240 = 76,800 像素
缓冲区:    320×240×4 = 300 KB
刷新率:    60Hz (10ms)
SPI 速率:  80MHz
色彩深度:  32-bit ARGB (LV_COLOR_DEPTH=32)
```

---

## ✨ 修改检查表

- [x] `lv_drv_conf.h`: USE_ST7789 = 1, USE_GC9A01 = 0
- [x] `myHAL.cpp`: 初始化 320×240，添加日志
- [x] `ui/camera.cpp`: 确认输出 320×240
- [x] `UI` 布局: 已兼容新分辨率
- [ ] **编译测试** （在 Linux 中执行）
- [ ] **硬件验证** （在设备上运行）

---

**状态**: ✅ 已完成 Windows 端配置  
**待完成**: 🔄 Linux 编译和硬件测试  
**最后更新**: 2026-04-14
