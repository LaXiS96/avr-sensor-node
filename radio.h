#pragma once

#include <stdint.h>

#define RADIO_SYNC_WORD 0b11110101

/**
 * Initialize IO and Timer.
 * Remembre to enable global interrupts afterwards.
 */
void radio_init(void);

/**
 * Transmit raw buffer.
 * Blocks until transmission is complete.
 */
void radio_tx_buffer(uint8_t *data, uint8_t len);

/**
 * Transmit packet with given payload data.
 * Blocks until transmission is complete.
 */
void radio_tx_packet(uint8_t *data, uint8_t len);
