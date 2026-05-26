#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    PLAYBACK_STOPPED = 0,
    PLAYBACK_PLAYING,
    PLAYBACK_PAUSED,
} playback_state_t;

typedef struct {
    uint32_t time_ms;
    uint8_t midi_note;
    bool note_on;
} note_event_t;

typedef struct {
    playback_state_t state;
    uint32_t current_time_ms;
    uint32_t song_duration_ms;
    uint32_t last_tick_ms;

    const note_event_t *events;
    size_t event_count;
    size_t event_cursor;

    bool active_notes[128];
} song_player_t;

void song_player_init(song_player_t *player, const note_event_t *events, size_t event_count);
void song_player_load_test_song(song_player_t *player);

void song_player_update(song_player_t *player, uint32_t now_ms);
void song_player_toggle_play_pause(song_player_t *player);
void song_player_restart(song_player_t *player);
void song_player_seek_relative(song_player_t *player, int32_t delta_ms);

const bool *song_player_get_active_notes(const song_player_t *player);
void song_player_get_active_note_text(const song_player_t *player, char *out, size_t out_len);

#ifdef __cplusplus
}
#endif
