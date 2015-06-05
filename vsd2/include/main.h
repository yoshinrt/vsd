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
#define PULSE_PER_1KM	(( double )15473.76689 )	// CE28N
//#define PULSE_PER_1KM	(( double )14958.80127 )	// ノーマル

#define SRAM_TOP	0x20000000
#define SRAM_END	0x20005000

// AD 変換機レジスタ
#define	G_SENSOR_X		ADC_GetInjectedConversionValue( ADC1, ADC_InjectedChannel_1 )
#define	G_SENSOR_Y		ADC_GetInjectedConversionValue( ADC1, ADC_InjectedChannel_2 )
#define	G_SENSOR_Z		ADC_GetInjectedConversionValue( ADC1, ADC_InjectedChannel_3 )
#define	ADC_THROTTLE	ADC_GetInjectedConversionValue( ADC1, ADC_InjectedChannel_4 )
//#define	ADC_BRAKE	

// G センサの 1G 値
#define GX_1G			5400
#define GY_1G			5400
#define GZ_1G			5400

#define LedOn()		( GPIOC->ODR |= 0x40 )
#define LedOff()	( GPIOC->ODR &= ~0x40 )
#define LedToggle()	( GPIOC->ODR ^= 0x40 )

/*** const ******************************************************************/

enum {
	MODE_LAPTIME,
	MODE_ZERO_FOUR,
	MODE_ZERO_ONE,
	MODE_ZERO_ONE_WAIT,	// 0-100 ゴール待ち
};

/*** new type ***************************************************************/

typedef struct {
	volatile USHORT	uPulseCnt;
	volatile USHORT	uLastTime;
	USHORT	uPrevTime;
	USHORT	uVal;
} PULSE_t;

typedef struct {
	UINT	uLapTime;						// ラップタイム
	UINT	uInputParam;					// シリアル入力値
	UINT	uComputeMeterConst;				// スピード計算定数
	
	// 上詰めなので下に追記する ★ほんまか?
	struct {
		UCHAR	uLapMode		:2;
		
		BOOL	bNewLap			:1;
		BOOL	bConnected		:1;
	} Flags;
	
	PULSE_t	Tacho;							// スピードパルス
	PULSE_t	Speed;							// タコパルス
	USHORT	uMileage;						// 走行距離
	USHORT	uGx, uGy;						// G
	USHORT	uThrottle;						// アクセルペダル
	
	USHORT	uStartGTh;						// 発進 G 加速スレッショルド
	USHORT	uOutputPrevTime;				// シリアル出力 prev time
	
	USHORT	uRemainedMileage;				// ラップ開始までの距離
	USHORT	uMileage_0_400;					// 0-400m のパルス数
	
	UCHAR	uCalibTimer;					// キャリブレーションタイマー
	UCHAR	Reserved1;						// ★ROM 化の時消す
	UCHAR	uLogHz;							// ログ Hz
	
} VSD_DATA_t;

/*** prototype **************************************************************/

void EXTI0_IRQHandler( void );
void EXTI1_IRQHandler( void );
void EXTI2_IRQHandler( void );

/*** extern *****************************************************************/

extern VSD_DATA_t	*g_pVsd;

/*** gloval vars ************************************************************/

#endif	// _MAIN2_H
