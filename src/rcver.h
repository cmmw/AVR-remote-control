/*
 * rcver.h
 *
 *  Created on: 22. Jän. 2016
 *      Author: Christian
 */

#ifndef SRC_RCVER_H_
#define SRC_RCVER_H_


#ifdef __cplusplus
extern "C"
{
#endif

void init_rcver();
void receive();
void start_receiving(uint16_t* buffer, uint16_t size);

//returns nr of read words
uint16_t done();

#ifdef __cplusplus
}
#endif

#endif /* SRC_RCVER_H_ */
