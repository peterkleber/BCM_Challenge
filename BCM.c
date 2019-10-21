/*
 * BCM.c
 *
 *  Created on: Oct 19, 2019
 *      Author: Ehab
 
 */

#include"BCM.h"


//------------------------------ Section of Macros Constant  -------------------------------------//
//#define BCM_ID     'S'
#define BCM_ID      0xA5

#define LOCKED		1
#define UNLOCKED	0


#define NO_REQUEST			0
#define REQUEST				1
#define FINISHED_REQUEST	2

#define BUSY		0
#define NOT_BUSY	1

//--------------------------------------------------------------------------------------------------//
	
	
//------------------------------ Section of creating variables from structure  ---------------------//

static ST_Frame_t Frame;

//--------------------------------------------------------------------------------------------------//


//------------------------------ Section of creating variables from enum ---------------------------//
	
static Dispatcher_Status_t TX_Dispatcher_Status;		//this is the static variable which is similar to BCM State Machine

//--------------------------------------------------------------------------------------------------//


//------------------------------ Section of Static Global Counters ---------------------------------//
	
static uint16 TX_Dispatcher_Counter = 0;
//--------------------------------------------------------------------------------------------------//


//------------------------------ Section of Static Global Flag -------------------------------------//
//Initial Value for the Flags (Idle State of each Flag)
static uint8 TX_Lock_Flag = UNLOCKED;			
static uint8 TX_BCM_Request = NO_REQUEST ;		
static uint8 TX_Dispatcher_Busy_Flag = NOT_BUSY;

//--------------------------------------------------------------------------------------------------//


//------------------------------ Section of Private Function ---------------------------------------//

static void TX_BCM_ISR_Handling (void);

//--------------------------------------------------------------------------------------------------//


EnmBCMError_t BCM_Init(const BCM_ConfigType *ConfigPtr)
{
	EnmBCMError_t BCM_Error_Status = BCM_OK;

	if(ConfigPtr->Comm_ID == UART)
	{
		UART_init();
		UART_Set_Callback_TX(TX_BCM_ISR_Handling);	
		// UART_Set_Callback_RX(void (*ptr)());
		TX_Dispatcher_Status = IDLE;
	}
	
	else
	{
		BCM_Error_Status = BCM_NOK;
	}
	
	return BCM_Error_Status;

}

void BCM_Send(uint8 *Buffer_ptr ,uint16 Buffer_Size)
{
	//Construct the Header of the Frame as an initiation of the Transmission 
	if(TX_Lock_Flag==UNLOCKED)
	{	
		Frame._BCM_ID = BCM_ID;
		Frame.Data_length = Buffer_Size;
		Frame.Buffer_Ptr= Buffer_ptr;
		Frame.Check_Sum=0;
		TX_BCM_Request = REQUEST;
		
		LCD_4Bits_Print_Number(2,2,Buffer_Size);			//testing the input of Buffer size

	}
	 
	else if (TX_Lock_Flag==LOCKED)
	{
		return;
	}
}


void TX_BCM_ISR_Handling (void)
{
	TX_Dispatcher_Status = SENDING_BYTE_COMPLETE;	
}




void BCM_TX_dispatcher(void)
{
	static uint16 TX_Buffer_Index = 0;
	static uint8 Current_Byte_test = 0;

	switch (TX_Dispatcher_Status)
	{
		case IDLE:  
		{	
			TX_Buffer_Index = 0;
			TX_Dispatcher_Counter=0;
			TX_Dispatcher_Busy_Flag = NOT_BUSY;
			
			if(TX_BCM_Request == REQUEST)
			{
				TX_Dispatcher_Status = SENDING_BYTE;
				TX_Lock_Flag = LOCKED;
			}
				 
			break;
		}
			
			
		case SENDING_BYTE:
		{
			if(TX_Dispatcher_Busy_Flag == NOT_BUSY)										//this flag is used to avoid calling of UART_Send function since the dispatcher is faster than UART_Sending byte by hardware which lead to overwrite to the UDR and cause data overrun
				{	
					if(TX_Dispatcher_Counter == 0)									     // means it's the turn to send BCM_ID of the Frame
					{
						TX_Dispatcher_Busy_Flag = BUSY; 
						UART_send (Frame._BCM_ID);
						Current_Byte_test = Frame._BCM_ID;
					}
						
					else if(TX_Dispatcher_Counter == 1)	                                 // means it's the turn to send Least_Nibble_Data_size of the Frame
					{
						TX_Dispatcher_Busy_Flag = BUSY; 
						uint8 Least_Nibble_Data_Length = (uint8) Frame.Data_length;
						//Least_Nibble_Data_Length = 'L';								 //any value for testing only
						UART_send (Least_Nibble_Data_Length);
							
						Current_Byte_test = Least_Nibble_Data_Length;
					}
						
					else if(TX_Dispatcher_Counter == 2)									 // means it's the turn to send Most_Nibble_Data_size of the Frame
					{
						TX_Dispatcher_Busy_Flag = BUSY; 
						uint8 Most_Nibble_Data_Length = (uint8) (Frame.Data_length>>8);
						//Most_Nibble_Data_Length = 'H';							     //any value for testing only
						UART_send (Most_Nibble_Data_Length);
							
						Current_Byte_test = Most_Nibble_Data_Length;
							
					}
										
					else																
					{
						TX_Dispatcher_Busy_Flag = BUSY;
						uint8 Current_Byte = *(Frame.Buffer_Ptr + TX_Buffer_Index);		
							
						if (TX_Buffer_Index == ((Frame.Data_length)))						// means it's turn to send the checksum since we reached the end of th data as our first index is zero and last index will be Data Length - 1 and the checksum will be the when we reach the Data Length
						{
							//Frame.Check_Sum = 'C';
							UART_send (Frame.Check_Sum);
							Current_Byte_test = Frame.Check_Sum;
							TX_Buffer_Index++;
						}
							
						else if (TX_Buffer_Index < ((Frame.Data_length)))					// means it's the turn to send the payload Data of the Frame
						{
							
							UART_send(Current_Byte);
								
							TX_Buffer_Index++;
							Current_Byte_test = Current_Byte;
								
							Frame.Check_Sum = Frame.Check_Sum + Current_Byte;
						}
							
					}
	
				}
				
			break;
		}
			
			
		case SENDING_BYTE_COMPLETE:
		{
			static uint8 row =1;	// Used for testing only as it decides the row at which LCD prints
			static uint8 col =1;	// Used for testing only as it decides the col at which LCD prints
				 
			if (TX_Buffer_Index > ((Frame.Data_length)) )								// Reached the End of the Buffer
			{
				TX_Dispatcher_Status = FRAME_COMPLETE;
				LCD_4Bits_Print_Character( row, col , Current_Byte_test );				// Used for testing only
			}
				
			else
			{		
					TX_Dispatcher_Status = SENDING_BYTE;
					 
					if( (TX_Dispatcher_Counter >= 0) && (TX_Dispatcher_Counter <= 2 )	)  // we don't need the TX_Dispatcher_Counter to increment more than this boundary which is used to send the header of the fram
					{
					TX_Dispatcher_Counter++;
					}
					 
					TX_Dispatcher_Busy_Flag = NOT_BUSY; 
					 
				//this section below is used for testing only the payload of the data 

					LCD_4Bits_Print_Character( row, col , Current_Byte_test );			   // Used for testing only
					col++;
			}
				
			break;
		}
			
			
		case FRAME_COMPLETE:
		{
				
			TX_Dispatcher_Status = IDLE;
			TX_BCM_Request = FINISHED_REQUEST;
			TX_Lock_Flag = UNLOCKED ;
				
			// Call the Callback_Function related to the user application to process after all the data sent
			// Or Raise a Flag to indicate that the Frame has been sent correctly
				
			break;
		}
			
			
		default:
		{
			// Shouldn't be here
			// Do Nothing
			break;
		}
			
	}
	
	return;
}

