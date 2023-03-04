#include "shared.h"
#include "modes.h"

//Callbacks for mode SYSTEM_MODE_TEST_MODE
static mode_callbacks_type mode_test = {mode_test_setup,
                                        mode_test_teardown,
                                        mode_test_loop,
                                        mode_test_handle_note_on,
                                        mode_test_handle_note_off,
                                        mode_test_all_notes_off};

static uint16_t test_value_out = 0;
static unsigned long test_last_gate_update = millis();
static unsigned long test_last_cv_update = millis();
static uint16_t test_gate_pattern = 1;

void mode_test_setup(void)
{
    test_value_out = 0;
    test_last_gate_update = millis();
    test_last_cv_update = millis();
    test_gate_pattern = 1;
}

void mode_test_teardown(void)
{
    mode_test_all_notes_off();
}

void mode_test_loop(void)
{
    if (time_check_and_update(&test_last_cv_update, 4000))
    {
        for (uint8_t i = 0; i < 16; i++)
        {
            updateCVandOutput(i, test_value_out);
        }
        test_value_out += 0x111;
        if (test_value_out > 0x0FFF)
        {
            test_value_out = 0;
        }
    }

    if (time_check_and_update(&test_last_gate_update, 100))
    {
        for (uint8_t i = 0; i < 16; i++)
        {
            update_gate_state(i, (test_gate_pattern >> i) & 0x0001);
        }
        update_gates();
        test_gate_pattern = test_gate_pattern << 1;
        if (test_gate_pattern == 0)
        {
            test_gate_pattern = 1;
        }
    }
}

void mode_test_handle_note_on(byte inChannel, byte inNote, byte inVelocity)
{
    updateCVandOutput(min(inNote, 15), 0xFFF);
    update_gate_state(min(inNote, 15), 1);
    update_gates();
}

void mode_test_handle_note_off(byte inChannel, byte inNote, byte inVelocity)
{
    updateCVandOutput(min(inNote, 15), 0);
    update_gate_state(min(inNote, 15), 0);
    update_gates();
}

void mode_test_all_notes_off(void)
{
    for (uint8_t i = 0; i < 16; i++)
    {
        update_gate_state(i, 0);
        updateCVandOutput(i, 0);
    }
    update_gates();
}
