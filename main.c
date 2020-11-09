#include <avr/interrupt.h>
#include <avr/io.h>
#include <avr/sleep.h>
#include <avr/wdt.h>
#include <util/delay.h>

/*
MCU sleeps in power down mode for most of the time, wakes up every ~4 seconds
and checks if 1 minute has passed (with pretty low accuracy), if so it should
do a measurement and send it to the base station.
*/

#define MAX_WAKE_COUNT 15

volatile uint8_t wakeCount = 0;

int main(void)
{
    // Set all pins as inputs with pull-up
    DDRB &= 0;
    PORTB |= 0x3F;

    // Setup PB4 as output
    DDRB |= _BV(DDB4);

    // Setup watchdog to interrupt every 4 seconds
    wdt_enable(WDTO_4S);
    WDTCR |= _BV(WDIE);
    sei();

    set_sleep_mode(SLEEP_MODE_PWR_DOWN);

    while (1)
    {
        // Enter sleep mode
        sleep_mode();

        if (wakeCount >= MAX_WAKE_COUNT)
        {
            PORTB |= _BV(PB4);
            _delay_ms(500);
            PORTB &= ~_BV(PB4);

            wakeCount = 0;
        }
    }
}

ISR(WDT_vect)
{
    // Re-enable interrupt, otherwise next timeout will result in a reset
    WDTCR |= _BV(WDIE);

    wakeCount++;
}
