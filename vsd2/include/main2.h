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
	volatile USHORT	uPrevTime;
	volatile USHORT	uLastTime;
	volatile USHORT	uPulseCnt;
	volatile USHORT	uVal;
} PULSE_t;

/*** prototype **************************************************************/

void TimerInit( void );
UINT GetCurrentTime( void );
void PulseInit( void );
void EXTI0_IRQHandler( void );
void EXTI1_IRQHandler( void );
void EXTI2_IRQHandler( void );
void ComputeMeterTacho( void );
void ComputeMeterSpeed( void );

/*** extern *****************************************************************/

extern PULSE_t	g_Speed;
extern PULSE_t	g_Tacho;
extern UINT		g_uLapTime;

/*** gloval vars ************************************************************/

#endif	// _MAIN2_H
