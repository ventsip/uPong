#pragma once
#include "hardware/gpio.h"

namespace rotary_encoder
{

    typedef struct
    {
        uint8_t a;
        uint8_t b;
        uint8_t sw;

        volatile uint8_t a_b_trail;
        volatile int32_t counter;
        volatile uint8_t sw_trail;
        volatile uint8_t sw_state;
    } rotary_encoder;

    enum
    {
        ROTARY_ENCODER_SW_RELEASED = 0,
        ROTARY_ENCODER_SW_PRESSED,
    };

#define NUM_ROTARY_ENCODERS 2
    extern rotary_encoder rotary_encoders[NUM_ROTARY_ENCODERS];

    int32_t rotary_encoder_fetch_counter(rotary_encoder *re);

    uint8_t rotary_encoder_fetch_sw_state(rotary_encoder *re);

    void rotary_encoders_init(void);
}