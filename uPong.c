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
    PIO pio = pio0;
    uint offset = pio_add_program(pio, &blink_program);
    printf("Loaded program at %d\n", offset);

#ifdef PICO_DEFAULT_LED_PIN
    blink_pin_forever(pio, 0, offset, PICO_DEFAULT_LED_PIN, 2);
#else
    blink_pin_forever(pio, 0, offset, 6, 3);
#endif
    // For more pio examples see https://github.com/raspberrypi/pico-examples/tree/master/pio
    // For more examples of timer use see https://github.com/raspberrypi/pico-examples/tree/master/timer

    while (true)
    {
        printf("Hello, world!\n");
        sleep_ms(1000);
    }
}
