#if (MODE == TEST_MIDI_MODE)

#include "shared.h"

#define TEST_MIDI_OR_SERIAL (1) // 1 = MIDI, 0 = Serial
#define TEST_MIDI_CHANNEL (11)

#if TEST_MIDI_OR_SERIAL
#include <MIDI.h>
#endif

// Create and bind the MIDI interface to the default hardware Serial port
#if TEST_MIDI_OR_SERIAL
MIDI_CREATE_DEFAULT_INSTANCE();
#endif

void setup_test_midi()
{
    //Gate Serial to Parallel IO setup 74HC595
    setupGates();

#if TEST_MIDI_OR_SERIAL
    MIDI.setHandleNoteOn(test_midi_handleNoteOn);
    MIDI.setHandleNoteOff(test_midi_handleNoteOff);
    MIDI.begin(TEST_MIDI_CHANNEL); // Listen to all incoming messages
#else
    Serial.begin(115200);
#endif
    delay(1000);
}

static uint16_t test_midi_pattern = 0;
static uint16_t last_test_midi_pattern = 0;
static unsigned long last_test_midi_note_out_update = millis();
static uint8_t test_midi_toggle_on_off = 0;
void loop_test_midi()
{
    if (time_check_and_update(&last_test_midi_note_out_update, 500))
    {
        if (test_midi_toggle_on_off)
        {
#if TEST_MIDI_OR_SERIAL
            MIDI.sendNoteOn(12, 127, TEST_MIDI_CHANNEL);
#else
            Serial.println("Note ON");
#endif
        }
        else
        {
#if TEST_MIDI_OR_SERIAL
            MIDI.sendNoteOff(12, 127, TEST_MIDI_CHANNEL);
#else
            Serial.println("Note OFF");
#endif
        }
        test_midi_toggle_on_off = !test_midi_toggle_on_off;
    }
    // Read incoming messages
    MIDI.read();

    //if (time_check_and_update(&last_gate_update, 100))
    if (last_test_midi_pattern != test_midi_pattern)
    {
        for (uint8_t i = 0; i < 16; i++)
        {
            updateGateState(i, (test_midi_pattern >> i) & 0x0001);
        }
        updateGates();
        last_test_midi_pattern = test_midi_pattern;
    }
}

void test_midi_handleNoteOn(byte inChannel, byte inNote, byte inVelocity)
{
    test_midi_pattern = (uint16_t)inNote | ((uint16_t)inVelocity << 8);
}

void test_midi_handleNoteOff(byte inChannel, byte inNote, byte inVelocity)
{
    test_midi_pattern = 0;
}
#endif //TEST_MIDI_MODE