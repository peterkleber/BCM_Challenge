/*
 * BCM.h
 *
 *  Created on: Oct 19, 2019
 *      Author: PeterKleber
 */

#ifndef BCM_H_
#define BCM_H_

#include "std_types.h"
#include"UART.h"


void BCM_Init();

void BCM_Send(uint8 *Buffer_ptr ,uint16 Buffer_Size);

void BCM_receive(uint8 *Buffer_ptr ,uint16 Buffer_Size);

void BCM_RX_dispatcher(void);

void BCM_TX_dispatcher(void);

void BCM_RX_Buffer_Unlock(void);



#endif /* BCM_H_ */
