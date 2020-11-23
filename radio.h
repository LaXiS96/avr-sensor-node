#pragma once

#include <stdint.h>

/** Initialize IO and Timer */
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
