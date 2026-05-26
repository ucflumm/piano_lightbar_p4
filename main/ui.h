#pragma once

#include <stdint.h>

#include "song_player.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct ui_ctx ui_ctx_t;

ui_ctx_t *ui_create(song_player_t *player);
void ui_update(ui_ctx_t *ui);
void ui_destroy(ui_ctx_t *ui);

#ifdef __cplusplus
}
#endif
