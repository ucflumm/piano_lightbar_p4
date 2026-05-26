#include "renderer.h"

#include <stdlib.h>

#include "note_mapping.h"

typedef struct {
    lv_obj_t *key_obj;
    bool is_black;
    bool was_active;
} key_slot_t;

struct renderer {
    lv_obj_t *container;
    key_slot_t keys[PIANO_KEY_COUNT];
};

static void set_key_style(lv_obj_t *obj, bool is_black, bool active)
{
    lv_color_t color = lv_color_hex(0xF2F2F2);
    if (is_black) {
        color = lv_color_hex(0x2D2D2D);
    }
    if (active) {
        color = is_black ? lv_color_hex(0xFF7A00) : lv_color_hex(0x4AA8FF);
    }

    lv_obj_set_style_bg_color(obj, color, LV_PART_MAIN);
}

static void apply_key_static_style(lv_obj_t *obj)
{
    lv_obj_set_style_bg_opa(obj, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_style_border_color(obj, lv_color_hex(0x666666), LV_PART_MAIN);
    lv_obj_set_style_border_width(obj, 1, LV_PART_MAIN);
    lv_obj_set_style_radius(obj, 2, LV_PART_MAIN);
}

renderer_t *renderer_create_virtual_keyboard(lv_obj_t *parent)
{
    if (parent == NULL) {
        return NULL;
    }

    renderer_t *renderer = (renderer_t *)calloc(1, sizeof(renderer_t));
    if (renderer == NULL) {
        return NULL;
    }

    int32_t parent_w = lv_obj_get_width(parent);
    int32_t parent_h = lv_obj_get_height(parent);
    if (parent_w <= 0 || parent_h <= 0) {
        lv_disp_t *disp = lv_obj_get_disp(parent);
        if (disp != NULL) {
            parent_w = lv_disp_get_hor_res(disp);
            parent_h = lv_disp_get_ver_res(disp);
        }
    }

    if (parent_w <= 0 || parent_h <= 0) {
        parent_w = 800;
        parent_h = 480;
    }

    const int32_t container_x = 6;
    const int32_t container_y = 170;
    int32_t container_w = parent_w - (container_x * 2);
    int32_t container_h = parent_h - container_y - 8;
    if (container_w < 120) {
        container_w = 120;
    }
    if (container_h < 80) {
        container_h = 80;
    }

    renderer->container = lv_obj_create(parent);
    lv_obj_set_size(renderer->container, container_w, container_h);
    lv_obj_set_pos(renderer->container, container_x, container_y);
    lv_obj_set_style_pad_all(renderer->container, 6, LV_PART_MAIN);
    lv_obj_set_style_bg_color(renderer->container, lv_color_hex(0x111111), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(renderer->container, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_style_border_color(renderer->container, lv_color_hex(0x444444), LV_PART_MAIN);
    lv_obj_set_style_border_width(renderer->container, 1, LV_PART_MAIN);

    const int32_t inner_x = 6;
    const int32_t inner_y = 6;
    int32_t inner_w = container_w - (inner_x * 2);
    int32_t inner_h = container_h - (inner_y * 2);
    if (inner_w < PIANO_KEY_COUNT) {
        inner_w = PIANO_KEY_COUNT;
    }
    if (inner_h < 30) {
        inner_h = 30;
    }
    const int32_t white_h = inner_h;
    const int32_t black_h = (inner_h * 2) / 3;

    int32_t base_total_w = 0;
    for (int i = 0; i < PIANO_KEY_COUNT; i++) {
        uint8_t midi = note_mapping_key_index_to_midi(i);
        bool is_black = note_mapping_is_black_key(midi);
        base_total_w += is_black ? 6 : 8;
        if (i < (PIANO_KEY_COUNT - 1)) {
            base_total_w += 1;
        }
    }

    int32_t base_cursor = 0;

    for (int i = 0; i < PIANO_KEY_COUNT; i++) {
        uint8_t midi = note_mapping_key_index_to_midi(i);
        bool is_black = note_mapping_is_black_key(midi);

        lv_obj_t *key = lv_obj_create(renderer->container);
        lv_obj_set_style_pad_all(key, 0, LV_PART_MAIN);
        lv_obj_set_style_pad_right(key, 0, LV_PART_MAIN);

        const int32_t base_key_w = is_black ? 6 : 8;
        const int32_t x_start = inner_x + (base_cursor * inner_w) / base_total_w;
        base_cursor += base_key_w;
        const int32_t x_end = inner_x + (base_cursor * inner_w) / base_total_w;
        const int32_t key_w = (x_end > x_start) ? (x_end - x_start) : 1;
        const int32_t key_h = is_black ? black_h : white_h;
        const int32_t key_y = is_black ? (inner_y + (white_h - key_h)) : inner_y;

        lv_obj_set_size(key, key_w, key_h);
        lv_obj_set_pos(key, x_start, key_y);

        if (i < (PIANO_KEY_COUNT - 1)) {
            base_cursor += 1;
        }

        apply_key_static_style(key);
        set_key_style(key, is_black, false);

        renderer->keys[i].key_obj = key;
        renderer->keys[i].is_black = is_black;
        renderer->keys[i].was_active = false;
    }

    return renderer;
}

void renderer_update(renderer_t *renderer, const bool *active_notes)
{
    if (renderer == NULL || active_notes == NULL) {
        return;
    }

    for (int i = 0; i < PIANO_KEY_COUNT; i++) {
        lv_obj_t *key = renderer->keys[i].key_obj;
        if (key == NULL) {
            continue;
        }

        uint8_t midi = note_mapping_key_index_to_midi(i);
        bool is_active = active_notes[midi];
        if (is_active == renderer->keys[i].was_active) {
            continue;
        }

        set_key_style(key, renderer->keys[i].is_black, is_active);
        renderer->keys[i].was_active = is_active;
    }
}

void renderer_destroy(renderer_t *renderer)
{
    if (renderer == NULL) {
        return;
    }

    if (renderer->container != NULL) {
        lv_obj_del(renderer->container);
    }

    free(renderer);
}
