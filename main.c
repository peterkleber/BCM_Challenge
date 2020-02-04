/*
 * main.c
 *
 *  Created on: Oct 19, 2019
 *      Author: PeterKleber and Ehab
 */

#include"BCM.h"


	
void After_Data_Receive() 
{
	//ALL is good
	PORTC = 0xFF;
	LCD_4Bits_Clear();
	LCD_4Bits_Print_String(1,1,"Wsal ya basha");
	BCM_RX_Buffer_Unlock();
}

	 
int main (void)
{
	 volatile uint8 Buffer[1000] = { 1 };
	 uint8 Data_Buffer[]="Send_Data";
	 uint16 Buffer_Size = sizeof(Data_Buffer)/sizeof(Data_Buffer[0]);

	 LCD_4Bits_Initialization();			//LCD is used for testing only

	 BCM_Init(&BCM_cnfg);
	 
	 BCM_RX_Set_CallBack_func(After_Data_Receive);
	 
	 
	 BCM_Send( &Data_Buffer[0] , Buffer_Size);
	
	 BCM_receive(Buffer, 1004);
 
	 
	while(1)
	{
		 BCM_TX_dispatcher();
		 BCM_RX_dispatcher();
	}
	
	return 0;

}
