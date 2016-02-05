/*
* sender.h
*
* Created: 24.01.2016 01:50:58
*  Author: Christian
*/

#ifndef SENDER_H_
#define SENDER_H_

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

void send();
void sender_set(uint16_t* buffer, uint_fast16_t size);
void init_pwm();
bool sending_done();
void start_sending();

#ifdef __cplusplus
}
#endif

#endif