idf_component_register(
    SRCS
        "src/bsp_i2c.c"
        "src/bsp_lvgl_adapter.c"
        "src/bsp_touch.c"
        "src/bsp_display.c"
        "src/bsp_display_touch.c"
    INCLUDE_DIRS "include"
    REQUIRES lvgl
    PRIV_REQUIRES esp_lcd esp_lcd_st7701 esp_lcd_touch esp_lcd_touch_gt911 esp_lcd_panel_io_additions driver esp_hw_support
)
