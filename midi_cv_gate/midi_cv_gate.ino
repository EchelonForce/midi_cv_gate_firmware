#include "shared.h"

#define NORMAL_MODE 0
#define TEST_CV_GATE_MODE 1
#define TEST_MIDI_MODE 2

#define MODE TEST_CV_GATE_MODE

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

void setup_normal_mode()
{
}

void loop_normal_mode()
{
}

#endif //NORMAL_MODE
