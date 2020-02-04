/*
 * TMU_cnfg.h
 *
 *  Created on: Oct 12, 2019
 *       Author: PeterKleber and Ehab
 */

#ifndef BCM_CNFG_H_
#define BCM_CNFG_H_

#include"std_types.h"

typedef enum { UART=0,SPI,I2C}Comm_ID_t;



typedef struct 
{
	Comm_ID_t Comm_ID;	
} BCM_ConfigType  ;


extern BCM_ConfigType BCM_cnfg ;

#endif /* BCM_CNFG_H_ */
