/*
 * main.c
 *
 *  Created on: Oct 19, 2019
 *      Author: PeterKleber
 */

#include"BCM.h"


	 
int main (void)
{
	 uint8 Data_Buffer[]="HELLO";
	 uint16 Buffer_Size = sizeof(Data_Buffer)/sizeof(Data_Buffer[0]);

	 LCD_4Bits_Initialization();		//LCD is used for testing only
	 LCD_4Bits_Print_String(1,2,"HEY");
	
	 DDRC = 0xFF;					    //test port 
	 
	 BCM_Init(&BCM_cnfg);
	 
	 BCM_Send( &Data_Buffer[0] , Buffer_Size);
	
	while(1)
	{
		 BCM_TX_dispatcher();

	}
	
	return 0;

}
