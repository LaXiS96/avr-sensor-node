#include "radio.h"

#include <stddef.h>
#include <avr/io.h>
#include <avr/interrupt.h>

/**
 * Implementation of Manchester line coding for use with 433MHz ASK/OOK RF modules.
 * 
 * Uses IEEE 802.3 standard (0 = falling edge, 1 = rising edge).
 * Transmits MSB first.
 * 
 * TODO: with Manchester line coding, symbol rate is half the modulation frequency,
 * which is very inefficient. Use something like 8b/10b or some variation
 * (see RadioHead library, which uses 4 to 6 bit encoding).
 * 
 * https://en.wikipedia.org/wiki/Manchester_code
 */

#define RADIO_IO PB0
#define RADIO_SYNC_LEN 3    // Duration of steady level for SYNC condition in bit-times
#define RADIO_SYNC_COUNT 11 // Number of edge transitions for SYNC condition
#define RADIO_STOP_LEN 3    // Duration of steady level for STOP condition in bit-times
#define RADIO_STOP_COUNT 10 // Number of edge transitions for STOP condition

#define RADIO_IO_HIGH() (PORTB |= _BV(RADIO_IO))
#define RADIO_IO_LOW() (PORTB &= ~_BV(RADIO_IO))
#define RADIO_IO_TOGGLE() (PINB |= _BV(RADIO_IO))

typedef enum
{
    TX_IDLE,        // no trasmission is in progress, output should be LOW
    TX_START,       // data is ready and trasmission should start
    TX_SYNC,        // TODO
    TX_DATA_PHASE1, // first half of bit output, prepare for next edge
    TX_DATA_PHASE2, // second half of bit output, sets next bit and byte counters,
                    // changes state to STOP if all bytes have been transmitted
    TX_STOP,        // TODO
    TX_END,         // transmission complete, sets output LOW and goes idle
} radio_tx_state_t;

static volatile radio_tx_state_t tx_state = TX_IDLE;
static volatile uint8_t *tx_data = NULL;
static volatile uint8_t tx_data_len = 0;

void radio_init(void)
{
    DDRB |= _BV(RADIO_IO);
    PORTB &= ~_BV(RADIO_IO);

    // Interrupt frequency: Fint = FclkIO / prescaler / (OCR + 1)
    // Manchester symbol rate = Fint / 2
    OCR0A = 28;
    TIMSK = 0b00010000;  // enable interrupt on OCR0A match
    TCCR0A = 0b00000010; // CTC mode
    TCCR0B = 0b00000010; // start, prescaler 8
}

void radio_tx_buffer(uint8_t *data, uint8_t len)
{
    tx_data = data;
    tx_data_len = len;

    tx_state = TX_START;

    // TODO make asynchronous
    while (tx_state != TX_IDLE)
        ;
}

void radio_tx_packet(uint8_t *data, uint8_t len)
{
    // TODO encapsulate data into packet: start, length, payload, crc, stop?
    // 'start' is for receiver clock recovery
    // 'length' is the payload length, excluding crc
    // 'crc' is an error check code for payload data (include length?)
}

ISR(TIMER0_COMPA_vect)
{
    static uint8_t cur_cycle = 0;
    static uint8_t cur_pulse = 0;

    switch (tx_state)
    {
    case TX_IDLE:
        break;
    case TX_START:
        cur_cycle = 0;
        cur_pulse = 0;
        tx_state = TX_SYNC;
        break;
    case TX_SYNC:
        if (cur_pulse == 0)
        {
            RADIO_IO_TOGGLE();
            cur_cycle++;
        }

        if (++cur_pulse == RADIO_SYNC_LEN)
        {
            cur_pulse = 0;
            if (cur_cycle == RADIO_SYNC_COUNT)
            {
                cur_cycle = 0;
                cur_pulse = 0;
                tx_state = TX_DATA_PHASE1;
            }
        }
        break;
    case TX_DATA_PHASE1:
        // cur_cucle is the current byte, cur_pulse is the current bit
        (tx_data[cur_cycle] & _BV(7 - cur_pulse)) ? RADIO_IO_LOW() : RADIO_IO_HIGH();
        tx_state = TX_DATA_PHASE2;
        break;
    case TX_DATA_PHASE2:
        // cur_cucle is the current byte, cur_pulse is the current bit
        (tx_data[cur_cycle] & _BV(7 - cur_pulse)) ? RADIO_IO_HIGH() : RADIO_IO_LOW();
        tx_state = TX_DATA_PHASE1;
        if (++cur_pulse == 8)
        {
            cur_pulse = 0;
            if (++cur_cycle == tx_data_len)
            {
                cur_cycle = 0;
                cur_pulse = 0;
                tx_state = TX_STOP;
            }
        }
        break;
    case TX_STOP:
        if (cur_pulse == 0)
        {
            RADIO_IO_TOGGLE();
            cur_cycle++;
        }

        if (++cur_pulse == RADIO_STOP_LEN)
        {
            cur_pulse = 0;
            if (cur_cycle == RADIO_STOP_COUNT)
                tx_state = TX_END;
        }
        break;
    case TX_END:
        RADIO_IO_LOW();
        tx_state = TX_IDLE;
        break;
    }
}
