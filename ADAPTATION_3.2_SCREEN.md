# LiThermal UI 屏幕适配指南 - 3.2 寸 ST7789 (320x240)

**日期**: 2026年4月14日  
**适配目标**: 从 GC9A01（240x240 圆形屏）→ ST7789（320x240 3.2寸矩形屏）

---

## 📋 适配内容概览

| 项目 | 原配置 | 新配置 | 修改状态 |
|------|-------|--------|---------|
| **屏幕驱动** | GC9A01 | ST7789 | ✅ 已完成 |
| **屏幕分辨率** | 240x240 | 320x240 | ✅ 已完成 |
| **屏幕类型** | 圆形 | 矩形 | ✅ 已完成 |
| **显示驱动芯片** | GC9A01 | ST7789 | ✅ 已完成 |
| **显示总线** | SPI | SPI | ✅ 无需改动 |

---

## 🔧 关键修改文件

### 1️⃣ **include/lv_drv_conf.h**
```c
/* 改动1: 启用 ST7789 驱动 */
#ifndef USE_ST7789
#  define USE_ST7789            1  // 从 0 改为 1
#endif

/* 改动2: 禁用 GC9A01 驱动 */
#ifndef USE_GC9A01
#  define USE_GC9A01            0  // 从 1 改为 0
#endif

/* 自动配置（无需手动改） */
#if USE_ST7789
#  define ST7789_HOR_RES        320
#  define ST7789_VER_RES        240
#endif
```

**重要**: `st7789_spi.h` 已在行 41 注释掉（因为编译 GC9A01），使用 ST7789 时不依赖该文件。

### 2️⃣ **src/myHAL.cpp**
```cpp
/* ST7789 屏幕初始化（已正确配置520x240） */
void HAL::init() {
    // ...
    disp_drv.hor_res = 320;  // 320 像素宽度
    disp_drv.ver_res = 240;  // 240 像素高度
    
    /* ST7789 SPI LCD 初始化 */
    st7789_spi_init();
    st7789_init();
    printf("ST7789 LCD initialized at 320x240\n");
    // ...
}
```

### 3️⃣ **UI 尺寸（已兼容）**

以下文件已兼容 320x240 分辨率：

- ✅ `src/ui/camera.cpp` - 使用 320x240 摄像头输出
- ✅ `src/ui/poweroff.cpp` - 黑色覆盖层 320x240
- ✅ `src/ui/button_2.cpp` - 菜单按钮使用相对尺寸
- ✅ `src/ui/brightnessSlider.cpp` - 亮度滑块自适应
- ✅ `src/videoPlayer.cpp` - 视频播放器 320x240

---

## 📐 分辨率对比

### GC9A01（240x240 - 原配置）
```
┌──────────────┐
│              │
│   240x240    │ 圆形屏幕
│   圆形显示   │
│              │
└──────────────┘
```

### ST7789（320x240 - 新配置）
```
┌─────────────────┐
│                 │
│    320x240      │ 矩形屏幕
│    3.2 寸屏     │ 更宽的显示空间
│                 │
└─────────────────┘
```

**优势**：
- 额外的 80 像素宽度（320 vs 240）
- 完整的矩形显示区域
- 更适合功能菜单布局
- 提供更好的可读性

---

## 🔌 硬件连接验证

### ST7789 引脚配置（Allwinner T113）

| 功能 | T113 GPIO | 二级管脚 | SPI1 引脚 |
|------|-----------|---------|---------|
| MOSI | GPIO98 | PD0 | SPI1_MOSI |
| CLK  | GPIO99 | PD1 | SPI1_CLK |
| CS   | GPIO101 | PD3 | SPI1_CS |
| DC   | GPIO37 | PB5 | GPIO |
| RST  | GPIO36 | PB4 | GPIO |
| BL   | GPIO34 | PB2 | PWM0 |
| VCC  | - | 3.3V | 电源 |
| GND  | - | GND | 地线 |

**验证步骤**：
```bash
# 检查 GPIO 导出
cat /sys/class/gpio/export

# 验证 SPI 总线
ls -la /dev/spidev1.0

# 测试 ST7789 驱动（使用 cat 读取）
cat /sys/class/gpio/gpio36/value  # RST 状态
```

---

## 🛠️ 构建和部署

### 1. 本地编译

确保配置文件已同步到 Linux 编译环境：

```bash
# 如果使用 SCP 同步
scp -r f:\RE-NOW\LiThermal\include\lv_drv_conf.h \
       f:\RE-NOW\LiThermal\libs\lv_drivers\display\ST7789.* \
       [用户@IP]:/OW/LiThermal_Compiler-master/LiThermal/

# 或直接编辑 Linux 系统中的文件
ssh [用户@IP] "nano /OW/LiThermal_Compiler-master/LiThermal/include/lv_drv_conf.h"
```

### 2. 清理并重新编译

```bash
cd /OW/LiThermal_Compiler-master/build
rm -rf *              # 清空缓存
cmake ..              # 重新生成
make -j4              # 编译
```

### 3. 刷入设备

```bash
# 使用 adb 部署
adb push LiThermal /mnt/UDISK/
adb shell chmod +x /mnt/UDISK/LiThermal
adb shell /mnt/UDISK/LiThermal
```

---

## ✅ 预期编译输出

编译成功时应看到：

```
[ 85%] Building C object libs/lv_drivers/display/CMakeFiles/...
[100%] Linking CXX executable thermalcamera
[100%] Built target thermalcamera

ST7789 LCD initialized at 320x240
```

---

## 🎯 用户界面调整建议

为充分利用新的 320x240 分辨率，建议：

### 主界面优化
- **布局宽度**: 利用新增的 80px 宽度优化菜单布局
- **文字显示**: 可显示更多信息
- **功能按钮**: 可排列更多按钮或使用更大的触摸区域

### 菜单界面
```
原 240x240：      新 320x240：
┌────────────┐   ┌──────────────────┐
│  菜单 1    │   │  菜单项1  菜单项2 │
│  菜单 2    │   │  菜单项3  菜单项4 │
│  菜单 3    │   │  菜单项5  菜单项6 │
└────────────┘   └──────────────────┘
```

---

## 🔍 故障排查

### 编译错误：`fatal error: st7789_spi.h: No such file or directory`
**原因**: 文件未同步到 Linux 编译系统  
**解决**: 
```bash
# 确保文件存在
ls -la /OW/LiThermal_Compiler-master/LiThermal/libs/lv_drivers/display/st7789_spi.*

# 如果不存在，从 Windows 复制
scp f:\RE-NOW\LiThermal\libs\lv_drivers\display\st7789_spi.* \
    [用户@IP]:/OW/LiThermal_Compiler-master/LiThermal/libs/lv_drivers/display/
```

### 屏幕显示异常
**检查项**:
1. GPIO 初始化: `st7789_spi_init()`
2. 屏幕复位: RST 引脚（GPIO36）
3. DC 引脚: GPIO37（命令/数据切换）
4. SPI 频率: 80MHz
5. 色深: LV_COLOR_DEPTH = 32

### 性能问题
如果 FPS 下降：
```c
/* lv_conf.h 中优化 */
#define LV_DISP_DEF_REFR_PERIOD 10  /* 增加到 20ms */
#define LV_LAYER_SIMPLE_BUF_SIZE (32 * 1024)  /* 减小缓冲 */
```

---

## 📊 性能参数

| 参数 | 值 | 备注 |
|------|-----|------|
| 分辨率 | 320x240 | 76,800 像素 |
| 刷新率 | 60Hz 推荐 | 配置为 10ms |
| 内存占用 | ~300KB | 320x240x4 字节缓冲 |
| SPI 速度 | 80MHz | T113 最优速度 |
| 色深 | 32-bit ARGB | LV_COLOR_DEPTH = 32 |

---

## 🎉 适配完成检查清单

- [x] 启用 ST7789 驱动
- [x] 禁用 GC9A01 驱动
- [x] 设置分辨率 320x240
- [x] 验证 UI 尺寸兼容
- [x] 更新 myHAL.cpp 初始化
- [x] 验证 st7789_spi.h 包含路径
- [x] 创建适配文档
- [ ] 编译测试（在 Linux 系统中执行）
- [ ] 动态测试（在硬件上验证显示效果）

---

## 📝 备注

- **向后兼容**: GC9A01 驱动仍保留在代码中，更改 `USE_GC9A01 = 1` 可快速切换回原配置
- **文件位置**: 所有配置改动已在 Windows 项目中完成，需要同步到 Linux 编译环境
- **测试建议**: 首先在模拟器中验证编译，再在真实硬件上测试显示效果

---

**文档维护**: 2026年4月14日  
**项目**: LiThermal 热成像相机  
**平台**: Allwinner T113 + Tina Linux + LVGL 8.4.0
