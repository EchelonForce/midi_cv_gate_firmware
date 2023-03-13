#ifndef _MODES_H
#define _MODES_H

#include <Arduino.h>

typedef enum
{
    SYSTEM_MODE_FIRST_PRIO_POLY,
    SYSTEM_MODE_32_GATES,
    SYSTEM_MODE_FIRST_PRIO_POLY_QUAD_HARMONIC,
    SYSTEM_MODE_TEST_MODE,
    SYSTEM_MODE_CNT,
    SYSTEM_MODE_CONFIG, // After SYSTEM_MODE_CNT so it isn't saved.
    SYSTEM_MODE_DEFAULT = SYSTEM_MODE_FIRST_PRIO_POLY
};
typedef uint8_t mode_type;

typedef struct
{
    void (*setup)(void);
    void (*teardown)(void);
    void (*loop)(void);
    void (*handle_note_on)(byte inChannel, byte inNote, byte inVelocity);
    void (*handle_note_off)(byte inChannel, byte inNote, byte inVelocity);
    void (*all_notes_off)(void);
} mode_callbacks_type;

extern mode_callbacks_type mode_first_prio_polyphonic;
void mode_first_prio_poly_setup(void);
void mode_first_prio_poly_teardown(void);
void mode_first_prio_poly_loop(void);
void mode_first_prio_poly_handle_note_on(byte inChannel, byte inNote, byte inVelocity);
void mode_first_prio_poly_handle_note_off(byte inChannel, byte inNote, byte inVelocity);
void mode_first_prio_poly_all_notes_off(void);

extern mode_callbacks_type mode_32_gates;
void mode_32_gates_setup();
void mode_32_gates_teardown();
void mode_32_gates_loop();
void mode_32_gates_all_notes_off();
void mode_32_gates_handle_note_on(byte inChannel, byte inNote, byte inVelocity);
void mode_32_gates_handle_note_off(byte inChannel, byte inNote, byte inVelocity);

extern mode_callbacks_type mode_first_prio_poly_quad_harmonic_gates;
void mode_first_prio_poly_quad_harmonic_setup(void);
void mode_first_prio_poly_quad_harmonic_teardown(void);
void mode_first_prio_poly_quad_harmonic_loop(void);
void mode_first_prio_poly_quad_harmonic_handle_note_on(byte inChannel, byte inNote, byte inVelocity);
void mode_first_prio_poly_quad_harmonic_handle_note_off(byte inChannel, byte inNote, byte inVelocity);
void mode_first_prio_poly_quad_harmonic_all_notes_off(void);

extern mode_callbacks_type mode_test;
void mode_test_setup(void);
void mode_test_teardown(void);
void mode_test_loop(void);
void mode_test_handle_note_on(byte inChannel, byte inNote, byte inVelocity);
void mode_test_handle_note_off(byte inChannel, byte inNote, byte inVelocity);
void mode_test_all_notes_off(void);

extern mode_callbacks_type mode_config;
void mode_config_setup(void);
void mode_config_teardown(void);
void mode_config_loop(void);
void mode_config_handle_note_on(byte inChannel, byte inNote, byte inVelocity);
void mode_config_handle_note_off(byte inChannel, byte inNote, byte inVelocity);
void mode_config_all_notes_off(void);

#endif /* _MODES_H */