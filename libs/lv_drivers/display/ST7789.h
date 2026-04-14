/**
 * @file ST7789.h
 *
 */

#ifndef ST7789_H
#define ST7789_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/
#ifndef LV_DRV_NO_CONF
#ifdef LV_CONF_INCLUDE_SIMPLE
#include "lv_drv_conf.h"
#else
#include "../../lv_drv_conf.h"
#endif
#endif

#if USE_ST7789

#ifdef LV_LVGL_H_INCLUDE_SIMPLE
#include "lvgl.h"
#else
#include "lvgl/lvgl.h"
#endif

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 * GLOBAL PROTOTYPES
 **********************/
void st7789_init(void);
void st7789_flush(lv_disp_drv_t * disp_drv, const lv_area_t * area, lv_color_t * color_p);
void st7789_fill(int32_t x1, int32_t y1, int32_t x2, int32_t y2, lv_color_t color);

/**********************
 *      MACROS
 **********************/

#endif /* USE_ST7789 */

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* ST7789_H */
