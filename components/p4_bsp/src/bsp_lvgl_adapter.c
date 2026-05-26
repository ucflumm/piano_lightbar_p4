#include "bsp_lvgl_adapter.h"

#include "esp_log.h"

static bsp_lvgl_flush_cb_t s_flush_cb = NULL;
static void *s_flush_ctx = NULL;
static bsp_lvgl_touch_read_cb_t s_touch_read_cb = NULL;
static void *s_touch_read_ctx = NULL;

#if LVGL_VERSION_MAJOR < 9

static lv_disp_draw_buf_t s_draw_buf;
static lv_disp_drv_t s_disp_drv;
static lv_indev_drv_t s_touch_indev_drv;
static lv_indev_t *s_touch_indev = NULL;

static void v8_flush_wrapper(lv_disp_drv_t *disp_drv, const lv_area_t *area, lv_color_t *color_p)
{
    if (s_flush_cb != NULL) {
        s_flush_cb(area, color_p, s_flush_ctx);
    }
    lv_disp_flush_ready(disp_drv);
}

static void v8_headless_flush(lv_disp_drv_t *disp_drv, const lv_area_t *area, lv_color_t *color_p)
{
    (void)area;
    (void)color_p;
    lv_disp_flush_ready(disp_drv);
}

static void v8_touch_wrapper(lv_indev_drv_t *indev_drv, lv_indev_data_t *data)
{
    (void)indev_drv;
    if (s_touch_read_cb != NULL) {
        s_touch_read_cb(data, s_touch_read_ctx);
    }
}

bool bsp_lvgl_register_display(uint16_t hor_res,
                               uint16_t ver_res,
                               lv_color_t *buf1,
                               lv_color_t *buf2,
                               uint32_t buf_pixel_count,
                               bsp_lvgl_flush_cb_t flush_cb,
                               void *flush_ctx)
{
    s_flush_cb = flush_cb;
    s_flush_ctx = flush_ctx;

    lv_disp_draw_buf_init(&s_draw_buf, buf1, buf2, buf_pixel_count);

    lv_disp_drv_init(&s_disp_drv);
    s_disp_drv.hor_res = hor_res;
    s_disp_drv.ver_res = ver_res;
    s_disp_drv.draw_buf = &s_draw_buf;
    s_disp_drv.flush_cb = v8_flush_wrapper;
    return lv_disp_drv_register(&s_disp_drv) != NULL;
}

void bsp_lvgl_create_headless_display(uint16_t hor_res,
                                      uint16_t ver_res,
                                      lv_color_t *buf1,
                                      lv_color_t *buf2,
                                      uint32_t buf_pixel_count)
{
    lv_disp_draw_buf_init(&s_draw_buf, buf1, buf2, buf_pixel_count);

    lv_disp_drv_init(&s_disp_drv);
    s_disp_drv.hor_res = hor_res;
    s_disp_drv.ver_res = ver_res;
    s_disp_drv.draw_buf = &s_draw_buf;
    s_disp_drv.flush_cb = v8_headless_flush;
    (void)lv_disp_drv_register(&s_disp_drv);
}

bool bsp_lvgl_register_touch(bsp_lvgl_touch_read_cb_t read_cb, void *read_ctx)
{
    s_touch_read_cb = read_cb;
    s_touch_read_ctx = read_ctx;

    lv_indev_drv_init(&s_touch_indev_drv);
    s_touch_indev_drv.type = LV_INDEV_TYPE_POINTER;
    s_touch_indev_drv.read_cb = v8_touch_wrapper;
    s_touch_indev = lv_indev_drv_register(&s_touch_indev_drv);
    return s_touch_indev != NULL;
}

#else

// LVGL 9 migration scaffold: keep BSP API stable while v9 backend is implemented.
bool bsp_lvgl_register_display(uint16_t hor_res,
                               uint16_t ver_res,
                               lv_color_t *buf1,
                               lv_color_t *buf2,
                               uint32_t buf_pixel_count,
                               bsp_lvgl_flush_cb_t flush_cb,
                               void *flush_ctx)
{
    (void)hor_res;
    (void)ver_res;
    (void)buf1;
    (void)buf2;
    (void)buf_pixel_count;
    (void)flush_cb;
    (void)flush_ctx;
    ESP_LOGE("bsp_lvgl_adapter", "LVGL v9 adapter path not implemented yet");
    return false;
}

void bsp_lvgl_create_headless_display(uint16_t hor_res,
                                      uint16_t ver_res,
                                      lv_color_t *buf1,
                                      lv_color_t *buf2,
                                      uint32_t buf_pixel_count)
{
    (void)hor_res;
    (void)ver_res;
    (void)buf1;
    (void)buf2;
    (void)buf_pixel_count;
    ESP_LOGW("bsp_lvgl_adapter", "LVGL v9 headless path not implemented yet");
}

bool bsp_lvgl_register_touch(bsp_lvgl_touch_read_cb_t read_cb, void *read_ctx)
{
    (void)read_cb;
    (void)read_ctx;
    ESP_LOGE("bsp_lvgl_adapter", "LVGL v9 touch adapter path not implemented yet");
    return false;
}

#endif
