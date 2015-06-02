/*****************************************************************************
	
	VSD2 - vehicle data logger system2
	Copyright(C) by DDS
	
	speed.c -- speed, tacho, ラップセンサ
	
*****************************************************************************/

#include "stm32f10x_rcc.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_timer.h"
#include "dds.h"
#include "main2.h"

/*** 初期化 *****************************************************************/
// { TIM3, TIM2 } を 32bit タイマとして連結する

void TimerInit( void ){
	// APB クロック
	RCC_APB1PeriphClockCmd( RCC_APB1Periph_TIM2 | RCC_APB1Periph_TIM3, ENABLE );
	
	TIM_DeInit( TIM2 );
	TIM_DeInit( TIM3 );
	
	// TimeBase 初期化
	TIM2->APR = 0; TIM3->APR = 0;	// カウンタリロード値
	TIM2->PSC = 0; TIM3->PSC = 0;	// プリスケーラ
	TIM2->CR2 = ( 2 << 4 );			// TIM2 更新イベントを選択
	TIM3->SMCR = ( 1 << 4 ) |		// TIM3 は入力として TIM2 を選択
				7;					// TRGI rise edge でカウント
	TIM2->CR1 = TIM3->CR1 =
		( 1 << 7 ) |	// auto preload
		1;				// counter enable
}

/*** 現在時刻取得 *************************************************/

UINT GetCurrentTime( void ){
	UINT uTimeL, uTimeL2, uTimeH;
	
	uTimeL2 = TIM2->CNT;
	do{
		uTimeL = uTimeL2;
		uTimeH = TIM3->CNT;
	}while( uTimeL > ( uTimeL2 = TIM2->CNT ))
	
	return ( uTimeH << 16 ) | uTimeL;
}
/*** 初期化 *****************************************************************/
// { TIM3, TIM2 } を 32bit タイマとして連結する

void PulseInit( void ){
	// APB クロック
	RCC_APB1PeriphClockCmd( RCC_APB1Periph_TIM2 | RCC_APB1Periph_TIM3, ENABLE );
	
	TIM_DeInit( TIM2 );
	TIM_DeInit( TIM3 );
	
	// TimeBase 初期化
	TIM2->APR = 0; TIM3->APR = 0;	// カウンタリロード値
	TIM2->PSC = 0; TIM3->PSC = 0;	// プリスケーラ
	TIM2->CR2 = ( 2 << 4 );			// TIM2 更新イベントを選択
	TIM3->SMCR = ( 1 << 4 ) |		// TIM3 は入力として TIM2 を選択
				7;					// TRGI rise edge でカウント
	TIM2->CR1 = TIM3->CR1 =
		( 1 << 7 ) |	// auto preload
		1;				// counter enable
}

typedef struct PULSE_t {
	UINT	uPrevTime;
	UINT	uLastTime;
	USHORT	uPulseCnt;
	USHORT	uVal;
}

/*** スピードパルス *********************************************************/

PULSE_t	SpeedPulse;

void EXTI0_IRQHandler( void ){
	SpeedPulse.uLastTime	= GetCurrentTime();
	++SpeedPulse.uPulseCnt;
}

/*** タコパルス *************************************************************/

PULSE_t	TachoPulse;

void EXTI0_IRQHandler( void ){
	TachoPulse.uLastTime	= GetCurrentTime();
	++TachoPulse.uPulseCnt;
}

