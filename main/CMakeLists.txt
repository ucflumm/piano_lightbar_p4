idf_component_register(
    SRCS
        "main.c"
        "ui.c"
        "song_player.c"
        "note_mapping.c"
        "renderer.c"
    INCLUDE_DIRS "."
    REQUIRES p4_bsp
)
