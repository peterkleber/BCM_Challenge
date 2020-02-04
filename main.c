/*
 * main.c
 *
 *  Created on: Oct 19, 2019
 *      Author: PeterKleber and Ehab
 */

#include"BCM.h"

#define RX_BUFFER_SIZE 1000

static void Received_Frame_Notification(void) ;
static void Transmit_Frame_Notification (void);
	
void Received_Frame_Notification(void) 
{
	//ALL is good
	PORTC |= (1<<PC0);
	LCD_4Bits_Clear();
	LCD_4Bits_Print_String(1,1,"Wsal ya basha");
	BCM_RX_Buffer_Unlock();
}

void Transmit_Frame_Notification (void)
{
	
	PORTC |= (1<<PC1);
	
}
	 
	 
	 
int main (void)
{
	 uint8 RX_Data_Buffer[RX_BUFFER_SIZE];
	 uint8 TX_Data_Buffer[]="Send_Data";
	 uint16 TX_Buffer_Size = sizeof(TX_Data_Buffer)/sizeof(TX_Data_Buffer[0]);

	 LCD_4Bits_Initialization();			//LCD is used for testing only

	 BCM_Init(&BCM_cnfg);
	 
	 
	 BCM_Send( TX_Data_Buffer , TX_Buffer_Size , Transmit_Frame_Notification);
	
	 BCM_Setup_Rx_Buffer(RX_Data_Buffer, RX_BUFFER_SIZE , Received_Frame_Notification);
 
	 
	while(1)
	{
		 BCM_TX_Dispatcher();
		 BCM_RX_Dispatcher();
	}
	
	return 0;

}

