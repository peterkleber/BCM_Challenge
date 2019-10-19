/*
 * main.c
 *
 *  Created on: Oct 19, 2019
 *      Author: PeterKleber
 */

#include "std_types.h"
#include"BCM.h"

volatile uint8 Buffer[1004] = { 0 };

void After_Data_Receive() {

	PORTA = 0xFF;

	BCM_RX_Buffer_Unlock();
}

int main() {

	DDRA = 0xFF;

	BCM_RX_Set_CallBack_func(After_Data_Receive);
	BCM_Init();

	BCM_receive(Buffer, 1004);

	while (1) {
		BCM_RX_dispatcher();
	}
}
