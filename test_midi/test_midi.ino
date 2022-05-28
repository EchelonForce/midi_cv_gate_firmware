#define MIDI_OR_SERIAL (1) // 1 = MIDI, 0 = Serial
#define MIDI_CHANNEL (11)

#if MIDI_OR_SERIAL
#include <MIDI.h>
#endif

// Create and bind the MIDI interface to the default hardware Serial port
#if MIDI_OR_SERIAL
MIDI_CREATE_DEFAULT_INSTANCE();
#endif

const uint8_t GATE_SER_IN_PIN = A0;
const uint8_t GATE_STORE_REG_CLK_PIN = A1;
const uint8_t GATE_SHIFT_REG_CLK_PIN = A2;
const uint8_t GATE_OUT_ENABLE_PIN = A3; //Active Low
const uint8_t GATE_CLEAR_PIN = A4;      //Active Low

void setup()
{
    //Gate Serial to Parallel IO setup 74HC595
    pinMode(GATE_SER_IN_PIN, OUTPUT);
    digitalWrite(GATE_SER_IN_PIN, LOW);
    pinMode(GATE_STORE_REG_CLK_PIN, OUTPUT);
    digitalWrite(GATE_STORE_REG_CLK_PIN, LOW);
    pinMode(GATE_SHIFT_REG_CLK_PIN, OUTPUT);
    digitalWrite(GATE_SHIFT_REG_CLK_PIN, LOW);
    pinMode(GATE_OUT_ENABLE_PIN, OUTPUT);
    digitalWrite(GATE_OUT_ENABLE_PIN, HIGH);
    pinMode(GATE_CLEAR_PIN, OUTPUT);
    digitalWrite(GATE_CLEAR_PIN, HIGH);

    //CLEAR GATES
    digitalWrite(GATE_CLEAR_PIN, LOW);
    digitalWrite(GATE_STORE_REG_CLK_PIN, HIGH);
    digitalWrite(GATE_STORE_REG_CLK_PIN, LOW);
    digitalWrite(GATE_CLEAR_PIN, HIGH);

#if MIDI_OR_SERIAL

    MIDI.setHandleNoteOn(handleNoteOn);
    MIDI.setHandleNoteOff(handleNoteOff);
    MIDI.begin(MIDI_CHANNEL); // Listen to all incoming messages
#else
    Serial.begin(115200);
#endif
    delay(1000);
}

static uint16_t test_pattern = 0;
static uint16_t last_test_pattern = 0;
static uint16_t gate_states = 0;
static unsigned long last_gate_update = millis();
static unsigned long last_note_out_update = millis();
static uint8_t toggle_on_off = 0;
void loop()
{
    if (time_check_and_update(&last_note_out_update, 500))
    {
        if (toggle_on_off)
        {
            MIDI.sendNoteOn(12, 127, MIDI_CHANNEL);
        }
        else
        {
            MIDI.sendNoteOff(12, 127, MIDI_CHANNEL);
        }
        toggle_on_off = !toggle_on_off;
    }
    // Read incoming messages
    MIDI.read();

    //if (time_check_and_update(&last_gate_update, 100))
    if (last_test_pattern != test_pattern)
    {
        for (uint8_t i = 0; i < 16; i++)
        {
            updateGateState(i, (test_pattern >> i) & 0x0001);
        }
        updateGates();
        last_test_pattern = test_pattern;
    }
}
void handleNoteOn(byte inChannel, byte inNote, byte inVelocity)
{
    test_pattern = (uint16_t)inNote | ((uint16_t)inVelocity << 8);
}

void handleNoteOff(byte inChannel, byte inNote, byte inVelocity)
{
    test_pattern = 0;
}

uint8_t time_check_and_update(unsigned long *prev_update, unsigned long delta)
{
    unsigned long now = millis();
    uint8_t ret = (now - *prev_update) > delta;
    if (ret)
    {
        *prev_update = now;
    }
    return ret;
}

void updateGateState(uint8_t gate_num, uint8_t state)
{
    if (state == HIGH)
    {
        gate_states |= 1 << gate_num;
    }
    else
    {
        gate_states &= ~(1 << gate_num);
    }
}

uint8_t getGateState(uint8_t gate_num)
{
    return (gate_states >> gate_num) & 0x0001;
}

uint8_t updateGates()
{
    uint16_t states = gate_states;
    //Two daisychained 74HC595 shift registers. Bit bang out all the state bits.
    digitalWrite(GATE_SHIFT_REG_CLK_PIN, LOW);
    for (uint8_t i = 0; i < 16; i++)
    {
        digitalWrite(GATE_SER_IN_PIN, states & 0x0001);
        digitalWrite(GATE_SHIFT_REG_CLK_PIN, HIGH);
        states = states >> 1;
        digitalWrite(GATE_SHIFT_REG_CLK_PIN, LOW);
    }
    //Move shift register val to tore register
    digitalWrite(GATE_STORE_REG_CLK_PIN, HIGH);
    digitalWrite(GATE_STORE_REG_CLK_PIN, LOW);
    //Make sure output is enabled.
    digitalWrite(GATE_OUT_ENABLE_PIN, LOW);
}