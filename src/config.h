/*
* config.h
*
* Created: 23.01.2016 16:45:33
*  Author: Christian
*/


#ifndef CONFIG_H_
#define CONFIG_H_

#define _BUFFSIZE 288
#define BUFFSIZE (_BUFFSIZE/2)

#define F_CPU 1000000UL
#define _RATE 15000UL
#define FRQ 38000UL

//Max length of pause signal, receive aborts if no signal receives for given time
#define _PAUSE_TIME 8
#define MAX_PAUSE_TIME ((_RATE * _PAUSE_TIME) / 1000)

#define GREEN_ON (PORTB |= (1 << PB2))
#define GREEN_OFF (PORTB &= ~(1 << PB2))
#define GREEN_SWITCH (PORTB ^= (1 << PB2))
#define GREEN_IS_ON (PORTB & (1 << PB2))
//
// #define RED_ON (PORTB |= (1 << PB0))
// #define RED_OFF (PORTB &= ~(1 << PB0))
// #define RED_SWITCH (PORTB ^= (1 << PB0))
// #define RED_IS_ON (PORTB & (1 << PB0))


#define SWITCH1_ON (!(PINB & (1 << PINB0)))
#define SWITCH2_ON (!(PINB & (1 << PINB3)))

//Press both switches for x cycles until reset eeprom
#define RESET_WAIT 50000

//Wait x ms since last received signal to receive next one
#define _SAFE_WAIT 500
#define SAFE_WAIT ((_RATE * _SAFE_WAIT) / 1000)

#endif /* CONFIG_H_ */
