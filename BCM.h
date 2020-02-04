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
#include "LCD.h"		//test


//------------------------------ Section of Structure  ------------------------------------------//

typedef struct
{
	uint8 _BCM_ID;
	uint16 Data_length;
	uint8 *Buffer_Data_Ptr;
	uint8 Check_Sum;
} ST_Frame_t ;


typedef enum
{
	bcm_ok=0,
	BCM_OK=0,
	bcm_nok=1,
	BCM_NOK=1,
	repeted_init,
	bcm_busy,
	void_size,
	invalid_adress,
	repeted_deinit,
	locked_buffer,

}EnmBCMError_t;

typedef enum
{
	RX_IDLE=0,
	RECEIVING_BCM_ID=0,
	RECEIVING_DATA_LENGTH,
	RECEIVING_DATA,
	RECEIVING_Checksum,
	RX_FRAME_COMPLETE,
	RX_FRAME_ERROR
}RX_State;

typedef enum { TX_IDLE=0, SENDING_BYTE, SENDING_BYTE_COMPLETE , TX_FRAME_COMPLETE} Dispatcher_Status_t;

typedef void (*BCM_ptrToFuncTX)(void);
typedef void (*BCM_ptrToFuncRX)(void);
//--------------------------------------------------------------------------------------------------//

//------------------------------ Section of GLobal Variables ---------------------------------------//


//--------------------------------------------------------------------------------------------------//


//------------------------------ Section of enum ---------------------------------------------------//

//typedef enum { IDLE=0, SENDING_BYTE, SENDING_BYTE_COMPLETE , FRAME_COMPLETE} Dispatcher_Status_t;

//typedef enum {BCM_NOK, BCM_OK} EnmBCMError_t ;

//--------------------------------------------------------------------------------------------------//



//------------------------------ Section of Public Function Prototype API's ------------------------//

EnmBCMError_t BCM_Init(const BCM_ConfigType *ConfigPtr);

void BCM_Send(uint8 *Buffer_ptr ,uint16 Buffer_Size , BCM_ptrToFuncTX COPY_BCM_ptrConsumerFunc );

EnmBCMError_t BCM_Setup_Rx_Buffer(uint8* COPY_ptrRxBuffer,uint16 COPY_u16BufferSize,BCM_ptrToFuncRX COPY_BCM_ptrConsumerFunc);

void BCM_RX_Dispatcher(void);

void BCM_TX_Dispatcher(void);

void BCM_RX_Buffer_Unlock(void);

//--------------------------------------------------------------------------------------------------//


#endif /* BCM_H_ */
