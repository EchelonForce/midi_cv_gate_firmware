#include "shared.h"
#include "modes.h"

//Callbacks for mode SYSTEM_MODE_CONFIG
static mode_callbacks_type mode_config = {mode_config_setup,
                                          mode_config_teardown,
                                          mode_config_loop,
                                          mode_config_handle_note_on,
                                          mode_config_handle_note_off,
                                          mode_config_all_notes_off};

static unsigned long configure_mode_last_blink_time = millis();
static mode_type shown_mode = SYSTEM_MODE_CNT;
static uint8_t flashing_midi_channel = 0;

void mode_config_setup(void)
{
    configure_mode_last_blink_time = millis();
    shown_mode = SYSTEM_MODE_CNT;
    flashing_midi_channel = get_midi_channel();
}

void mode_config_teardown(void)
{
    // Turn off LEDs
    update_gate_state(flashing_midi_channel - 1, 0);
    update_gate_state(shown_mode, 0);
    update_gates();
}

void mode_config_loop(void)
{
    if (time_check_and_update(&configure_mode_last_blink_time, 250))
    {
        //Turn off prev channel gate LED if changed.
        if (flashing_midi_channel != get_midi_channel())
        {
            update_gate_state(flashing_midi_channel - 1, 0);
            flashing_midi_channel = get_midi_channel();
        }

        //blink a gate LED for channel
        if (getGateState(flashing_midi_channel - 1))
        {
            update_gate_state(flashing_midi_channel - 1, 0);
        }
        else
        {
            update_gate_state(flashing_midi_channel - 1, 1);
        }

        //turn a gate LED on solid for mode if not same as channel
        if ((flashing_midi_channel - 1) != get_system_mode())
        {
            update_gate_state(shown_mode, 0);
            shown_mode = get_system_mode();
            update_gate_state(shown_mode, 1);
        }

        update_gates();
    }
}

void mode_config_handle_note_on(byte inChannel, byte inNote, byte inVelocity) {}

void mode_config_handle_note_off(byte inChannel, byte inNote, byte inVelocity) {}

void mode_config_all_notes_off(void) {}
