#include "modes.h"
#include "shared.h"

//Callbacks for mode SYSTEM_MODE_DRONE
static mode_callbacks_type mode_drone = {mode_drone_setup,
                                         mode_drone_teardown,
                                         mode_drone_loop,
                                         mode_drone_handle_note_on,
                                         mode_drone_handle_note_off,
                                         mode_drone_all_notes_off};

typedef struct
{
    uint8_t note_value;
    boolean on;
    unsigned long time_on;
} mode_drone_note_type;

static bool note_drone_update = false;
#define DRONE_OUTPUTS 16
static mode_drone_note_type note_drone_state;

void mode_drone_setup(void)
{
    note_drone_state.note_value = 255;
    note_drone_state.on = false;
    note_drone_state.time_on = millis();
    note_drone_update = true;
}

void mode_drone_teardown(void)
{
    mode_drone_all_notes_off();
    mode_drone_loop();
}

void mode_drone_loop(void)
{
    if (note_drone_update)
    {
        for (uint8_t i = 0; i < DRONE_OUTPUTS; i++)
        {
            update_gate_state(i, note_drone_state.on);
            if (note_drone_state.on)
            {
                updateCV(i, noteValueToCV(i, note_drone_state.note_value));
            }
            else if (note_drone_state.note_value == 255)
            {
                updateCV(i, 0);
            }
        }
        updateCVOutput();
        update_gates();
        note_drone_update = false;
    }
}

void mode_drone_all_notes_off(void)
{
    note_drone_state.note_value = 255;
    note_drone_state.on = false;
    note_drone_state.time_on = millis();
    note_drone_update = true;
}

void mode_drone_handle_note_on(byte inChannel, byte inNote, byte inVelocity)
{
    if (note_drone_state.note_value == inNote || !note_drone_state.on)
    {
        note_drone_state.note_value = inNote;
        note_drone_state.on = true;
        note_drone_state.time_on = millis();
    }
    note_drone_update = true;
}

void mode_drone_handle_note_off(byte inChannel, byte inNote, byte inVelocity)
{
    if (note_drone_state.on)
    {
        //note_drone_state[oldest_note_idx].note_value = 255; Don't change the note value, this would break release in the ADSR.
        note_drone_state.on = false;
        note_drone_state.time_on = millis();
        note_drone_update = true;
    }
}
