#include "shared.h"
#include <MIDI.h>
#include <serialMIDI.h>

#define NORMAL_MODE 0
#define TEST_CV_GATE_MODE 1
#define TEST_MIDI_MODE 2

#define MODE NORMAL_MODE
#define MIDI_CHANNEL 11

void setup()
{
#if (MODE == TEST_MIDI_MODE)
    setup_test_midi();
#elif (MODE == TEST_CV_GATE_MODE)
    setup_test_cv_gate();
#else //NORMAL_MODE
    setup_normal_mode();
#endif
}

void loop()
{
#if (MODE == TEST_MIDI_MODE)
    loop_test_midi();
#elif (MODE == TEST_CV_GATE_MODE)
    loop_test_cv_gate();
#else //NORMAL_MODE
    loop_normal_mode();
#endif
}

#if (MODE == NORMAL_MODE)
MIDI_CREATE_DEFAULT_INSTANCE();

typedef struct
{
    uint8_t note_value;
    boolean on;
    unsigned long time_on;
    uint8_t output_num;
} midi_note_type;

static bool note_fifo_update = false;
static midi_note_type note_fifo[16];
unsigned long last_switch_check = millis();

void setup_normal_mode()
{
    //VC DAC IO Setup MCP4922
    setupDACs();

    //Gate Serial to Parallel IO setup 74HC595
    setupGates();

    pinMode(SWITCH_1_PIN, INPUT);

    for (uint8_t i = 0; i < 16; i++)
    {
        note_fifo[i].note_value = 255;
        note_fifo[i].on = false;
        note_fifo[i].time_on = millis();
    }
    note_fifo_update = true;

    MIDI.setHandleNoteOn(handleNoteOn);
    MIDI.setHandleNoteOff(handleNoteOff);
    MIDI.begin(MIDI_CHANNEL); // Listen to all incoming messages
    delay(1000);
}

void loop_normal_mode()
{
    // if (time_check_and_update(&last_test_midi_note_out_update, 500))
    // {
    //     if (test_midi_toggle_on_off)
    //     {
    //         MIDI.sendNoteOn(12, 127, TEST_MIDI_CHANNEL);
    //     }
    //     else
    //     {
    //         MIDI.sendNoteOff(12, 127, TEST_MIDI_CHANNEL);
    //     }
    //     test_midi_toggle_on_off = !test_midi_toggle_on_off;
    // }

    // Read incoming messages
    MIDI.read();

    if (note_fifo_update)
    {

        for (uint8_t i = 0; i < 16; i++)
        {
            updateGateState(i, note_fifo[i].on);
            if (note_fifo[i].on)
            {
                updateCV(i, noteValueToCV(note_fifo[i].note_value));
            }
            else if (note_fifo[i].note_value = 255)
            {
                updateCV(i, 0);
            }
            updateCVOutput();
            updateGates();
        }
        note_fifo_update = false;
    }

    if (time_check_and_update(&last_switch_check, 100))
    {
        if (digitalRead(SWITCH_1_PIN))
        {
            allNotesOff();
        }
    }
}

void allNotesOff()
{
    for (uint8_t i = 0; i < 16; i++)
    {
        note_fifo[i].note_value = 255;
        note_fifo[i].on = false;
        note_fifo[i].time_on = millis();
    }
    note_fifo_update = true;
}

uint16_t noteValueToCV(uint32_t note_val)
{
    //C-2 = 0 Volts. note_vals is a 7 bit value
    // DACs are 12 bit
    //Max Voltage = 9.21 = fff
    // C-2 = 0V
    // C-1 = 1V
    // C0  = 2V
    // C1  = 3V
    // C2  = 4V
    // C3  = 5V
    // C4  = 6V
    // C5  = 7V
    // C6  = 8V
    // C7  = 9V
    // 409=1V 409/12=34.08, 3380 used because it works well emperichally.
    static const uint32_t cal_factor = 3380; //DAC uints per semitone * 100.
    return min(0xFFF, (note_val * cal_factor) / 100);
}

void handleNoteOn(byte inChannel, byte inNote, byte inVelocity)
{
    uint8_t i = 0;

    for (i = 0; i < 16; i++)
    {
        if (!note_fifo[i].on)
        {
            break;
        }
    }
    if (i < 16)
    {
        note_fifo[i].note_value = inNote;
        note_fifo[i].on = true;
        note_fifo[i].time_on = millis();
    }
    note_fifo_update = true;
}

void handleNoteOff(byte inChannel, byte inNote, byte inVelocity)
{
    unsigned long now = millis();
    uint8_t i = 0;
    int8_t oldest_note_idx = -1;
    for (i = 0; i < 16; i++)
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
        note_fifo[oldest_note_idx].note_value = 255;
        note_fifo[oldest_note_idx].on = false;
        note_fifo[oldest_note_idx].time_on = now;
        note_fifo_update = true;
    }
}
#endif //NORMAL_MODE
