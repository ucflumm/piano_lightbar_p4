#pragma once

#include <stdbool.h>
#include <stdint.h>

#include "lvgl.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct renderer renderer_t;

renderer_t *renderer_create_virtual_keyboard(lv_obj_t *parent);
void renderer_update(renderer_t *renderer, const bool *active_notes);
void renderer_destroy(renderer_t *renderer);

#ifdef __cplusplus
}
#endif
