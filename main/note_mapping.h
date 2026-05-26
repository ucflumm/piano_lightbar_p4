#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define PIANO_MIDI_MIN 21
#define PIANO_MIDI_MAX 108
#define PIANO_KEY_COUNT 88

bool note_mapping_is_piano_midi(uint8_t midi_note);
int note_mapping_midi_to_key_index(uint8_t midi_note);
uint8_t note_mapping_key_index_to_midi(int key_index);
bool note_mapping_is_black_key(uint8_t midi_note);
void note_mapping_midi_to_name(uint8_t midi_note, char *out, size_t out_len);

#ifdef __cplusplus
}
#endif
