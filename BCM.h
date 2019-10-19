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

#define BCM_ID (uint8)(0xA5)

void BCM_Init();

void BCM_Send(uint8 *Buffer_ptr ,uint16 Buffer_Size);

void BCM_receive(uint8 *Buffer_ptr ,uint16 Buffer_Size);

void BCM_RX_dispatcher(void);

void BCM_TX_dispatcher(void);

void BCM_RX_Buffer_Unlock(void);

void BCM_RX_Set_CallBack_func(void (*callback_func)(void));


#endif /* BCM_H_ */
