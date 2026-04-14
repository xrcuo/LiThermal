/**
 * @file st7789_spi.h
 * ST7789 SPI interface
 */

#ifndef ST7789_SPI_H
#define ST7789_SPI_H

#include <stdint.h>

/*********************
 * SPI GPIO PINS CONFIGURATION (Allwinner T113)
 * SPI1接口配置 + GPIO控制脚
 *********************/

/* T113 GPIO引脚编号 */
#define ST7789_SPI_CS_PIN    101   /* PD3 = GPIO101: Chip Select */
#define ST7789_SPI_DC_PIN    37    /* PB5 = GPIO37:  Data/Command */
#define ST7789_SPI_RST_PIN   36    /* PB4 = GPIO36:  Reset */
#define ST7789_SPI_BL_PIN    34    /* PB2 = GPIO34:  Backlight (PWM0) */

/* SPI总线: PD0(MOSI=98), PD1(CLK=99) 作为SPI1 */
#define ST7789_SPI_BUS       "/dev/spidev1.0"  /* SPI1总线设备 */
#define ST7789_SPI_SPEED_HZ  80000000          /* 80MHz SPI时钟 */
#define ST7789_SPI_MODE      0                 /* SPI Mode 0 */

/*********************
 * FUNCTION DECLARATIONS
 *********************/

/**
 * Initialize GPIO pins for ST7789 (internal use)
 */
static void st7789_gpio_init(void);

/**
 * Initialize SPI interface for ST7789
 */
void st7789_spi_init(void);

/**
 * Write command to ST7789
 */
void st7789_spi_write_cmd(uint8_t cmd);

/**
 * Write data to ST7789
 */
void st7789_spi_write_data(uint8_t data);

/**
 * Write data array to ST7789
 */
void st7789_spi_write_data_array(uint8_t *data, uint16_t len);

/**
 * Set/clear DC (Data/Command) pin
 */
void st7789_spi_set_dc(int level);

/**
 * Set/clear RST (Reset) pin
 */
void st7789_spi_set_rst(int level);

/**
 * Set/clear CS (Chip Select) pin
 */
void st7789_spi_set_cs(int level);

#endif /* ST7789_SPI_H */
