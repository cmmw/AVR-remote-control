/*
* main.cpp
*
*  Created on: 22. Jän. 2016
*      Author: Christian
*/

#include "config.h"

#include <avr/interrupt.h>
#include <avr/eeprom.h>
#include <avr/cpufunc.h>
#include <util/delay.h>
#include <avr/io.h>
#include <avr/sleep.h>

#include "rcver.h"
#include "sender.h"

enum Action
{
    SEND,
    RECEIVE,
    NOTHING
};


struct signal_t
{
    uint16_t size;
    uint16_t* ptr;
};

static void init_irq();
//Reads signal from eeprom into data starting at address start, returns nr of read bytes
static uint16_t read_signal(uint16_t* start, uint16_t* data, uint16_t size);
static void recv_keys(signal_t* signals, uint_fast8_t ssize, uint16_t* buffer, uint_fast8_t bsize);

static Action action = NOTHING;

ISR(PCINT0_vect)
{
}

ISR(TIMER0_COMPA_vect)
{
    switch (action)
    {
        case SEND:
            GREEN_ON;
            send();
            break;
        case RECEIVE:
            receive();
            break;
        case NOTHING:
            GREEN_OFF;
            break;
    }

}

int main()
{
    // Set output ports
    DDRB |= (1 << DDB2);		//green led
    DDRB |= (1 << DDB1);		//IR led
    // DDRB |= (1 << DDB0);		//red led

    // Set input pins
    DDRB &= ~(1 << DDB0);		//switch1
    DDRB &= ~(1 << DDB3);		//switch2
    PORTB |= (1 << PB0);		//use intern pull-up resistor
    PORTB |= (1 << PB3);

    init_rcver();
    init_pwm();
    init_irq();
    //Enable PCINT for switches at pin 0 and 3
    GIMSK |= (1 << PCIE);
    PCMSK |= (1 << PCINT0) | (1 << PCINT3);

    //Disable Analog Comparator
    ACSR |= (1 << ACD);

    sei();

    signal_t signals[2] = {0,0};
    uint16_t buffer[BUFFSIZE];
    uint16_t* addr = 0;
    uint16_t* ptr = buffer;
    uint16_t size = BUFFSIZE;

    for(unsigned int i = 0; i < sizeof(signals)/sizeof(signals[0]); i++)
    {
        uint_fast16_t read = read_signal(addr, ptr, size);
        if(read > 0)
        {
            GREEN_ON;
            signals[i].ptr = ptr;
            signals[i].size = read;
            addr += read;
            ptr += read;
            size -= read;
        }
        else
        {
            break;
        }
    }
    GREEN_OFF;

    //Receive Key
    if(signals[0].ptr == NULL)
        recv_keys(signals, sizeof(signals)/sizeof(signals[0]), buffer, BUFFSIZE);

    uint_fast16_t wait = RESET_WAIT;
    while(true)
    {
        //Send Key
        //wait for switch

        if(SWITCH1_ON && SWITCH2_ON)
        {
            wait--;
            if(wait == 0)
            {
                GREEN_ON;
                _delay_ms(1000);
                GREEN_OFF;
                recv_keys(signals, sizeof(signals)/sizeof(signals[0]), buffer, BUFFSIZE);
                wait = RESET_WAIT;
            }
            continue;
        }
        else
        {
            wait = RESET_WAIT;
            if(SWITCH1_ON)
            {
                if(signals[0].ptr != NULL)
                {
                    sender_set(signals[0].ptr, signals[0].size-1);
                }
            }
            else if(SWITCH2_ON)
            {
                if(signals[1].ptr != NULL)
                {
                    sender_set(signals[1].ptr, signals[1].size-1);
                }
            }
            else
            {
                set_sleep_mode(SLEEP_MODE_PWR_DOWN);
                sleep_mode();
                continue;
            }
        }


        GREEN_ON;
        start_sending();
        action = SEND;
        TIMSK |= (1 << OCIE0A);				//Turn on interrupt
        while(!sending_done());
        action = NOTHING;
        TIMSK &= ~(1 << OCIE0A);			//Turn off interrupt
        GREEN_OFF;
        _delay_ms(200);
    }
}

void init_irq()
{
    //CTC mode
    TCCR0A |= (1 << WGM01);
    //No prescaling
    TCCR0B |= (1 << CS00);
    OCR0A = (F_CPU / _RATE) - 1;
}

uint16_t read_signal(uint16_t* start, uint16_t* data, uint16_t size)
{
    uint16_t idx = 0;
    uint16_t word = eeprom_read_word(start);
    if(word == 0xFFFF || word == 0xEEEE)
        return 0;
    data[idx] = word;

    idx++;
    while(word != 0xEEEE && idx < size)
    {
        word = eeprom_read_word(start+idx);
        data[idx] = word;
        idx++;
    }
    return idx;
}


void recv_keys(signal_t* signals, uint_fast8_t ssize, uint16_t* ptr, uint_fast8_t size)
{
    uint16_t* addr = 0;
    for(unsigned int i = 0; i < ssize &&  size > 0; i++)
    {
        start_receiving(ptr, size);
        action = RECEIVE;
        TIMSK |= (1 << OCIE0A);			//Turn on interrupt
        while(!(signals[i].size = done()));
        TIMSK &= ~(1 << OCIE0A);			//Turn off interrupt
        action = NOTHING;
        signals[i].ptr = ptr;
        eeprom_write_block(signals[i].ptr, addr, signals[i].size * sizeof(*(signals[i].ptr)));
        addr += signals[i].size;
        ptr += signals[i].size;
        size -= signals[i].size;
    }
}