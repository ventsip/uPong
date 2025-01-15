#pragma once
#include "hardware/gpio.h"

#define ROTARY_ENCODER_1_A_PIN 22
#define ROTARY_ENCODER_1_B_PIN 26
#define ROTARY_ENCODER_1_SW_PIN 27

#define ROTARY_ENCODER_2_A_PIN 19
#define ROTARY_ENCODER_2_B_PIN 20
#define ROTARY_ENCODER_2_SW_PIN 21

enum
{
    ROTARY_ENCODER_SW_NA = 0,
    ROTARY_ENCODER_SW_RELEASED,
    ROTARY_ENCODER_SW_PRESSED,
};

// rotary encoder state
static volatile uint8_t rotary_encoder_1_a_b_trail = 0;
static volatile int32_t rotary_encoder_1_counter = 0;
static volatile uint8_t rotary_encoder_2_a_b_trail = 0;
static volatile int32_t rotary_encoder_2_counter = 0;
// rotary encoder switch state
static volatile uint8_t rotary_encoder_1_sw_trail = 0;
static volatile uint8_t rotary_encoder_1_sw_state = ROTARY_ENCODER_SW_NA;
static volatile uint8_t rotary_encoder_2_sw_trail = 0;
static volatile uint8_t rotary_encoder_2_sw_state = ROTARY_ENCODER_SW_NA;

int32_t rotary_encoder_1_fetch_counter(void)
{
    const int32_t counter = rotary_encoder_1_counter;
    rotary_encoder_1_counter = 0;
    return counter;
}

int32_t rotary_encoder_2_fetch_counter(void)
{
    const int32_t counter = rotary_encoder_2_counter;
    rotary_encoder_2_counter = 0;
    return counter;
}

uint8_t rotary_encoder_1_fetch_sw_state(void)
{
    const uint8_t sw_state = rotary_encoder_1_sw_state;
    rotary_encoder_1_sw_state = ROTARY_ENCODER_SW_NA;
    return sw_state;
}

uint8_t rotary_encoder_2_fetch_sw_state(void)
{
    const uint8_t sw_state = rotary_encoder_2_sw_state;
    rotary_encoder_2_sw_state = ROTARY_ENCODER_SW_NA;
    return sw_state;
}

void static rotary_encoder_callback(uint gpio, __unused uint32_t events, const uint PIN_A, const uint PIN_B, const uint PIN_SW, uint8_t volatile *a_b_trial, int32_t volatile *counter, uint8_t volatile *sw_trial, uint8_t volatile *sw_state)
{
    if (gpio == PIN_A || gpio == PIN_B)
    {
        *a_b_trial = (((*a_b_trial) << 2) | (gpio_get(PIN_A) << 1) | gpio_get(PIN_B)) & 0xf;

        switch (*a_b_trial)
        {
        case 0b0001:
        case 0b0111:
        case 0b1110:
        case 0b1000:
            // clockwise
            (*counter)++;
            break;
        case 0b0010:
        case 0b1011:
        case 0b1101:
        case 0b0100:
            // counter clockwise
            (*counter)--;
            break;
        }
    }

    if (gpio == PIN_SW)
    {
        *sw_trial = (((*sw_trial) << 1) | gpio_get(PIN_SW)) & 0b11;
        switch (*sw_trial)
        {
        case 0b01:
            *sw_state = ROTARY_ENCODER_SW_PRESSED;
            break;
        case 0b10:
            *sw_state = ROTARY_ENCODER_SW_RELEASED;
            break;
        }
    }
}
// isr for the rotary encoder
void rotary_encoders_callback(uint gpio, __unused uint32_t events)
{
    rotary_encoder_callback(gpio, events, ROTARY_ENCODER_1_A_PIN, ROTARY_ENCODER_1_B_PIN, ROTARY_ENCODER_1_SW_PIN, &rotary_encoder_1_a_b_trail, &rotary_encoder_1_counter, &rotary_encoder_1_sw_trail, &rotary_encoder_1_sw_state);
    rotary_encoder_callback(gpio, events, ROTARY_ENCODER_2_A_PIN, ROTARY_ENCODER_2_B_PIN, ROTARY_ENCODER_2_SW_PIN, &rotary_encoder_2_a_b_trail, &rotary_encoder_2_counter, &rotary_encoder_2_sw_trail, &rotary_encoder_2_sw_state);
}

void configure_rotary_encoder(void)
{
    // configure the rotary encoder pins
    gpio_init(ROTARY_ENCODER_1_A_PIN);
    gpio_set_dir(ROTARY_ENCODER_1_A_PIN, GPIO_IN);
    gpio_pull_up(ROTARY_ENCODER_1_A_PIN);

    gpio_init(ROTARY_ENCODER_1_B_PIN);
    gpio_set_dir(ROTARY_ENCODER_1_B_PIN, GPIO_IN);
    gpio_pull_up(ROTARY_ENCODER_1_B_PIN);

    gpio_init(ROTARY_ENCODER_1_SW_PIN);
    gpio_set_dir(ROTARY_ENCODER_1_SW_PIN, GPIO_IN);
    gpio_pull_up(ROTARY_ENCODER_1_SW_PIN);

    gpio_init(ROTARY_ENCODER_2_A_PIN);
    gpio_set_dir(ROTARY_ENCODER_2_A_PIN, GPIO_IN);
    gpio_pull_up(ROTARY_ENCODER_2_A_PIN);

    gpio_init(ROTARY_ENCODER_2_B_PIN);
    gpio_set_dir(ROTARY_ENCODER_2_B_PIN, GPIO_IN);
    gpio_pull_up(ROTARY_ENCODER_2_B_PIN);

    gpio_init(ROTARY_ENCODER_2_SW_PIN);
    gpio_set_dir(ROTARY_ENCODER_2_SW_PIN, GPIO_IN);
    gpio_pull_up(ROTARY_ENCODER_2_SW_PIN);

    // configure the interrupt for the rotary encoder
    gpio_set_irq_enabled_with_callback(ROTARY_ENCODER_1_A_PIN, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true, rotary_encoders_callback);
    gpio_set_irq_enabled_with_callback(ROTARY_ENCODER_1_B_PIN, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true, rotary_encoders_callback);
    gpio_set_irq_enabled_with_callback(ROTARY_ENCODER_1_SW_PIN, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true, rotary_encoders_callback);

    gpio_set_irq_enabled_with_callback(ROTARY_ENCODER_2_A_PIN, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true, rotary_encoders_callback);
    gpio_set_irq_enabled_with_callback(ROTARY_ENCODER_2_B_PIN, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true, rotary_encoders_callback);
    gpio_set_irq_enabled_with_callback(ROTARY_ENCODER_2_SW_PIN, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true, rotary_encoders_callback);

    // read the initial state of the rotary encoder
    rotary_encoder_1_a_b_trail = ((gpio_get(ROTARY_ENCODER_1_A_PIN) << 1) | gpio_get(ROTARY_ENCODER_1_B_PIN)) & 0xf;
    rotary_encoder_1_sw_trail = (gpio_get(ROTARY_ENCODER_1_SW_PIN)) & 0b11;

    rotary_encoder_2_a_b_trail = ((gpio_get(ROTARY_ENCODER_2_A_PIN) << 1) | gpio_get(ROTARY_ENCODER_2_B_PIN)) & 0xf;
    rotary_encoder_2_sw_trail = (gpio_get(ROTARY_ENCODER_2_SW_PIN)) & 0b11;
}