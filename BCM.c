/*
 * BCM.c
 *
 *  Created on: Oct 19, 2019
 *      Author: PeterKleber
 */

#include"BCM.h"

static uint8 *g_Buffer_ptr = NULL_PTR;
static uint16 g_Buffer_Size = 0;
static uint16 g_Buffer_index = 0;

static uint8 Received_Data = 0;

static uint8 BCM_RX_Dispatcer_Start_Flag = 0;
static uint8 BCM_ID_Check_Flag = 1;
static uint8 BCM_Incoming_Data_Too_Big_Flag = 0;
static uint8 BCM_CheckSum_Error_Flag = 0;

static uint8 BCM_Lock_Buffer_Flag = 0;

void (*g_Callback_Func)(void) = NULL_PTR;

static void BCM_Receive_ISR() {
	if (BCM_ID_Check_Flag == 1) { //Check if the incoming data is for the BCM
		if (BCM_Incoming_Data_Too_Big_Flag != 1) {
			if (BCM_Lock_Buffer_Flag != 1) {

				UART_recieve(&Received_Data); //Receive the data

				*(g_Buffer_ptr + g_Buffer_index) = Received_Data; // Put the received data into the buffer

				if (g_Buffer_index == 0) {
					BCM_RX_Dispatcer_Start_Flag = 1; //Start the Dispatcher with the first interrupt
				}

				g_Buffer_index++; // increment the index
			}
		}
	}
}

void BCM_Init() {
	UART_Set_Callback_RX(BCM_Receive_ISR);
	UART_init();
}

void BCM_receive(uint8 *Buffer_ptr, uint16 Buffer_Size) {
	g_Buffer_ptr = Buffer_ptr;
	g_Buffer_Size = Buffer_Size;
}

void BCM_RX_dispatcher(void) {
	if (BCM_RX_Dispatcer_Start_Flag) {

		static uint16 BCM_RX_dispatcher_Counter = 0;
		static uint16 BCM_Incoming_Data_Size = 0;
		static uint8 BCM_CheckSum = 0;

		if (BCM_RX_dispatcher_Counter == 0) { //Check the first byte in the buffer for the BCM ID
			//Check the BCM ID
			if (*(g_Buffer_ptr + 0) != BCM_ID) {
				BCM_ID_Check_Flag = 0; //Set BCM Check flag to false
			}
			BCM_RX_dispatcher_Counter++;
		}

		//Check the second and third byte for the incoming data size
		else if (BCM_RX_dispatcher_Counter == 1) {

			BCM_Incoming_Data_Size |=
					*(g_Buffer_ptr + BCM_RX_dispatcher_Counter);
			//set the first byte of the BCM_RX_dispatcher_Counter
			BCM_RX_dispatcher_Counter++;
		} else if (BCM_RX_dispatcher_Counter == 2) {

			BCM_Incoming_Data_Size |= (*(g_Buffer_ptr
					+ BCM_RX_dispatcher_Counter) << 8);
			//set the second byte of the BCM_RX_dispatcher_Counter
			if (BCM_Incoming_Data_Size > (g_Buffer_Size - 4)) {
				//if the incoming data is bigger than the buffer (4 is for the frame )
				BCM_Incoming_Data_Too_Big_Flag = 1;
			}
			BCM_RX_dispatcher_Counter++;
		}

		else if (g_Buffer_index == (BCM_Incoming_Data_Size + 4)) {
			//if we received the full data
			//Check ChechkSum
			if (BCM_CheckSum == *(g_Buffer_ptr + (g_Buffer_index - 1))) {
				//Check ChechkSum Byte
				BCM_Lock_Buffer_Flag = 1;
				g_Callback_Func();

			} else {
				BCM_CheckSum_Error_Flag = 1;
			}
		}

		else {
			//Receiving Data
			BCM_CheckSum += BCM_CheckSum;
			BCM_RX_dispatcher_Counter++;
		}
	}
}
void BCM_RX_Set_CallBack_func(void (*callback_func)(void)) {
	g_Callback_Func = callback_func;
}

void BCM_RX_Buffer_Unlock(void) {
	BCM_Lock_Buffer_Flag = 0;
}
