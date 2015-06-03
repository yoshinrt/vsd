/*****************************************************************************
	
	VSD2 - vehicle data logger system2
	Copyright(C) by DDS
	
	timer.h -- timer
	
*****************************************************************************/

#ifndef _MAIN2_H
#define _MAIN2_H

#include "dds.h"

/*** macros *****************************************************************/

#define TIMER_HZ	200000
#define LOG_HZ		16.0

/*** const ******************************************************************/
/*** new type ***************************************************************/

typedef struct {
	USHORT	uPrevTime;
	USHORT	uLastTime;
	USHORT	uPulseCnt;
	USHORT	uVal;
} PULSE_t;

/*** prototype **************************************************************/

void TimerInit( void );
UINT GetCurrentTime( void );

/*** extern *****************************************************************/
/*** gloval vars ************************************************************/

#endif	// _MAIN2_H
