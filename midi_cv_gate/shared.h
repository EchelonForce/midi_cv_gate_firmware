#ifndef _MIDI_CV_GATE_SHARED_H
#define _MIDI_CV_GATE_SHARED_H

#include <Arduino.h>

#define GATE_SER_IN_PIN (A0)
#define GATE_STORE_REG_CLK_PIN (A1)
#define GATE_SHIFT_REG_CLK_PIN (A2)
#define GATE_OUT_ENABLE_PIN (A3) //Active Low
#define GATE_CLEAR_PIN (A4)      //Active Low
#define CV_DAC_SHUTDOWN_PIN (A5) //Active Low

#define CV_DAC_LOAD_DAC_PIN (2)       //Active Low
#define CV_DAC_CHIP_SELECT_H_PIN (3)  //Active Low
#define CV_DAC_CHIP_SELECT_G_PIN (4)  //Active Low
#define CV_DAC_CHIP_SELECT_F_PIN (5)  //Active Low
#define CV_DAC_CHIP_SELECT_E_PIN (6)  //Active Low
#define CV_DAC_CHIP_SELECT_D_PIN (7)  //Active Low
#define CV_DAC_CHIP_SELECT_C_PIN (8)  //Active Low
#define CV_DAC_CHIP_SELECT_B_PIN (9)  //Active Low
#define CV_DAC_CHIP_SELECT_A_PIN (10) //Active Low
#define CV_DAC_SPI_MOSI_PIN (11)
#define SWITCH_1_PIN (12)
#define CV_DAC_SPI_SCK_PIN (13)

#define GATE_CV_CNT (16)
#define MIDI_NOTE_CNT (128)

void setup_test_cv_gate();
void setup_test_midi();
void setup_normal_mode();
void loop_test_cv_gate();
void loop_test_midi();
void loop_normal_mode();

void setupDACs();

void setupGates();

uint8_t time_check_and_update(unsigned long *prev_update, unsigned long delta);

void updateCVandOutput(uint8_t cv_num, uint16_t value);

void updateCV(uint8_t cv_num, uint16_t value);

void updateCVOutput();

void updateDAC(uint8_t chip_select_pin, uint8_t channel, uint16_t value);

void update_gate_state(uint8_t gate_num, uint8_t state);

uint8_t getGateState(uint8_t gate_num);

uint8_t update_gates();

#endif /* _MIDI_CV_GATE_SHARED_H */