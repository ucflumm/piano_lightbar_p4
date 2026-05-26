#include "bsp_display.h"

#include "bsp_board.h"
#include "bsp_lvgl_adapter.h"

#include <stdbool.h>
#include <stdint.h>

#include "driver/gpio.h"
#include "esp_err.h"
#include "esp_lcd_mipi_dsi.h"
#include "esp_lcd_panel_io.h"
#include "esp_lcd_panel_ops.h"
#include "esp_lcd_st7701.h"
#include "esp_ldo_regulator.h"
#include "esp_log.h"
#include "lvgl.h"

static const char *TAG = "bsp_display";

static lv_color_t s_draw_pixels[LVGL_H_RES * LCD_DRAW_BUF_LINES];
static lv_color_t s_rotated_pixels[LVGL_H_RES * LCD_DRAW_BUF_LINES];
static esp_lcd_panel_handle_t s_panel_handle = NULL;

// JC4880P443 (ST7701) initialization sequence adapted from known working board config.
static const st7701_lcd_init_cmd_t s_jc4880p443_init_cmds[] = {
    {0xFF, (uint8_t[]){0x77, 0x01, 0x00, 0x00, 0x13}, 5, 0},
    {0xEF, (uint8_t[]){0x08}, 1, 0},
    {0xFF, (uint8_t[]){0x77, 0x01, 0x00, 0x00, 0x10}, 5, 0},
    {0xC0, (uint8_t[]){0x63, 0x00}, 2, 0},
    {0xC1, (uint8_t[]){0x0D, 0x02}, 2, 0},
    {0xC2, (uint8_t[]){0x10, 0x08}, 2, 0},
    {0xCC, (uint8_t[]){0x10}, 1, 0},
    {0xB0, (uint8_t[]){0x80, 0x09, 0x53, 0x0C, 0xD0, 0x07, 0x0C, 0x09, 0x09, 0x28, 0x06, 0xD4, 0x13, 0x69, 0x2B, 0x71}, 16, 0},
    {0xB1, (uint8_t[]){0x80, 0x94, 0x5A, 0x10, 0xD3, 0x06, 0x0A, 0x08, 0x08, 0x25, 0x03, 0xD3, 0x12, 0x66, 0x6A, 0x0D}, 16, 0},
    {0xFF, (uint8_t[]){0x77, 0x01, 0x00, 0x00, 0x11}, 5, 0},
    {0xB0, (uint8_t[]){0x5D}, 1, 0},
    {0xB1, (uint8_t[]){0x58}, 1, 0},
    {0xB2, (uint8_t[]){0x87}, 1, 0},
    {0xB3, (uint8_t[]){0x80}, 1, 0},
    {0xB5, (uint8_t[]){0x4E}, 1, 0},
    {0xB7, (uint8_t[]){0x85}, 1, 0},
    {0xB8, (uint8_t[]){0x21}, 1, 0},
    {0xB9, (uint8_t[]){0x10, 0x1F}, 2, 0},
    {0xBB, (uint8_t[]){0x03}, 1, 0},
    {0xBC, (uint8_t[]){0x00}, 1, 0},
    {0xC1, (uint8_t[]){0x78}, 1, 0},
    {0xC2, (uint8_t[]){0x78}, 1, 0},
    {0xD0, (uint8_t[]){0x88}, 1, 0},
    {0xE0, (uint8_t[]){0x00, 0x3A, 0x02}, 3, 0},
    {0xE1, (uint8_t[]){0x04, 0xA0, 0x00, 0xA0, 0x05, 0xA0, 0x00, 0xA0, 0x00, 0x40, 0x40}, 11, 0},
    {0xE2, (uint8_t[]){0x30, 0x00, 0x40, 0x40, 0x32, 0xA0, 0x00, 0xA0, 0x00, 0xA0, 0x00, 0xA0, 0x00}, 13, 0},
    {0xE3, (uint8_t[]){0x00, 0x00, 0x33, 0x33}, 4, 0},
    {0xE4, (uint8_t[]){0x44, 0x44}, 2, 0},
    {0xE5, (uint8_t[]){0x09, 0x2E, 0xA0, 0xA0, 0x0B, 0x30, 0xA0, 0xA0, 0x05, 0x2A, 0xA0, 0xA0, 0x07, 0x2C, 0xA0, 0xA0}, 16, 0},
    {0xE6, (uint8_t[]){0x00, 0x00, 0x33, 0x33}, 4, 0},
    {0xE7, (uint8_t[]){0x44, 0x44}, 2, 0},
    {0xE8, (uint8_t[]){0x08, 0x2D, 0xA0, 0xA0, 0x0A, 0x2F, 0xA0, 0xA0, 0x04, 0x29, 0xA0, 0xA0, 0x06, 0x2B, 0xA0, 0xA0}, 16, 0},
    {0xEB, (uint8_t[]){0x00, 0x00, 0x4E, 0x4E, 0x00, 0x00, 0x00}, 7, 0},
    {0xEC, (uint8_t[]){0x08, 0x01}, 2, 0},
    {0xED, (uint8_t[]){0xB0, 0x2B, 0x98, 0xA4, 0x56, 0x7F, 0xFF, 0xFF, 0xFF, 0xFF, 0xF7, 0x65, 0x4A, 0x89, 0xB2, 0x0B}, 16, 0},
    {0xEF, (uint8_t[]){0x08, 0x08, 0x08, 0x45, 0x3F, 0x54}, 6, 0},
    {0xFF, (uint8_t[]){0x77, 0x01, 0x00, 0x00, 0x00}, 5, 0},
    {0x11, NULL, 0, 120},
    {0x29, NULL, 0, 0},
};

static void panel_flush(const lv_area_t *area, lv_color_t *color_p, void *ctx)
{
    (void)ctx;

    if (s_panel_handle == NULL) {
        return;
    }

    const int32_t width = area->x2 - area->x1 + 1;
    const int32_t height = area->y2 - area->y1 + 1;
    const size_t pixel_count = (size_t)width * (size_t)height;
    if (pixel_count > (size_t)(LVGL_H_RES * LCD_DRAW_BUF_LINES)) {
        ESP_LOGE(TAG, "flush area too large: %ld px", (long)pixel_count);
        return;
    }

    // Rotate LVGL landscape buffer to panel portrait coordinates (90 deg CCW on panel).
    for (int32_t src_y = 0; src_y < height; src_y++) {
        for (int32_t src_x = 0; src_x < width; src_x++) {
            int32_t dst_x = src_y;
            int32_t dst_y = (width - 1) - src_x;
            s_rotated_pixels[(size_t)dst_y * (size_t)height + (size_t)dst_x] = color_p[(size_t)src_y * (size_t)width + (size_t)src_x];
        }
    }

    const int32_t panel_x1 = area->y1;
    const int32_t panel_x2 = area->y2;
    const int32_t panel_y1 = (PANEL_V_RES - 1) - area->x2;
    const int32_t panel_y2 = (PANEL_V_RES - 1) - area->x1;

    esp_err_t err = esp_lcd_panel_draw_bitmap(s_panel_handle,
                                              panel_x1,
                                              panel_y1,
                                              panel_x2 + 1,
                                              panel_y2 + 1,
                                              s_rotated_pixels);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "panel draw failed: %s", esp_err_to_name(err));
    }
}

static esp_err_t init_backlight(void)
{
    gpio_config_t bk_gpio_config = {
        .mode = GPIO_MODE_OUTPUT,
        .pin_bit_mask = 1ULL << PIN_LCD_BK_LIGHT,
    };

    esp_err_t err = gpio_config(&bk_gpio_config);
    if (err != ESP_OK) {
        return err;
    }

    gpio_set_level(PIN_LCD_BK_LIGHT, LCD_BK_LIGHT_OFF_LEVEL);
    return ESP_OK;
}

static void set_backlight(bool on)
{
    gpio_set_level(PIN_LCD_BK_LIGHT, on ? LCD_BK_LIGHT_ON_LEVEL : LCD_BK_LIGHT_OFF_LEVEL);
}

static esp_err_t enable_mipi_dsi_phy_power(void)
{
    esp_ldo_channel_handle_t ldo_mipi_phy = NULL;
    esp_ldo_channel_config_t ldo_mipi_phy_config = {
        .chan_id = MIPI_DSI_PHY_PWR_LDO_CHAN,
        .voltage_mv = MIPI_DSI_PHY_PWR_LDO_VOLTAGE_MV,
    };
    return esp_ldo_acquire_channel(&ldo_mipi_phy_config, &ldo_mipi_phy);
}

bool bsp_display_init(void)
{
    ESP_LOGI(TAG, "LCD init step: backlight off + power on");
    esp_err_t err = init_backlight();
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "backlight init failed: %s", esp_err_to_name(err));
        return false;
    }

    err = enable_mipi_dsi_phy_power();
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "MIPI DSI PHY power init failed: %s", esp_err_to_name(err));
        return false;
    }

    ESP_LOGI(TAG, "LCD init step: create DSI bus (%d lanes @ %d Mbps)", MIPI_DSI_LANE_NUM, MIPI_DSI_LANE_BITRATE_MBPS);
    esp_lcd_dsi_bus_handle_t mipi_dsi_bus = NULL;
    esp_lcd_dsi_bus_config_t bus_config = {
        .bus_id = 0,
        .num_data_lanes = MIPI_DSI_LANE_NUM,
        .lane_bit_rate_mbps = MIPI_DSI_LANE_BITRATE_MBPS,
    };
    err = esp_lcd_new_dsi_bus(&bus_config, &mipi_dsi_bus);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "create DSI bus failed: %s", esp_err_to_name(err));
        goto fail;
    }

    esp_lcd_panel_io_handle_t mipi_dbi_io = NULL;
    esp_lcd_dbi_io_config_t dbi_config = {
        .virtual_channel = 0,
        .lcd_cmd_bits = 8,
        .lcd_param_bits = 8,
    };
    ESP_LOGI(TAG, "LCD init step: create DBI control channel");
    err = esp_lcd_new_panel_io_dbi(mipi_dsi_bus, &dbi_config, &mipi_dbi_io);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "create DBI control channel failed: %s", esp_err_to_name(err));
        goto fail;
    }

    esp_lcd_dpi_panel_config_t dpi_config = {
        .virtual_channel = 0,
        .dpi_clk_src = MIPI_DSI_DPI_CLK_SRC_DEFAULT,
        .dpi_clock_freq_mhz = LCD_DPI_CLK_MHZ,
        .in_color_format = LCD_COLOR_FMT_RGB565,
        .video_timing = {
            .h_size = PANEL_H_RES,
            .v_size = PANEL_V_RES,
            .hsync_back_porch = LCD_HBP,
            .hsync_pulse_width = LCD_HSYNC,
            .hsync_front_porch = LCD_HFP,
            .vsync_back_porch = LCD_VBP,
            .vsync_pulse_width = LCD_VSYNC,
            .vsync_front_porch = LCD_VFP,
        },
    };

    st7701_vendor_config_t vendor_config = {
        .init_cmds = s_jc4880p443_init_cmds,
        .init_cmds_size = sizeof(s_jc4880p443_init_cmds) / sizeof(st7701_lcd_init_cmd_t),
        .flags.use_mipi_interface = 1,
        .mipi_config = {
            .dsi_bus = mipi_dsi_bus,
            .dpi_config = &dpi_config,
        },
    };

    esp_lcd_panel_dev_config_t lcd_dev_config = {
        .reset_gpio_num = PIN_LCD_RST,
        .rgb_ele_order = LCD_RGB_ELEMENT_ORDER_RGB,
        .bits_per_pixel = 16,
        .vendor_config = &vendor_config,
    };

    ESP_LOGI(TAG, "LCD init step: create ST7701 panel (reset gpio=%d)", PIN_LCD_RST);
    err = esp_lcd_new_panel_st7701(mipi_dbi_io, &lcd_dev_config, &s_panel_handle);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "create ST7701 panel failed: %s", esp_err_to_name(err));
        goto fail;
    }

    ESP_LOGI(TAG, "LCD init step: reset panel");
    err = esp_lcd_panel_reset(s_panel_handle);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "panel reset failed: %s", esp_err_to_name(err));
        goto fail;
    }

    ESP_LOGI(TAG, "LCD init step: run panel init sequence");
    err = esp_lcd_panel_init(s_panel_handle);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "panel init sequence failed: %s", esp_err_to_name(err));
        goto fail;
    }

    ESP_LOGI(TAG, "LCD init step: display on");
    err = esp_lcd_panel_disp_on_off(s_panel_handle, true);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "panel display-on failed: %s", esp_err_to_name(err));
        goto fail;
    }

    if (!bsp_lvgl_register_display(LVGL_H_RES,
                                   LVGL_V_RES,
                                   s_draw_pixels,
                                   NULL,
                                   LVGL_H_RES * LCD_DRAW_BUF_LINES,
                                   panel_flush,
                                   NULL)) {
        ESP_LOGE(TAG, "LVGL display registration failed");
        goto fail;
    }

    set_backlight(true);
    ESP_LOGI(TAG, "Panel init complete (MIPI DSI + ST7701 JC4880P443 path)");
    return true;

fail:
    if (s_panel_handle != NULL) {
        esp_lcd_panel_del(s_panel_handle);
        s_panel_handle = NULL;
    }
    if (mipi_dbi_io != NULL) {
        esp_lcd_panel_io_del(mipi_dbi_io);
    }
    if (mipi_dsi_bus != NULL) {
        esp_lcd_del_dsi_bus(mipi_dsi_bus);
    }
    set_backlight(false);
    return false;
}

void bsp_display_create_headless(void)
{
    bsp_lvgl_create_headless_display(LVGL_H_RES,
                                     LVGL_V_RES,
                                     s_draw_pixels,
                                     NULL,
                                     LVGL_H_RES * LCD_DRAW_BUF_LINES);
}
