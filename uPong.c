#include "hardware/dma.h"
#include "hardware/pio.h"
#include "pico/stdlib.h"
#include <stdio.h>

#include "blink.pio.h"

void blink_pin_forever(PIO pio, uint sm, uint offset, uint pin, uint freq)
{
    blink_program_init(pio, sm, offset, pin);
    pio_sm_set_enabled(pio, sm, true);

    printf("Blinking pin %d at %d Hz\n", pin, freq);

    // PIO counter program takes 3 more cycles in total than we pass as
    // input (wait for n + 1; mov; jmp)
    pio->txf[sm] = (150000000 / (2 * freq)) - 3;
}

int main()
{
    stdio_init_all();

    // PIO Blinking example
    PIO pio;
    uint sm;
    uint offset;
    bool success = pio_claim_free_sm_and_add_program_for_gpio_range(&blink_program, &pio, &sm, &offset, PICO_DEFAULT_LED_PIN, 1, true);
    printf("Loaded program at %d\n", offset);
    blink_pin_forever(pio, 0, offset, PICO_DEFAULT_LED_PIN, 2);

    // For more pio examples see https://github.com/raspberrypi/pico-examples/tree/master/pio
    // For more examples of timer use see https://github.com/raspberrypi/pico-examples/tree/master/timer

    while (true)
    {
        printf("Loaded program at %d\n", offset);
        sleep_ms(1000);
    }
}
