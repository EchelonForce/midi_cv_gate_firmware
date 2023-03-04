#include "modes.h"
#include "shared.h"

//Callbacks for mode SYSTEM_MODE_FIFO_POLY
static mode_callbacks_type mode_fifo_polyphonic = {mode_fifo_poly_setup,
                                                   mode_fifo_poly_teardown,
                                                   mode_fifo_poly_loop,
                                                   mode_fifo_poly_handle_note_on,
                                                   mode_fifo_poly_handle_note_off,
                                                   mode_fifo_poly_all_notes_off};

typedef struct
{
    uint8_t note_value;
    boolean on;
    unsigned long time_on;
    uint8_t output_num;
} midi_note_type;

static bool note_fifo_update = false;
#define MAX_POLY_NOTES 16
static midi_note_type note_fifo[MAX_POLY_NOTES];

void mode_fifo_poly_setup(void)
{
    for (uint8_t i = 0; i < MAX_POLY_NOTES; i++)
    {
        note_fifo[i].note_value = 255;
        note_fifo[i].on = false;
        note_fifo[i].time_on = millis();
    }
    note_fifo_update = true;
}

void mode_fifo_poly_teardown(void)
{
    mode_fifo_poly_all_notes_off();
    mode_fifo_poly_loop();
}

void mode_fifo_poly_loop(void)
{
    if (note_fifo_update)
    {

        for (uint8_t i = 0; i < MAX_POLY_NOTES; i++)
        {
            update_gate_state(i, note_fifo[i].on);
            if (note_fifo[i].on)
            {
                updateCV(i, noteValueToCV(i, note_fifo[i].note_value));
            }
            else if (note_fifo[i].note_value == 255)
            {
                updateCV(i, 0);
            }
            updateCVOutput();
            update_gates();
        }
        note_fifo_update = false;
    }
}

void mode_fifo_poly_all_notes_off(void)
{
    for (uint8_t i = 0; i < MAX_POLY_NOTES; i++)
    {
        note_fifo[i].note_value = 255;
        note_fifo[i].on = false;
        note_fifo[i].time_on = millis();
    }
    note_fifo_update = true;
}

void mode_fifo_poly_handle_note_on(byte inChannel, byte inNote, byte inVelocity)
{
    uint8_t i = 0;

    for (i = 0; i < MAX_POLY_NOTES; i++)
    {
        if (note_fifo[i].note_value == inNote && note_fifo[i].on)
        {
            break;
        }
    }
    if (i == MAX_POLY_NOTES)
    {
        for (i = 0; i < MAX_POLY_NOTES; i++)
        {
            if (!note_fifo[i].on)
            {
                break;
            }
        }
    }
    if (i < MAX_POLY_NOTES)
    {
        note_fifo[i].note_value = inNote;
        note_fifo[i].on = true;
        note_fifo[i].time_on = millis();
    }
    note_fifo_update = true;
}

void mode_fifo_poly_handle_note_off(byte inChannel, byte inNote, byte inVelocity)
{
    unsigned long now = millis();
    uint8_t i = 0;
    int8_t oldest_note_idx = -1;
    for (i = 0; i < MAX_POLY_NOTES; i++)
    {
        if (note_fifo[i].on && note_fifo[i].note_value == inNote)
        {
            if (oldest_note_idx == -1 || note_fifo[i].time_on < note_fifo[oldest_note_idx].time_on)
            {
                oldest_note_idx = i;
            }
        }
    }
    if (oldest_note_idx != -1)
    {
        //note_fifo[oldest_note_idx].note_value = 255; Don't change the note value, this would break release in the ADSR.
        note_fifo[oldest_note_idx].on = false;
        note_fifo[oldest_note_idx].time_on = now;
        note_fifo_update = true;
    }
}
