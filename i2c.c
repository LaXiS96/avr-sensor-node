#include "i2c.h"

#include <avr/io.h>
#include <util/delay.h>

/**
 * Simple software I2C master implementation
 * Based on:
 * https://i2c.info/i2c-bus-specification
 * https://en.wikipedia.org/wiki/I%C2%B2C
 */

#define I2C_SDA_IO PB3
#define I2C_SCL_IO PB4
#define I2C_SCL_HOLD_US 5

static inline void i2c_io_high(uint8_t io)
{
    // HIGH bus level means tri-state input
    DDRB &= ~_BV(io);
    PORTB &= ~_BV(io);
}

static inline void i2c_io_low(uint8_t io)
{
    // LOW bus level means output low
    DDRB |= _BV(io);
    PORTB &= ~_BV(io);
}

void i2c_init(void)
{
    i2c_io_high(I2C_SDA_IO);
    i2c_io_high(I2C_SCL_IO);
}

void i2c_start(void)
{
    // Falling edge on SDA with SCL high
    i2c_io_high(I2C_SCL_IO);
    i2c_io_high(I2C_SDA_IO);
    i2c_io_low(I2C_SDA_IO);
}

void i2c_stop(void)
{
    // Rising edge on SDA with SCL high
    i2c_io_low(I2C_SDA_IO);
    i2c_io_high(I2C_SCL_IO);
    i2c_io_high(I2C_SDA_IO);
}

void i2c_write(uint8_t byte)
{
    i2c_io_low(I2C_SDA_IO);
    i2c_io_low(I2C_SCL_IO);

    // Shift value bit by bit, MSB first
    for (int8_t i = 7; i >= 0; i--)
    {
        (byte >> i & 1) ? i2c_io_high(I2C_SDA_IO) : i2c_io_low(I2C_SDA_IO);
        i2c_io_high(I2C_SCL_IO);
        _delay_us(I2C_SCL_HOLD_US);
        i2c_io_low(I2C_SCL_IO);
    }

    // Switch SDA to input, listen for acknowledgement
    i2c_io_high(I2C_SDA_IO);

    i2c_io_high(I2C_SCL_IO);
    _delay_us(I2C_SCL_HOLD_US / 2);
    // TODO read ack value here
    _delay_us(I2C_SCL_HOLD_US / 2);
    i2c_io_low(I2C_SCL_IO);
}

uint8_t i2c_read(void)
{
    i2c_io_high(I2C_SDA_IO);
    i2c_io_low(I2C_SCL_IO);

    uint8_t byte = 0;

    for (int8_t i = 7; i >= 0; i--)
    {
        i2c_io_high(I2C_SCL_IO);
        _delay_us(I2C_SCL_HOLD_US / 2);
        byte |= (PINB & ~_BV(I2C_SDA_IO)) << i;
        _delay_us(I2C_SCL_HOLD_US / 2);
        i2c_io_low(I2C_SCL_IO);
    }

    // Always reply NACK (implicit)
    // i2c_io_high(I2C_SDA_IO);

    i2c_io_high(I2C_SCL_IO);
    _delay_us(I2C_SCL_HOLD_US);
    i2c_io_low(I2C_SCL_IO);

    i2c_io_low(I2C_SDA_IO);

    return byte;
}

uint8_t i2c_read_buffer(uint8_t *buf)
{
    // TODO read while ACK, stop at NACK
    return 0;
}
