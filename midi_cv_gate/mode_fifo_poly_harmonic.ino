#include "modes.h"
#include "shared.h"

//Callbacks for mode SYSTEM_MODE_FIFO_POLY_HARMONIC
static mode_callbacks_type mode_fifo_poly_harmonic_gates = {mode_fifo_poly_harmonic_setup,
                                                            mode_fifo_poly_harmonic_teardown,
                                                            mode_fifo_poly_harmonic_loop,
                                                            mode_fifo_poly_harmonic_handle_note_on,
                                                            mode_fifo_poly_harmonic_handle_note_off,
                                                            mode_fifo_poly_harmonic_all_notes_off};

#define SECOND_HARMONIC_CV 705 // 1.585V / (9.21 V / 0xFFF bits)

typedef struct
{
    uint8_t note_value;
    boolean on;
    unsigned long time_on;
    uint8_t output_num;
} ph_note_type;

static bool ph_note_fifo_update = false;
#define MAX_POLY_HARMONIC_NOTES 4
static ph_note_type ph_note_fifo[MAX_POLY_HARMONIC_NOTES];

void mode_fifo_poly_harmonic_setup(void)
{
    for (uint8_t i = 0; i < MAX_POLY_HARMONIC_NOTES; i++)
    {
        ph_note_fifo[i].note_value = 255;
        ph_note_fifo[i].on = false;
        ph_note_fifo[i].time_on = millis();
    }
    ph_note_fifo_update = true;
}

void mode_fifo_poly_harmonic_teardown(void)
{
    mode_fifo_poly_harmonic_all_notes_off();
    mode_fifo_poly_harmonic_loop();
}

void mode_fifo_poly_harmonic_loop(void)
{
    if (ph_note_fifo_update)
    {
        for (uint8_t i = 0; i < MAX_POLY_HARMONIC_NOTES; i++)
        {
            uint8_t base_idx = i * (16 / MAX_POLY_HARMONIC_NOTES);
            update_gate_state(base_idx, ph_note_fifo[i].on);
            update_gate_state(base_idx + 1, ph_note_fifo[i].on);
            update_gate_state(base_idx + 2, ph_note_fifo[i].on);
            update_gate_state(base_idx + 3, ph_note_fifo[i].on);
            if (ph_note_fifo[i].on)
            {
                updateCV(base_idx, noteValueToCV(base_idx, ph_note_fifo[i].note_value));
                updateCV(base_idx + 1, noteValueToCV(base_idx + 1, ph_note_fifo[i].note_value + 12));
                updateCV(base_idx + 2, noteValueToCV(base_idx + 2, ph_note_fifo[i].note_value) + SECOND_HARMONIC_CV);
                updateCV(base_idx + 3, noteValueToCV(base_idx + 3, ph_note_fifo[i].note_value + 24));
            }
            else if (ph_note_fifo[i].note_value == 255)
            {
                updateCV(base_idx, 0);
                updateCV(base_idx + 1, 0);
                updateCV(base_idx + 2, 0);
                updateCV(base_idx + 3, 0);
            }
            updateCVOutput();
            update_gates();
        }
        ph_note_fifo_update = false;
    }
}

void mode_fifo_poly_harmonic_all_notes_off(void)
{
    for (uint8_t i = 0; i < MAX_POLY_HARMONIC_NOTES; i++)
    {
        ph_note_fifo[i].note_value = 255;
        ph_note_fifo[i].on = false;
        ph_note_fifo[i].time_on = millis();
    }
    ph_note_fifo_update = true;
}

void mode_fifo_poly_harmonic_handle_note_on(byte inChannel, byte inNote, byte inVelocity)
{
    uint8_t i = 0;

    for (i = 0; i < MAX_POLY_HARMONIC_NOTES; i++)
    {
        if (ph_note_fifo[i].note_value == inNote && ph_note_fifo[i].on)
        {
            break;
        }
    }
    if (i == MAX_POLY_HARMONIC_NOTES)
    {
        for (i = 0; i < MAX_POLY_HARMONIC_NOTES; i++)
        {
            if (!ph_note_fifo[i].on)
            {
                break;
            }
        }
    }
    if (i < MAX_POLY_HARMONIC_NOTES)
    {
        ph_note_fifo[i].note_value = inNote;
        ph_note_fifo[i].on = true;
        ph_note_fifo[i].time_on = millis();
    }
    ph_note_fifo_update = true;
}

void mode_fifo_poly_harmonic_handle_note_off(byte inChannel, byte inNote, byte inVelocity)
{
    unsigned long now = millis();
    uint8_t i = 0;
    int8_t oldest_note_idx = -1;
    for (i = 0; i < MAX_POLY_HARMONIC_NOTES; i++)
    {
        if (ph_note_fifo[i].on && ph_note_fifo[i].note_value == inNote)
        {
            if (oldest_note_idx == -1 || ph_note_fifo[i].time_on < ph_note_fifo[oldest_note_idx].time_on)
            {
                oldest_note_idx = i;
            }
        }
    }
    if (oldest_note_idx != -1)
    {
        //ph_note_fifo[oldest_note_idx].note_value = 255; Don't change the note value, this would break release in the ADSR.
        ph_note_fifo[oldest_note_idx].on = false;
        ph_note_fifo[oldest_note_idx].time_on = now;
        ph_note_fifo_update = true;
    }
}
