/*
* sender.c
*
* Created: 24.01.2016 01:50:41
*  Author: Christian
*/

#define START_PWM (TCCR1 |= (1 << COM1A1))
#define STOP_PWM (TCCR1 &= ~(1 << COM1A1))

#include <string.h>
#include <avr/interrupt.h>
#include "sender.h"
#include "config.h"



enum CMD
{
    START,
    PULSE,
    PAUSE,
    NOTHING,
    DONE
};


static const uint16_t* _buffer;
static uint_fast16_t _size;
static enum CMD cmd = NOTHING;


void send()
{
    static uint8_t idx = 0;
    static uint16_t duration = 0;

    if(cmd == NOTHING || cmd == DONE)
    {
        return;
    }

    if(cmd == START)
    {
        cmd = PULSE;
        idx = 0;
        duration = _buffer[idx];
        START_PWM;
    }
    else if(cmd == PULSE)
    {
        duration--;
        if(duration == 0)
        {
            STOP_PWM;
            idx++;
            if(idx >= _size)
            {
                cmd = DONE;
                return;
            }
            cmd = PAUSE;
            duration = _buffer[idx];
        }
    }
    else if(cmd == PAUSE)
    {
        duration--;
        if(duration == 0)
        {
            idx++;
            if(idx >= _size)
            {
                cmd = DONE;
                return;
            }
            cmd = PULSE;
            duration = _buffer[idx];
            START_PWM;
        }
    }
}

void sender_set(uint16_t* buffer, uint_fast16_t size)
{
    _buffer = buffer;
    _size = size;
}

void init_pwm()
{
    //Set pin PB1(OC1A) as output (ATTiny85)
    DDRB |= (1 << DDB1);

    //No prescale
    TCCR1 |= (1 << CS10);
    //PWM mode
    TCCR1 |= (1 << PWM1A);
    //Top
    OCR1C = (F_CPU / FRQ) - 1;
    //Duty cycle
    OCR1A = ((OCR1C + 1) / 2) - 1;
    //Clear OC1A on compare match, set when TCNT1 = $00		(Set in macro START_PWM/STOP_PWM)
//     TCCR1 |= (1 << COM1A1);
}

bool sending_done()
{
    return cmd == DONE;
}


void start_sending()
{
    cmd = START;
}