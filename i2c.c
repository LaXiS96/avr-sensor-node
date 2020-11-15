#include "i2c.h"

#include <avr/io.h>
#include <util/delay.h>

/**
 * Simple software I2C master implementation
 * Based on: https://i2c.info/i2c-bus-specification
 */

#define I2C_DELAY_US 5
#define I2C_DELAY_DIV 4
#define I2C_SDA_IO PB3
#define I2C_SCL_IO PB4

#define IO_INPUT_TRISTATE(x) ({ DDRB &= ~_BV((x)); PORTB &= ~_BV((x)); })
#define IO_INPUT_PULLUP(x) ({ DDRB &= ~_BV((x)); PORTB |= _BV((x)); })
#define IO_OUTPUT_HIGH(x) ({ DDRB |= _BV((x)); PORTB |= _BV((x)); })
#define IO_OUTPUT_LOW(x) ({ DDRB |= _BV((x)); PORTB &= ~_BV((x)); })

void i2c_init(void)
{
    IO_INPUT_TRISTATE(I2C_SDA_IO);
    IO_INPUT_TRISTATE(I2C_SCL_IO);
}

void i2c_start(void)
{
    // Falling edge on SDA with SCL high
    IO_INPUT_PULLUP(I2C_SCL_IO); // Intermediate state required by hardware
    IO_OUTPUT_HIGH(I2C_SCL_IO);
    // _delay_us(I2C_DELAY_US / I2C_DELAY_DIV);
    IO_INPUT_PULLUP(I2C_SDA_IO); // Intermediate state required by hardware
    IO_OUTPUT_HIGH(I2C_SDA_IO);
    IO_OUTPUT_LOW(I2C_SDA_IO);
}

void i2c_stop(void)
{
    // Rising edge on SDA with SCL high
    IO_OUTPUT_LOW(I2C_SDA_IO);
    IO_INPUT_TRISTATE(I2C_SCL_IO);
    // _delay_us(I2C_DELAY_US / I2C_DELAY_DIV);
    IO_INPUT_TRISTATE(I2C_SDA_IO);
}

void i2c_write(uint8_t byte)
{
    IO_OUTPUT_LOW(I2C_SDA_IO);
    IO_OUTPUT_LOW(I2C_SCL_IO);

    // Shift value bit by bit, MSB first
    for (int8_t i = 7; i >= 0; i--)
    {
        (byte >> i & 1) ? IO_OUTPUT_HIGH(I2C_SDA_IO) : IO_OUTPUT_LOW(I2C_SDA_IO);
        // _delay_us(I2C_DELAY_US / I2C_DELAY_DIV);
        IO_OUTPUT_HIGH(I2C_SCL_IO);
        _delay_us(I2C_DELAY_US);
        IO_OUTPUT_LOW(I2C_SCL_IO);
    }

    // Switch SDA to input, listen for acknowledgement
    // _delay_us(I2C_DELAY_US / I2C_DELAY_DIV);
    IO_INPUT_TRISTATE(I2C_SDA_IO);

    IO_OUTPUT_HIGH(I2C_SCL_IO);
    _delay_us(I2C_DELAY_US);
    // TODO read ack value here
    IO_OUTPUT_LOW(I2C_SCL_IO);
    // _delay_us(I2C_DELAY_US / I2C_DELAY_DIV);
}

uint8_t i2c_read(void)
{
    IO_INPUT_TRISTATE(I2C_SDA_IO);
    IO_OUTPUT_LOW(I2C_SCL_IO);

    uint8_t byte = 0;

    for (int8_t i = 7; i >= 0; i--)
    {
        // _delay_us(I2C_DELAY_US / I2C_DELAY_DIV);
        IO_OUTPUT_HIGH(I2C_SCL_IO);
        _delay_us(I2C_DELAY_US / 2);
        byte |= (PINB & ~_BV(I2C_SDA_IO)) << i;
        _delay_us(I2C_DELAY_US / 2);
        IO_OUTPUT_LOW(I2C_SCL_IO);
    }

    // Always reply NACK (implicit)
    // IO_INPUT_PULLUP(I2C_SDA_IO); // Intermediate state required by hardware 
    // IO_OUTPUT_HIGH(I2C_SDA_IO);

    IO_OUTPUT_HIGH(I2C_SCL_IO);
    _delay_us(I2C_DELAY_US);
    IO_OUTPUT_LOW(I2C_SCL_IO);

    IO_OUTPUT_LOW(I2C_SDA_IO);

    return byte;
}

uint8_t i2c_read_buffer(uint8_t *buf)
{
    // TODO read while ACK, stop at NACK
    return 0;
}
