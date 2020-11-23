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
 * https://en.wikipedia.org/wiki/Manchester_code
 */

#define RADIO_IO PB0
#define RADIO_LEAD_IN_LEN 20

#define RADIO_IO_HIGH() (PORTB |= _BV(RADIO_IO))
#define RADIO_IO_LOW() (PORTB &= ~_BV(RADIO_IO))

typedef enum
{
    TX_IDLE = 0,
    TX_START,
    TX_LEAD_IN,
    TX_BIT_PHASE1,
    TX_BIT_PHASE2,
    TX_END,
} radio_tx_state_t;

static volatile radio_tx_state_t tx_state = TX_IDLE;
static volatile uint8_t *tx_data = NULL;
static volatile uint8_t tx_data_len = 0;
static volatile uint8_t tx_cur_byte = 0;
static volatile uint8_t tx_cur_bit = 0;
static volatile uint8_t tx_cur_lead_in = 0;

void radio_init(void)
{
    DDRB |= _BV(RADIO_IO);
    PORTB &= ~_BV(RADIO_IO);

    // Interrupt frequency: Fint = FclkIO / prescaler / OCR
    // Manchester bit rate = Fint / 2
    // Tested OCR values:
    // - 3 = ~15kHz bit rate (maybe upper limit, decodable at a few centimeters)
    OCR0A = 3;
    TIMSK = 0b00010000;  // enable interrupt on OCR0A match
    TCCR0A = 0b00000010; // CTC mode
    TCCR0B = 0b00000011; // start, prescaler 64 (with FclkIO=8MHz, Fint=[490Hz, 125kHz])
    sei();
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

/**
 * State machine states explanation:
 * 1. TX_IDLE: no trasmission is in progress, output should be LOW
 * 2. TX_START: data is ready and trasmission should start
 * 3. TX_LEAD_IN: HIGH output period, allows receiver to lock onto signal and adjust its gain
 * 4. TX_BIT_PHASE1: first half of bit output, prepare for next edge
 * 5. TX_BIT_PHASE2: second half of bit output, sets next bit and byte counters,
 *                   ends transmission if all bytes have been transmitted
 * 6. TX_END: transmission complete, sets output LOW and goes idle
 */
ISR(TIMER0_COMPA_vect)
{
    switch (tx_state)
    {
    case TX_IDLE:
        break;
    case TX_START:
        tx_cur_byte = 0;
        tx_cur_bit = 0;
        tx_cur_lead_in = 0;

        RADIO_IO_HIGH();
        tx_state = TX_LEAD_IN;
        // No break, fall-through TX_LEAD_IN
    case TX_LEAD_IN:
        if (tx_cur_lead_in++ == RADIO_LEAD_IN_LEN)
            tx_state = TX_BIT_PHASE1;
        break;
    case TX_BIT_PHASE1:
        (tx_data[tx_cur_byte] & _BV(7 - tx_cur_bit)) ? RADIO_IO_LOW() : RADIO_IO_HIGH();
        tx_state = TX_BIT_PHASE2;
        break;
    case TX_BIT_PHASE2:
        (tx_data[tx_cur_byte] & _BV(7 - tx_cur_bit)) ? RADIO_IO_HIGH() : RADIO_IO_LOW();
        tx_state = TX_BIT_PHASE1;
        if (++tx_cur_bit == 8)
        {
            tx_cur_bit = 0;
            if (++tx_cur_byte == tx_data_len)
                tx_state = TX_END;
        }
        break;
    case TX_END:
        RADIO_IO_LOW();
        tx_state = TX_IDLE;
        break;
    }
}
