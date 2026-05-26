#pragma once

#include <stdbool.h>
#include <stdint.h>

#include "lvgl.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef void (*bsp_lvgl_flush_cb_t)(const lv_area_t *area, lv_color_t *color_p, void *ctx);
typedef void (*bsp_lvgl_touch_read_cb_t)(lv_indev_data_t *data, void *ctx);

bool bsp_lvgl_register_display(uint16_t hor_res,
                               uint16_t ver_res,
                               lv_color_t *buf1,
                               lv_color_t *buf2,
                               uint32_t buf_pixel_count,
                               bsp_lvgl_flush_cb_t flush_cb,
                               void *flush_ctx);

void bsp_lvgl_create_headless_display(uint16_t hor_res,
                                      uint16_t ver_res,
                                      lv_color_t *buf1,
                                      lv_color_t *buf2,
                                      uint32_t buf_pixel_count);

bool bsp_lvgl_register_touch(bsp_lvgl_touch_read_cb_t read_cb, void *read_ctx);

#ifdef __cplusplus
}
#endif
