/**
 * @file ST7789.c
 *
 */

/*********************
 *      INCLUDES
 *********************/
#include "ST7789.h"
#if USE_ST7789

#include <stddef.h>
#include <stdbool.h>
#include <string.h>

#include LV_DRV_DISP_INCLUDE
#include LV_DRV_DELAY_INCLUDE

/*********************
 *      DEFINES
 *********************/

#define ST7789_CMD_MODE  0
#define ST7789_DATA_MODE 1

#define ST7789_HOR_RES  LV_HOR_RES
#define ST7789_VER_RES  LV_VER_RES

#define ST7789_XSTART   0
#define ST7789_YSTART   0

/* ST7789 Commands */
#define ST7789_NOP              0x00
#define ST7789_RESET            0x01
#define ST7789_RDID             0x04
#define ST7789_RDDST            0x09
#define ST7789_SLPIN            0x10
#define ST7789_SLPOUT           0x11
#define ST7789_PTLON            0x12
#define ST7789_NORON            0x13
#define ST7789_INVOFF           0x20
#define ST7789_INVON            0x21
#define ST7789_GAMSET           0x26
#define ST7789_DISPOFF          0x28
#define ST7789_DISPON           0x29
#define ST7789_CASET            0x2A
#define ST7789_RASET            0x2B
#define ST7789_RAMWR            0x2C
#define ST7789_RAMRD            0x2E
#define ST7789_PTLAR            0x30
#define ST7789_MADCTL           0x36
#define ST7789_COLMOD           0x3A
#define ST7789_FRMCTR1          0xB1
#define ST7789_FRMCTR2          0xB2
#define ST7789_FRMCTR3          0xB3
#define ST7789_INVCTR           0xB4
#define ST7789_DISSET5          0xB6
#define ST7789_GCTRL            0xB7
#define ST7789_GTADJ            0xB9
#define ST7789_VCOMS            0xBB
#define ST7789_LCMCTRL          0xC0
#define ST7789_IDSET            0xC1
#define ST7789_VDVS             0xC2
#define ST7789_VDVVREGEN       0xC7
#define ST7789_FRCTRL2          0xC6
#define ST7789_CMDSET           0xDF
#define ST7789_PWCTRL1          0xD0
#define ST7789_PVGAMCTRL        0xE0
#define ST7789_NVGAMCTRL        0xE1
#define ST7789_CHUNKCTRL        0xF0

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *  STATIC PROTOTYPES
 **********************/
static void st7789_io_init(void);
static void st7789_reset(void);
static void st7789_send_cmd(uint8_t cmd);
static void st7789_send_data(uint8_t data);
static void st7789_send_data_array(uint8_t *data, uint16_t len);
static void st7789_set_addr_win(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1);

/**********************
 *  STATIC VARIABLES
 **********************/
static bool cmd_mode = true;

/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

void st7789_init(void)
{
    st7789_io_init();
    st7789_reset();

    st7789_send_cmd(ST7789_SLPOUT);         /* Sleep out */
    LV_DRV_DELAY_MS(120);

    st7789_send_cmd(ST7789_COLMOD);         /* Set color mode to 16bit */
    st7789_send_data(0x05);

    st7789_send_cmd(ST7789_MADCTL);         /* Memory access control */
    st7789_send_data(0x00);

    st7789_send_cmd(ST7789_INVON);          /* Inversion ON */

    st7789_send_cmd(ST7789_DISPON);         /* Display ON */
    LV_DRV_DELAY_MS(120);
}

void st7789_flush(lv_disp_drv_t * disp_drv, const lv_area_t * area, lv_color_t * color_p)
{
    uint32_t w = lv_area_get_width(area);
    uint32_t h = lv_area_get_height(area);

    st7789_set_addr_win(area->x1 + ST7789_XSTART, area->y1 + ST7789_YSTART,
                         area->x2 + ST7789_XSTART, area->y2 + ST7789_YSTART);

    st7789_send_cmd(ST7789_RAMWR);

    uint32_t size = w * h;
    uint32_t i = 0;
    for(i = 0; i < size; i++) {
        st7789_send_data((color_p[i].full >> 8) & 0xFF);
        st7789_send_data(color_p[i].full & 0xFF);
    }

    lv_disp_flush_ready(disp_drv);
}

void st7789_fill(int32_t x1, int32_t y1, int32_t x2, int32_t y2, lv_color_t color)
{
    uint32_t w = x2 - x1 + 1;
    uint32_t h = y2 - y1 + 1;
    uint32_t i = 0;

    st7789_set_addr_win(x1 + ST7789_XSTART, y1 + ST7789_YSTART,
                         x2 + ST7789_XSTART, y2 + ST7789_YSTART);

    st7789_send_cmd(ST7789_RAMWR);

    for(i = 0; i < (w * h); i++) {
        st7789_send_data((color.full >> 8) & 0xFF);
        st7789_send_data(color.full & 0xFF);
    }
}

/**********************
 *   STATIC FUNCTIONS
 **********************/

/**
 * Initialize the IO pins for ST7789
 */
static void st7789_io_init(void)
{
    /*You may need to implement this based on your platform*/
    LV_DRV_DISP_RST(1);
}

/**
 * Reset the ST7789
 */
static void st7789_reset(void)
{
    LV_DRV_DISP_RST(0);
    LV_DRV_DELAY_MS(10);
    LV_DRV_DISP_RST(1);
    LV_DRV_DELAY_MS(120);
}

/**
 * Send a command to ST7789
 */
static void st7789_send_cmd(uint8_t cmd)
{
    LV_DRV_DISP_CMD_DATA(ST7789_CMD_MODE);
    LV_DRV_DISP_SPI_WR_BYTE(cmd);
}

/**
 * Send data to ST7789
 */
static void st7789_send_data(uint8_t data)
{
    LV_DRV_DISP_CMD_DATA(ST7789_DATA_MODE);
    LV_DRV_DISP_SPI_WR_BYTE(data);
}

/**
 * Send data array to ST7789
 */
static void st7789_send_data_array(uint8_t *data, uint16_t len)
{
    LV_DRV_DISP_CMD_DATA(ST7789_DATA_MODE);
    LV_DRV_DISP_SPI_WR_ARRAY(data, len);
}

/**
 * Set the address window for display
 */
static void st7789_set_addr_win(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1)
{
    /* Column address */
    st7789_send_cmd(ST7789_CASET);
    st7789_send_data(x0 >> 8);
    st7789_send_data(x0 & 0xFF);
    st7789_send_data(x1 >> 8);
    st7789_send_data(x1 & 0xFF);

    /* Row address */
    st7789_send_cmd(ST7789_RASET);
    st7789_send_data(y0 >> 8);
    st7789_send_data(y0 & 0xFF);
    st7789_send_data(y1 >> 8);
    st7789_send_data(y1 & 0xFF);
}

#endif /* USE_ST7789 */
