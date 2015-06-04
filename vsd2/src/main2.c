/*****************************************************************************
	
	VSD2 - vehicle data logger system2
	Copyright(C) by DDS
	
	speed.c -- speed, tacho, ラップセンサ
	
*****************************************************************************/

#include "dds.h"
#include <stdio.h>
#include "stm32f10x_rcc.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_nvic.h"
#include "stm32f10x_tim.h"
#include "stm32f10x_adc.h"
#include "main2.h"
#include "usart.h"

/*** NVIC IE/ID *************************************************************/

void NvicIntEnable( UINT IRQChannel ){
	NVIC->ISER[(IRQChannel >> 0x05)] = (u32)0x01 << (IRQChannel & (u8)0x1F);
}

void NvicIntDisable( UINT IRQChannel ){
	NVIC->ICER[(IRQChannel >> 0x05)] = (u32)0x01 << (IRQChannel & (u8)0x1F);
}

/*** 初期化 *****************************************************************/
// { TIM3, TIM2 } を 32bit タイマとして連結する

void TimerInit( void ){
	// APB クロック
	RCC_APB1PeriphClockCmd( RCC_APB1Periph_TIM2 | RCC_APB1Periph_TIM3, ENABLE );
	
	TIM_DeInit( TIM2 );
	TIM_DeInit( TIM3 );
	
	// TimeBase 初期化
	TIM2->ARR = -1; TIM3->ARR = -1;	// カウンタ上限値
	TIM2->PSC = 72000000 / TIMER_HZ - 1;
	TIM3->PSC = 0;	// プリスケーラ
	TIM2->CR2 = ( 2 << 4 );			// TIM2 更新イベントを選択
	TIM2->SMCR = 0;
	TIM3->SMCR = ( 1 << 4 ) |		// TIM3 は入力として TIM2 を選択
				7;					// TRGI rise edge でカウント
	TIM2->EGR = 1; TIM3->EGR = 1;	// UG (プリスケーラ値更新)
	TIM2->CR1 = TIM3->CR1 =
		( 0 << 7 ) |	// auto preload
		1;				// counter enable
}

/*** 現在時刻取得 *************************************************/

UINT GetCurrentTime( void ){
	UINT uTimeL, uTimeL2, uTimeH;
	
	uTimeL2 = TIM2->CNT;
	do{
		uTimeL = uTimeL2;
		uTimeH = TIM3->CNT;
	}while( uTimeL > ( uTimeL2 = TIM2->CNT ));
	
	return ( uTimeH << 16 ) | uTimeL;
}

UINT GetCurrentTime16( void ){
	return TIM2->CNT;
}

/*** AD *********************************************************************/

void AdcInit( void ){
	GPIO_InitTypeDef	GPIO_InitStructure;
	ADC_InitTypeDef		ADC_InitStructure;
	
	RCC_APB2PeriphClockCmd( RCC_APB2Periph_ADC1 | RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOC, ENABLE );
	
	GPIO_StructInit( &GPIO_InitStructure );
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
	GPIO_Init( GPIOC, &GPIO_InitStructure );
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4;
	GPIO_Init( GPIOC, &GPIO_InitStructure );
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5;
	GPIO_Init( GPIOC, &GPIO_InitStructure );
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4;
	GPIO_Init( GPIOA, &GPIO_InitStructure );
	
	/* ADC1 Configuration ------------------------------------------------------*/
	ADC_StructInit( &ADC_InitStructure );
	ADC_InitStructure.ADC_Mode					= ADC_Mode_Independent;
	ADC_InitStructure.ADC_ScanConvMode			= ENABLE;
	ADC_InitStructure.ADC_ContinuousConvMode 	= DISABLE;
	ADC_InitStructure.ADC_ExternalTrigConv   	= ADC_ExternalTrigConv_None;
	ADC_InitStructure.ADC_DataAlign          	= ADC_DataAlign_Left;
	ADC_InitStructure.ADC_NbrOfChannel       	= 1;
	ADC_Init( ADC1, &ADC_InitStructure );
	ADC_ExternalTrigInjectedConvConfig( ADC1, ADC_ExternalTrigInjecConv_None );
	
	/* ADC1 regular channel14 configuration to sample time = 55.5 cycles */
	ADC_InjectedSequencerLengthConfig( ADC1, 4 );
	ADC_InjectedChannelConfig( ADC1, ADC_Channel_15, 1, ADC_SampleTime_55Cycles5 );
	ADC_InjectedChannelConfig( ADC1, ADC_Channel_14, 2, ADC_SampleTime_55Cycles5 );
	ADC_InjectedChannelConfig( ADC1, ADC_Channel_4,  3, ADC_SampleTime_55Cycles5 );
	ADC_InjectedChannelConfig( ADC1, ADC_Channel_13, 4, ADC_SampleTime_55Cycles5 );
	
	/* Enable ADC1  */
	ADC_Cmd( ADC1, ENABLE );
	
	/* Enable ADC1 reset calibaration register */  
	ADC_ResetCalibration( ADC1 );
	
	/* Check the end of ADC1 reset calibration register */
	while( ADC_GetResetCalibrationStatus( ADC1 ));
	
	/* Start ADC1 calibaration */
	ADC_StartCalibration( ADC1 );
	/* Check the end of ADC1 calibration */
	while( ADC_GetCalibrationStatus( ADC1 ));
}

/*** ADC 変換 ***************************************************************/

void AdcConversion( void ){
	// Start ADC1 Software Conversion
	ADC_SoftwareStartInjectedConvCmd( ADC1, ENABLE );
	
	// Wait until conversion completion
	while( ADC_GetFlagStatus( ADC1, ADC_FLAG_JEOC ) == RESET );
}

/*** スピード・タコ・磁気センサー 初期化 ************************************/
// PD0: speed
// PD1: tacho
// PD2: 磁気センサー

void PulseInit( void ){
	// APB クロック
	RCC_APB2PeriphClockCmd( RCC_APB2Periph_GPIOD, ENABLE );
	
	// GPIO 設定
	GPIO_InitTypeDef GPIO_InitStruct;
	GPIO_StructInit( &GPIO_InitStruct );
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	
	GPIO_InitStruct.GPIO_Pin  = GPIO_Pin_0;
	GPIO_Init( GPIOD, &GPIO_InitStruct );
	GPIO_InitStruct.GPIO_Pin  = GPIO_Pin_1;
	GPIO_Init( GPIOD, &GPIO_InitStruct );
	GPIO_InitStruct.GPIO_Pin  = GPIO_Pin_2;
	GPIO_Init( GPIOD, &GPIO_InitStruct );
	
	// GPIO 割り込みイネーブル
	GPIO_EXTILineConfig( GPIO_PortSourceGPIOD, GPIO_PinSource0 );
	GPIO_EXTILineConfig( GPIO_PortSourceGPIOD, GPIO_PinSource1 );
	GPIO_EXTILineConfig( GPIO_PortSourceGPIOD, GPIO_PinSource2 );
	EXTI->IMR  |= 0x7;
	EXTI->RTSR |= 0x7;
	EXTI->FTSR &= ~0x7;
	
	// NVIC 設定
	NVIC_InitTypeDef NVIC_InitStructure;
	
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_InitStructure.NVIC_IRQChannel = EXTI0_IRQChannel;
	NVIC_Init( &NVIC_InitStructure );
	
	NVIC_InitStructure.NVIC_IRQChannel = EXTI1_IRQChannel;
	NVIC_Init( &NVIC_InitStructure );
	
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannel = EXTI2_IRQChannel;
	NVIC_Init( &NVIC_InitStructure );
}

/*** スピードパルス *********************************************************/

UINT	g_uMileage;
PULSE_t	g_Speed;

void EXTI0_IRQHandler( void ){
	EXTI->PR = 1 << 0;
	g_Speed.uLastTime	= GetCurrentTime16();
	++g_Speed.uPulseCnt;
}

/*** タコパルス *************************************************************/

PULSE_t	g_Tacho;

void EXTI1_IRQHandler( void ){
	EXTI->PR = 1 << 1;
	g_Tacho.uLastTime	= GetCurrentTime16();
	++g_Tacho.uPulseCnt;
}

/*** 磁気センサ *************************************************************/

UINT g_uLapTime;

void EXTI2_IRQHandler( void ){
	EXTI->PR = 1 << 2;
	++g_uLapTime;
}

/*** Tacho / Speed 計算 *****************************************************/

UINT g_uSpeedCalcConst = ( UINT )( 3600.0 * 100.0 / PULSE_PER_1KM * ( 1 << 11 ));

// 0rpm に切り下げる EG 回転数のパルス幅 = 200rpm (clk数@16MHz)
#define TACHO_0RPM_TH	(( UINT )( TIMER_HZ / ( 200 / 60.0 * 2 )))

// 0km/h に切り下げる speed パルス幅 = 1km/h (clk数@16MHz)
#define SPEED_0KPH_TH	(( UINT )( TIMER_HZ / ( PULSE_PER_1KM / 3600.0 )))

void ComputeMeterTacho( void ){
	ULONG	uPrevTime, uTime;
	UINT	uPulseCnt;
	
	// パラメータロード
	NvicIntDisable( EXTI1_IRQChannel );	// Tacho IRQ disable
	uTime				= g_Tacho.uLastTime;
	uPulseCnt			= g_Tacho.uPulseCnt;
	g_Tacho.uPulseCnt	= 0;
	NvicIntEnable( EXTI1_IRQChannel );	// Tacho IRQ enable
	uPrevTime			= g_Tacho.uPrevTime;
	
	// Tacho 計算
	if( uPulseCnt ){
		g_Tacho.uPrevTime = uTime;
		g_Tacho.uVal = TIMER_HZ * 30 * uPulseCnt / (( uTime - uPrevTime ) & 0xFFFF );
	}else if(
		uTime = GetCurrentTime16(),
		// 0.2秒後 に0に (0.2s = 150rpm)
		(( uTime - uPrevTime ) & 0xFFFF ) >= TACHO_0RPM_TH
	){
		g_Tacho.uVal		= 0;
		g_Tacho.uPrevTime	= uTime - TACHO_0RPM_TH;
	}
}

void ComputeMeterSpeed( void ){
	UINT	uTime;
	UINT	uPulseCnt;
	UINT	uPulseCntTmp;
	
	// パラメータロード
	NvicIntDisable( EXTI0_IRQChannel );	// Speed IRQ disable
	uTime				= g_Speed.uLastTime;
	uPulseCnt			= g_Speed.uPulseCnt;
	g_Speed.uPulseCnt	= 0;
	NvicIntEnable( EXTI0_IRQChannel );	// Speed IRQ disable
	
	uPulseCntTmp = uPulseCnt;
	
	// Speed 計算
	if( uPulseCnt || g_Speed.uVal ){
		UINT uPrevTime = g_Speed.uPrevTime;
		
		if( uPulseCnt ){
			g_uMileage += uPulseCnt;
			g_Speed.uPrevTime = uTime;
		}else{
			// パルスが入ってなかったら，パルスが1回だけ入ったものとして速度計算
			uPulseCnt		= 1;
			uTime			= GetCurrentTime16();
		}
		
		g_Speed.uVal = ( UINT )(
			TIMER_HZ * 3600.0 * 100 / PULSE_PER_1KM
		) * uPulseCnt / (( uTime - uPrevTime ) & 0xFFFF );
	}
	
	if( uPulseCntTmp ){
		// パルスが入ったときは必ず 1km/h 以上
		if( g_Speed.uVal < 100 ) g_Speed.uVal = 100;
	}else{
		// 1km/h 未満は 0km/h 扱い
		if( g_Speed.uVal < 100 ){
			g_Speed.uVal = 0;
			g_Speed.uPrevTime = uTime - SPEED_0KPH_TH;
		}
	}
	
	// 0-100ゴール待ちモードで100km/hに達したらNewLap起動
	#if 0
	// ★あとで
	if( g_Flags.uLapMode == MODE_ZERO_ONE_WAIT && g_Speed.uVal >= 10000 ){
		g_IR.uLastTime = GetRTC();
		g_Flags.bNewLap = TRUE;
		g_Flags.uLapMode = MODE_LAPTIME;
	}
	#endif
}

/*** S レコードローダ *******************************************************/

#ifndef EXEC_SRAM
static __noreturn void JumpTo( u32 uJmpAddr, u32 uSP ){
	asm( "MSR MSP, r1\nBX r0\n" );
}

UINT GetHex( UINT uBytes ){
	
	uBytes <<= 1;;
	UINT	uRet = 0;
	UINT	c;
	
	do{
		uRet <<= 4;
		c = UsartGetcharWaitUnbuffered();
		
		if( '0' <= c && c <= '9' )	uRet |= c - '0';
		else						uRet |= c - ( 'A' - 10 );
	}while( --uBytes );
	
	//DbgMsg(( "%02X ", uRet ));
	return uRet;
}

__noreturn void LoadSRecordSub( void ){
	
	UINT	uAddr, uLen;
	UINT	c;
	
	while( 1 ){
		// 'S' までスキップ
		while( UsartGetcharWaitUnbuffered() != 'S' );
		
		// 終了ヘッダなら break;
		if(( c = UsartGetcharWaitUnbuffered()) == '7' ) break;
		
		if( c == '3' ){
			// データを書き込む
			uLen	= GetHex( 1 ) - 5;
			uAddr	= GetHex( 4 );
			
			DbgMsg(( "Addr:%X Len:%X\n", uAddr, uLen ));
			while( uLen-- ) *( UCHAR *)( uAddr++ ) = GetHex( 1 );
		}
	}
	
	UsartPutstrUnbuffered( "starting program\n" );
	JumpTo( *( u32 *)0x20000004, *( u32 *)0x08003000 );
}

__noreturn void LoadSRecord( void ){
	// 全割り込み禁止
	NVIC->ICER[ 0 ] = -1;
	NVIC->ICER[ 1 ] = -1;
	
	UsartInit( USART_BAUDRATE, NULL );
	UsartPutstrUnbuffered( "\nWaiting for S record...\n" );
	JumpTo(( u32 )LoadSRecordSub, SRAM_END );
}
#endif

/*** バイナリ出力 ***********************************************************/

void SerialOutchar( UINT c ){
	if( c == 0xFE ){
		putchar( 0xFE ); putchar( 0x00 );
	}else if( c == 0xFF ){
		putchar( 0xFE ); putchar( 0x01 );
	}else{
		putchar( c );
	}
}
void SerialPack( UINT uVal, UINT uBytes ){
	do{
		SerialOutchar( uVal & 0xFF );
		uVal >>= 8;
	}while( --uBytes );
}

/*** シリアル出力 ***********************************************************/

#if 0
void OutputSerial( void ){
	SerialPack( g_Tacho.uVal, 2 );
	SerialPack( g_Speed.uVal, 2 );
	SerialPack( g_uMileage, 2 );
	//SerialPack(( TA.TCA << 8 ) | g_IR.uVal & 0xFF, 2 );
	//SerialPack( g_DispVal.uGy, 2 );
	//SerialPack( g_DispVal.uGx, 2 );
	SerialPack( 0, 6 );
	
	/*** ラップタイム表示 ***/
	if( g_Flags.bNewLap ){
		g_Flags.bNewLap = FALSE;
		SerialPack( g_uLapTime, 4 );
	}
	
	putchar( 0xFF );
}

/*** シリアル入力 ***********************************************************/

UINT g_uParam;

void DoInputSerial( char c ){
	
	if( 'A' <= c && c <= 'F' ){
		g_uParam = ( g_uParam << 4 ) + c - 'A' + 10;
	}else if( '0' <= c && c <= '9' ){
		g_uParam = ( g_uParam << 4 ) + c - '0';
	}else if( !g_Flags.bOpenCmd ){
		if( c == '*' && g_uParam == 0xF15EF117 ) g_Flags.bOpenCmd = 1;
	}else{
		switch( c ){
			case 'l': g_Flags.uLapMode	= MODE_LAPTIME;		g_uRemainedMillage = 0;
			Case 'g': g_Flags.uLapMode	= MODE_LAPTIME;		g_uRemainedMillage = g_uParam;
			Case 'f': g_Flags.uLapMode	= MODE_ZERO_FOUR;	g_uRemainedMillage = 1; g_uStartGTh = g_uParam;
			Case 'o': g_Flags.uLapMode	= MODE_ZERO_ONE;	g_uRemainedMillage = 1; g_uStartGTh = g_uParam;
			Case 'c': g_uVideoCaribCnt = 96;	// キャリブレーション
			Case 'z': GoMonitor();
			Case 'S': g_Flags.bOutputSerial = g_uParam;
			Case 'P': g_cLEDBar = g_uParam;
		}
		g_uParam = 0;
	}
}
#endif
