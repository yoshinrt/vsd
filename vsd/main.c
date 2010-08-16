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
 #define MINIMIZE	// 最小 FIRMWARE
#endif

#ifndef MINIMIZE
/*** macros *****************************************************************/
/*** const ******************************************************************/
/*** new type ***************************************************************/
/*** prototype **************************************************************/
/*** extern *****************************************************************/
/*** gloval vars ************************************************************/

/*** スピード・タコのみ出力 *************************************************/

INLINE void OutputSerialS( void ){
	UCHAR c = 0xFF;
	
	SerialPack( g_Tacho.uVal );
	SerialPack( g_Speed.uVal );
	sci_write( &c, 1 );
}

/*** main *******************************************************************/

#ifdef MONITOR_ROM
	#pragma entry( main )
#else
	__entry( vect = 0 )
#endif
int main( void ){
	
	UCHAR	bProcessUIOFlag = 0;
	
	#ifdef MONITOR_ROM
		if( !IO.PDR5.BIT.B4 ) IR_Flasher();
	#else
		InitSector( __sectop( "B" ), __secend( "B" ));
	#endif
	
	InitMain();
	
//	Print( g_szMsgOpening );
	
	for(;;){
		WaitStateChange();
		
		/* 
		//デバッグ用コード
		g_Tacho.Time.dw = g_Speed.Time.dw = GetTimerW32();
		g_Tacho.uPulseCnt = (( GetRTC() >> 7 ) & 0x7 ) + 1;
		g_Speed.uPulseCnt = (( GetRTC() >> 6 ) & 0xF ) + 1;
		if( IO.PDR5.BYTE & ( 1 << 6 )){
			g_Tacho.uPulseCnt = 8;
		}
		*/
		
		ComputeMeter();			// speed, tacho, gear 計算
		DispLED_Carib();		// LED 表示データ生成
		CheckStartByGSensor();	// Gセンサーによるスタート検出
		ProcessAutoMode();		// オートモード
		
		/*** シリアル出力処理 ***/
		// SIO, sw 等の UserIO 処理
		if( bProcessUIOFlag = ~bProcessUIOFlag ){
			ProcessUIO();
		//}else{
		//	// スピード・タコ only 出力
		//	if( g_Flags.bOutputSerial )	OutputSerialS();
		}
	}
}

#else
__entry( vect = 0 ) int main( void ){
	SoftReset(); // RAM と ROM が何も変わらなければここ
}
#endif
