#include "song_player.h"

#include <stdio.h>
#include <string.h>

#include "note_mapping.h"

static const note_event_t k_test_song_events[] = {
    {0, 60, true},
    {500, 60, false},
    {500, 62, true},
    {1000, 62, false},
    {1000, 64, true},
    {1500, 64, false},
    {1500, 65, true},
    {2000, 65, false},
    {2000, 67, true},
    {2500, 67, false},
    {2500, 69, true},
    {3000, 69, false},
    {3000, 71, true},
    {3500, 71, false},
    {3500, 72, true},
    {4000, 72, false},
};

static uint32_t calc_song_duration_ms(const note_event_t *events, size_t event_count)
{
    if (events == NULL || event_count == 0) {
        return 0;
    }
    return events[event_count - 1].time_ms;
}

static void clear_active(song_player_t *player)
{
    (void)memset(player->active_notes, 0, sizeof(player->active_notes));
}

static void apply_event(song_player_t *player, const note_event_t *event)
{
    if (event->midi_note < sizeof(player->active_notes)) {
        player->active_notes[event->midi_note] = event->note_on;
    }
}

static void rebuild_state_to_time(song_player_t *player, uint32_t target_time_ms)
{
    clear_active(player);
    player->event_cursor = 0;

    while (player->event_cursor < player->event_count &&
           player->events[player->event_cursor].time_ms <= target_time_ms) {
        apply_event(player, &player->events[player->event_cursor]);
        player->event_cursor++;
    }

    player->current_time_ms = target_time_ms;
}

void song_player_init(song_player_t *player, const note_event_t *events, size_t event_count)
{
    if (player == NULL) {
        return;
    }

    (void)memset(player, 0, sizeof(*player));
    player->events = events;
    player->event_count = event_count;
    player->song_duration_ms = calc_song_duration_ms(events, event_count);
    player->state = PLAYBACK_STOPPED;
}

void song_player_load_test_song(song_player_t *player)
{
    song_player_init(player, k_test_song_events, sizeof(k_test_song_events) / sizeof(k_test_song_events[0]));
}

void song_player_update(song_player_t *player, uint32_t now_ms)
{
    if (player == NULL) {
        return;
    }

    if (player->state != PLAYBACK_PLAYING) {
        player->last_tick_ms = now_ms;
        return;
    }

    if (player->last_tick_ms == 0) {
        player->last_tick_ms = now_ms;
    }

    uint32_t delta = now_ms - player->last_tick_ms;
    player->last_tick_ms = now_ms;

    uint32_t new_time = player->current_time_ms + delta;
    if (new_time >= player->song_duration_ms) {
        new_time = player->song_duration_ms;
    }

    while (player->event_cursor < player->event_count &&
           player->events[player->event_cursor].time_ms <= new_time) {
        apply_event(player, &player->events[player->event_cursor]);
        player->event_cursor++;
    }

    player->current_time_ms = new_time;

    if (player->current_time_ms >= player->song_duration_ms) {
        player->state = PLAYBACK_STOPPED;
        clear_active(player);
    }
}

void song_player_toggle_play_pause(song_player_t *player)
{
    if (player == NULL) {
        return;
    }

    if (player->state == PLAYBACK_PLAYING) {
        player->state = PLAYBACK_PAUSED;
    } else {
        if (player->state == PLAYBACK_STOPPED && player->current_time_ms >= player->song_duration_ms) {
            song_player_restart(player);
        }
        player->state = PLAYBACK_PLAYING;
        player->last_tick_ms = 0;
    }
}

void song_player_restart(song_player_t *player)
{
    if (player == NULL) {
        return;
    }

    player->state = PLAYBACK_STOPPED;
    player->last_tick_ms = 0;
    rebuild_state_to_time(player, 0);
}

void song_player_seek_relative(song_player_t *player, int32_t delta_ms)
{
    if (player == NULL) {
        return;
    }

    int64_t target = (int64_t)player->current_time_ms + delta_ms;
    if (target < 0) {
        target = 0;
    }
    if (target > (int64_t)player->song_duration_ms) {
        target = player->song_duration_ms;
    }

    rebuild_state_to_time(player, (uint32_t)target);
    player->last_tick_ms = 0;

    if (player->state == PLAYBACK_STOPPED && player->current_time_ms < player->song_duration_ms) {
        clear_active(player);
    }
}

const bool *song_player_get_active_notes(const song_player_t *player)
{
    if (player == NULL) {
        return NULL;
    }
    return player->active_notes;
}

void song_player_get_active_note_text(const song_player_t *player, char *out, size_t out_len)
{
    if (out == NULL || out_len == 0) {
        return;
    }

    out[0] = '\0';
    if (player == NULL) {
        return;
    }

    bool first = true;
    char note_name[8];

    for (uint8_t midi = PIANO_MIDI_MIN; midi <= PIANO_MIDI_MAX; midi++) {
        if (!player->active_notes[midi]) {
            continue;
        }

        note_mapping_midi_to_name(midi, note_name, sizeof(note_name));

        size_t used = strlen(out);
        if (used >= out_len - 1) {
            break;
        }

        int written = snprintf(out + used, out_len - used, first ? "%s" : " %s", note_name);
        if (written <= 0) {
            break;
        }

        first = false;
    }
}
