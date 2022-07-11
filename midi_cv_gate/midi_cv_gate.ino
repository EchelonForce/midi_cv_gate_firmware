#include "shared.h"
#include <MIDI.h>
#include <serialMIDI.h>
#include <EEPROM.h>

#define NORMAL_MODE 0
#define TEST_CV_GATE_MODE 1
#define TEST_MIDI_MODE 2

#define MODE NORMAL_MODE
#define MIDI_CHANNEL_DEFAULT 11
#define EEPROM_MAGIC_NUMBER 0x1234ABCD
#define EEPROM_VERSION_1 1

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

    setupEEPROM();
    setupMIDI();

    delay(1000);
}

void setupMIDI()
{
    MIDI.setHandleNoteOn(handleNoteOn);
    MIDI.setHandleNoteOff(handleNoteOff);
    MIDI.begin(getMIDIChannel()); // Listen to all incoming messages
}

typedef struct
{
    uint32_t eeprom_magic_value; //offset 0
    uint8_t eeprom_data_version; //offset 4
    uint8_t midi_channel;        //offset 5
} eeprom_data_type;

static eeprom_data_type eeprom_data = {0, 0, 0};
static const eeprom_data_type eeprom_data_dflts = {EEPROM_MAGIC_NUMBER, EEPROM_VERSION_1, MIDI_CHANNEL_DEFAULT};

void setupEEPROM()
{
    for (uint8_t i = 0; i < sizeof(eeprom_data_type); i++)
    {
        ((uint8_t *)(&eeprom_data))[i] = EEPROM.read(i);
    }
    if (eeprom_data.eeprom_magic_value != EEPROM_MAGIC_NUMBER)
    {
        memcpy(&eeprom_data, &eeprom_data_dflts, sizeof(eeprom_data_type));
        updateEEPROM();
    }
}

void updateEEPROM()
{
    for (uint8_t i = 0; i < sizeof(eeprom_data_type); i++)
    {
        EEPROM.update(i, ((uint8_t *)(&eeprom_data))[i]);
    }
}

void updateMIDIChannel(uint8_t midi_channel)
{
    if (midi_channel < 17 && midi_channel > 0)
    {
        eeprom_data.midi_channel = midi_channel;
    }
    else
    {
        eeprom_data.midi_channel = 1;
    }
    updateEEPROM();
    MIDI.setInputChannel(midi_channel);
}

uint8_t getMIDIChannel()
{
    return eeprom_data.midi_channel;
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
            else if (note_fifo[i].note_value == 255)
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
        updateSwitchState();
        handleSwitchInNormalMode();
    }
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

void updateSwitchState()
{
    if (digitalRead(SWITCH_1_PIN))
    {
        switch (switch_state.state)
        {
        case SWITCH_DOWN:
        case SWITCH_HELD:
        case SWITCH_HOLD_RELEASE:
            switch_state.state = SWITCH_HELD;
            break;
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
            switch_state.state = SWITCH_HOLD_RELEASE;
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

void handleSwitchInNormalMode()
{
    switch (switch_state.state)
    {
    case SWITCH_DOWN:
        allNotesOff();
        break;
    case SWITCH_HELD:
        if (time_check_and_update(&switch_state.press_start, 5000))
        {
            switch_state.state = SWITCH_UNKNOWN;
            configure_mode_loop();
        }
        break;
    case SWITCH_UP:
    case SWITCH_UNKNOWN:
    case SWITCH_HOLD_RELEASE:
    default:
        break;
    }
}

void configure_mode_loop()
{
    static unsigned long configure_mode_last_blink_time = millis();
    static unsigned long configure_mode_switch_check_time = millis();

    boolean exit_configure = false;

    while (!exit_configure)
    {
        if (time_check_and_update(&configure_mode_last_blink_time, 250))
        {
            if (getGateState(getMIDIChannel() - 1))
            {
                updateGateState(getMIDIChannel() - 1, 0);
            }
            else
            {
                updateGateState(getMIDIChannel() - 1, 1);
            }
            updateGates();
        }
        if (time_check_and_update(&configure_mode_switch_check_time, 200))
        {
            updateSwitchState();
            exit_configure = handleSwitchInConfigMode();
        }
    }

    updateGateState(getMIDIChannel() - 1, 0);
    updateGates();
}

boolean handleSwitchInConfigMode()
{
    switch (switch_state.state)
    {
    case SWITCH_DOWN:
        break;
    case SWITCH_HELD:
        if (time_check_and_update(&switch_state.press_start, 5000))
        {
            return true;
        }
        break;
    case SWITCH_UP:
        updateGateState(getMIDIChannel() - 1, 0); // clear blinking
        uint8_t new_midi_channel = getMIDIChannel() + 1;
        new_midi_channel = new_midi_channel > 16 ? 1 : new_midi_channel;
        updateMIDIChannel(new_midi_channel);
        break;
    case SWITCH_UNKNOWN:
    case SWITCH_HOLD_RELEASE:
    default:
        break;
    }
    return false;
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
        if (note_fifo[i].note_value == inNote && note_fifo[i].on)
        {
            break;
        }
    }
    if (i == 16)
    {
        for (i = 0; i < 16; i++)
        {
            if (!note_fifo[i].on)
            {
                break;
            }
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
        //note_fifo[oldest_note_idx].note_value = 255; Don't change the note value, this would break release in the ADSR.
        note_fifo[oldest_note_idx].on = false;
        note_fifo[oldest_note_idx].time_on = now;
        note_fifo_update = true;
    }
}
#endif //NORMAL_MODE
