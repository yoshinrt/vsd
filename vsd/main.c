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

UINT	g_uThrottle;

#define rx_buffer_end	RX_BUFFER_LENGTH	// ★ROM 化では消す
#define tx_buffer_end	TX_BUFFER_LENGTH

#undef sci_write
size_t sci_write(UB* data, size_t n)
{
	UB *data_begin, *data_end;

	if(!data){
		int szbuf = ( UINT )tx_idx_tail - tx_idx_head;
		if(szbuf < 0)szbuf += RX_BUFFER_LENGTH;
		return (size_t)szbuf;
	}

	data_begin = data;
	data_end = data + n;
	while(data != data_end){
		tx_buffer[ tx_idx_tail++ ] = *data++;
		if(tx_idx_tail == tx_buffer_end)
			tx_idx_tail = 0;
	}
	/* 送信レジスタが空の場合は送信開始 */
	SCI3.SCR3.BIT.TIE = 1;
	return data - data_begin;
}

// ★ROM 化の際，関数名修正
#pragma interrupt( sci_int_handler_new )
void sci_int_handler_new( void ){
	/* 送信データエンプティ */
	if(SCI3.SSR.BIT.TDRE){
		/* バッファが空の場合は送信終了 */
		if(tx_idx_head == tx_idx_tail){
			/* TEND検出 */
			SCI3.SCR3.BIT.TIE = 0;
		}else{
			SCI3.TDR = tx_buffer[ tx_idx_head++ ];	/* 書き込むことでフラグをクリアする */
			if(tx_idx_head == tx_buffer_end)
				tx_idx_head = 0;
		}
	}
	/* 受信データフル */
	if(SCI3.SSR.BIT.RDRF){
		rx_buffer[ rx_idx_tail++ ] = SCI3.RDR;	/* 読むことでフラグをクリアする */
		if(rx_idx_tail == rx_buffer_end)
			rx_idx_tail = 0;
	}
	/* オーバラン フレーム パリティ エラー */
	if(SCI3.SSR.BYTE & 0x38 ){
//		if(error_handler)error_handler();	// ★RAM 化の際の暫定，後でもどす
		SCI3.SSR.BYTE &= ~0x38;
	}
}

/*** 割り込みベクタテーブル *************************************************/

//#undef VectorTable
void * const VectorTable2[] = {
	( void *)0x100,	//  0
	( void *)0x100,	//  1
	( void *)0x100,	//  2
	( void *)0x100,	//  3
	( void *)0x100,	//  4
	( void *)0x100,	//  5
	( void *)0x100,	//  6
	( void *)0x100,	//  7
	( void *)0x100,	//  8
	( void *)0x100,	//  9
	( void *)0x100,	// 10
	( void *)0x100,	// 11
	( void *)0x100,	// 12
	( void *)0x100,	// 13
	int_irq0,		// 14
	int_irq1,		// 15
	int_irq2,		// 16
	int_irq3,		// 17
	int_wkp,		// 18
	int_timer_a,	// 19
	( void *)0x100,	// 20
	int_timer_w,	// 21
	int_timer_v_IR,	// 22
	sci_int_handler_new,// 23
	( void *)0x100,	// 24
	( void *)0x100,	// 25
	( void *)0x100,	// 26
	( void *)0x100,	// 27
	( void *)0x100,	// 28
	( void *)0x100,	// 29
	( void *)0x100,	// 30
	( void *)0x100,	// 31
};

/*** itoa *******************************************************************/

#undef SerialPack
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
INLINE void OutputSerial( void ){
	UCHAR c = 0xFF;
	
	SerialPack( g_Tacho.uVal );
	SerialPack( g_Speed.uVal );
	
	SerialPack( g_uMileage );
	SerialPack(( TA.TCA << 8 ) | g_IR.uVal & 0xFF );
	SerialPack( g_DispVal.uGy );
	SerialPack( g_DispVal.uGx );
	SerialPack( g_uThrottle );
	
	/*** ラップタイム表示 ***/
	if( g_Flags.bNewLap ){
		g_Flags.bNewLap = FALSE;
		SerialPack( g_IR.Time.w.l );
		SerialPack( g_IR.Time.w.h );
	}
	
	sci_write( &c, 1 );
}

/*** ユーザ IO 処理 *********************************************************/

#undef ProcessUIO
void ProcessUIO( void ){
	UCHAR c;
	if( g_Flags.bOutputSerial )	OutputSerial();		// serial 出力
	while( sci_read( &c, 1 )) DoInputSerial( c );	// serial 入力
	ProcessPushSW();								// sw 入力
}

/*** ステート変化待ち & LED 表示 ********************************************/

#undef WaitStateChange
INLINE void WaitStateChange( void ){
	ULONG	uGx 		= 0;
	ULONG	uGy 		= 0;
	ULONG	uThrottle	= 0;
	UINT	uCnt		= 0;
	
	UCHAR	cTimerA;
	
	/*** ステート変化待ち ***/
	
	/*** WDT ***/
	WDT.TCSRWD.BYTE = ( 1 << 6 );	// TCWE
	WDT.TCWD		= 0;
	
	cTimerA = TA.TCA & ~( CALC_DIVCNT - 1 );
	
	while( cTimerA == ( TA.TCA & ~( CALC_DIVCNT - 1 ))){
		uGx			+= G_SENSOR_Z;	// 前後 G の検出軸変更
		uGy			+= G_SENSOR_Y;
		uThrottle	+= ADC_THROTTLE;
		++uCnt;
		
		// ★とりあえず高速化のためスキップ，ROM 化の際は復活させる
		//if( !( uCnt & ( 128 - 1 ))) LED_Driver();
	}
	
	// G の計算
	g_DispVal.uGx = ((( UINT )( uGx / uCnt )) >> 1 ) + ( g_DispVal.uGx >> 1 );
	g_DispVal.uGy = ((( UINT )( uGy / uCnt )) >> 1 ) + ( g_DispVal.uGy >> 1 );
	
	// スロットル
	g_uThrottle	= uThrottle	/ uCnt;
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
		// RAM 実行のメモリ初期化
		InitSector( __sectop( "B" ), __secend( "B" ));
	#endif
	
	InitMain();
	
	// ★
	g_uSpeedCalcConst = ( UINT )( 3600.0 * 100.0 / PULSE_PER_1KM * ( 1 << 11 ));
	VectorTblPtr = ( void *)VectorTable2;
	
	// ★ SCI の初期値を変更する
	IO.PMR1.BIT.TXD = 1;
	SCI3.SCR3.BIT.TE = 1;
	
//	Print( g_szMsgOpening );
	
	for(;;){
		WaitStateChange();
		CheckStartByGSensor();	// Gセンサーによるスタート検出
		
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
		
		if( bProcessUIOFlag = ~bProcessUIOFlag ){
			ProcessUIO();		// SIO, sw 等の UserIO 処理
		}
		
		ProcessAutoMode();		// オートモード
	}
}

#else
__entry( vect = 0 ) int main( void ){
	SoftReset(); // RAM と ROM が何も変わらなければここ
}
#endif
