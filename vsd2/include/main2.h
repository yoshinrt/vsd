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
#define LOG_HZ		16

#define USART_BAUDRATE	38400

// たぶん，ホイル一周が30パルス
#define PULSE_PER_1KM_NORMAL	(( double )14958.80127 )	// ノーマル
#define PULSE_PER_1KM			(( double )15473.76689 )	// CE28N

#define SRAM_TOP	0x20000000
#define SRAM_END	0x20005000

/*** const ******************************************************************/
/*** new type ***************************************************************/

typedef struct {
	volatile USHORT	uPulseCnt;
	volatile USHORT	uLastTime;
	USHORT	uPrevTime;
	USHORT	uVal;
} PULSE_t;

/*** prototype **************************************************************/

void NvicIntEnable( UINT IRQChannel );
void NvicIntDisable( UINT IRQChannel );
void TimerInit( void );
UINT GetCurrentTime( void );
UINT GetCurrentTime16( void );
void AdcInit( void );
void AdcConversion( void );
void PulseInit( void );
void EXTI0_IRQHandler( void );
void EXTI1_IRQHandler( void );
void EXTI2_IRQHandler( void );
void ComputeMeterTacho( void );
void ComputeMeterSpeed( void );
void ComputeMeter( void );
void LoadSRecord( void );

/*** extern *****************************************************************/

extern PULSE_t	g_Speed;
extern PULSE_t	g_Tacho;
extern UINT		g_uMileage;
extern UINT		g_uLapTime;

/*** gloval vars ************************************************************/

#endif	// _MAIN2_H
