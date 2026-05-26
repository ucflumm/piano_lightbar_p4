#include "note_mapping.h"

#include <stdio.h>

static const char *k_note_names[12] = {
    "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B"
};

bool note_mapping_is_piano_midi(uint8_t midi_note)
{
    return midi_note >= PIANO_MIDI_MIN && midi_note <= PIANO_MIDI_MAX;
}

int note_mapping_midi_to_key_index(uint8_t midi_note)
{
    if (!note_mapping_is_piano_midi(midi_note)) {
        return -1;
    }
    return (int)midi_note - PIANO_MIDI_MIN;
}

uint8_t note_mapping_key_index_to_midi(int key_index)
{
    if (key_index < 0 || key_index >= PIANO_KEY_COUNT) {
        return 0;
    }
    return (uint8_t)(PIANO_MIDI_MIN + key_index);
}

bool note_mapping_is_black_key(uint8_t midi_note)
{
    if (!note_mapping_is_piano_midi(midi_note)) {
        return false;
    }

    uint8_t semitone = midi_note % 12;
    return semitone == 1 || semitone == 3 || semitone == 6 || semitone == 8 || semitone == 10;
}

void note_mapping_midi_to_name(uint8_t midi_note, char *out, size_t out_len)
{
    if (out == NULL || out_len == 0) {
        return;
    }

    if (!note_mapping_is_piano_midi(midi_note)) {
        (void)snprintf(out, out_len, "?");
        return;
    }

    uint8_t semitone = midi_note % 12;
    int octave = ((int)midi_note / 12) - 1;
    (void)snprintf(out, out_len, "%s%d", k_note_names[semitone], octave);
}
