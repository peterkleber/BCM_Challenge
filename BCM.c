/*
 * BCM.c
 *
 *  Created on: Oct 19, 2019
 *      Author: PeterKleber and Ehab
 
 */


#include"BCM.h"


//------------------------------ Section of Macros Constant  -------------------------------------//
//#define BCM_ID     'S'
#define BCM_ID      0xA5

#define LOCKED		1
#define UNLOCKED	0

#define LOCK		1
#define UNLOCK		0

#define NO_REQUEST			0
#define REQUEST				1
#define FINISHED_REQUEST	2

#define BUSY		0
#define NOT_BUSY	1

//--------------------------------------------------------------------------------------------------//


//------------------------------ Section of creating variables from structure  ---------------------//

static ST_Frame_t TX_Frame;
static ST_Frame_t RX_Frame;
//--------------------------------------------------------------------------------------------------//


//------------------------------ Section of creating variables from enum ---------------------------//

static Dispatcher_Status_t TX_Dispatcher_Status;		//this is the static variable which is similar to BCM State Machine

//--------------------------------------------------------------------------------------------------//


//------------------------------ Section of Static Global Pointers ---------------------------------//

volatile static uint8 *g_BCM_RX_Buffer_ptr = NULL_PTR;


//--------------------------------------------------------------------------------------------------//


//------------------------------ Section of Static Global Counters ---------------------------------//

static sint16 TX_Dispatcher_Counter = 0;
volatile static uint16 g_BCM_RX_Buffer_ISR_index = 0;

//--------------------------------------------------------------------------------------------------//


//------------------------------ Section of Static Global variables ---------------------------------//

volatile static uint8 BCM_RX_CheckSum = 0;
volatile static uint8 BCM_RX_Received_CheckSum = 0;
volatile static uint16 g_BCM_RX_Buffer_Size = 0;

volatile static uint16 BCM_RX_Incoming_Data_Size = 0;



static volatile RX_State g_u8_Bcm_Rx_Status=RX_IDLE;

static volatile uint8 g_u8_Bcm_Rx_Req_Flag=UNLOCKED;


//--------------------------------------------------------------------------------------------------//


//------------------------------ Section of Static Global Flag -------------------------------------//
//Initial Value for the Flags (Idle State of each Flag)
static uint8 TX_Lock_Flag = UNLOCKED;
static uint8 TX_BCM_Request = NO_REQUEST ;
static uint8 TX_Dispatcher_Busy_Flag = NOT_BUSY;

//--------------------------------------------------------------------------------------------------//


//------------------------------ Section of  Global pointer to function ----------------------------//
static volatile BCM_ptrToFuncTX BCM_ptrConsumerFuncTX= NULL_PTR;
static volatile BCM_ptrToFuncRX BCM_ptrConsumerFuncRX= NULL_PTR;
//--------------------------------------------------------------------------------------------------//


//------------------------------ Section of Private Function ---------------------------------------//

static void TX_BCM_ISR_Handler (void);
static void RX_BCM_ISR_Handler(void);
//--------------------------------------------------------------------------------------------------//


EnmBCMError_t BCM_Init(const BCM_ConfigType *ConfigPtr)
{
	EnmBCMError_t BCM_Error_Status = BCM_OK;

	if(ConfigPtr->Comm_ID == UART)
	{
		UART_init();
		UART_Set_Callback_RX(RX_BCM_ISR_Handler);
		UART_Set_Callback_TX(TX_BCM_ISR_Handler);	
		// UART_Set_Callback_RX(void (*ptr)());
		TX_Dispatcher_Status = TX_IDLE;
	}
	
	else
	{
		BCM_Error_Status = BCM_NOK;
	}
	
	return BCM_Error_Status;

}


/*******************************************************************  BCM_TX  *****************************************************************************************************/


void BCM_Send(uint8 *Buffer_ptr ,uint16 Buffer_Size , BCM_ptrToFuncTX COPY_BCM_ptrConsumerFunc )
{
	//Construct the Header of the Frame as an initiation of the Transmission 
	if(TX_Lock_Flag==UNLOCKED)
	{	
		TX_Frame._BCM_ID = BCM_ID;
		TX_Frame.Data_length = Buffer_Size;
		TX_Frame.Buffer_Data_Ptr= Buffer_ptr;
		TX_Frame.Check_Sum=0;
		TX_BCM_Request = REQUEST;
		BCM_ptrConsumerFuncTX = COPY_BCM_ptrConsumerFunc;
		LCD_4Bits_Print_Number(2,2,Buffer_Size);			//testing the input of Buffer size

	}
	 
	else if (TX_Lock_Flag==LOCKED)
	{
		return;
	}
}


void TX_BCM_ISR_Handler (void)
{
	TX_Dispatcher_Status = SENDING_BYTE_COMPLETE;	
}




void BCM_TX_Dispatcher(void)
{
	static uint16 TX_Buffer_Index = 0;
	static uint8 Current_Byte_test = 0;

	switch (TX_Dispatcher_Status)
	{
		case TX_IDLE:  
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
					if(TX_Dispatcher_Counter == 0)									     // means it's the turn to send BCM_ID of the TX_Frame
					{
						TX_Dispatcher_Busy_Flag = BUSY; 
						UART_send (TX_Frame._BCM_ID);
						Current_Byte_test = TX_Frame._BCM_ID;
					}
						
					else if(TX_Dispatcher_Counter == 1)	                                 // means it's the turn to send Least_Byte_Data_size of the TX_Frame
					{
						TX_Dispatcher_Busy_Flag = BUSY; 
						uint8 Least_Nibble_Data_Length = (uint8) TX_Frame.Data_length;
						//Least_Nibble_Data_Length = 'L';								 //any value for testing only
						UART_send (Least_Nibble_Data_Length);
							
						Current_Byte_test = Least_Nibble_Data_Length;
					}
						
					else if(TX_Dispatcher_Counter == 2)									 // means it's the turn to send Most_Byte_Data_size of the TX_Frame
					{
						TX_Dispatcher_Busy_Flag = BUSY; 
						uint8 Most_Nibble_Data_Length = (uint8) (TX_Frame.Data_length>>8);
						//Most_Nibble_Data_Length = 'H';							     //any value for testing only
						UART_send (Most_Nibble_Data_Length);
							
						Current_Byte_test = Most_Nibble_Data_Length;
							
					}
										
					else																
					{
						TX_Dispatcher_Busy_Flag = BUSY;
						uint8 Current_Byte = *(TX_Frame.Buffer_Data_Ptr + TX_Buffer_Index);		
							
						if (TX_Buffer_Index == ((TX_Frame.Data_length)))						// means it's turn to send the checksum since we reached the end of th data as our first index is zero and last index will be Data Length - 1 and the checksum will be the when we reach the Data Length
						{
							//TX_Frame.Check_Sum = 'C';
							UART_send (TX_Frame.Check_Sum);
							Current_Byte_test = TX_Frame.Check_Sum;
							TX_Buffer_Index++;
						}
							
						else if (TX_Buffer_Index < ((TX_Frame.Data_length)))					// means it's the turn to send the payload Data of the TX_Frame
						{
							
							UART_send(Current_Byte);
								
							TX_Buffer_Index++;
							Current_Byte_test = Current_Byte;
								
							TX_Frame.Check_Sum = TX_Frame.Check_Sum + Current_Byte;
						}
							
					}
	
				}
				
			break;
		}
			
			
		case SENDING_BYTE_COMPLETE:
		{
			static uint8 row =1;	// Used for testing only as it decides the row at which LCD prints	test
			static uint8 col =1;	// Used for testing only as it decides the col at which LCD prints	test
				 
			if (TX_Buffer_Index > ((TX_Frame.Data_length)) )								// Reached the End of the Buffer
			{
				TX_Dispatcher_Status = TX_FRAME_COMPLETE;
				//LCD_4Bits_Print_Character( row, col , Current_Byte_test );				// Used for testing only
				//LCD_4Bits_Print_Number( row, col , Current_Byte_test );
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
					//LCD_4Bits_Print_Number( row, col , Current_Byte_test );
					col++;
			}
				
			break;
		}
			
			
		case TX_FRAME_COMPLETE:
		{
				
			TX_Dispatcher_Status = TX_IDLE;
			TX_BCM_Request = FINISHED_REQUEST;
			TX_Lock_Flag = UNLOCKED ;
			
			
			// Call the Callback_Function related to the user application to process after all the data sent
			// Or Raise a Flag to indicate that the TX_Frame has been sent correctly
			
			BCM_ptrConsumerFuncTX();
			
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


/*******************************************************************  BCM_RX  *****************************************************************************************************/

static void RX_BCM_ISR_Handler(void)
{
	static volatile uint16 Local_u16_Data_Counter=0;
	uint8 Local_u8_RX_Data;
	UART_recieve(&Local_u8_RX_Data);
	
	if(LOCK==g_u8_Bcm_Rx_Req_Flag)
	{

	}
	else if (UNLOCK==g_u8_Bcm_Rx_Req_Flag)
	{
		switch(g_u8_Bcm_Rx_Status)
		{
			case RECEIVING_BCM_ID:
			{
				Local_u16_Data_Counter=0;
				if(Local_u8_RX_Data==RX_Frame._BCM_ID)
				{
					g_u8_Bcm_Rx_Status=RECEIVING_DATA_LENGTH;
				}
				break;
			}
			
			
			case RECEIVING_DATA_LENGTH:
			{
				static uint8 Receiving_Data_length_Counter = 0;
				 
				RX_Frame.Data_length |= (uint16)Local_u8_RX_Data;				//Receive Least Byte
				
				Receiving_Data_length_Counter++;
				
				if(Receiving_Data_length_Counter == 2)
				{
					RX_Frame.Data_length |= (((uint16)Local_u8_RX_Data)<<8);		//Receive Highest Byte					
					g_u8_Bcm_Rx_Status=RECEIVING_DATA;	
					Receiving_Data_length_Counter=0;
				}

				break;
			}
			
			case RECEIVING_DATA:
			{
				if(Local_u16_Data_Counter < RX_Frame.Data_length)
				{
					*(RX_Frame.Buffer_Data_Ptr)=Local_u8_RX_Data;
					RX_Frame.Check_Sum += Local_u8_RX_Data;
					RX_Frame.Buffer_Data_Ptr++;
					Local_u16_Data_Counter++;
				}
				if(Local_u16_Data_Counter == (RX_Frame.Data_length))
				{
					g_u8_Bcm_Rx_Status=RECEIVING_Checksum;
				}
				else
				{

				}
				break;
			}
			
			
			case RECEIVING_Checksum:
			{
				if(RX_Frame.Check_Sum==Local_u8_RX_Data)					//Local_u8_RX_Data now should be the transmitted Checksum from the transmitter
				{			
					g_u8_Bcm_Rx_Status=RX_FRAME_COMPLETE;
				}
				else
				{
					g_u8_Bcm_Rx_Status=RX_FRAME_ERROR;
				}
				break;
			}
			
			default:
			{
				break;
			}
			
		}

	}
}


void BCM_RX_Buffer_Unlock(void)
{
	g_u8_Bcm_Rx_Req_Flag=UNLOCK;
}





void BCM_RX_Dispatcher(void)
{
	
	if(g_u8_Bcm_Rx_Status==RX_FRAME_COMPLETE)
	{

		g_u8_Bcm_Rx_Req_Flag=LOCK;
		g_u8_Bcm_Rx_Status=RX_IDLE;
	
		BCM_ptrConsumerFuncRX();

	}
	else if(g_u8_Bcm_Rx_Status==RX_FRAME_ERROR)
	{
		g_u8_Bcm_Rx_Status=RX_IDLE;
	}
	
	else
	{
		//Do Nothing
	}

}


EnmBCMError_t BCM_Setup_Rx_Buffer(uint8* COPY_ptrRxBuffer,uint16 COPY_u16BufferSize,BCM_ptrToFuncRX COPY_BCM_ptrConsumerFunc)
{
	EnmBCMError_t Local_u8_BCM_Error=bcm_ok;
	if(NULL_PTR==COPY_ptrRxBuffer)
	{
		Local_u8_BCM_Error=invalid_adress;
	}
	else if (0==COPY_u16BufferSize)
	{
		Local_u8_BCM_Error=void_size;
	}
	else if(LOCK==g_u8_Bcm_Rx_Req_Flag)
	{
		Local_u8_BCM_Error=locked_buffer;
	}
	else if(NULL_PTR == COPY_BCM_ptrConsumerFunc)
	{
		Local_u8_BCM_Error=invalid_adress;
	}
	else
	{
		RX_Frame._BCM_ID=BCM_ID;
		RX_Frame.Data_length=0;
		RX_Frame.Buffer_Data_Ptr=COPY_ptrRxBuffer;
		RX_Frame.Check_Sum=0;
		BCM_ptrConsumerFuncRX = COPY_BCM_ptrConsumerFunc;
	}
	return Local_u8_BCM_Error;
}



