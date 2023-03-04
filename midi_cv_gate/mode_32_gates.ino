#include "shared.h"
#include "modes.h"

//Callbacks for mode SYSTEM_MODE_32_GATES
static mode_callbacks_type mode_32_gates = {mode_32_gates_setup,
                                            mode_32_gates_teardown,
                                            mode_32_gates_loop,
                                            mode_32_gates_handle_note_on,
                                            mode_32_gates_handle_note_off,
                                            mode_32_gates_all_notes_off};

typedef struct
{
    uint8_t note_value;
    boolean on;
    unsigned long time_on;
} mode_32_gates_midi_note_type;

static bool my_note_state_update = false;
#define SIMULTANEOUS_NOTES 32
static mode_32_gates_midi_note_type my_note_states[SIMULTANEOUS_NOTES];

void mode_32_gates_setup()
{
    for (uint8_t i = 0; i < SIMULTANEOUS_NOTES; i++)
    {
        my_note_states[i].note_value = i;
        my_note_states[i].on = false;
        my_note_states[i].time_on = millis();
    }
    my_note_state_update = true;
}

void mode_32_gates_teardown()
{
    mode_32_gates_all_notes_off();
    mode_32_gates_loop();
}

void mode_32_gates_loop()
{
    if (my_note_state_update)
    {

        for (uint8_t i = 0; i < SIMULTANEOUS_NOTES; i++)
        {
            if (my_note_states[i].on)
            {
                if (i < 16)
                {
                    update_gate_state(i, HIGH);
                }
                else
                {
                    updateCV(i & 0x0F, 0xFFF);
                }
            }
            else
            {
                if (i < 16)
                {
                    update_gate_state(i, LOW);
                }
                else
                {
                    updateCV(i & 0x0F, 0);
                }
            }
            updateCVOutput();
            update_gates();
        }
        my_note_state_update = false;
    }
}

void mode_32_gates_all_notes_off()
{
    for (uint8_t i = 0; i < SIMULTANEOUS_NOTES; i++)
    {
        my_note_states[i].on = false;
        my_note_states[i].time_on = millis();
    }
    my_note_state_update = true;
}

void mode_32_gates_handle_note_on(byte inChannel, byte inNote, byte inVelocity)
{
    uint8_t i = 0;

    for (i = 0; i < SIMULTANEOUS_NOTES; i++)
    {
        if (my_note_states[i].note_value == inNote)
        {
            my_note_states[i].on = true;
            my_note_states[i].time_on = millis();
        }
    }
    my_note_state_update = true;
}

void mode_32_gates_handle_note_off(byte inChannel, byte inNote, byte inVelocity)
{
    unsigned long now = millis();
    uint8_t i = 0;
    for (i = 0; i < SIMULTANEOUS_NOTES; i++)
    {
        if (my_note_states[i].on && my_note_states[i].note_value == inNote)
        {
            my_note_states[i].on = false;
            my_note_states[i].time_on = now;
            my_note_state_update = true;
        }
    }
}
