/*
 * BCM.c
 *
 *  Created on: Oct 19, 2019
 *      Author: PeterKleber
 */

#include"BCM.h"

volatile static uint8 *g_BCM_RX_Buffer_ptr = NULL_PTR;
volatile static uint16 g_BCM_RX_Buffer_Size = 0;

volatile static uint16 g_BCM_RX_Buffer_ISR_index = 0;
static uint8 BCM_RX_Received_Data = 0;

volatile static uint8 BCM_RX_Dispatcer_Start_Flag = 0;
volatile static uint16 BCM_RX_Incoming_Data_Size = 0;

volatile static uint8 BCM_RX_CheckSum = 0;
volatile static uint8 BCM_RX_Received_CheckSum = 0;
volatile static uint8 BCM_RX_Lock_Buffer_Flag = 0;

volatile static uint8 BCM_Full_Data_Received_Flag = 0 ;

void (*g_Callback_Func)() = NULL_PTR;

static void BCM_Receive_ISR() {

	UART_recieve(&BCM_RX_Received_Data); //Receive the data

	if (g_BCM_RX_Buffer_ISR_index == 0) {
		if (BCM_RX_Received_Data != BCM_ID) {
			//The ID is not correct
			BCM_RX_Lock_Buffer_Flag = 1; //lock the buffer
		} else {
			//IF the ID is correct
			BCM_RX_Lock_Buffer_Flag = 0; //unlock the buffer
			g_BCM_RX_Buffer_ISR_index++;
		}
	}

	else if (g_BCM_RX_Buffer_ISR_index == 1){
		BCM_RX_Incoming_Data_Size |= BCM_RX_Received_Data;
		g_BCM_RX_Buffer_ISR_index++;

	}
	else if (g_BCM_RX_Buffer_ISR_index == 2){
		BCM_RX_Incoming_Data_Size |= (BCM_RX_Received_Data << 8);

		if (BCM_RX_Incoming_Data_Size > (g_BCM_RX_Buffer_Size)) {
			//if the incoming data is bigger than the buffer
			g_BCM_RX_Buffer_ISR_index = 0 ; // reset the g_Buffer_ISR_index
			BCM_RX_Lock_Buffer_Flag = 1;//lock the buffer
		}
		else {
			//If the size is OK
			BCM_RX_Lock_Buffer_Flag = 0; //unlock the buffer
			g_BCM_RX_Buffer_ISR_index++;
		}

	}

	else if (g_BCM_RX_Buffer_ISR_index > 2) {
		//Checksum capture

		if (g_BCM_RX_Buffer_ISR_index == (BCM_RX_Incoming_Data_Size + 3)) {
			BCM_RX_Received_CheckSum = BCM_RX_Received_Data ;
			BCM_Full_Data_Received_Flag = 1;
			BCM_RX_Dispatcer_Start_Flag = 1; //Start the Dispatcher
		}

		else if (BCM_RX_Lock_Buffer_Flag != 1) { //Check if the buffer is not locked
			BCM_RX_Dispatcer_Start_Flag = 1; //Start the Dispatcher
			*(g_BCM_RX_Buffer_ptr + (g_BCM_RX_Buffer_ISR_index - 3)) = BCM_RX_Received_Data; // Put the received data into the buffer
			g_BCM_RX_Buffer_ISR_index++;
				}
			}
		}

void BCM_Init() {
	UART_Set_Callback_RX(BCM_Receive_ISR);
	UART_init();
}

void BCM_receive(uint8 *Buffer_ptr, uint16 Buffer_Size) {
	g_BCM_RX_Buffer_ptr = Buffer_ptr;
	g_BCM_RX_Buffer_Size = Buffer_Size;
}

void BCM_RX_dispatcher(void) {

	static uint8 RX_Dispatcer_Counter = 0;

	if (BCM_RX_Dispatcer_Start_Flag == 1) {

		if (BCM_Full_Data_Received_Flag == 1) {

			BCM_Full_Data_Received_Flag = 0 ;
			if (BCM_RX_CheckSum == BCM_RX_Received_CheckSum ){
				//Call the Call back function
				if (g_Callback_Func != NULL_PTR) {
					BCM_RX_Lock_Buffer_Flag = 1;
					g_Callback_Func();
				}
			}
			else{
	        //Checksum error
			}
			//Get ready for the next data coming
			g_BCM_RX_Buffer_ISR_index = 0;

			RX_Dispatcer_Counter = 0;
			BCM_RX_Incoming_Data_Size = 0;

			BCM_RX_CheckSum = 0;
			BCM_RX_Received_CheckSum = 0;

		}
		else {
			//Receiving Data
			BCM_RX_CheckSum += *(g_BCM_RX_Buffer_ptr + RX_Dispatcer_Counter);
			RX_Dispatcer_Counter++;
		}
		BCM_RX_Dispatcer_Start_Flag = 0;
	}
}


void BCM_RX_Set_CallBack_func(void (*callback_func)()) {
	g_Callback_Func = callback_func;
}

void BCM_RX_Buffer_Unlock(void) {
	BCM_RX_Lock_Buffer_Flag = 0;
}
