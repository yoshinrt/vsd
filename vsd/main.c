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

/*** TIMER A ****************************************************************/

ULONG g_uPrevTW;	// TimerW cnt
ULONG g_uHz;		// TimerW cnt @ 1s

#undef int_timer_a
#pragma interrupt( int_timer_a )
void int_timer_a( void ){
	ULONG	uTWCnt;
	
	++g_uRTC;
	IRR1.BIT.IRRTA = 0;	// IRRI2 クリア
	
	uTWCnt		= GetTimerW32();
	g_uHz		= uTWCnt - g_uPrevTW;
	g_uPrevTW	= uTWCnt;
}

/*** get 32bit of TimerW ****************************************************/

INLINE ULONG GetTimerW32( void ){
	
	UINT	uL, uH;
	
	uL = TW.TCNT;
	uH = g_TimerWovf.w.l;
	
	if( !( uL & 0x8000 ) && TW.TSRW.BIT.OVF ){
		++uH;
	}
	
	return ((( ULONG )uH ) << 16 ) | uL;
}

/*** itoa *******************************************************************/

void SerialPack( UINT uVal ){
	
	UCHAR	szBuf[ 4 ];
	UCHAR	p = szBuf;
	
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
	SerialPack( val->uTacho );
	SerialPack( val->uSpeed );
	SerialPack( g_uMileage );
	SerialPack( g_IR.uVal );
	SerialPack( val->uGy );
	SerialPack( val->uGx );
	
	/*** ラップタイム表示 ***/
	if( g_Flags.bNewLap ){
		g_Flags.bNewLap = FALSE;
		SerialPack( g_IR.Time.w.l );
		SerialPack( g_IR.Time.w.h );
	}
	
	sci_write( "\xFF", 1 );
}

/*** 各センサー値平滑化 *****************************************************/

#undef OutputSerialSmooth
INLINE void OutputSerialSmooth( DispVal_t *pDispVal ){
	pDispVal->uSpeed	= pDispVal->uSpeed / ( SERIAL_DIVCNT / CALC_DIVCNT );
	pDispVal->uTacho	= pDispVal->uTacho / ( SERIAL_DIVCNT / CALC_DIVCNT );
	pDispVal->uGx		= pDispVal->uGx / pDispVal->uCnt;
	pDispVal->uGy		= pDispVal->uGy / pDispVal->uCnt;
	
	OutputSerial( pDispVal );
	pDispVal->uSpeed	=
	pDispVal->uTacho	=
	pDispVal->uGx		=
	pDispVal->uGy		= 0;
	pDispVal->uCnt 		= 0;
}

/*** Tacho / Speed 計算 *****************************************************/

#undef ComputeMeter
INLINE void ComputeMeter( void ){
	
	IENR1.BIT.IEN2 = 0;	// Tacho IRQ disable
	
	// Tacho 計算
	if( g_Tacho.uPulseCnt ){
		g_Tacho.uVal = ( TACO_ADJ >> 8 ) * g_Tacho.uPulseCnt / (( g_Tacho.Time.dw - g_Tacho.PrevTime.dw ) >> 8 );
		g_Tacho.PrevTime.dw = g_Tacho.Time.dw;
		g_Tacho.uPulseCnt = 0;
	}else if(
		// 0.2秒後 に0に
		g_TimerWovf.w.l - g_Tacho.Time.w.h >= ( UINT )(( ULONG )( H8HZ * 0.2 ) >> 16 )
	){
		g_Tacho.uVal = 0;
		g_Tacho.PrevTime.dw = g_Tacho.Time.dw;
		g_Tacho.Time.w.h	= g_TimerWovf.w.l;
		g_Tacho.Time.w.l	= 0;
	}
	
	IENR1.BIT.IEN2 = 1;	// Tacho IRQ enable
	
	// Speed 計算
	BOOL	bNoSpeedPulse	= FALSE;
	
	IENR1.BIT.IEN3 = 0;	// Speed IRQ disable
	if( !g_Speed.uPulseCnt ){
		// パルスが入ってなかったら，パルスが1回だけ入ったものとして速度計算
		// ★Time - PrevTime の差が膨大になることのストッパーが要る
		g_Speed.uPulseCnt	= 1;
		g_Speed.Time.dw		= GetTimerW32();
		bNoSpeedPulse		= TRUE;
	}
	
	g_Speed.uVal = ( SPEED_ADJ >> 8 ) * g_Speed.uPulseCnt / (( g_Speed.Time.dw - g_Speed.PrevTime.dw ) >> 8 );
	
	if( !bNoSpeedPulse ){
		g_Speed.PrevTime.dw = g_Speed.Time.dw;
		g_uMileage += g_Speed.uPulseCnt;
	}
	g_Speed.uPulseCnt = 0;
	
	IENR1.BIT.IEN3 = 1;	// Speed IRQ enable
	
	// 0-100ゴール待ちモードで100km/hに達したらNewLap起動
	if( g_Flags.uLapMode == MODE_ZERO_ONE_WAIT && g_Speed.uVal >= 10000 ){
		g_IR.Time.dw = GetRTC();
		g_Flags.bNewLap = TRUE;
		g_Flags.uLapMode = MODE_LAPTIME;
	}
}

/*** main *******************************************************************/

#ifdef MONITOR_ROM
	#pragma entry( main )
#else
	__entry( vect = 0 )
#endif
int main( void ){
	
	UINT			uTWovf;
	UINT			uAutoModeTimer	= g_uRTC;
	DispVal_t		DispVal;
	TouchPanel_t	TP;
	
	set_imask_ccr( 1 );
	if( !IO.PDR5.BIT.B4 ) IR_Flasher();
	_INITSCT();
	InitMain();
	
	set_imask_ccr( 0 );			/* CPU permit interrupts */
	
	Print( g_szMsgOpening );
	g_Flags.uAutoMode = AM_DISP;
	
	BZero( DispVal );
	BZero( TP );
	
	uTWovf = g_TimerWovf.w.l;
	
	for(;;){
		
		/*** ステート変化待ち ***/
		
		while( uTWovf == VOLATILE( UINT, g_TimerWovf.w.l )){
			//DispVal.uGx += G_SENSOR_X;
			DispVal.uGx += G_SENSOR_Z;	// 前後 G の検出軸変更
			DispVal.uGy += G_SENSOR_Y;
			++DispVal.uCnt;
			if( !( DispVal.uCnt & ( 128 - 1 ))) LED_Driver( g_TimerWovf.w.l );
		}
		
		++uTWovf;
		
		if( !( uTWovf & ( CALC_DIVCNT - 1 ))){
			// 約30Hz
			
			ComputeMeter();
			
			DispVal.uTacho += g_Tacho.uVal;
			DispVal.uSpeed += g_Speed.uVal;
			
			/*** ギア計算 ***/
			ComputeGear2();
			
			/*** LED 表示 ***/
			DispLED_Carib( &DispVal, uTWovf );
			
			/*** Gセンサーによるスタート検出 ***/
			/*** 未検証，うまくいかなかったら削る ***/
			CheckStartByGSensor( &DispVal );
			
			/*** シリアル出力処理 ***/
			if(
				( CALC_DIVCNT == SERIAL_DIVCNT ) ||
				!( uTWovf & ( SERIAL_DIVCNT - 1 ))
			){
				// key 入力
				UCHAR c;
				while( sci_read( &c, 1 )) DoInputSerial( c );
				
				OutputSerialSmooth( &DispVal );
				
				// sw 入力
				ProcessPushSW( &TP );
			}
			
			/*** オートモード ***/
			uAutoModeTimer = ProcessAutoMode( uAutoModeTimer );
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
