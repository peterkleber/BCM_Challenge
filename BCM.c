/*
 * BCM.c
 *
 *  Created on: Oct 19, 2019
 *      Author: PeterKleber
 */

#include"BCM.h"

void BCM_Init(){

	// UART_Set_Callback_RX(void (*ptr)());

	// UART_Set_Callback_TX(void (*ptr)());

	UART_init();

}

void BCM_Send(uint8 *Buffer_ptr ,uint16 Buffer_Size){

}

void BCM_receive(uint8 *Buffer_ptr ,uint16 Buffer_Size){

}

void BCM_RX_dispatcher(void){

}

void BCM_TX_dispatcher(void){

}

void BCM_RX_Buffer_Unlock(void){

}
