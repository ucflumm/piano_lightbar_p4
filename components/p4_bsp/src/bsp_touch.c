#include "bsp_touch.h"

#include "bsp_board.h"
#include "bsp_i2c.h"
#include "bsp_lvgl_adapter.h"

#include <stdint.h>

#include "esp_err.h"
#include "esp_lcd_panel_io.h"
#include "esp_lcd_panel_io_additions.h"
#include "esp_lcd_touch.h"
#include "esp_lcd_touch_gt911.h"
#include "esp_log.h"

static const char *TAG = "bsp_touch";

static esp_lcd_panel_io_handle_t s_touch_io_handle = NULL;
static esp_lcd_touch_handle_t s_touch_handle = NULL;
static bool s_touch_seen_once = false;

static void touch_read_cb(lv_indev_data_t *data, void *ctx)
{
    (void)ctx;

    data->state = LV_INDEV_STATE_RELEASED;
    data->point.x = 0;
    data->point.y = 0;
    data->continue_reading = false;

    if (s_touch_handle == NULL) {
        return;
    }

    if (esp_lcd_touch_read_data(s_touch_handle) != ESP_OK) {
        return;
    }

    esp_lcd_touch_point_data_t touch_points[1] = {0};
    uint8_t touch_cnt = 0;
    if (esp_lcd_touch_get_data(s_touch_handle, touch_points, &touch_cnt, 1) != ESP_OK || touch_cnt == 0) {
        return;
    }

    uint16_t panel_x = touch_points[0].x;
    uint16_t panel_y = touch_points[0].y;
    if (panel_x >= PANEL_H_RES) {
        panel_x = PANEL_H_RES - 1;
    }
    if (panel_y >= PANEL_V_RES) {
        panel_y = PANEL_V_RES - 1;
    }

    // Map native portrait touch coordinates back into the rotated LVGL landscape space.
    data->point.x = (lv_coord_t)((PANEL_V_RES - 1) - panel_y);
    data->point.y = (lv_coord_t)panel_x;
    data->state = LV_INDEV_STATE_PRESSED;

    if (!s_touch_seen_once) {
        s_touch_seen_once = true;
        ESP_LOGI(TAG, "First touch seen: panel(%u,%u) -> lvgl(%d,%d)", panel_x, panel_y, data->point.x, data->point.y);
    }
}

static bool try_init_gt911_with_addr(i2c_master_bus_handle_t i2c_bus, uint8_t dev_addr)
{
    esp_lcd_panel_io_i2c_config_t tp_io_config = ESP_LCD_TOUCH_IO_I2C_GT911_CONFIG();
    tp_io_config.scl_speed_hz = TOUCH_I2C_CLK_HZ;
    tp_io_config.dev_addr = dev_addr;

    esp_lcd_touch_io_gt911_config_t tp_gt911_config = {
        .dev_addr = dev_addr,
    };

    const esp_lcd_touch_config_t tp_cfg = {
        .x_max = PANEL_H_RES,
        .y_max = PANEL_V_RES,
        .rst_gpio_num = PIN_TOUCH_RST,
        .int_gpio_num = PIN_TOUCH_INT,
        .levels = {
            .reset = 0,
            .interrupt = 0,
        },
        .flags = {
            .swap_xy = 0,
            .mirror_x = 0,
            .mirror_y = 0,
        },
        .driver_data = &tp_gt911_config,
    };

    ESP_LOGI(TAG, "Touch init: try GT911 addr 0x%02X (int=%d rst=%d)", dev_addr, PIN_TOUCH_INT, PIN_TOUCH_RST);
    esp_err_t err = esp_lcd_new_panel_io_i2c(i2c_bus, &tp_io_config, &s_touch_io_handle);
    if (err != ESP_OK) {
        ESP_LOGW(TAG, "touch panel IO init failed for 0x%02X: %s", dev_addr, esp_err_to_name(err));
        return false;
    }

    err = esp_lcd_touch_new_i2c_gt911(s_touch_io_handle, &tp_cfg, &s_touch_handle);
    if (err != ESP_OK) {
        ESP_LOGW(TAG, "GT911 init failed for 0x%02X: %s", dev_addr, esp_err_to_name(err));
        esp_lcd_panel_io_del(s_touch_io_handle);
        s_touch_io_handle = NULL;
        s_touch_handle = NULL;
        return false;
    }

    ESP_LOGI(TAG, "GT911 init OK at I2C addr 0x%02X", dev_addr);
    return true;
}

bool bsp_touch_init(void)
{
    if (!bsp_i2c_touch_bus_init()) {
        return false;
    }

    i2c_master_bus_handle_t i2c_bus = bsp_i2c_touch_bus_get();
    if (i2c_bus == NULL) {
        return false;
    }

    const uint8_t primary_addr = ESP_LCD_TOUCH_IO_I2C_GT911_ADDRESS;
    const uint8_t backup_addr = ESP_LCD_TOUCH_IO_I2C_GT911_ADDRESS_BACKUP;
    esp_err_t probe_primary = i2c_master_probe(i2c_bus, primary_addr, 100);
    esp_err_t probe_backup = i2c_master_probe(i2c_bus, backup_addr, 100);
    ESP_LOGI(TAG, "Touch probe: 0x%02X=%s, 0x%02X=%s",
             primary_addr, (probe_primary == ESP_OK) ? "ACK" : "NOACK",
             backup_addr, (probe_backup == ESP_OK) ? "ACK" : "NOACK");

    bool gt911_ok = false;
    if (probe_primary == ESP_OK) {
        gt911_ok = try_init_gt911_with_addr(i2c_bus, primary_addr);
    }
    if (!gt911_ok && probe_backup == ESP_OK) {
        gt911_ok = try_init_gt911_with_addr(i2c_bus, backup_addr);
    }
    if (!gt911_ok) {
        gt911_ok = try_init_gt911_with_addr(i2c_bus, primary_addr);
    }
    if (!gt911_ok) {
        gt911_ok = try_init_gt911_with_addr(i2c_bus, backup_addr);
    }
    if (!gt911_ok) {
        ESP_LOGW(TAG, "GT911 init failed for both addresses");
        bsp_touch_deinit();
        return false;
    }

    if (!bsp_lvgl_register_touch(touch_read_cb, NULL)) {
        ESP_LOGW(TAG, "LVGL touch input registration failed");
        bsp_touch_deinit();
        return false;
    }

    return true;
}

void bsp_touch_deinit(void)
{
    if (s_touch_handle != NULL) {
        esp_lcd_touch_del(s_touch_handle);
        s_touch_handle = NULL;
    }
    if (s_touch_io_handle != NULL) {
        esp_lcd_panel_io_del(s_touch_io_handle);
        s_touch_io_handle = NULL;
    }
    bsp_i2c_touch_bus_deinit();
}
