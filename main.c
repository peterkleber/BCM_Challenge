/*
 * main.c
 *
 *  Created on: Oct 19, 2019
 *      Author: PeterKleber ane Ehab
 */

#include"BCM.h"


	
void After_Data_Receive() 
{
	//ALL is good
	PORTC = 0xFF;
	LCD_4Bits_Print_String(1,1,"Wasal ya Basha");
	BCM_RX_Buffer_Unlock();
}

	 
int main (void)
{
	 volatile uint8 RX_Buffer[1000] = { 1 };

	 LCD_4Bits_Initialization();
	
	 BCM_Init(&BCM_cnfg);
	 
	 BCM_RX_Set_CallBack_func(After_Data_Receive);
	 
	 BCM_receive(RX_Buffer, 1004);
 
	 
	while(1)
	{
		 BCM_RX_dispatcher();
	}
	
	return 0;

}
