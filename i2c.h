#pragma once

#include <stdint.h>

#define I2C_ADDR7_READ(x) ((x) << 1 | 1)
#define I2C_ADDR7_WRITE(x) ((x) << 1 & ~1)

void i2c_init(void);
void i2c_start(void);
void i2c_stop(void);
void i2c_write(uint8_t byte);
uint8_t i2c_read(void);
uint8_t i2c_read_buffer(uint8_t* buf);
