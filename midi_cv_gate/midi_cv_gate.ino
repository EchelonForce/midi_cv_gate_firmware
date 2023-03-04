#include "shared.h"
#include "modes.h"
#include "eeprom_settings.h"
#include <MIDI.h>
#include <serialMIDI.h>

// Time to hold switch to enter/exit config mode.
#define PRESS_AND_HOLD_TIME 2000

static mode_callbacks_type *my_mode_clbks = &mode_fifo_polyphonic;
static mode_type my_current_mode = SYSTEM_MODE_DEFAULT;
#define call_mode_clbk_if_valid(_clbk_name) (my_mode_clbks->_clbk_name) ? my_mode_clbks->_clbk_name() : (void)0

unsigned long last_switch_check = millis();

MIDI_CREATE_DEFAULT_INSTANCE();

void setup()
{
    //VC DAC IO Setup MCP4922
    setupDACs();

    //Gate Serial to Parallel IO setup 74HC595
    setupGates();

    pinMode(SWITCH_1_PIN, INPUT);

    setupEEPROM();

    mode_change(get_system_mode());

    MIDI.begin(get_midi_channel()); // Listen to all incoming messages
    MIDI.turnThruOn();

    delay(1000);
}

void loop()
{

    if (time_check_and_update(&last_switch_check, 100))
    {
        update_switch_state();
        handle_switch_normal_modes();
    }
    // Read incoming messages
    MIDI.read();
    call_mode_clbk_if_valid(loop);
}

void setup_midi()
{
    MIDI.setHandleNoteOn(my_mode_clbks->handle_note_on);
    MIDI.setHandleNoteOff(my_mode_clbks->handle_note_off);
    MIDI.setHandleControlChange(handle_midi_control_change);
}

void handle_midi_control_change(byte inChannel, byte inControlChangeNumber, byte inValue)
{
    auto cn = static_cast<midi::MidiControlChangeNumber>(inControlChangeNumber);

    switch (cn)
    {
    case midi::MidiControlChangeNumber::AllSoundOff:
    case midi::MidiControlChangeNumber::AllNotesOff:
        call_mode_clbk_if_valid(all_notes_off);
        break;
    case midi::MidiControlChangeNumber::DataEntryLSB: //Arbitrary Choice
        if (inValue < SYSTEM_MODE_CNT)
        {
            mode_change(inValue);
        }
        break;
    default:
        break;
    }
}

void mode_change(mode_type new_mode)
{
    call_mode_clbk_if_valid(teardown);

    update_system_mode(new_mode);
    switch (new_mode)
    {
    case SYSTEM_MODE_32_GATES:
        my_mode_clbks = &mode_32_gates;
        break;

    case SYSTEM_MODE_FIFO_POLY_HARMONIC:
        my_mode_clbks = &mode_fifo_poly_harmonic_gates;
        break;

    case SYSTEM_MODE_TEST_MODE:
        my_mode_clbks = &mode_test;
        break;

    case SYSTEM_MODE_CONFIG:
        my_mode_clbks = &mode_config;
        break;

    case SYSTEM_MODE_FIFO_POLY:
    default:
        my_mode_clbks = &mode_fifo_polyphonic;
        break;
    }
    my_current_mode = new_mode;

    call_mode_clbk_if_valid(setup);
    setup_midi();
}

enum
{
    SWITCH_UNKNOWN,
    SWITCH_DOWN,
    SWITCH_HELD,
    SWITCH_UP,
    SWITCH_HOLD_RELEASE,
};

typedef struct
{
    uint8_t state;
    unsigned long press_start;
} switch_state_type;

static switch_state_type switch_state = {SWITCH_UNKNOWN, millis()};

void update_switch_state()
{
    if (digitalRead(SWITCH_1_PIN))
    {
        switch (switch_state.state)
        {
        case SWITCH_DOWN:
        case SWITCH_HELD:
            switch_state.state = SWITCH_HELD;
            break;
        case SWITCH_HOLD_RELEASE:
        case SWITCH_UP:
        case SWITCH_UNKNOWN:
        default:
            switch_state.state = SWITCH_DOWN;
            switch_state.press_start = millis();
            break;
        }
    }
    else
    {
        switch (switch_state.state)
        {
        case SWITCH_DOWN:
            switch_state.state = SWITCH_UP;
            break;
        case SWITCH_HELD:
            if (time_check_and_update(&switch_state.press_start, PRESS_AND_HOLD_TIME))
            {
                switch_state.state = SWITCH_HOLD_RELEASE;
            }
            else
            {
                switch_state.state = SWITCH_UP;
            }
            break;
        case SWITCH_UP:
        case SWITCH_HOLD_RELEASE:
        case SWITCH_UNKNOWN:
        default:
            switch_state.state = SWITCH_UNKNOWN;
            break;
        }
    }
}

void handle_switch_normal_modes()
{
    switch (switch_state.state)
    {
    case SWITCH_DOWN:
        if (my_current_mode != SYSTEM_MODE_CONFIG)
        {
            call_mode_clbk_if_valid(all_notes_off);
        }
        break;
    case SWITCH_HOLD_RELEASE:
        switch_state.state = SWITCH_UNKNOWN;
        if (my_current_mode != SYSTEM_MODE_CONFIG)
        {
            mode_change(SYSTEM_MODE_CONFIG);
        }
        else
        {
            mode_change(get_system_mode()); //Back to saved mode.
        }
        break;
    case SWITCH_UP:
        if (my_current_mode == SYSTEM_MODE_CONFIG)
        {
            uint8_t new_midi_channel = get_midi_channel() + 1;
            new_midi_channel = new_midi_channel > 16 ? 1 : new_midi_channel;
            updateMIDIChannel(new_midi_channel);
            MIDI.setInputChannel(get_midi_channel());
        }
        break;
    case SWITCH_HELD:
    case SWITCH_UNKNOWN:
    default:
        break;
    }
}