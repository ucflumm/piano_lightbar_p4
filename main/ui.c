#include "ui.h"

#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "lvgl.h"

#include "renderer.h"

typedef struct {
    lv_obj_t *title;
    lv_obj_t *song_name;
    lv_obj_t *action_label;
    lv_obj_t *time_label;
    lv_obj_t *tempo_label;
    lv_obj_t *notes_label;
    lv_obj_t *seek_minus_btn;
    lv_obj_t *seek_minus_btn_label;
    lv_obj_t *restart_btn;
    lv_obj_t *restart_btn_label;
    lv_obj_t *play_pause_btn;
    lv_obj_t *play_pause_btn_label;
    lv_obj_t *seek_plus_btn;
    lv_obj_t *seek_plus_btn_label;
} ui_widgets_t;

struct ui_ctx {
    song_player_t *player;
    renderer_t *renderer;
    ui_widgets_t w;
    uint32_t last_time_sec;
    playback_state_t last_state;
    char last_notes[96];
    uint32_t feedback_until_ms;
};

static void show_action_feedback(ui_ctx_t *ui, const char *message)
{
    if (ui == NULL || ui->w.action_label == NULL || message == NULL) {
        return;
    }

    lv_label_set_text_fmt(ui->w.action_label, "Action: %s", message);
    lv_obj_clear_flag(ui->w.action_label, LV_OBJ_FLAG_HIDDEN);
    ui->feedback_until_ms = ui->player->current_time_ms + 1200;
}

static void on_play_pause(lv_event_t *event)
{
    ui_ctx_t *ui = (ui_ctx_t *)lv_event_get_user_data(event);
    if (ui == NULL || ui->player == NULL) {
        return;
    }

    song_player_toggle_play_pause(ui->player);
    show_action_feedback(ui, (ui->player->state == PLAYBACK_PLAYING) ? "Play" : "Pause");
}

static void on_seek_minus_10(lv_event_t *event)
{
    ui_ctx_t *ui = (ui_ctx_t *)lv_event_get_user_data(event);
    if (ui == NULL || ui->player == NULL) {
        return;
    }

    song_player_seek_relative(ui->player, -10000);
    show_action_feedback(ui, "Seek -10s");
}

static void on_seek_plus_10(lv_event_t *event)
{
    ui_ctx_t *ui = (ui_ctx_t *)lv_event_get_user_data(event);
    if (ui == NULL || ui->player == NULL) {
        return;
    }

    song_player_seek_relative(ui->player, 10000);
    show_action_feedback(ui, "Seek +10s");
}

static void on_restart(lv_event_t *event)
{
    ui_ctx_t *ui = (ui_ctx_t *)lv_event_get_user_data(event);
    if (ui == NULL || ui->player == NULL) {
        return;
    }

    song_player_restart(ui->player);
    show_action_feedback(ui, "Restart");
}

static void style_transport_button(lv_obj_t *button, lv_color_t default_color, lv_color_t pressed_color)
{
    lv_obj_set_style_bg_color(button, default_color, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(button, pressed_color, LV_PART_MAIN | LV_STATE_PRESSED);
    lv_obj_set_style_bg_opa(button, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_style_radius(button, 8, LV_PART_MAIN);
    lv_obj_set_style_border_width(button, 0, LV_PART_MAIN);
    lv_obj_set_style_outline_width(button, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_outline_width(button, 2, LV_PART_MAIN | LV_STATE_PRESSED);
    lv_obj_set_style_outline_color(button, lv_color_hex(0xE2E8F0), LV_PART_MAIN | LV_STATE_PRESSED);
    lv_obj_set_style_outline_opa(button, LV_OPA_60, LV_PART_MAIN | LV_STATE_PRESSED);
    lv_obj_set_style_translate_y(button, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_translate_y(button, 2, LV_PART_MAIN | LV_STATE_PRESSED);
}

ui_ctx_t *ui_create(song_player_t *player)
{
    if (player == NULL) {
        return NULL;
    }

    ui_ctx_t *ui = (ui_ctx_t *)calloc(1, sizeof(ui_ctx_t));
    if (ui == NULL) {
        return NULL;
    }

    ui->player = player;
    ui->last_time_sec = UINT32_MAX;
    ui->last_state = (playback_state_t)255;
    ui->last_notes[0] = '\0';

    lv_obj_t *screen = lv_scr_act();
    lv_obj_set_style_bg_color(screen, lv_color_hex(0x0A1424), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(screen, LV_OPA_COVER, LV_PART_MAIN);

    lv_obj_t *root = lv_obj_create(screen);
    lv_obj_set_size(root, lv_disp_get_hor_res(NULL), lv_disp_get_ver_res(NULL));
    lv_obj_set_style_bg_opa(root, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_border_width(root, 0, LV_PART_MAIN);
    lv_obj_set_style_pad_all(root, 0, LV_PART_MAIN);

    const int32_t content_x = 12;
    const int32_t content_w = lv_disp_get_hor_res(NULL) - (content_x * 2);
    const int32_t button_gap = 12;
    const int32_t button_y = 110;
    const int32_t button_w = (content_w - (button_gap * 3)) / 4;
    const int32_t button_h = 40;

    ui->w.title = lv_label_create(root);
    lv_obj_set_style_text_font(ui->w.title, &lv_font_montserrat_14, LV_PART_MAIN);
    lv_obj_set_style_text_color(ui->w.title, lv_color_hex(0xF3F4F6), LV_PART_MAIN);
    lv_label_set_text(ui->w.title, "Piano Light Bar");
    lv_obj_set_pos(ui->w.title, 12, 8);

    ui->w.song_name = lv_label_create(root);
    lv_obj_set_style_text_font(ui->w.song_name, &lv_font_montserrat_14, LV_PART_MAIN);
    lv_obj_set_style_text_color(ui->w.song_name, lv_color_hex(0xB4CCF4), LV_PART_MAIN);
    lv_label_set_text(ui->w.song_name, "Test Scale");
    lv_obj_set_pos(ui->w.song_name, 12, 30);

    ui->w.action_label = lv_label_create(root);
    lv_obj_set_style_text_font(ui->w.action_label, &lv_font_montserrat_14, LV_PART_MAIN);
    lv_obj_set_style_text_color(ui->w.action_label, lv_color_hex(0x93C5FD), LV_PART_MAIN);
    lv_obj_set_pos(ui->w.action_label, 12, 54);
    lv_label_set_text(ui->w.action_label, "Action: Ready");
    lv_obj_add_flag(ui->w.action_label, LV_OBJ_FLAG_HIDDEN);

    ui->w.time_label = lv_label_create(root);
    lv_obj_set_style_text_font(ui->w.time_label, &lv_font_montserrat_14, LV_PART_MAIN);
    lv_obj_set_style_text_color(ui->w.time_label, lv_color_hex(0xF9FAFB), LV_PART_MAIN);
    lv_obj_set_pos(ui->w.time_label, 12, 76);

    ui->w.tempo_label = lv_label_create(root);
    lv_obj_set_style_text_font(ui->w.tempo_label, &lv_font_montserrat_14, LV_PART_MAIN);
    lv_obj_set_style_text_color(ui->w.tempo_label, lv_color_hex(0xF9FAFB), LV_PART_MAIN);
    lv_label_set_text(ui->w.tempo_label, "Tempo: 100%");
    lv_obj_set_pos(ui->w.tempo_label, content_x + content_w - 120, 76);

    ui->w.notes_label = lv_label_create(root);
    lv_obj_set_width(ui->w.notes_label, content_w);
    lv_obj_set_style_text_font(ui->w.notes_label, &lv_font_montserrat_14, LV_PART_MAIN);
    lv_obj_set_style_text_color(ui->w.notes_label, lv_color_hex(0xE8EEFB), LV_PART_MAIN);
    lv_obj_set_pos(ui->w.notes_label, 12, 100);

    ui->w.seek_minus_btn = lv_btn_create(root);
    lv_obj_set_size(ui->w.seek_minus_btn, button_w, button_h);
    lv_obj_set_pos(ui->w.seek_minus_btn, content_x, button_y);
    style_transport_button(ui->w.seek_minus_btn, lv_color_hex(0x334155), lv_color_hex(0x1E293B));
    lv_obj_add_event_cb(ui->w.seek_minus_btn, on_seek_minus_10, LV_EVENT_CLICKED, ui);

    ui->w.seek_minus_btn_label = lv_label_create(ui->w.seek_minus_btn);
    lv_obj_set_style_text_font(ui->w.seek_minus_btn_label, &lv_font_montserrat_14, LV_PART_MAIN);
    lv_obj_set_style_text_color(ui->w.seek_minus_btn_label, lv_color_hex(0xF9FAFB), LV_PART_MAIN);
    lv_label_set_text(ui->w.seek_minus_btn_label, "-10s");
    lv_obj_center(ui->w.seek_minus_btn_label);

    ui->w.restart_btn = lv_btn_create(root);
    lv_obj_set_size(ui->w.restart_btn, button_w, button_h);
    lv_obj_set_pos(ui->w.restart_btn, content_x + button_w + button_gap, button_y);
    style_transport_button(ui->w.restart_btn, lv_color_hex(0x1E293B), lv_color_hex(0x0F172A));
    lv_obj_add_event_cb(ui->w.restart_btn, on_restart, LV_EVENT_CLICKED, ui);

    ui->w.restart_btn_label = lv_label_create(ui->w.restart_btn);
    lv_obj_set_style_text_font(ui->w.restart_btn_label, &lv_font_montserrat_14, LV_PART_MAIN);
    lv_obj_set_style_text_color(ui->w.restart_btn_label, lv_color_hex(0xF9FAFB), LV_PART_MAIN);
    lv_label_set_text(ui->w.restart_btn_label, "Restart");
    lv_obj_center(ui->w.restart_btn_label);

    ui->w.play_pause_btn = lv_btn_create(root);
    lv_obj_set_size(ui->w.play_pause_btn, button_w, button_h);
    lv_obj_set_pos(ui->w.play_pause_btn, content_x + (button_w + button_gap) * 2, button_y);
    style_transport_button(ui->w.play_pause_btn, lv_color_hex(0x1D4ED8), lv_color_hex(0x1E40AF));
    lv_obj_add_event_cb(ui->w.play_pause_btn, on_play_pause, LV_EVENT_CLICKED, ui);

    ui->w.play_pause_btn_label = lv_label_create(ui->w.play_pause_btn);
    lv_obj_set_style_text_font(ui->w.play_pause_btn_label, &lv_font_montserrat_14, LV_PART_MAIN);
    lv_obj_set_style_text_color(ui->w.play_pause_btn_label, lv_color_hex(0xF9FAFB), LV_PART_MAIN);
    lv_label_set_text(ui->w.play_pause_btn_label, "Play");
    lv_obj_center(ui->w.play_pause_btn_label);

    ui->w.seek_plus_btn = lv_btn_create(root);
    lv_obj_set_size(ui->w.seek_plus_btn, button_w, button_h);
    lv_obj_set_pos(ui->w.seek_plus_btn, content_x + (button_w + button_gap) * 3, button_y);
    style_transport_button(ui->w.seek_plus_btn, lv_color_hex(0x334155), lv_color_hex(0x1E293B));
    lv_obj_add_event_cb(ui->w.seek_plus_btn, on_seek_plus_10, LV_EVENT_CLICKED, ui);

    ui->w.seek_plus_btn_label = lv_label_create(ui->w.seek_plus_btn);
    lv_obj_set_style_text_font(ui->w.seek_plus_btn_label, &lv_font_montserrat_14, LV_PART_MAIN);
    lv_obj_set_style_text_color(ui->w.seek_plus_btn_label, lv_color_hex(0xF9FAFB), LV_PART_MAIN);
    lv_label_set_text(ui->w.seek_plus_btn_label, "+10s");
    lv_obj_center(ui->w.seek_plus_btn_label);

    ui->renderer = renderer_create_virtual_keyboard(root);

    ui_update(ui);
    return ui;
}

void ui_update(ui_ctx_t *ui)
{
    if (ui == NULL || ui->player == NULL) {
        return;
    }

    uint32_t total_secs = ui->player->current_time_ms / 1000;
    uint32_t mins = total_secs / 60;
    uint32_t secs = total_secs % 60;

    if (total_secs != ui->last_time_sec) {
        lv_label_set_text_fmt(ui->w.time_label, "Time: %02" PRIu32 ":%02" PRIu32, mins, secs);
        ui->last_time_sec = total_secs;
    }

    if (ui->player->state != ui->last_state) {
        if (ui->w.play_pause_btn_label != NULL) {
            const char *label = (ui->player->state == PLAYBACK_PLAYING) ? "Pause" : "Play";
            lv_label_set_text(ui->w.play_pause_btn_label, label);
        }
        ui->last_state = ui->player->state;
    }

    char notes[96] = {0};
    song_player_get_active_note_text(ui->player, notes, sizeof(notes));

    if (ui->feedback_until_ms != 0 && ui->player->current_time_ms >= ui->feedback_until_ms) {
        lv_obj_add_flag(ui->w.action_label, LV_OBJ_FLAG_HIDDEN);
        ui->feedback_until_ms = 0;
    }

    const char *note_text = (notes[0] == '\0') ? "-" : notes;
    if (strcmp(note_text, ui->last_notes) != 0) {
        if (notes[0] == '\0') {
            lv_label_set_text(ui->w.notes_label, "Notes: -");
        } else {
            lv_label_set_text_fmt(ui->w.notes_label, "Notes: %s", notes);
        }
        (void)snprintf(ui->last_notes, sizeof(ui->last_notes), "%s", note_text);
    }

    renderer_update(ui->renderer, song_player_get_active_notes(ui->player));
}

void ui_destroy(ui_ctx_t *ui)
{
    if (ui == NULL) {
        return;
    }

    renderer_destroy(ui->renderer);
    free(ui);
}
