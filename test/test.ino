// Test Code for MCP4922 12bit Two Channel DAC (SPI)
#include <SPI.h>

const uint8_t GATE_SER_IN_PIN = A0;
const uint8_t GATE_STORE_REG_CLK_PIN = A1;
const uint8_t GATE_SHIFT_REG_CLK_PIN = A2;
const uint8_t GATE_OUT_ENABLE_PIN = A3; //Active Low
const uint8_t GATE_CLEAR_PIN = A4;      //Active Low
const uint8_t CV_DAC_SHUTDOWN_PIN = A5; //Active Low
const uint8_t LED_1_PIN = A6;
const uint8_t LED_2_PIN = A7;

const uint8_t CV_DAC_LOAD_DAC_PIN = 2;       //Active Low
const uint8_t CV_DAC_CHIP_SELECT_H_PIN = 3;  //Active Low
const uint8_t CV_DAC_CHIP_SELECT_G_PIN = 4;  //Active Low
const uint8_t CV_DAC_CHIP_SELECT_F_PIN = 5;  //Active Low
const uint8_t CV_DAC_CHIP_SELECT_E_PIN = 6;  //Active Low
const uint8_t CV_DAC_CHIP_SELECT_D_PIN = 7;  //Active Low
const uint8_t CV_DAC_CHIP_SELECT_C_PIN = 8;  //Active Low
const uint8_t CV_DAC_CHIP_SELECT_B_PIN = 9;  //Active Low
const uint8_t CV_DAC_CHIP_SELECT_A_PIN = 10; //Active Low
const uint8_t CV_DAC_SPI_MOSI_PIN = 11;
const uint8_t SWITCH_1_PIN = 12;
const uint8_t CV_DAC_SPI_SCK_PIN = 13;

void setup()
{
    //VC DAC IO Setup
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

    //Gate Serial to Parallel IO setup
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

    pinMode(LED_1_PIN, OUTPUT);
    digitalWrite(LED_1_PIN, 0);
    pinMode(LED_2_PIN, OUTPUT);
    digitalWrite(LED_2_PIN, 0);
    pinMode(SWITCH_1_PIN, INPUT);

    Serial.begin(9600);
    delay(100);
}

static uint16_t value_out = 0;
static uint16_t gate_states = 0;
static unsigned long last_gate_update = millis();
static unsigned long last_cv_update = millis();
static unsigned long last_cv_num = 0;
static uint16_t test_pattern = 1;
void loop()
{

    if (time_check_and_update(&last_cv_update, 1000))
    {
        updateCV(last_cv_num, 0);
        value_out = 0;
        last_cv_num++;
        if (last_cv_num > 15)
        {
            last_cv_num = 0;
        }
    }
    updateCV(last_cv_num, value_out);
    //Serial.println(value_out);
    value_out += 1;
    if (value_out > 0x0FFF)
    {
        value_out = 0;
    }

    if (time_check_and_update(&last_gate_update, 100))
    {
        for (uint8_t i = 0; i < 16; i++)
        {
            updateGateState(i, (test_pattern >> i) & 0x0001);
        }
        updateGates();
        test_pattern = test_pattern << 1;
        if (test_pattern == 0)
        {
            test_pattern = 1;
        }
    }
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
    digitalWrite(CV_DAC_LOAD_DAC_PIN, LOW);
    digitalWrite(CV_DAC_LOAD_DAC_PIN, HIGH);
}

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
    digitalWrite(GATE_SHIFT_REG_CLK_PIN, LOW);
    for (uint8_t i = 0; i < 16; i++)
    {
        digitalWrite(GATE_SER_IN_PIN, states & 0x0001);
        digitalWrite(GATE_SHIFT_REG_CLK_PIN, HIGH);
        states = states >> 1;
        digitalWrite(GATE_SHIFT_REG_CLK_PIN, LOW);
    }
    digitalWrite(GATE_STORE_REG_CLK_PIN, HIGH);
    digitalWrite(GATE_STORE_REG_CLK_PIN, LOW);
    digitalWrite(GATE_OUT_ENABLE_PIN, LOW);
}