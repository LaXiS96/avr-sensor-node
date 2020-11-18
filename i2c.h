#pragma once

#include <stdint.h>

#define I2C_ADDR7_READ(x) ((x) << 1 | 1)
#define I2C_ADDR7_WRITE(x) ((x) << 1 & ~1)

typedef enum
{
    I2C_RET_ACK = 1,
    I2C_RET_NACK = 2
} i2c_ret_ack_t;

/** Initialize IO pins */
void i2c_init(void);

/** Send a START condition */
void i2c_start(void);

/** Send a STOP condition */
void i2c_stop(void);

/** Write one byte. Returns ACK value */
i2c_ret_ack_t i2c_write(uint8_t byte);

/** Read one byte */
uint8_t i2c_read(void);

/** Read len bytes into buf */
void i2c_read_buffer(uint8_t *buf, uint8_t len);
