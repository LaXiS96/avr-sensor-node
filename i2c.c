#include "i2c.h"

#include <avr/io.h>
#include <util/delay.h>

/**
 * Simple software I2C master implementation
 * 
 * Only supports the bare minimum to achieve successful communication.
 * Timings are highly dependant on the CPU clock frequency, with exception for SCL
 * hold time which can be configured with I2C_SCL_HOLD_US (microseconds).
 * 
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

i2c_ret_ack_t i2c_write(uint8_t byte)
{
    i2c_ret_ack_t ack = 0;

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
    ack = (PINB & ~_BV(I2C_SDA_IO)) ? I2C_RET_NACK : I2C_RET_ACK;
    _delay_us(I2C_SCL_HOLD_US / 2);
    i2c_io_low(I2C_SCL_IO);

    return ack;
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

    // Always reply NACK (implicit because SDA is HIGH)

    i2c_io_high(I2C_SCL_IO);
    _delay_us(I2C_SCL_HOLD_US);
    i2c_io_low(I2C_SCL_IO);

    i2c_io_low(I2C_SDA_IO);

    return byte;
}

void i2c_read_buffer(uint8_t *buf, uint8_t len)
{
    i2c_io_high(I2C_SDA_IO);
    i2c_io_low(I2C_SCL_IO);

    for (uint8_t i = 0; i < len; i++)
    {
        buf[i] = 0;

        for (int8_t b = 7; b >= 0; b--)
        {
            i2c_io_high(I2C_SCL_IO);
            _delay_us(I2C_SCL_HOLD_US / 2);
            buf[i] |= (PINB & ~_BV(I2C_SDA_IO)) << b;
            i2c_io_low(I2C_SCL_IO);
            _delay_us(I2C_SCL_HOLD_US / 2);
        }

        // Respond ACK until we have read all bytes
        if (i != len - 1)
            i2c_io_low(I2C_SDA_IO);

        i2c_io_high(I2C_SCL_IO);
        _delay_us(I2C_SCL_HOLD_US);
        i2c_io_low(I2C_SCL_IO);

        i2c_io_high(I2C_SDA_IO);
    }

    i2c_io_low(I2C_SDA_IO);
}
