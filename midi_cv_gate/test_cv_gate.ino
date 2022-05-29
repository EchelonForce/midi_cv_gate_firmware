// Test Code for MCP4922 12bit Two Channel DAC (SPI)
#if (MODE == TEST_CV_GATE_MODE)

#include "shared.h"
#include <SPI.h>

void setup_test_cv_gate()
{
    //VC DAC IO Setup MCP4922
    setupDACs();

    //Gate Serial to Parallel IO setup 74HC595
    setupGates();

    pinMode(SWITCH_1_PIN, INPUT);

    Serial.begin(115200);
    delay(100);
}

static uint16_t test_value_out = 0;
static unsigned long test_last_gate_update = millis();
static unsigned long test_last_cv_update = millis();
static unsigned long test_last_switch_update = millis();
static unsigned long test_last_cv_num = 0;
static uint16_t test_gate_pattern = 1;
void loop_test_cv_gate()
{

    if (time_check_and_update(&test_last_cv_update, 4000))
    {
        updateCVandOutput(test_last_cv_num, 0);
        for (uint8_t i = 0; i < 16; i++)
        {
            updateCVandOutput(i, test_value_out);
        }
        //Serial.println(value_out);
        test_value_out += 0x111;
        if (test_value_out > 0x0FFF)
        {
            test_value_out = 0;
        }
        Serial.print("test_value_out=");
        Serial.println(test_value_out, 16);
    }

    if (time_check_and_update(&test_last_gate_update, 100))
    {
        for (uint8_t i = 0; i < 16; i++)
        {
            updateGateState(i, (test_gate_pattern >> i) & 0x0001);
        }
        updateGates();
        test_gate_pattern = test_gate_pattern << 1;
        if (test_gate_pattern == 0)
        {
            test_gate_pattern = 1;
        }
    }

    if (time_check_and_update(&test_last_switch_update, 100))
    {
        if (digitalRead(SWITCH_1_PIN))
        {
            Serial.println("Switch ON");
            test_value_out = 0;
        }
        else
        {
            Serial.println("Switch OFF");
        }
    }
}
#endif // TEST_CV_GATE_MODE