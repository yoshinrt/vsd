/*****************************************************************************
	
	$Id$
	
	VSD - poor VSD
	Copyright(C) by DDS
	
	main.c -- main routine
	
*****************************************************************************/

#include <machine.h>
#include "dds.h"
#include "3664s.h"
#include "sci.h"
#include "main.h"
#include "led_charcode.h"

#ifdef MONITOR_ROM
 #include "main2.c"
#else
 #include "rom_entry.h"
 //#define MINIMIZE	// 最小 FIRMWARE
#endif

#ifndef MINIMIZE
/*** macros *****************************************************************/
/*** const ******************************************************************/
/*** new type ***************************************************************/
/*** prototype **************************************************************/
/*** extern *****************************************************************/
/*** gloval vars ************************************************************/

UINT	g_uPrevTW;	// TimerW >> 8
UINT	g_uHz;		// Hz >> 8 @ 1s

UINT			g_uAutoModeTimer;	//★ UCHAR にできる
DispVal_t		g_DispVal;
TouchPanel_t	g_TP;

/*** init sector ************************************************************/

INLINE void _INITSCT( void ){
	unsigned *uSrc, *uDst, *uDstEnd;
	
	#ifdef MONITOR_ROM
		uSrc	= __sectop( "D" );
		uDst	= __sectop( "R" );
		uDstEnd	= __secend( "R" );
		
		do{
			*uDst++ = *uSrc++;
		}while( uDst < uDstEnd );
	#else
		_INITSCT_ROM();
	#endif
	
	uDst	= __sectop( "B" );
	uDstEnd = __secend( "B" );
	do{ *uDst++ = 0; }while(( unsigned )uDst < ( unsigned )uDstEnd );
}

/*** get 32bit of TimerW ****************************************************/

/*INLINE*/ ULONG GetTimerW32( void ){
	
	UINT	uL, uH;
	
	uL = TW.TCNT;
	uH = g_TimerWovf.w.l;
	
	if( !( uL & 0x8000 ) && TW.TSRW.BIT.OVF ){
		++uH;
	}
	
	return ((( ULONG )uH ) << 16 ) | uL;
}

/*** TIMER A ****************************************************************/

#undef int_timer_a
#pragma interrupt( int_timer_a )
void int_timer_a( void ){
	ULONG	uTWCnt;
	
	++g_uRTC;
	IRR1.BIT.IRRTA = 0;	// IRRI2 クリア
	
	uTWCnt		= GetTimerW32() >> 8;
	g_uHz		= uTWCnt - g_uPrevTW;
	g_uPrevTW	= uTWCnt;
}

/*** itoa *******************************************************************/

void SerialPack( UINT uVal ){
	
	UCHAR	szBuf[ 4 ];
	UCHAR	*p = szBuf;
	
	if(( uVal & 0xFF ) >= 0xFE ){ *p++ = 0xFE; uVal -= 0xFE; }
	*p++ = uVal & 0xFF;
	uVal >>= 8;
	
	if( uVal >= 0xFE ){ *p++ = 0xFE; uVal -= 0xFE; }
	*p++ = uVal;
	
	sci_write( szBuf, p - szBuf );
}

/*** シリアル出力 ***********************************************************/

#undef OutputSerial
INLINE void OutputSerial( DispVal_t *val ){
	UCHAR c = 0xFF;
	
	SerialPack( g_Tacho.uVal );
	SerialPack( g_Speed.uVal );
	
	SerialPack( g_uMileage );
	SerialPack(( TA.TCA << 8 ) | g_IR.uVal & 0xFF );
	SerialPack( val->uGy );
	SerialPack( val->uGx );
	
	/*** ラップタイム表示 ***/
	if( g_Flags.bNewLap ){
		g_Flags.bNewLap = FALSE;
		SerialPack( g_IR.Time.w.l );
		SerialPack( g_IR.Time.w.h );
	}
	
	sci_write( &c, 1 );
}

/*** 各センサー値平滑化 *****************************************************/

#undef OutputSerialSmooth
INLINE void OutputSerialSmooth( DispVal_t *pDispVal ){
	pDispVal->uGx		= pDispVal->uGx / pDispVal->uCnt;
	pDispVal->uGy		= pDispVal->uGy / pDispVal->uCnt;
	
	OutputSerial( pDispVal );
	
	pDispVal->uGx		=
	pDispVal->uGy		= 0;
	pDispVal->uCnt 		= 0;
}

/*** Tacho / Speed 計算 *****************************************************/

// 0rpm に切り下げる EG 回転数のパルス幅 = 200rpm (clk数@16MHz)
#define TACHO_0RPM_TH	(( ULONG )( H8HZ / ( 200 / 60.0 * 2 )))

// 0km/h に切り下げる speed パルス幅 = 1km/h (clk数@16MHz)
#define SPEED_0KPH_TH	(( ULONG )( H8HZ / ( PULSE_PER_1KM / 3600.0 )))

#undef ComputeMeter
/*INLINE*/ void ComputeMeter( void ){
	ULONG	uPrevTime, uTime;
	UINT	uPulseCnt;
	
	if( g_uHz < (( H8HZ - 500000 ) >> 8 )){
		g_uHz = H8HZ >> 8;
	}
	
	// パラメータロード
	IENR1.BIT.IEN2 = 0;	// Tacho IRQ disable
	uTime				= g_Tacho.Time.dw;
	uPulseCnt			= g_Tacho.uPulseCnt;
	g_Tacho.uPulseCnt	= 0;
	IENR1.BIT.IEN2 = 1;	// Tacho IRQ enable
	uPrevTime			= g_Tacho.PrevTime.dw;
	
	// Tacho 計算
	if( uPulseCnt ){
		// 30 = 60[min/sec] / 2[pulse/roll]
		// << 7 は，分母 >> 1 と g_uHz を << 8 する分
		g_Tacho.uVal = (
			( UINT )(
				( ULONG )g_uHz * (( 30 << 7 ) * uPulseCnt ) /
				(( uTime - uPrevTime ) >> 1 )
			) +
			g_Tacho.uVal
		) >> 1;
		
		g_Tacho.PrevTime.dw = uTime;
	}else if(
		uTime = GetTimerW32(),
		// 0.2秒後 に0に (0.2s = 150rpm)
		uTime - uPrevTime >= TACHO_0RPM_TH
	){
		g_Tacho.uVal		= 0;
		g_Tacho.PrevTime.dw	= uTime - TACHO_0RPM_TH;
	}
	
	// パラメータロード
	IENR1.BIT.IEN2 = 0;	// Speed IRQ disable
	uTime				= g_Speed.Time.dw;
	uPulseCnt			= g_Speed.uPulseCnt;
	g_Speed.uPulseCnt	= 0;
	IENR1.BIT.IEN2 = 1;	// Speed IRQ enable
	uPrevTime			= g_Speed.PrevTime.dw;
	
	// Speed 計算
	if( uPulseCnt || g_Speed.uVal ){
		if( uPulseCnt ){
			g_uMileage += uPulseCnt;
			g_Speed.PrevTime.dw = uTime;
		}else{
			// パルスが入ってなかったら，パルスが1回だけ入ったものとして速度計算
			uPulseCnt		= 1;
			uTime			= GetTimerW32();
		}
		
		// 「ギア計算とか.xls」参照
		// 5 = 13(本来の定数) - 8(g_uHz のシフト分)
		g_Speed.uVal = (
			( UINT )(
				((( ULONG )g_uHz * uPulseCnt ) >> 5 ) *
				( UINT )( 3600.0 * 100.0 / PULSE_PER_1KM * ( 1 << 11 )) /
				(( uTime - uPrevTime ) >> 2 )
			) +
			g_Speed.uVal
		) >> 1;
	}
	
	// 1km/h 未満は 0km/h 扱い
	if( g_Speed.uVal < 100 ){
		g_Speed.uVal = 0;
		g_Speed.PrevTime.dw = uTime - SPEED_0KPH_TH;
	}
	
	// 0-100ゴール待ちモードで100km/hに達したらNewLap起動
	if( g_Flags.uLapMode == MODE_ZERO_ONE_WAIT && g_Speed.uVal >= 10000 ){
		g_IR.Time.dw = GetRTC();
		g_Flags.bNewLap = TRUE;
		g_Flags.uLapMode = MODE_LAPTIME;
	}
}

/******/

#undef DispLED_Carib
INLINE void DispLED_Carib( DispVal_t *pDispVal ){
	if( g_uVideoCaribCnt ){
		/*** video キャリブレーション表示 すべてを override ***/
		--g_uVideoCaribCnt;
		
		if( g_uVideoCaribCnt <= 64 ){
			g_Flags.bBlinkMain	= 0;
			g_Flags.bBlinkSub	= 0;
			
			g_cLEDBar = 0x40;
			PrintLEDStr(( UCHAR *)"CARb" );
			
			g_Tacho.uVal = 0;
			g_Speed.uVal = 30000;
		}
	}else{
		/*** LED 表示 ***/
		DispLED( g_Flags.uDispMode );
	}
}

/*** main *******************************************************************/

#ifdef MONITOR_ROM
	#pragma entry( main )
#else
	__entry( vect = 0 )
#endif
int main( void ){
	
	UCHAR			cTimerA;
	
	set_imask_ccr( 1 );
#ifdef MONITOR_ROM
	if( !IO.PDR5.BIT.B4 ) IR_Flasher();
#endif
	_INITSCT();
	InitMain();
	SetVector( 19, int_timer_a );	// ★後で消す
	set_imask_ccr( 0 );			/* CPU permit interrupts */
	
	Print( g_szMsgOpening );
	g_Flags.uAutoMode	= AM_DISP;
	cTimerA = TA.TCA;
	
	for(;;){
		
		/*** ステート変化待ち ***/
		
		while( cTimerA == TA.TCA ){
			g_DispVal.uGx += G_SENSOR_Z;	// 前後 G の検出軸変更
			g_DispVal.uGy += G_SENSOR_Y;
			++g_DispVal.uCnt;
			if( !( g_DispVal.uCnt & ( 128 - 1 ))) LED_Driver( cTimerA );
		}
		
		++cTimerA;
		
		if( !( cTimerA & ( CALC_DIVCNT - 1 ))){
			
			/* 
			//デバッグコード
			g_Tacho.Time.dw = g_Speed.Time.dw = GetTimerW32();
			g_Tacho.uPulseCnt = (( GetRTC() >> 7 ) & 0x7 ) + 1;
			g_Speed.uPulseCnt = (( GetRTC() >> 6 ) & 0xF ) + 1;
			if( IO.PDR5.BYTE & ( 1 << 6 )){
				g_Tacho.uPulseCnt = 8;
			}
			*/
			
			// 32Hz
			
			ComputeMeter();
			
			/*** ギア計算 ***/
			ComputeGear2();
			
			/*** LED 表示 ***/
			DispLED_Carib( &g_DispVal, cTimerA );
			
			/*** Gセンサーによるスタート検出 ***/
			/*** 未検証，うまくいかなかったら削る ***/
			CheckStartByGSensor( &g_DispVal );
			
			/*** シリアル出力処理 ***/
			if(
				( CALC_DIVCNT == SERIAL_DIVCNT ) ||
				!( cTimerA & ( SERIAL_DIVCNT - 1 ))
			){
				// key 入力
				UCHAR c;
				while( sci_read( &c, 1 )) DoInputSerial( c );
				
				OutputSerialSmooth( &g_DispVal );
				
				// sw 入力
				ProcessPushSW( &g_TP );
			}
			
			/*** オートモード ***/
			g_uAutoModeTimer = ProcessAutoMode( g_uAutoModeTimer );
		}
		
		/*** WDT ***/
		
		WDT.TCSRWD.BYTE = ( 1 << 6 );	// TCWE
		WDT.TCWD		= 0;
	}
}

#else
__entry( vect = 0 ) int main( void ){
	SoftReset(); // RAM と ROM が何も変わらなければここ
}
#endif
