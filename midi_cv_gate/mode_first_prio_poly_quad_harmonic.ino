#include "modes.h"
#include "shared.h"

//Callbacks for mode SYSTEM_MODE_FIRST_PRIO_POLY_QUAD_HARMONIC
static mode_callbacks_type mode_first_prio_poly_quad_harmonic_gates = {mode_first_prio_poly_quad_harmonic_setup,
                                                                       mode_first_prio_poly_quad_harmonic_teardown,
                                                                       mode_first_prio_poly_quad_harmonic_loop,
                                                                       mode_first_prio_poly_quad_harmonic_handle_note_on,
                                                                       mode_first_prio_poly_quad_harmonic_handle_note_off,
                                                                       mode_first_prio_poly_quad_harmonic_all_notes_off};

typedef struct
{
    uint8_t note_value;
    boolean on;
    unsigned long time_on;
} fppqh_note_type;

static bool fppqh_note_state_update = false;
#define MAX_POLY_HARMONIC_NOTES 4
static fppqh_note_type fppqh_note_state[MAX_POLY_HARMONIC_NOTES];

void mode_first_prio_poly_quad_harmonic_setup(void)
{
    for (uint8_t i = 0; i < MAX_POLY_HARMONIC_NOTES; i++)
    {
        fppqh_note_state[i].note_value = 255;
        fppqh_note_state[i].on = false;
        fppqh_note_state[i].time_on = millis();
    }
    fppqh_note_state_update = true;
}

void mode_first_prio_poly_quad_harmonic_teardown(void)
{
    mode_first_prio_poly_quad_harmonic_all_notes_off();
    mode_first_prio_poly_quad_harmonic_loop();
}

void mode_first_prio_poly_quad_harmonic_loop(void)
{
    if (fppqh_note_state_update)
    {
        for (uint8_t i = 0; i < MAX_POLY_HARMONIC_NOTES; i++)
        {
            uint8_t base_idx = i * (16 / MAX_POLY_HARMONIC_NOTES);
            update_gate_state(base_idx, fppqh_note_state[i].on);
            update_gate_state(base_idx + 1, fppqh_note_state[i].on);
            update_gate_state(base_idx + 2, fppqh_note_state[i].on);
            update_gate_state(base_idx + 3, fppqh_note_state[i].on);
            if (fppqh_note_state[i].on)
            {
                updateCV(base_idx, noteValueToCV(base_idx, fppqh_note_state[i].note_value));
                updateCV(base_idx + 1, noteValueToCV(base_idx + 1, fppqh_note_state[i].note_value + 12));
                //Interpolate between the first and third harmonic to get close to the second harmonic.
                //The first harmonic is 1 V above the base note.
                //The second harmonic is 1.585 V above the base note.
                //The third harmonic is 2 V above the base note.
                //Have to use the noteValueToCV(base_idx + 2...) so that the calibration is accounted for.
                // uint16_t first_harmonic_cv = noteValueToCV(base_idx + 2, fppqh_note_state[i].note_value + 12);
                // uint16_t third_harmonic_cv = noteValueToCV(base_idx + 2, fppqh_note_state[i].note_value + 24);
                // uint32_t second_harmonic_cv = third_harmonic_cv - first_harmonic_cv;
                // second_harmonic_cv = (second_harmonic_cv * 585) / 1000;
                // second_harmonic_cv += first_harmonic_cv;
                // second_harmonic_cv = min(0xFFF, second_harmonic_cv);
                //updateCV(base_idx + 2, second_harmonic_cv);
                updateCV(base_idx + 2, noteValueToCV(base_idx + 2, fppqh_note_state[i].note_value + 19));
                updateCV(base_idx + 3, noteValueToCV(base_idx + 3, fppqh_note_state[i].note_value + 24));
            }
            else if (fppqh_note_state[i].note_value == 255)
            {
                updateCV(base_idx, 0);
                updateCV(base_idx + 1, 0);
                updateCV(base_idx + 2, 0);
                updateCV(base_idx + 3, 0);
            }
            updateCVOutput();
            update_gates();
        }
        fppqh_note_state_update = false;
    }
}

void mode_first_prio_poly_quad_harmonic_all_notes_off(void)
{
    for (uint8_t i = 0; i < MAX_POLY_HARMONIC_NOTES; i++)
    {
        fppqh_note_state[i].note_value = 255;
        fppqh_note_state[i].on = false;
        fppqh_note_state[i].time_on = millis();
    }
    fppqh_note_state_update = true;
}

void mode_first_prio_poly_quad_harmonic_handle_note_on(byte inChannel, byte inNote, byte inVelocity)
{
    uint8_t i = 0;

    for (i = 0; i < MAX_POLY_HARMONIC_NOTES; i++)
    {
        if (fppqh_note_state[i].note_value == inNote && fppqh_note_state[i].on)
        {
            break;
        }
    }
    if (i == MAX_POLY_HARMONIC_NOTES)
    {
        for (i = 0; i < MAX_POLY_HARMONIC_NOTES; i++)
        {
            if (!fppqh_note_state[i].on)
            {
                break;
            }
        }
    }
    if (i < MAX_POLY_HARMONIC_NOTES)
    {
        fppqh_note_state[i].note_value = inNote;
        fppqh_note_state[i].on = true;
        fppqh_note_state[i].time_on = millis();
    }
    fppqh_note_state_update = true;
}

void mode_first_prio_poly_quad_harmonic_handle_note_off(byte inChannel, byte inNote, byte inVelocity)
{
    unsigned long now = millis();
    uint8_t i = 0;
    int8_t oldest_note_idx = -1;
    for (i = 0; i < MAX_POLY_HARMONIC_NOTES; i++)
    {
        if (fppqh_note_state[i].on && fppqh_note_state[i].note_value == inNote)
        {
            if (oldest_note_idx == -1 || fppqh_note_state[i].time_on < fppqh_note_state[oldest_note_idx].time_on)
            {
                oldest_note_idx = i;
            }
        }
    }
    if (oldest_note_idx != -1)
    {
        //fppqh_note_state[oldest_note_idx].note_value = 255; Don't change the note value, this would break release in the ADSR.
        fppqh_note_state[oldest_note_idx].on = false;
        fppqh_note_state[oldest_note_idx].time_on = now;
        fppqh_note_state_update = true;
    }
}
