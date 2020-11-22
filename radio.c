#include "radio.h"

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

#define RADIO_IO_HIGH() (PORTB |= _BV(RADIO_IO))
#define RADIO_IO_LOW() (PORTB &= ~_BV(RADIO_IO))

typedef enum
{
    TX_IDLE = 0,
    TX_PHASE1,
    TX_PHASE2,
} radio_tx_state_t;
static volatile radio_tx_state_t tx_state = TX_IDLE;
static volatile uint8_t tx_data[] = {255, 255, 1, 2, 3, 127};
static volatile uint8_t tx_byte = 0; // Current byte in data array
static volatile uint8_t tx_bit = 0;  // Current bit in current byte

void radio_init(void)
{
    DDRB |= _BV(RADIO_IO);

    // Interrupt frequency = FclkIO / prescaler / OCR
    OCR0A = 30;
    TIMSK = 0b00010000;  // enable interrupt on OCR0A match
    TCCR0A = 0b00000010; // CTC mode
    TCCR0B = 0b00000011; // start counter, prescaler 64 (with FclkIO=8MHz, Finterrupt=[490Hz, 125kHz])
    sei();

    tx_state = TX_PHASE1; // Start transmission
}

void radio_tx(uint8_t byte)
{
    // TODO
}

ISR(TIMER0_COMPA_vect)
{
    switch (tx_state)
    {
    case TX_IDLE:
        // RADIO_IO_LOW();
        break;
    case TX_PHASE1:
        (tx_data[tx_byte] & _BV(7 - tx_bit)) ? RADIO_IO_LOW() : RADIO_IO_HIGH();
        tx_state = TX_PHASE2;
        break;
    case TX_PHASE2:
        (tx_data[tx_byte] & _BV(7 - tx_bit)) ? RADIO_IO_HIGH() : RADIO_IO_LOW();
        tx_state = TX_PHASE1;
        tx_bit++;
        if (tx_bit == 8)
        {
            tx_bit = 0;
            tx_byte++;
            if (tx_byte == sizeof(tx_data))
            {
                tx_byte = 0;
                // tx_state = TX_IDLE;
                // RADIO_IO_LOW();
            }
        }
        break;
    }
}
