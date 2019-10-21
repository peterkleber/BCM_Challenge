/*
 * BCM.h
 *
 *  Created on: Oct 19, 2019
 *     Author: PeterKleber and Ehab
 */

#ifndef BCM_H_
#define BCM_H_

#include "std_types.h"
#include "BCM_cnfg.h"
#include"UART.h"
#include "LCD.h"

//------------------------------ Section of Structure  ------------------------------------------//

typedef struct
{
	uint8 _BCM_ID;
	uint16 Data_length;
	uint8 *Buffer_Ptr;
	uint8 Check_Sum;
} ST_Frame_t ;

//--------------------------------------------------------------------------------------------------//

//------------------------------ Section of GLobal Variables ---------------------------------------//


//--------------------------------------------------------------------------------------------------//


//------------------------------ Section of enum ---------------------------------------------------//

//typedef enum { IDLE=0, SENDING_BYTE, SENDING_BYTE_COMPLETE , FRAME_COMPLETE} Dispatcher_Status_t;

typedef enum {BCM_NOK, BCM_OK} EnmBCMError_t ;

//--------------------------------------------------------------------------------------------------//



//------------------------------ Section of Public Function Prototype API's ------------------------//

EnmBCMError_t BCM_Init(const BCM_ConfigType *ConfigPtr);

void BCM_Send(uint8 *Buffer_ptr , uint16 Buffer_Size);

void BCM_receive(uint8 *Buffer_ptr ,uint16 Buffer_Size);

void BCM_RX_dispatcher(void);

void BCM_TX_dispatcher(void);

void BCM_RX_Buffer_Unlock(void);

void BCM_RX_Set_CallBack_func(void (*callback_func)());

//--------------------------------------------------------------------------------------------------//


#endif /* BCM_H_ */
