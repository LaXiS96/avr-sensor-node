#include <avr/interrupt.h>
#include <avr/io.h>
#include <avr/sleep.h>
#include <avr/wdt.h>
#include <util/delay.h>

#include "i2c.h"

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

    // // Setup PB4 as output
    // DDRB |= _BV(DDB4);

    // // Setup watchdog to interrupt every 4 seconds
    // wdt_enable(WDTO_4S);
    // WDTCR |= _BV(WDIE);
    // sei();

    // set_sleep_mode(SLEEP_MODE_PWR_DOWN);

    i2c_init();

    // BME280 reset
    i2c_start();
    i2c_write(I2C_ADDR7_WRITE(0x76));
    i2c_write(0xE0); // reset
    i2c_write(0xB6);
    i2c_stop();

    // BME280 enable humidity sensing
    i2c_start();
    i2c_write(I2C_ADDR7_WRITE(0x76));
    i2c_write(0xF2); // ctrl_hum
    i2c_write(0x01); // oversampling x1
    i2c_stop();

    while (1)
    {
        // // Enter sleep mode
        // sleep_mode();

        // if (wakeCount >= MAX_WAKE_COUNT)
        // {
        //     PORTB |= _BV(PB4);
        //     _delay_ms(500);
        //     PORTB &= ~_BV(PB4);

        //     wakeCount = 0;
        // }

        uint8_t bytes[8];

        // BME280 enable temperature and pressure sensing, set mode (starts forced measurement)
        i2c_start();
        i2c_write(I2C_ADDR7_WRITE(0x76));
        i2c_write(0xF4);       // ctrl_meas
        i2c_write(0b00100101); // oversampling pressure:x1 temperature:x1, forced mode
        i2c_stop();

        _delay_ms(10); // 8ms measurement time with this configuration (see datasheet chapter 9)

        // BME280 read out measurement
        i2c_start();
        i2c_write(I2C_ADDR7_WRITE(0x76));
        i2c_write(0xF7);
        i2c_start();
        i2c_write(I2C_ADDR7_READ(0x76));
        i2c_read_buffer(bytes, 8);
        i2c_stop();

        _delay_ms(1000);
    }
}

ISR(WDT_vect)
{
    // Re-enable interrupt, otherwise next timeout will result in a reset
    WDTCR |= _BV(WDIE);

    wakeCount++;
}
