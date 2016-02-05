/*
* rcver.c
*
*  Created on: 22. Jän. 2016
*      Author: Christian
*/

#include <string.h>
#include <stdbool.h>
#include <stddef.h>
#include <avr/io.h>
#include <avr/cpufunc.h>

#include "rcver.h"
#include "config.h"

typedef enum _STATE
{
    START,
    BUSY,
    WAIT,
    DONE_WAIT,
    DONE
} STATE;

static uint_fast16_t* _storage;
static uint_fast16_t _idx = 0;
static uint_fast16_t _sizeRead = 0;
static uint_fast16_t _buffSize = 0;
static STATE _state = START;

static inline bool on()
{
    return !(PINB & (1 << PINB4));
}

void init_rcver()
{
    DDRB &= ~(1 << DDB4);		//Set input pin IR receiver
    PORTB |= (1 << PB4);		//Set pull-up resistor
}

void receive()
{
    static uint_fast16_t pulse_time = 0;
    static uint_fast16_t pause_time = 0;
    static uint_fast16_t waitct = 0;


    if(_state == DONE_WAIT)
    {
        if(on())
            waitct = 0;
        if(waitct++ >= SAFE_WAIT)
        {
            _state = DONE;
            waitct = 0;
        }
        return;
    }

    if(_state == DONE)
    {
        GREEN_OFF;
        return;
    }


    if (_state == START)
    {
        if (on())
        {
//             RED_ON;
            GREEN_ON;
            _state = BUSY;
            pause_time = 0;
            pulse_time = 1;
        }
    }
    else if (_state == BUSY)
    {
        if (on())
        {
            pulse_time++;
        }
        else
        {
            _storage[_idx] = pulse_time;
            _idx++;
            if(_idx >= _buffSize)
            {
                _sizeRead = _buffSize;
//                 RED_OFF;

                _MemoryBarrier();
                _state = DONE_WAIT;
                return;
            }
            pulse_time = 0;
            pause_time = 1;
            _state = WAIT;
        }
    }
    else if (_state == WAIT)
    {
        if (!on())
        {
            pause_time++;
            if (pause_time >= MAX_PAUSE_TIME)  	//abort receive
            {
                _sizeRead = _idx+1;
                _storage[_idx] = 0xEEEE;
                _idx = 0;
                pause_time = 0;
//                 RED_OFF;

                _MemoryBarrier();
                _state = DONE_WAIT;
                return;
            }
        }
        else
        {
            _storage[_idx] = pause_time;
            _idx++;
            if(_idx >= _buffSize)
            {
                _sizeRead = _buffSize;
//                 RED_OFF;

                _MemoryBarrier();
                _state = DONE_WAIT;
                return;
            }
            pause_time = 0;
            pulse_time = 1;
            _state = BUSY;
        }
    }
}


uint16_t done()
{
    if(_state == DONE)
        return _sizeRead;
    return 0;
}

void start_receiving(uint16_t* buffer, uint16_t size)
{
    _state = START;
    _sizeRead = 0;
    _idx = 0;
    _storage = buffer;
    _buffSize = size;
}