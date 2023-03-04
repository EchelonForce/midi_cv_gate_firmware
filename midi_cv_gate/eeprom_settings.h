#ifndef _EEPROM_SETTINGS_H
#define _EEPROM_SETTINGS_H

#include <Arduino.h>

#define MIDI_CHANNEL_DEFAULT 11
#define EEPROM_MAGIC_NUMBER 0x1234ABCD
#define EEPROM_VERSION 2

typedef struct
{
    uint32_t eeprom_magic_value; //offset 0
    uint8_t eeprom_data_version; //offset 4
    uint8_t midi_channel;        //offset 5
    uint8_t system_mode;         //offset 6
} eeprom_data_type;

void setupEEPROM();

void updateEEPROM();

void updateMIDIChannel(uint8_t midi_channel);

uint8_t get_midi_channel();

void update_system_mode(uint8_t mode);

uint8_t get_system_mode(void);

#endif /* _EEPROM_SETTINGS_H */