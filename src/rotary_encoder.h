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
    rotary_encoder rotary_encoders[] = {
        {22, 26, 27, 0, 0, 0, ROTARY_ENCODER_SW_RELEASED},
        {19, 20, 21, 0, 0, 0, ROTARY_ENCODER_SW_RELEASED},
    };

    int32_t rotary_encoder_fetch_counter(rotary_encoder *re)
    {
        const int32_t counter = re->counter;
        re->counter = 0;
        return counter;
    }

    uint8_t rotary_encoder_fetch_sw_state(rotary_encoder *re)
    {
        return re->sw_state;
    }

    void static rotary_encoder_callback(uint gpio, __unused uint32_t events, rotary_encoder *re)
    {
        if (gpio == re->a || gpio == re->b)
        {
            re->a_b_trail = ((re->a_b_trail << 2) | (gpio_get(re->a) << 1) | gpio_get(re->b)) & 0xf;

            switch (re->a_b_trail)
            {
            case 0b0001:
            case 0b0111:
            case 0b1110:
            case 0b1000:
                // clockwise
                (re->counter)++;
                break;
            case 0b0010:
            case 0b1011:
            case 0b1101:
            case 0b0100:
                // counter clockwise
                (re->counter)--;
                break;
            }
        }

        if (gpio == re->sw)
        {
            re->sw_trail = ((re->sw_trail << 1) | gpio_get(re->sw)) & 0b11;
            switch (re->sw_trail)
            {
            case 0b10:
                re->sw_state = ROTARY_ENCODER_SW_PRESSED;
                break;
            case 0b01:
                re->sw_state = ROTARY_ENCODER_SW_RELEASED;
                break;
            }
        }
    }

    // isr for the rotary encoder
    void rotary_encoders_callback(uint gpio, __unused uint32_t events)
    {
        for (int i = 0; i < NUM_ROTARY_ENCODERS; i++)
        {
            rotary_encoder_callback(gpio, events, &rotary_encoders[i]);
        }
    }

    void static configure_rotary_encoder(rotary_encoder *re)
    {
        // configure the rotary encoder pins
        gpio_init(re->a);
        gpio_set_dir(re->a, GPIO_IN);
        gpio_pull_up(re->a);

        gpio_init(re->b);
        gpio_set_dir(re->b, GPIO_IN);
        gpio_pull_up(re->b);

        gpio_init(re->sw);
        gpio_set_dir(re->sw, GPIO_IN);
        gpio_pull_up(re->sw);

        // configure the interrupt for the rotary encoder
        gpio_set_irq_enabled_with_callback(re->a, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true, rotary_encoders_callback);
        gpio_set_irq_enabled_with_callback(re->b, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true, rotary_encoders_callback);
        gpio_set_irq_enabled_with_callback(re->sw, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true, rotary_encoders_callback);
    }

    void static configure_rotary_encoders(void)
    {
        for (int i = 0; i < NUM_ROTARY_ENCODERS; i++)
        {
            configure_rotary_encoder(&rotary_encoders[i]);
        }

        // read the initial state of the rotary encoder
        for (int i = 0; i < NUM_ROTARY_ENCODERS; i++)
        {
            rotary_encoders[i].a_b_trail = ((gpio_get(rotary_encoders[i].a) << 1) | gpio_get(rotary_encoders[i].b)) & 0xf;
            rotary_encoders[i].sw_trail = (gpio_get(rotary_encoders[i].sw)) & 0b11;
        }
    }
}