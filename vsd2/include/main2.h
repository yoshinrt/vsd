/*****************************************************************************
	
	VSD2 - vehicle data logger system2
	Copyright(C) by DDS
	
	timer.h -- timer
	
*****************************************************************************/

#ifndef _TIMER_H
#define _TIMER_H

#include "dds.h"

typedef struct {
	UINT	uPrevTime;
	UINT	uLastTime;
	USHORT	uPulseCnt;
	USHORT	uVal;
} PULSE_t;

void TimerInit( void );
UINT GetCurrentTime( void );

#endif	// _TIMER_H
