/*
 * BCM.c
 *
 *  Created on: Oct 19, 2019
 *      Author: PeterKleber
 */

#include"BCM.h"

volatile static uint8 *g_Buffer_ptr = NULL_PTR;
volatile static uint16 g_Buffer_Size = 0;

volatile static uint16 g_Buffer_ISR_index = 0;
static uint8 Received_Data = 0;

volatile static uint8 BCM_RX_Dispatcer_Start_Flag = 0;
volatile static uint16 BCM_Incoming_Data_Size = 0;

volatile static uint8 BCM_CheckSum = 0;
volatile static uint8 BCM_Received_CheckSum = 0;
volatile static uint8 BCM_Lock_Buffer_Flag = 0;

volatile static uint8 BCM_Full_Data_Received_Flag = 0 ;

void (*g_Callback_Func)() = NULL_PTR;

static void BCM_Receive_ISR() {

	UART_recieve(&Received_Data); //Receive the data

	if (g_Buffer_ISR_index == 0) {
		if (Received_Data != BCM_ID) {
			//The ID is not correct
			PORTC |= (1 << PC0);
			BCM_Lock_Buffer_Flag = 1; //lock the buffer
		} else {
			//IF the ID is correct
			PORTC &= ~(1 << PC0);
			BCM_Lock_Buffer_Flag = 0; //unlock the buffer
			g_Buffer_ISR_index++;
		}
	}

	else if (g_Buffer_ISR_index == 1){
		BCM_Incoming_Data_Size |= Received_Data;
		g_Buffer_ISR_index++;

	}
	else if (g_Buffer_ISR_index == 2){
		BCM_Incoming_Data_Size |= (Received_Data << 8);

		if (BCM_Incoming_Data_Size > (g_Buffer_Size)) {
			//if the incoming data is bigger than the buffer
			PORTC |= (1 << PC1);
			g_Buffer_ISR_index = 0 ; // reset the g_Buffer_ISR_index
			BCM_Lock_Buffer_Flag = 1;//lock the buffer
		}
		else {
			//If the size is OK
			BCM_Lock_Buffer_Flag = 0; //unlock the buffer
			PORTC &= ~(1 << PC1);
			g_Buffer_ISR_index++;
		}

	}

	else if (g_Buffer_ISR_index > 2) {
		//Checksum capture

		if (g_Buffer_ISR_index == (BCM_Incoming_Data_Size + 3)) {
			BCM_Received_CheckSum = Received_Data ;
			BCM_Full_Data_Received_Flag = 1;
			BCM_RX_Dispatcer_Start_Flag = 1; //Start the Dispatcher
		}

		else if (BCM_Lock_Buffer_Flag != 1) { //Check if the buffer is not locked
			BCM_RX_Dispatcer_Start_Flag = 1; //Start the Dispatcher
			*(g_Buffer_ptr + (g_Buffer_ISR_index - 3)) = Received_Data; // Put the received data into the buffer
			g_Buffer_ISR_index++;
				}
			}
		}

void BCM_Init() {

	DDRA = 0xFF;

	UART_Set_Callback_RX(BCM_Receive_ISR);
	UART_init();
}

void BCM_receive(uint8 *Buffer_ptr, uint16 Buffer_Size) {
	g_Buffer_ptr = Buffer_ptr;
	g_Buffer_Size = Buffer_Size;
}

void BCM_RX_dispatcher(void) {

	static uint8 RX_Dispatcer_Counter = 0;

	if (BCM_RX_Dispatcer_Start_Flag == 1) {

		if (BCM_Full_Data_Received_Flag == 1) {

			BCM_Full_Data_Received_Flag = 0 ;
			PORTC &= ~(1 << PC2);
			if (BCM_CheckSum == BCM_Received_CheckSum ){
				//Call the Call back function
				if (g_Callback_Func != NULL_PTR) {
					BCM_Lock_Buffer_Flag = 1;
					g_Callback_Func();
				}
			}
			else{
				PORTC |= (1 << PC2);
			}
			//Get ready for the next data coming
			g_Buffer_ISR_index = 0;

			RX_Dispatcer_Counter = 0;
			BCM_Incoming_Data_Size = 0;

			BCM_CheckSum = 0;
			BCM_Received_CheckSum = 0;

		}
		else {
			//Receiving Data
			BCM_CheckSum += *(g_Buffer_ptr + RX_Dispatcer_Counter);
			RX_Dispatcer_Counter++;
		}
		PORTC ^= (1 << PC7); //Check if the dispatcher is running
		BCM_RX_Dispatcer_Start_Flag = 0;
	}
}


void BCM_RX_Set_CallBack_func(void (*callback_func)()) {
	g_Callback_Func = callback_func;
}

void BCM_RX_Buffer_Unlock(void) {
	BCM_Lock_Buffer_Flag = 0;
}
