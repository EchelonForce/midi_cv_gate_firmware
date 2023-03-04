#include "shared.h"
#include "dac_cal_vals.h"
#include <SPI.h>

static uint16_t gate_states = 0;

//See dac_cal_vals.h
const PROGMEM int8_t dac_calibration[GATE_CV_CNT][MIDI_NOTE_CNT] = DAC_CAL_VALS;

/**
 * Setup IO pins for the 8 MCP4922 DACs.
 */
void setupDACs()
{
    //VC DAC IO Setup MCP4922
    pinMode(CV_DAC_CHIP_SELECT_A_PIN, OUTPUT);
    digitalWrite(CV_DAC_CHIP_SELECT_A_PIN, HIGH);
    pinMode(CV_DAC_CHIP_SELECT_B_PIN, OUTPUT);
    digitalWrite(CV_DAC_CHIP_SELECT_B_PIN, HIGH);
    pinMode(CV_DAC_CHIP_SELECT_C_PIN, OUTPUT);
    digitalWrite(CV_DAC_CHIP_SELECT_C_PIN, HIGH);
    pinMode(CV_DAC_CHIP_SELECT_D_PIN, OUTPUT);
    digitalWrite(CV_DAC_CHIP_SELECT_D_PIN, HIGH);
    pinMode(CV_DAC_CHIP_SELECT_E_PIN, OUTPUT);
    digitalWrite(CV_DAC_CHIP_SELECT_E_PIN, HIGH);
    pinMode(CV_DAC_CHIP_SELECT_F_PIN, OUTPUT);
    digitalWrite(CV_DAC_CHIP_SELECT_F_PIN, HIGH);
    pinMode(CV_DAC_CHIP_SELECT_G_PIN, OUTPUT);
    digitalWrite(CV_DAC_CHIP_SELECT_G_PIN, HIGH);
    pinMode(CV_DAC_CHIP_SELECT_H_PIN, OUTPUT);
    digitalWrite(CV_DAC_CHIP_SELECT_H_PIN, HIGH);
    pinMode(CV_DAC_LOAD_DAC_PIN, OUTPUT);
    digitalWrite(CV_DAC_LOAD_DAC_PIN, HIGH);
    pinMode(CV_DAC_SHUTDOWN_PIN, OUTPUT);
    digitalWrite(CV_DAC_SHUTDOWN_PIN, HIGH);
    SPI.begin();

    //CLEAR CVs
    for (uint8_t i = 0; i < 16; i++)
    {
        updateCV(i, 0);
    }
    updateCVOutput();
}
/**
 * Setup IO pins for the daisy chained serial to parallel 74HC595s
 * that produce the Gate outputs.
 */
void setupGates()
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
}

/**
 * Check the passed in time (in milliseconds) agains current time and if delta
 * ms has passed return true and set passed in time to current time.
 */
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

/**
 * Update a CV and enable the output immidiately.
 */
void updateCVandOutput(uint8_t cv_num, uint16_t value)
{
    updateCV(cv_num, value);
    updateCVOutput();
}

/**
 * Update a CV but do not update the output immidiately.
 */
void updateCV(uint8_t cv_num, uint16_t value)
{
    static const uint8_t dac_chip_selects[] = {
        CV_DAC_CHIP_SELECT_A_PIN,
        CV_DAC_CHIP_SELECT_B_PIN,
        CV_DAC_CHIP_SELECT_C_PIN,
        CV_DAC_CHIP_SELECT_D_PIN,
        CV_DAC_CHIP_SELECT_E_PIN,
        CV_DAC_CHIP_SELECT_F_PIN,
        CV_DAC_CHIP_SELECT_G_PIN,
        CV_DAC_CHIP_SELECT_H_PIN};
    updateDAC(dac_chip_selects[cv_num >> 1], cv_num & 0x01, value);
}

/**
 * Enable the CV outputs stored by previous updateCV call(s).
 */
void updateCVOutput()
{
    //MCP4922 LDAC toggle to load value to output
    digitalWrite(CV_DAC_LOAD_DAC_PIN, LOW);
    digitalWrite(CV_DAC_LOAD_DAC_PIN, HIGH);
}

/**
 * Send SPI data to the specified DAC.
 */
void updateDAC(uint8_t chip_select_pin, uint8_t channel, uint16_t value)
{
    uint16_t out = value & 0x0FFF;
    out |= (channel << 15);
    out |= (1 << 13); //gain=1
    out |= (1 << 12); //SHDN=1
    digitalWrite(chip_select_pin, LOW);
    SPI.transfer16(out);
    digitalWrite(chip_select_pin, HIGH);
}

/**
 * Update internal gate_state record. Doesn't update the outputs.
 */
void update_gate_state(uint8_t gate_num, uint8_t state)
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

/**
 * Returns the current internal gate_state record.
 */
uint8_t getGateState(uint8_t gate_num)
{
    return (gate_states >> gate_num) & 0x0001;
}

/**
 * Update all gates from the gate_states record and enable the output immidiately.
 */
uint8_t update_gates()
{
    uint16_t states = gate_states;
    //Two daisychained 74HC595 shift registers. Bit bang out all the state bits.

    uint8_t oldSREG = SREG;
    cli();
    PORTC &= 0xFB; //digitalWrite(GATE_SHIFT_REG_CLK_PIN, LOW); //PC2
    for (uint8_t i = 0; i < 16; i++)
    {
        //digitalWrite(GATE_SER_IN_PIN, (states & 0x8000) == 0x8000); //PC0
        if (states & 0x8000)
        {
            PORTC |= 0x01;
        }
        else
        {
            PORTC &= 0xFE;
        }

        PORTC |= 0x04; //digitalWrite(GATE_SHIFT_REG_CLK_PIN, HIGH);
        states = states << 1;
        PORTC &= 0xFB; //digitalWrite(GATE_SHIFT_REG_CLK_PIN, LOW);
    }
    //Move shift register val to store register
    PORTC |= 0x02; //digitalWrite(GATE_STORE_REG_CLK_PIN, HIGH); //PC1
    PORTC &= 0xFD; //digitalWrite(GATE_STORE_REG_CLK_PIN, LOW);
    //Make sure output is enabled.

    PORTC &= 0xF7; //digitalWrite(GATE_OUT_ENABLE_PIN, LOW); //PC3
    SREG = oldSREG;
}

uint16_t noteValueToCV(uint8_t cv, uint32_t note_val)
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
    // 409=1V 409/12=34.08, 3380 used because it works well empirically.
    //v1 stuff
    // static const uint32_t cal_factor = 3380; //DAC uints per semitone * 100.
    // uint16_t a = min(0xFFF, ((note_val * cal_factor) * 101) / 100);

    //1000=2.477 while plugged into vco.
    //3000=7.42 (missing digit makes this shakey, tweaking needed.)
    // 2000 dac units = 4.943v = 404.6 dac/v = 33.72 / semitone = 3372
    static const uint32_t cal_factor = CAL_FACTOR; //DAC uints per semitone * 100.
    int8_t cal_offset = pgm_read_byte_near(&dac_calibration[cv][min(note_val, 127)]);
    uint16_t a = min(0xFFF, (((note_val * cal_factor)) / 100) + (int32_t)cal_offset);
    return a;
}
