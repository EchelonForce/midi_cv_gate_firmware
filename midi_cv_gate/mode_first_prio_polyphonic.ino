#include "modes.h"
#include "shared.h"

//Callbacks for mode SYSTEM_MODE_FIRST_PRIO_POLY
static mode_callbacks_type mode_first_prio_polyphonic = {mode_first_prio_poly_setup,
                                                         mode_first_prio_poly_teardown,
                                                         mode_first_prio_poly_loop,
                                                         mode_first_prio_poly_handle_note_on,
                                                         mode_first_prio_poly_handle_note_off,
                                                         mode_first_prio_poly_all_notes_off};

typedef struct
{
    uint8_t note_value;
    boolean on;
    unsigned long time_on;
    uint8_t output_num;
} mode_first_prio_note_type;

static bool note_first_prio_update = false;
#define MAX_POLY_NOTES 16
static mode_first_prio_note_type note_first_prio_states[MAX_POLY_NOTES];

void mode_first_prio_poly_setup(void)
{
    for (uint8_t i = 0; i < MAX_POLY_NOTES; i++)
    {
        note_first_prio_states[i].note_value = 255;
        note_first_prio_states[i].on = false;
        note_first_prio_states[i].time_on = millis();
    }
    note_first_prio_update = true;
}

void mode_first_prio_poly_teardown(void)
{
    mode_first_prio_poly_all_notes_off();
    mode_first_prio_poly_loop();
}

void mode_first_prio_poly_loop(void)
{
    if (note_first_prio_update)
    {

        for (uint8_t i = 0; i < MAX_POLY_NOTES; i++)
        {
            update_gate_state(i, note_first_prio_states[i].on);
            if (note_first_prio_states[i].on)
            {
                updateCV(i, noteValueToCV(i, note_first_prio_states[i].note_value));
            }
            else if (note_first_prio_states[i].note_value == 255)
            {
                updateCV(i, 0);
            }
            updateCVOutput();
            update_gates();
        }
        note_first_prio_update = false;
    }
}

void mode_first_prio_poly_all_notes_off(void)
{
    for (uint8_t i = 0; i < MAX_POLY_NOTES; i++)
    {
        note_first_prio_states[i].note_value = 255;
        note_first_prio_states[i].on = false;
        note_first_prio_states[i].time_on = millis();
    }
    note_first_prio_update = true;
}

void mode_first_prio_poly_handle_note_on(byte inChannel, byte inNote, byte inVelocity)
{
    uint8_t i = 0;

    for (i = 0; i < MAX_POLY_NOTES; i++)
    {
        if (note_first_prio_states[i].note_value == inNote && note_first_prio_states[i].on)
        {
            break;
        }
    }
    if (i == MAX_POLY_NOTES)
    {
        for (i = 0; i < MAX_POLY_NOTES; i++)
        {
            if (!note_first_prio_states[i].on)
            {
                break;
            }
        }
    }
    if (i < MAX_POLY_NOTES)
    {
        note_first_prio_states[i].note_value = inNote;
        note_first_prio_states[i].on = true;
        note_first_prio_states[i].time_on = millis();
    }
    note_first_prio_update = true;
}

void mode_first_prio_poly_handle_note_off(byte inChannel, byte inNote, byte inVelocity)
{
    unsigned long now = millis();
    uint8_t i = 0;
    int8_t oldest_note_idx = -1;
    for (i = 0; i < MAX_POLY_NOTES; i++)
    {
        if (note_first_prio_states[i].on && note_first_prio_states[i].note_value == inNote)
        {
            if (oldest_note_idx == -1 || note_first_prio_states[i].time_on < note_first_prio_states[oldest_note_idx].time_on)
            {
                oldest_note_idx = i;
            }
        }
    }
    if (oldest_note_idx != -1)
    {
        //note_first_prio_states[oldest_note_idx].note_value = 255; Don't change the note value, this would break release in the ADSR.
        note_first_prio_states[oldest_note_idx].on = false;
        note_first_prio_states[oldest_note_idx].time_on = now;
        note_first_prio_update = true;
    }
}
