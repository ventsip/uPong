#pragma once
#include "hardware/gpio.h"

#define ROTARY_ENCODER_A_PIN 22
#define ROTARY_ENCODER_B_PIN 26
#define ROTARY_ENCODER_SW_PIN 27

// rotary encoder state
static volatile uint8_t rotary_encoder_a_b_trail = 0;
static volatile int32_t rotary_encoder_counter = 0;
// rotary encoder switch state
static volatile uint8_t rotary_encoder_sw_trail = 0;
enum
{
    ROTARY_ENCODER_SW_NA = 0,
    ROTARY_ENCODER_SW_RELEASED,
    ROTARY_ENCODER_SW_PRESSED,
};
static volatile uint8_t rotary_encoder_sw_state = ROTARY_ENCODER_SW_NA;

int32_t rotary_encoder_fetch_counter(void)
{
    const int32_t counter = rotary_encoder_counter;
    rotary_encoder_counter = 0;
    return counter;
}

uint8_t rotary_encoder_fetch_sw_state(void)
{
    const uint8_t sw_state = rotary_encoder_sw_state;
    rotary_encoder_sw_state = ROTARY_ENCODER_SW_NA;
    return sw_state;
}

// isr for the rotary encoder
void rotary_encoder_callback(uint gpio, __unused uint32_t events)
{
    if (gpio == ROTARY_ENCODER_A_PIN || gpio == ROTARY_ENCODER_B_PIN)
    {
        rotary_encoder_a_b_trail = ((rotary_encoder_a_b_trail << 2) | (gpio_get(ROTARY_ENCODER_A_PIN) << 1) | gpio_get(ROTARY_ENCODER_B_PIN)) & 0xf;

        switch (rotary_encoder_a_b_trail)
        {
        case 0b0001:
        case 0b0111:
        case 0b1110:
        case 0b1000:
            // clockwise
            rotary_encoder_counter++;
            break;
        case 0b0010:
        case 0b1011:
        case 0b1101:
        case 0b0100:
            // counter clockwise
            rotary_encoder_counter--;
            break;
        }
    }

    if (gpio == ROTARY_ENCODER_SW_PIN)
    {
        rotary_encoder_sw_trail = ((rotary_encoder_sw_trail << 1) | gpio_get(ROTARY_ENCODER_SW_PIN)) & 0b11;
        switch (rotary_encoder_sw_trail)
        {
        case 0b01:
            rotary_encoder_sw_state = ROTARY_ENCODER_SW_PRESSED;
            break;
        case 0b10:
            rotary_encoder_sw_state = ROTARY_ENCODER_SW_RELEASED;
            break;
        }
    }
}

void configure_rotary_encoder(void)
{
    // configure the rotary encoder pins
    gpio_init(ROTARY_ENCODER_A_PIN);
    gpio_set_dir(ROTARY_ENCODER_A_PIN, GPIO_IN);
    gpio_pull_up(ROTARY_ENCODER_A_PIN);

    gpio_init(ROTARY_ENCODER_B_PIN);
    gpio_set_dir(ROTARY_ENCODER_B_PIN, GPIO_IN);
    gpio_pull_up(ROTARY_ENCODER_B_PIN);

    gpio_init(ROTARY_ENCODER_SW_PIN);
    gpio_set_dir(ROTARY_ENCODER_SW_PIN, GPIO_IN);
    gpio_pull_up(ROTARY_ENCODER_SW_PIN);

    // configure the interrupt for the rotary encoder
    gpio_set_irq_enabled_with_callback(ROTARY_ENCODER_A_PIN, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true, rotary_encoder_callback);
    gpio_set_irq_enabled_with_callback(ROTARY_ENCODER_B_PIN, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true, rotary_encoder_callback);
    gpio_set_irq_enabled_with_callback(ROTARY_ENCODER_SW_PIN, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true, rotary_encoder_callback);

    rotary_encoder_a_b_trail = ((rotary_encoder_a_b_trail << 2) | (gpio_get(ROTARY_ENCODER_A_PIN) << 1) | gpio_get(ROTARY_ENCODER_B_PIN)) & 0xf;
    rotary_encoder_sw_trail = ((rotary_encoder_sw_trail << 1) | gpio_get(ROTARY_ENCODER_SW_PIN)) & 0b11;
}