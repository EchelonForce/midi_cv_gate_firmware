#include "eeprom_settings.h"
#include <EEPROM.h>

static eeprom_data_type my_eeprom_data = {0, 0, 0, 0};
static const eeprom_data_type eeprom_data_dflts = {EEPROM_MAGIC_NUMBER, EEPROM_VERSION, MIDI_CHANNEL_DEFAULT, SYSTEM_MODE_DEFAULT};

void setupEEPROM()
{
    for (uint8_t i = 0; i < sizeof(eeprom_data_type); i++)
    {
        ((uint8_t *)(&my_eeprom_data))[i] = EEPROM.read(i);
    }
    if (my_eeprom_data.eeprom_magic_value != EEPROM_MAGIC_NUMBER)
    {
        memcpy(&my_eeprom_data, &eeprom_data_dflts, sizeof(eeprom_data_type));
        updateEEPROM();
    }
    else
    {
        //Upgrade eeprom to v2 which has system_mode
        if (my_eeprom_data.eeprom_data_version < 2)
        {
            my_eeprom_data.system_mode = SYSTEM_MODE_DEFAULT;
            my_eeprom_data.eeprom_data_version = 2;
            updateEEPROM();
        }
    }
}

void updateEEPROM()
{
    for (uint8_t i = 0; i < sizeof(eeprom_data_type); i++)
    {
        EEPROM.update(i, ((uint8_t *)(&my_eeprom_data))[i]);
    }
}

void updateMIDIChannel(uint8_t midi_channel)
{
    if (midi_channel < 17 && midi_channel > 0)
    {
        my_eeprom_data.midi_channel = midi_channel;
    }
    else
    {
        my_eeprom_data.midi_channel = 1;
    }
    updateEEPROM();
}

uint8_t get_midi_channel()
{
    return my_eeprom_data.midi_channel;
}

void update_system_mode(uint8_t mode)
{
    if (mode < SYSTEM_MODE_CNT)
    {
        my_eeprom_data.system_mode = mode;
    }
    updateEEPROM();
}

uint8_t get_system_mode(void)
{
    return my_eeprom_data.system_mode;
}