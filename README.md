# Eurorack MIDI to 16x16 CV Gate Module Firmware

This the firmware for a Eurorack module with 16 CVs and 16 Gate outputs. It's meant to run on an Arduino Pro Mini.

Full writeup here: https://www.robotdialogs.com/2022/09/eurorack-midi-to-16x16-cv-gate-module.html

# More Info
Structure is weird because of Arduino IDE. All the .ino files are concatenated and built as one file, so the various modes have unique names to keep them from interfering with each other.

midi_cv_gate.ino is the main 'sketch' to build.

# Modes
Modes can be altered by sending a midi control change message (38) with the data byte equal to the desired mode. The selected mode is persistent (saved to EEPROM).

## Mode 0: SYSTEM_MODE_FIFO_POLY
The original 16 voice polyphonic mode is preserved as the first mode. First note priority.

## Mode 1: SYSTEM_MODE_32_GATES
A mode for using all 32 outputs as if they were gates was added. This mode is meant for something like drum triggers. It responds to note on/off 0-31 and each note maps to gate or cv.

## Mode 2: SYSTEM_MODE_FIFO_POLY_HARMONIC
The polyphonic harmonic mode is a work in progress mode that uses 4 CVs to play a note and it's first three harmonics. It currently doesn't account for the calibration correctly in the second harmonic.

## Mode 3: SYSTEM_MODE_TEST_MODE
Test functionality was converted to a mode as well. It displays a pattern on the gates and ramps the CV outputs through the full range.

## Mode (5 may change): SYSTEM_MODE_CONFIG
Enter with long press of switch.
Config mode is for changing the midi channel. It was previously separate from the other modes because of some special switch handling behaviors. That's been reworked so that the config mode can be entered like other modes accept that when config mode is entered, the saved mode in EEPROM isn't changed. The saved mode is restored when config mode is exited. Handling of the switch has been improved a bit and now only acts when the switch is released (accept for the panic function).

# Calibration
A per note calibration has been added that corrects the CV at each note for each CV output. The calibration requires additional scripting in another repository: midi_cv_gate_test_fixture_firmware

This work is licensed under a Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International License. https://creativecommons.org/licenses/by-nc-sa/4.0/