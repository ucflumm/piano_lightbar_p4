idf_component_register(
    SRCS
        "main.c"
        "bsp_i2c.c"
        "bsp_lvgl_adapter.c"
        "bsp_touch.c"
        "bsp_display.c"
        "bsp_display_touch.c"
        "ui.c"
        "song_player.c"
        "note_mapping.c"
        "renderer.c"
    INCLUDE_DIRS "."
)
