/*****************************************************************************
	
	$Id$
	
	VSD - poor VSD
	Copyright(C) by DDS
	
	main2.c -- main routine (ROMized)
	
*****************************************************************************/

/*** macros *****************************************************************/

//#define USE_IRQ0

/*** const ******************************************************************/
/*** new type ***************************************************************/
/*** prototype **************************************************************/

INLINE void _INITSCT( void );

/*** const ******************************************************************/
/*** extern *****************************************************************/

extern void *VectorTblPtr;

/*** gloval vars ************************************************************/

VRAM	g_VRAM = { MakeDisp( SP, SP, SP, SP ) };

// メーター数値
PULSE	g_Tacho;	//	= { 0 };
PULSE	g_Speed;	//	= { 0 };
PULSE	g_IR;		//	= { 0 };

ULONG	g_lParam;	// serial 数値入力
UINT	g_uTimerWovf;

UINT	g_uRemainedMillage;	// NewLap トリガを起動するまでの残り距離パルス

UINT	g_uStartGTh;	//	= 500;	/* 発進とみなす Gセンサーの差分値 */

// 総走行距離
UINT	g_uMileage;
UINT	g_uDispNext;
UINT	g_uRTC;

UINT	g_uPrevTW;	// TimerW >> 8
UINT	g_uHz;		// Hz >> 8 @ 1s

UCHAR			*g_szLEDMsg;

DispVal_t		g_DispVal;
Flags_t			g_Flags;
PushSW_t		g_PushSW;

char	g_cLEDBar;		//= 0;
UCHAR	g_cAutoModeTimer;
UCHAR	g_uVideoCaribCnt;

const UCHAR g_cFont[] = {
	#define LED_PATN( val, name ) val,
	#define LED_PAT( val, name )
	#include "led_font.h"
};

const UCHAR g_cFontR[] = {
	#define LED_PATN( val, name ) LED_REV( val ),
	#define LED_PAT( val, name )
	#include "led_font.h"
};

const char g_szMsgReadyToGo[]	= "REAdy to Go";
const char g_szMsgTacho[]		= "tACo";
const char g_szMsgSpeed[]		= "SPd";
const char g_szMsgWaterTemp[]	= "W-tMP";
const char g_szMsgIRCnt[]		= "IR";
const char g_szMsgFuel[]		= "FUEL";
const char g_szMsgMileage[]		= "MILE";
const char g_szMsgCalibrate[]	= "CALIbrAtE";
const char g_szMsgOpening[]		=
	EOL
	"VSD system v1.09" EOL
	"Copyright(C) by DDS" EOL;

const UCHAR g_LEDAnimeOpening[] = {
	LED_ANIME( _1, _1, _1, _1, 0x10, 32 ),
	LED_ANIME( _2, _2, _2, _2, 0x20, 32 ),
	LED_ANIME( _3, _3, _3, _3, 0x30, 32 ),
	LED_ANIME( _4, _4, _4, _4, 0x40, 32 ),
	LED_ANIME( _5, _5, _5, _5, 0x50, 96 ),
	LED_ANIME( _4, _4, _4, _4, 0x40, 32 ),
	LED_ANIME( _3, _3, _3, _3, 0x30, 32 ),
	LED_ANIME( _2, _2, _2, _2, 0x20, 32 ),
	LED_ANIME( _1, _1, _1, _1, 0x10, 32 ),
	LED_ANIME( SP, SP, SP, SP, 0x00, 96 ),
	0
};

const UCHAR g_cLEDFont[] = {
//	FONT_SP, FONT_SP, FONT_SP, FONT_SP, FONT_SP, FONT_SP, FONT_SP, FONT_SP, // 0x00
//	FONT_SP, FONT_SP, FONT_SP, FONT_SP, FONT_SP, FONT_SP, FONT_SP, FONT_SP, // 0x08
//	FONT_SP, FONT_SP, FONT_SP, FONT_SP, FONT_SP, FONT_SP, FONT_SP, FONT_SP, // 0x10
//	FONT_SP, FONT_SP, FONT_SP, FONT_SP, FONT_SP, FONT_SP, FONT_SP, FONT_SP, // 0x18
	
	#define LED_PAT( pat, ch )	pat,
	#define LED_PATN( pat, ch )	pat,
	#define LED_PATS( pat )		pat,
	#include "led_font.h"
};

const UINT g_uTachoBar[] = {
	334, 200, 150, 118, 97
};

/*** 10進->LED フォーマッタ *************************************************/

INLINE void PrintLED4( UINT uNum ){
	
	int i = 3;
	
	for( ; i >= 0; --i ){
		g_VRAM.cDisp[ i ] = ( 0/*g_Flags.bReverse*/ ? g_cFontR : g_cFont )[ uNum % 10 ];
		if(( uNum /= 10 ) == 0 ) break;
	}
	
	if( i >= 3 ) g_VRAM.cDisp[ 2 ] = FONT_SP;
	if( i >= 2 ) g_VRAM.cDisp[ 1 ] = FONT_SP;
	if( i >= 1 ) g_VRAM.cDisp[ 0 ] = FONT_SP;
}

INLINE void PrintLED( UINT uNum ){
	
	int i = 2;
	
	for( ; i >= 0; --i ){
		g_VRAM.cDisp[ i ] = ( 0/*g_Flags.bReverse*/ ? g_cFontR : g_cFont )[ uNum % 10 ];
		if(( uNum /= 10 ) == 0 ) break;
	}
	
	if( i >= 2 ) g_VRAM.cDisp[ 1 ] = FONT_SP;
	if( i >= 1 ) g_VRAM.cDisp[ 0 ] = FONT_SP;
}

/*** get 32bit of TimerW ****************************************************/

INLINE ULONG GetTimerW32( void ){
	
	UINT	uL, uH;
	
	uL = TW.TCNT;
	uH = g_uTimerWovf;
	
	if( !( uL & 0x8000 ) && TW.TSRW.BIT.OVF ){
		++uH;
	}
	
	return ((( ULONG )uH ) << 16 ) | uL;
}

/*** TIMER A ****************************************************************/

#pragma interrupt( int_timer_a )
void int_timer_a( void ){
	ULONG	uTWCnt;
	
	++g_uRTC;
	IRR1.BIT.IRRTA = 0;	// IRRI2 クリア
	
	uTWCnt		= GetTimerW32() >> 8;
	g_uHz		= uTWCnt - g_uPrevTW;
	g_uPrevTW	= uTWCnt;
}

INLINE ULONG GetRTC( void ){
	UINT	uRet;
	
	uRet = TA.TCA;
	if( !( uRet & 0x80 ) && IRR1.BIT.IRRTA ){
		uRet += 0x100;
	}
	
	return (( ULONG )g_uRTC << 8 ) + ( ULONG ) uRet;
}

/*** TIMER W ****************************************************************/

#pragma interrupt( int_timer_w )
/*__interrupt( vect = 21 )*/ void int_timer_w( void ){
	++g_uTimerWovf;	// 約240Hz
	TW.TSRW.BIT.OVF = 0;
}

/*** マグネットセンサー *****************************************************/

#pragma interrupt( int_irq0 )
/*__interrupt( vect = 14 )*/ void int_irq0( void ){
	
	ULONG	uNowTime = GetRTC();
	
	// 直前の NewLap から 3秒以上あいている
	if( uNowTime - g_IR.Time.dw >= NEWLAP_MIN_INTERVAL ){
		g_IR.Time.dw = uNowTime;
		g_Flags.bNewLap = TRUE;
	}
	IRR1.BIT.IRRI0 = 0;	// IRRI0 クリア
}

// wkp 版

#pragma interrupt( int_wkp )
/*__interrupt( vect = 18 )*/ void int_wkp( void ){
	
	if( IWPR.BYTE & ( 1 << 4 )){
		ULONG	uNowTime = GetRTC();
		
		// 直前の NewLap から 3秒以上あいている
		if( uNowTime - g_IR.Time.dw >= NEWLAP_MIN_INTERVAL ){
			g_IR.Time.dw = uNowTime;
			g_Flags.bNewLap = TRUE;
		}
		++g_IR.uVal;
	}
	IWPR.BYTE = 0;	// クリア
}

/*** IR *********************************************************************/

// 計算では 15.47回/約15Hz

/* 連続するパルスとみなすパルス幅
 * IR は 1KHz なので，本来は /1000 だが，1発はパルスを取り損ねる
 * 事を考慮すると /500, 更に 10% マージンで /450
 */
#define IR_PULSE_WIDTH	(( ULONG )H8HZ / 450 )
#define IR_PULSE_CNT	3 // NewLap とみなす連続パルス数

/* g_IR.Time のみ，1/256s 単位
 * 他の .Time, .PrevTime は 1clk単位
 */

// IR
#pragma interrupt( int_irq1 )
/*__interrupt( vect = 15 )*/ void int_irq1( void ){
	
	ULONG	uTime, uPrevTime;
	
	// 今時間ロード (RTC)
	ULONG	uNowTime = GetRTC();
	
	// 直前時間ロード (16MHz 時計)
	uPrevTime			= g_IR.PrevTime.dw;
	
	// 今時間ロード (16MHz 時計)
	g_IR.PrevTime.dw	= GetTimerW32();
	uTime				= g_IR.PrevTime.dw;
	
	if(
		// パルス幅が規定値以内だった
		uTime - uPrevTime < IR_PULSE_WIDTH &&
		
		// 直前の NewLap から 3秒以上あいている
		( uNowTime - g_IR.Time.dw >= NEWLAP_MIN_INTERVAL )
	){
		// 連続パルスとみなす
		if( ++g_IR.uPulseCnt == IR_PULSE_CNT ){
			g_IR.Time.dw = uNowTime;
			g_Flags.bNewLap = TRUE;
		}
	}else{
		g_IR.uPulseCnt = 0;
	}
	
	++g_IR.uVal;
	IRR1.BIT.IRRI1 = 0;	// IRRI0 クリア
}

/*** IRQ ********************************************************************/

// Tacho
#pragma interrupt( int_irq2 )
/*__interrupt( vect = 16 )*/ void int_irq2( void ){
	++g_Tacho.uPulseCnt;
	g_Tacho.Time.dw = GetTimerW32();
	IRR1.BIT.IRRI2 = 0;	// IRRI2 クリア
}

// Speed
#pragma interrupt( int_irq3 )
/*__interrupt( vect = 17 )*/ void int_irq3( void ){
	++g_Speed.uPulseCnt;
	g_Speed.Time.dw = GetTimerW32();
	
	// Millage 時限爆弾が発動したら，NewLap起動
	if( g_uRemainedMillage && !--g_uRemainedMillage ){
		g_IR.Time.dw = GetRTC();
		g_Flags.bNewLap = TRUE;
		
		if( g_Flags.uLapMode == MODE_ZERO_FOUR ){
			// 0-400モードなら，距離を400mに設定
			g_uRemainedMillage = ( UINT )( PULSE_PER_1KM * 400 / 1000 + 0.5 );
			g_Flags.uLapMode = MODE_LAPTIME;
		}else if( g_Flags.uLapMode == MODE_ZERO_ONE ){
			// 0-100 モードなら，0-100ゴール待ちモードに移行
			g_Flags.uLapMode = MODE_ZERO_ONE_WAIT;
		}
	}
	
	IRR1.BIT.IRRI3 = 0;	// IRRI3 クリア
}

// SCI
#pragma interrupt( INT_SCI3 )
/*__interrupt( vect = 23 )*/ void INT_SCI3( void ){
	sci_int_handler( 0 );
}

/*** TIMER V ****************************************************************/

#pragma interrupt( int_timer_v_IR )
/*__interrupt( vect = 22 )*/ void int_timer_v_IR( void ){
	
	// TV.TCORB を汎用変数の代わりに使ってる (^^;
	// 1/4 で ON
	if(( ++TV.TCORB ) & 3 ){
		// off
		TW.TMRW.BIT.CTS = 0;	// 38KHz カウンタ停止
		TW.TCRW.BIT.TOA = 0;	// 出力0
	}else{
		TW.TMRW.BIT.CTS = 1;	// 38KHz カウンタ開始
	}
	
	TV.TCSRV.BIT.CMFA = 0;		// 割り込みフラグクリア
}

/*** 割り込みベクタテーブル *************************************************/

void * const VectorTable[] = {
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
	INT_SCI3,		// 23
	( void *)0x100,	// 24
	( void *)0x100,	// 25
	( void *)0x100,	// 26
	( void *)0x100,	// 27
	( void *)0x100,	// 28
	( void *)0x100,	// 29
	( void *)0x100,	// 30
	( void *)0x100,	// 31
};

/*** モニター起動 ***********************************************************/

INLINE void GoMonitor( void ){
	
	// disable WDT
	WDT.TCSRWD.BYTE = ( 1 << 4 );	// TCSRWE
	WDT.TCSRWD.BYTE = ( 0 << 2 );	// WDON
	
	// disable LED
	IO.PCR5			= 0;
	IO.PCR8 		= 0;
	
	// disable interrupt
	set_imask_ccr( 1 );
	MONITOR_ENTRY();
}

/*** SetBeep ****************************************************************/

INLINE void SetBeep( UINT uCnt ){
	static UINT uCntPrev = BEEP_OFF;
	
	if( !g_Flags.bBeep ) uCnt = BEEP_OFF;
	
	if( uCntPrev != uCnt ){
		uCntPrev = uCnt;
		
		if( uCnt == BEEP_OFF ){
			TV.TCRV0.BYTE |= 4;
		}else{
			UINT	uBase;
			for( uBase = 0; uCnt >= 256; ++uBase, uCnt >>= 1 );
			
			TV.TCRV0.BIT.CKS	= uBase >> 1;	/* base[2:1]*/
			TV.TCRV1.BIT.ICKS	= uBase & 1;	/* base[0]	*/
			TV.TCORA			= uCnt;			/* カウンタ	*/
		}
	}
}

/*** Tacho / Speed 計算 *****************************************************/

// 0rpm に切り下げる EG 回転数のパルス幅 = 200rpm (clk数@16MHz)
#define TACHO_0RPM_TH	(( ULONG )( H8HZ / ( 200 / 60.0 * 2 )))

// 0km/h に切り下げる speed パルス幅 = 1km/h (clk数@16MHz)
#define SPEED_0KPH_TH	(( ULONG )( H8HZ / ( PULSE_PER_1KM / 3600.0 )))

INLINE void ComputeMeterTacho( void ){
	ULONG	uPrevTime, uTime;
	UINT	uPulseCnt;
	
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
}

// ★上に持っていく
UINT g_uSpeedCalcConst = ( UINT )( 3600.0 * 100.0 / PULSE_PER_1KM * ( 1 << 11 ));

INLINE void ComputeMeterSpeed( void ){
	ULONG	uPrevTime, uTime;
	UINT	uPulseCnt;
	UINT	uPulseCntTmp;
	
	// パラメータロード
	IENR1.BIT.IEN2 = 0;	// Speed IRQ disable
	uTime				= g_Speed.Time.dw;
	uPulseCnt			= g_Speed.uPulseCnt;
	g_Speed.uPulseCnt	= 0;
	IENR1.BIT.IEN2 = 1;	// Speed IRQ enable
	uPrevTime			= g_Speed.PrevTime.dw;
	
	uPulseCntTmp = uPulseCnt;
	
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
				//( UINT )( 3600.0 * 100.0 / PULSE_PER_1KM * ( 1 << 11 )) /
				g_uSpeedCalcConst /
				(( uTime - uPrevTime ) >> 2 )
			) +
			g_Speed.uVal
		) >> 1;
	}
	
	if( uPulseCntTmp ){
		// パルスが入ったときは必ず 1km/h 以上
		if( g_Speed.uVal < 100 ) g_Speed.uVal = 100;
	}else{
		// 1km/h 未満は 0km/h 扱い
		if( g_Speed.uVal < 100 ){
			g_Speed.uVal = 0;
			g_Speed.PrevTime.dw = uTime - SPEED_0KPH_TH;
		}
	}
	
	// 0-100ゴール待ちモードで100km/hに達したらNewLap起動
	if( g_Flags.uLapMode == MODE_ZERO_ONE_WAIT && g_Speed.uVal >= 10000 ){
		g_IR.Time.dw = GetRTC();
		g_Flags.bNewLap = TRUE;
		g_Flags.uLapMode = MODE_LAPTIME;
	}
}

/*** 現在ギアを求める *******************************************************/

INLINE void ComputeGear( UINT uTachoBar[] ){
	UINT	uGearRatio;
	UCHAR	cGear, cBestGear;
	int		i;
	
	uGearRatio = ( UINT )((( ULONG )g_Speed.uVal << 8 ) / g_Tacho.uVal );
	
	if     ( uGearRatio < GEAR_TH( 1 ))	cGear = 1;
	else if( uGearRatio < GEAR_TH( 2 )) cGear = 2;
	else if( uGearRatio < GEAR_TH( 3 ))	cGear = 3;
	else if( uGearRatio < GEAR_TH( 4 ))	cGear = 4;
	else								cGear = 5;
	
	/*** LEDバー表示 ********************************************************/
	
	// LED バー表示計算
	
	if( g_Flags.uGearMode == GM_TOWN ){
		// 街乗り
		g_cLEDBar = (( ULONG )g_Tacho.uVal << 4 ) / 1000;
	}else if( g_Tacho.uVal >= REV_LIMIT ){
		g_cLEDBar = 0x50;
	}else{
		i = 3 - ( int )( REV_LIMIT - g_Tacho.uVal ) / ( int )uTachoBar[ cGear - 1 ];
		g_cLEDBar =	i < 0 ? 0x00 : i << 4;
	}
	
	// シフトアップ警告音
	g_cLEDBar >= 0x50 ? SetBeep( 30578 >> 3 ) : SetBeep( BEEP_OFF );
	
	g_Flags.bBlinkMain = 0;
	g_Flags.bBlinkSub  = 0;
	
	if( g_Flags.uGearMode >= GM_GEAR ){	// シフト警告を使用する?
		if( g_cLEDBar >= 0x50 ){
			// シフトアップ警告(ギア)
			g_Flags.bBlinkSub  = 1;
			g_Flags.bBlinkMain = ( g_Flags.uGearMode >= GM_BL_MAIN );
			
			if( g_Flags.uGearMode >= GM_DESIRED_GEAR ) ++cGear;
		}else{
			// 最適なギアを計算する
			if     ( g_Speed.uVal < SH_DOWN_TH( 1 )) cBestGear = 1;
			else if( g_Speed.uVal < SH_DOWN_TH( 2 )) cBestGear = 2;
			else if( g_Speed.uVal < SH_DOWN_TH( 3 )) cBestGear = 3;
			else if( g_Speed.uVal < SH_DOWN_TH( 4 )) cBestGear = 4;
			else									 cBestGear = 5;
			
			// シフトダウン警告
			if( cBestGear != cGear ){
				if( g_Flags.uGearMode >= GM_DESIRED_GEAR ) cGear = cBestGear;
				g_Flags.bBlinkSub = 1;
			}
		}
	}
	
	// LED バー リバース
	//if( g_Flags.bReverse ) g_cLEDBar = -g_cLEDBar;
	
	// LED にギア表示
	g_VRAM.cDisp[ 3 ] = ( 0/*g_Flags.bReverse*/ ? g_cFontR : g_cFont )[ cGear ];
}

INLINE void ComputeGear2( void ){
	if( g_Flags.uDispMode >= DISPMODE_SPEED ){
		ComputeGear( g_uTachoBar );
	}else{
		g_Flags.bBlinkMain	= 0;
		g_Flags.bBlinkSub	= 0;
	}
}

/*** メーター計算 ***********************************************************/

INLINE void ComputeMeter( void ){
	
	if( g_uHz < (( H8HZ - 500000 ) >> 8 )){
		g_uHz = H8HZ >> 8;
	}
	
	ComputeMeterTacho();
	ComputeMeterSpeed();
	
	if( g_Flags.uDispMode >= DISPMODE_SPEED ){
		ComputeGear( g_uTachoBar );
	}else{
		g_Flags.bBlinkMain	= 0;
		g_Flags.bBlinkSub	= 0;
	}
}

/*** LED アニメーション *****************************************************/

INLINE UINT DoAnimation( UCHAR *LEDAnime, UINT uDispPtr ){
	UCHAR	cDispCnt = uDispPtr >> 8;
	uDispPtr &= 0xFF;
	
	// Font セット
	g_VRAM.cDisp[ 0 ] = LEDAnime[ uDispPtr * LED_ANIME_PARAM_NUM + 1 ];
	g_VRAM.cDisp[ 1 ] = LEDAnime[ uDispPtr * LED_ANIME_PARAM_NUM + 2 ];
	g_VRAM.cDisp[ 2 ] = LEDAnime[ uDispPtr * LED_ANIME_PARAM_NUM + 3 ];
	g_VRAM.cDisp[ 3 ] = LEDAnime[ uDispPtr * LED_ANIME_PARAM_NUM + 4 ];
	
	//*( ULONG *)g_VRAM.cDisp = *( ULONG *)LEDAnime[ uDispPtr ];
	g_cLEDBar		= LEDAnime[ uDispPtr * LED_ANIME_PARAM_NUM + 5 ];
	
	if( ++cDispCnt == LEDAnime[ uDispPtr * LED_ANIME_PARAM_NUM + 0 ] ){
		++uDispPtr;
		cDispCnt = 0;
	}
	
	return(
		LEDAnime[ uDispPtr * LED_ANIME_PARAM_NUM + 0 ] ?
			(((( UINT )cDispCnt ) << 8 ) | uDispPtr ) :
			DISP_FINISH
	);
}

INLINE UINT DispLEDMsg( UCHAR *szMsg, UINT uDispPtr ){
	
	ULONG	lDisp;
	UCHAR	cDispCnt = uDispPtr >> 8;
	uDispPtr &= 0xFF;
	
	// VRAM をロード
	if( uDispPtr == 0 ){
		lDisp = MakeDisp( SP, SP, SP, SP );
	}else{
		lDisp = g_VRAM.lDisp;
	}
	
	// wait 時間が過ぎたら，シフトする
	if( ++cDispCnt == ( 32 / CALC_DIVCNT )){
		cDispCnt = 0;
		
		if( szMsg[ uDispPtr ] ){
			lDisp <<= 8;
			lDisp |= g_cLEDFont[ szMsg[ uDispPtr ] - ' ' ];
			++uDispPtr;
		}else if( lDisp == MakeDisp( SP, SP, SP, SP )){
			return( DISP_FINISH );
		}else{
			lDisp <<= 8;
			lDisp |= FONT_SP;
		}
	}
	
	// VRAM に書き戻し
	g_VRAM.lDisp = lDisp;
	
	return(((( UINT )cDispCnt ) << 8 ) | uDispPtr );
}

/*** PC からのメッセージ ****************************************************/

INLINE void DispLEDMsgPC( UCHAR cPat ){
	
	ULONG	lDisp;
	
	// 現在表示モードの退避
	if( g_Flags.uDispMode != DISPMODE_MSG_PC ){
		g_Flags.uDispMode = DISPMODE_MSG_PC;
		
		if( g_Flags.uDispMode >= DISPMODE_SPEED ) g_Flags.uDispModeNext = g_Flags.uDispMode;
		
		lDisp = ( MakeDisp( SP, SP, SP, SP ) << 8 ) | cPat;
		g_cLEDBar			= 0;
		g_Flags.bBlinkMain	= 0;
		g_Flags.bBlinkSub	= 0;
		
	}else{
		lDisp = ( g_VRAM.lDisp << 8 ) | cPat;
	}
	
	g_VRAM.lDisp = lDisp;
}

/*** PC からのアニメーション ************************************************/

INLINE void DispLEDAnimePC( ULONG lPat ){
	
	// 現在表示モードの退避
	if( g_Flags.uDispMode != DISPMODE_ANIME_PC ){
		g_Flags.uDispMode = DISPMODE_ANIME_PC;
		
		if( g_Flags.uDispMode >= DISPMODE_SPEED ) g_Flags.uDispModeNext = g_Flags.uDispMode;
		
		g_cLEDBar			= 0;
		g_Flags.bBlinkMain	= 0;
		g_Flags.bBlinkSub	= 0;
	}
	
	g_Flags.uDispMode = DISPMODE_ANIME_PC;
	g_VRAM.lDisp = lPat;
}

/*** start message **********************************************************/

/*INLINE*/ void DispMsgStart( char *pMsg ){
	if( g_Flags.uDispMode >= DISPMODE_SPEED ){
		g_Flags.uDispMode = DISPMODE_MSG;
		g_uDispNext = 0;
		g_szLEDMsg = ( UCHAR *)pMsg;
	}
}

INLINE void DispAnimeStart( UCHAR *pMsg ){
	if( g_Flags.uDispMode >= DISPMODE_SPEED ){
		g_Flags.uDispMode = DISPMODE_OPENING;
		g_uDispNext = 0;
		g_szLEDMsg = pMsg;
	}
}

/*** 4文字 LED Disp *********************************************************/
/* 必ず 4文字 */

INLINE void PrintLEDStr( UCHAR *szMsg ){
	
	g_VRAM.cDisp[ 0 ] = g_cLEDFont[ szMsg[ 0 ] - ' ' ];
	g_VRAM.cDisp[ 1 ] = g_cLEDFont[ szMsg[ 1 ] - ' ' ];
	g_VRAM.cDisp[ 2 ] = g_cLEDFont[ szMsg[ 2 ] - ' ' ];
	g_VRAM.cDisp[ 3 ] = g_cLEDFont[ szMsg[ 3 ] - ' ' ];
}

/*** LED表示 ****************************************************************/

INLINE void DispLED( UCHAR cDispMode ){
	
	switch( cDispMode ){
	  case DISPMODE_SPEED:	PrintLED( g_Speed.uVal / 100 );
	  Case DISPMODE_TACHO:	PrintLED( g_Tacho.uVal / 10 );
	  Case DISPMODE_IR:		PrintLED4( g_IR.uVal );
							g_cLEDBar = IO.PDR1.BIT.B5 ? 0x0 : 0x40;
		
	  //Case 'w': PrintLED4( g_WaterTemp >> 6 );
	  //Case 'u': PrintLED4( g_Fuel >> 6 );
		
	  Case DISPMODE_OPENING:
		g_uDispNext = DoAnimation( g_szLEDMsg, g_uDispNext );
		
	  Case DISPMODE_MSG:
	  case DISPMODE_MSG_LOOP:
		g_uDispNext = DispLEDMsg( g_szLEDMsg, g_uDispNext );
		
	  Case DISPMODE_MSG_PC:
	  case DISPMODE_ANIME_PC:
		return;
	}
	
	if( g_uDispNext == DISP_FINISH ){
		if( g_Flags.uDispMode == DISPMODE_MSG_LOOP ){
			g_uDispNext = 0;
		}else{
			g_Flags.uDispMode = g_Flags.uDispModeNext;
		}
	}
}

INLINE void DispLED_Carib( void ){
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

/*** LED driver **************************************************************/

#define SetLEDBar1( i ) \
	if( \
		g_cLEDBar >= ( i ) || \
		( g_cLEDBar & 0xF0 ) == (( i ) - 0x10 ) && (( g_cLEDBar & 0xF ) > 0x7 ) && ( cTimerA & ( BLINK_RATE << 3 )) \
	) cPat &= 0xFE;

INLINE void LED_Driver( void ){
	UCHAR	cLEDPos = IO.PDR5.BYTE & 0xF;
	UCHAR	cPat;
	UCHAR	cTimerA = TA.TCA;
	
	cLEDPos >>= 1;
	
	switch( cLEDPos ){
	  case 0x1:
		cPat = g_VRAM.cDisp[ 3 ];
		SetLEDBar1( 0x40 );
		
	  Case 0x2:
		cPat = g_VRAM.cDisp[ 2 ];
		SetLEDBar1( 0x30 );
		
	  Case 0x4:
		cPat = g_VRAM.cDisp[ 1 ];
		SetLEDBar1( 0x20 );
		
	  Default:
		cPat = g_VRAM.cDisp[ 0 ];
		SetLEDBar1( 0x10 );
		cLEDPos = 1 << 3;
	}
	
	if(
		( cLEDPos == 1 && g_Flags.bBlinkSub || cLEDPos != 1 && g_Flags.bBlinkMain ) &&
		cTimerA & ( BLINK_RATE )
	){
		cPat |= 0xFE;
	}
	
	if(
		( g_cLEDBar >= 0x50 /*|| g_cLEDBar <= -0x50*/ ) &&
		cTimerA & ( BLINK_RATE )
	){
		cPat |= 1;
	}
	
	IO.PDR8.BYTE = cPat;
	IO.PDR5.BYTE = cLEDPos;
}

/*** sprintf %ud *************************************************************/

/*INLINE*/ void Print( char *szMsg ){
	UINT uLen = 0;
	
	for( ; szMsg[ uLen ]; ++uLen );
	sci_write(( UCHAR *)szMsg, uLen );
}

INLINE void Print_ud( UINT uNum, char c ){
	UCHAR szBuf[ 7 ];
	UCHAR *p = &szBuf[ 6 ];
	
	p[ 0 ] = ' ';
	
	for(;;){
		*--p = uNum % 10 + '0';
		if(( uNum /= 10 ) == 0 ) break;
	}
	
	while( p != &szBuf[ 1 ] ) *--p = '0';
	*--p = c;
	
	sci_write( szBuf, 7 );
}

/*** itoa *******************************************************************/

void Print_ItoA( ULONG u, char c ){
	
	char	szBuf[ ITOA_DIGIT_NUM + 3 ];
	char	*p = &szBuf[ ITOA_DIGIT_NUM ];
	
	szBuf[ ITOA_DIGIT_NUM + 1 ] = ' ';
	szBuf[ ITOA_DIGIT_NUM + 2 ] = '\0';
	
	do{
		*p-- = '@' + ( u & (( 1 << ITOA_RADIX_BIT ) - 1 ));
	} while( u >>= ITOA_RADIX_BIT );
	
	*p = c;
	
	Print( p );
}

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

/*** itoa *******************************************************************/

void ItoA128( UINT uUpper, UINT uLower ){
	
	UCHAR	szBuf[ ITOA_DIGIT_NUM ];
	ULONG	uNum = (( ULONG )uUpper << 16 ) | uLower;
	int		i;
	
	for( i = ITOA_DIGIT_NUM - 1; i >= 0; --i ){
		szBuf[ i ] = ( UCHAR )uNum | ~(( 1 << ITOA_RADIX_BIT ) - 1 );
		uNum >>= 7;
	};
	
	sci_write( szBuf, ITOA_DIGIT_NUM );
}

/*** シリアル出力 ***********************************************************/

INLINE void OutputSerial( void ){
	UCHAR c = 0xFF;
	
	SerialPack( g_Tacho.uVal );
	SerialPack( g_Speed.uVal );
	
	SerialPack( g_uMileage );
	SerialPack(( TA.TCA << 8 ) | g_IR.uVal & 0xFF );
	SerialPack( g_DispVal.uGy );
	SerialPack( g_DispVal.uGx );
	
	/*** ラップタイム表示 ***/
	if( g_Flags.bNewLap ){
		g_Flags.bNewLap = FALSE;
		SerialPack( g_IR.Time.w.l );
		SerialPack( g_IR.Time.w.h );
	}
	
	sci_write( &c, 1 );
}

/*** シリアル入力 ***********************************************************/

INLINE void DoInputSerial( char c ){
	
	if( 'A' <= c && c <= 'F' ){
		g_lParam = ( g_lParam << 4 ) + c - 'A' + 10;
	}else if( '0' <= c && c <= '9' ){
		g_lParam = ( g_lParam << 4 ) + c - '0';
	}else if( !g_Flags.bOpenCmd ){
		if( c == '*' && g_lParam == 0xF15EF117 ) g_Flags.bOpenCmd = 1;
	}else{
		switch( c ){
		  case 's':	DispMsgStart( g_szMsgSpeed );		g_Flags.uDispModeNext = DISPMODE_SPEED;
		  Case 't':	DispMsgStart( g_szMsgTacho );		g_Flags.uDispModeNext = DISPMODE_TACHO;
		  Case 'i':	DispMsgStart( g_szMsgIRCnt );		g_Flags.uDispModeNext = DISPMODE_IR;
		  Case 'M':	DispMsgStart( g_szMsgMileage );		g_Flags.uDispModeNext = DISPMODE_MILEAGE;
		  //Case 'w':	DispMsgStart( g_szMsgWaterTemp );	g_Flags.uDispModeNext = c;
		  //Case 'u':	DispMsgStart( g_szMsgFuel );		g_Flags.uDispModeNext = c;
			
		  Case 'l':	g_Flags.uLapMode	= MODE_LAPTIME;			g_uRemainedMillage = 0;
		  Case 'g':	g_Flags.uLapMode	= MODE_LAPTIME;			g_uRemainedMillage = g_lParam;
		  Case 'f':	g_Flags.uLapMode	= MODE_ZERO_FOUR;		g_uRemainedMillage = 1; g_uStartGTh = g_lParam;
		  Case 'o':	g_Flags.uLapMode	= MODE_ZERO_ONE;		g_uRemainedMillage = 1; g_uStartGTh = g_lParam;
			
		  Case 'a': g_Flags.uAutoMode	= g_lParam;	// オートモード
			
		  Case 'c': g_uVideoCaribCnt = 96;	// キャリブレーション
			
		  Case 'G': g_Flags.uGearMode 			= g_lParam;
		  //Case 'r': g_Flags.bReverse			= g_lParam;
		  Case 'b': g_Flags.bBeep				= g_lParam;
			
		  Case 'm': DispLEDMsgPC( g_lParam );
		  Case 'p': DispLEDAnimePC( g_lParam );
		  Case 'N': g_Flags.uDispMode = g_Flags.uDispModeNext;
		  Case 'z': GoMonitor();
			
		  Case 'S':	g_Flags.bOutputSerial = g_lParam;
		  Case 'P':	g_cLEDBar = g_lParam;
		}
		g_lParam = 0;
	}
}

/*** PushSW *****************************************************************/

INLINE UCHAR ScanPushSW( void ){
	
	UCHAR	cSW = (( IO.PDR5.BYTE >> 4 ) & 0xE ) | IO.PDR2.BIT.B0;
	
	g_PushSW.uTrig |= ~g_PushSW.uPrev & cSW;
	g_PushSW.uPrev = cSW;
	
	if( g_PushSW.uTrig & 1 ){
		g_PushSW.uPushElapsed	= 0;
		g_PushSW.uPushCnt++;
	}else if( g_PushSW.uPushElapsed < 255 ){
		++g_PushSW.uPushElapsed;
	}
	
	return( g_PushSW.uTrig );
}

INLINE void ProcessPushSW( void ){
	
	ScanPushSW();
	
	if( g_PushSW.uPushElapsed >= DBL_CLICK_TIME && g_PushSW.uPushCnt ){
		// タッチパネル処理 (ダブルクリック)
		
		switch( g_PushSW.uPushCnt ){
		  case SWCMD_TACHO_SPD:	/* speed <--> tacho 切り替え */
			g_Flags.uAutoMode = AM_TBAR;
			DoInputSerial( g_Flags.uDispMode != DISPMODE_SPEED ? 's' : 't' );
			
		  Case SWCMD_AUTOMODE:	/* town <--> circuit 切り替え */
			g_Flags.uAutoMode	= ( g_Flags.uAutoMode + 1 ) % AM_NUM;
			DispMsgStart(
				g_Flags.uAutoMode == AM_TBAR ? "A-tbAR" : "A-dISP"
			);
			
		  Case SWCMD_VCARIB:
			// video キャリブレーション
			g_uVideoCaribCnt = 64;
		}
		g_PushSW.uPushCnt = 0;
	}
	
	// タクトスイッチ処理
	if( g_PushSW.uTrig & ( 1 << 1 )){
		UCHAR cNextMode;
		
		#define DEF_DISP_MODE( mode, c )	cNextMode = c; Case mode:
		switch( g_Flags.uDispMode ){
			default:
			DEF_DISP_MODE( DISPMODE_SPEED, 's' )
			DEF_DISP_MODE( DISPMODE_TACHO, 't' )
			DEF_DISP_MODE( DISPMODE_MILEAGE, 'M' )
		//	DEF_DISP_MODE( 'w' )	// water temp
		//	DEF_DISP_MODE( 'u' )	// fuel
			DEF_DISP_MODE( DISPMODE_IR, 'i' )	// ir
			cNextMode = 's';
		}
		
		DoInputSerial( cNextMode );
	}
	g_PushSW.uTrig &= ~(( 1 << 1 ) | 1 );
}

/*** オートモード処理 *******************************************************/

INLINE UINT ProcessAutoMode( void ){
	if( g_Tacho.uVal >= 4500 ){
		// 4500rpm 以上で，Circuit モードに移行
		g_Flags.uGearMode	= GM_GEAR;
		g_cAutoModeTimer	= g_uRTC;
	}else if(( UCHAR )g_uRTC - g_cAutoModeTimer >= 120 ){
		// 2分間，4500rpm 以下なら，街乗りモードに移行
		g_Flags.uGearMode	= GM_TOWN;
	}
	
	if( g_Flags.uAutoMode == AM_DISP ){
		// AM_DISP のとき，Speed⇔Tacho の自動切換え
		if( g_Tacho.uVal < 1500 ){
			g_Flags.uDispModeNext = DISPMODE_TACHO;
		}else if( g_Speed.uVal >= 70 * 100 ){
			g_Flags.uDispModeNext = DISPMODE_SPEED;
		}
	}
}

/*** Gセンサーによるスタート検出 ********************************************/

INLINE void CheckStartByGSensor( void ){
	if( g_Flags.uLapMode == MODE_ZERO_FOUR || g_Flags.uLapMode == MODE_ZERO_ONE ){
		
		if(
			(
				( g_DispVal.uPrevGx > g_DispVal.uGx ) ?
					( g_DispVal.uPrevGx - g_DispVal.uGx ) :
					( g_DispVal.uGx - g_DispVal.uPrevGx )
			) >= g_uStartGTh
		){
			g_IR.Time.dw = GetRTC();
			g_Flags.bNewLap = TRUE;
			
			if( g_Flags.uLapMode == MODE_ZERO_FOUR ){
				// 0-400モードなら，距離を400mに設定
				g_uRemainedMillage = ( UINT )( PULSE_PER_1KM * 400 / 1000 + 0.5 );
				g_Flags.uLapMode = MODE_LAPTIME;
			}else /*if( g_Flags.uLapMode == MODE_ZERO_ONE )*/ {
				// 0-100 モードなら，0-100ゴール待ちモードに移行
				g_Flags.uLapMode = MODE_ZERO_ONE_WAIT;
				g_uRemainedMillage = 0;
			}
		}
	}
	
	g_DispVal.uPrevGx = g_DispVal.uGx;
}

/*** HW セットアップ ********************************************************/

INLINE void SetupHW( void ){
	IO.PMR5.BYTE	= ( 1 << 4 );	// WKP4 を割り込み端子に設定
	IO.PCR5			= 0x0F;			// Port5[3:0] を出力に設定
	IO.PDR5.BYTE	= 0;
	
	IO.PCR8 		= 0xFF;			// Port8 を出力に設定
	IO.PDR8.BYTE	= 1 << 7;		// Port8 を出力に設定
	
	TW.TCRW.BYTE	= 0;				// φ/1 = 244Hz で ovf
	TW.TIERW.BYTE	= 1 << 7;			// enable Timer-W ovf int.
	TW.TMRW.BYTE	= 1 << 7;			// start TimerW
	
	/*** IRQ0〜3 の設定 *****************************************************/
	
	IO.PMR1.BYTE =
		( 1 << 7 ) |	// IRQ3 ポートをIRQに設定
		( 1 << 6 ) |	// IRQ2
		( 1 << 5 ) |	// IRQ1
		( 1 << 4 ) |	// IRQ0
		0;
	
	//TMA = 0x04;
	
	IENR1.BYTE =
		( 1 << 6 ) |				// Enable timer-A interrupt
		( 1 << 5 ) |				// wakeup 割り込み
		( 1 << 3 ) |				// IRQ0 3-0 enable
		( 1 << 2 ) |
		( 1 << 1 ) |
  #ifdef USE_IRQ0
		( 1 << 0 );
  #else
		0;
  #endif
	
  #ifdef RISE_EDGE_CAR_SIGNAL
	IEGR1.BYTE =
		( 1 << 3 ) |		// IRQ3 rise
		( 1 << 2 );			// IRQ2 rise
  #endif
	
	/*** Timer A (RTC) ******************************************************/
	
	TA.TMA.BYTE =
		( 1 << 3 ) |	// φW 選択
		0;				// 1s 毎に割込み
	
	//IENR1.BIT.IENTA = 1;	// 上でやってる
	
	/*** AD 変換器設定 ******************************************************/
	
	AD.ADCSR.BIT.SCAN	= 1;	// スキャンモード
	AD.ADCSR.BIT.CH		= 3;	// CH0-3
	AD.ADCSR.BIT.ADST	= 1;	// AD 開始
	
	/* timer V 設定 */
	TV.TCRV0.BIT.CCLR	= 1;	// Counter A でクリア
	TV.TCSRV.BIT.OS		= 3;	// counter A でトグル出力
	
	SetBeep( BEEP_OFF );
	
	/*** WDT ****************************************************************/
	
	WDT.TCSRWD.BYTE = ( 1 << 6 );	// TCWE
	WDT.TCWD		= 0;
	WDT.TCSRWD.BYTE = ( 1 << 4 );	// TCSRWE
	WDT.TCSRWD.BYTE = ( 1 << 2 );	// WDON
}

/*** 初期化ルーチン *********************************************************/

INLINE void InitMain( void ){
	
	set_imask_ccr( 1 );
	
	/*** ベクタ設定 *********************************************************/
	
	VectorTblPtr = ( void *)VectorTable;
	
	/************************************************************************/
	
	SetupHW();
	
	g_Flags.bBeep			= 1;
	g_Flags.uDispMode		= DISPMODE_OPENING;
	g_Flags.uDispModeNext	= DISPMODE_TACHO;
	
	// ROM prog によってポインタが進むので
	// RAM 実行の際にポインタを元に戻す必要がある
	g_szLEDMsg				= g_LEDAnimeOpening;
	
	//sci_init( cRxBuf, sizeof( cRxBuf ), cTxBuf, sizeof( cTxBuf ));
	sci_init( 0 );
	
	/*** SCK3 を TouchPanel 入力に設定 ***/
	/*
	IO.PCR2.BIT.0	= 0;		// SCK3 を入力に
	IO.SCR3.BIT.1	=
	IO.SCR3.BIT.0	= 0;		// シリアルクロックを使用しない
	*/
	
	set_imask_ccr( 0 );			/* CPU permit interrupts */
}

/*** bzero ******************************************************************/

INLINE void bzero( UCHAR *p, UINT uSize ){
	UCHAR	*pEnd = p + uSize;
	while( p < pEnd ) *p++ = 0;
}

/*** init sector ************************************************************/

INLINE void InitSector( UINT *uBStart, UINT *uBEnd ){
	while( uBStart < uBEnd ){ *uBStart++ = 0; };
}

/*** ユーザ IO 処理 *********************************************************/

void ProcessUIO( void ){
	UCHAR c;
	while( sci_read( &c, 1 )) DoInputSerial( c );	// serial 入力
	if( g_Flags.bOutputSerial )	OutputSerial();		// serial 出力
	ProcessPushSW();								// sw 入力
}

/*** ステート変化待ち & LED 表示 ********************************************/

INLINE void WaitStateChange( void ){
	ULONG	uGx 	= 0;
	ULONG	uGy 	= 0;
	UINT	uCnt	= 0;
	
	UCHAR	cTimerA;
	
	/*** ステート変化待ち ***/
	
	/*** WDT ***/
	WDT.TCSRWD.BYTE = ( 1 << 6 );	// TCWE
	WDT.TCWD		= 0;
	
	cTimerA = TA.TCA & ~( CALC_DIVCNT - 1 );
	
	while( cTimerA == ( TA.TCA & ~( CALC_DIVCNT - 1 ))){
		uGx += G_SENSOR_Z;	// 前後 G の検出軸変更
		uGy += G_SENSOR_Y;
		++uCnt;
		if( !( uCnt & ( 128 - 1 ))) LED_Driver();
	}
	
	// G の計算
	if( g_DispVal.uGx ){
		g_DispVal.uGx = ((( UINT )( uGx / uCnt )) >> 1 ) + ( g_DispVal.uGx >> 1 );
		g_DispVal.uGy = ((( UINT )( uGy / uCnt )) >> 1 ) + ( g_DispVal.uGy >> 1 );
	}else{
		g_DispVal.uGx = ( UINT )( uGx / uCnt );
		g_DispVal.uGy = ( UINT )( uGy / uCnt );
	}
}

/*** IR 投光機用の main *****************************************************/

void IR_Flasher( void ){
	
	set_imask_ccr( 1 );
	
	/*** ベクタ設定 *********************************************************/
	
	IENR1.BYTE = 0;
	
	_INITSCT();
	
	// timer W 設定
	TW.GRA			= ( ULONG )H8HZ / 38e3 / 2;
	TW.TCRW.BYTE	= 1 << 7;			// counter clr cmp.match A
	TW.TMRW.BYTE	= 1 << 7;			// start TimerW
	//TW.TIERW.BYTE	= 1 << 0;			// enable cmp.match A int.
	
	TW.TIOR0.BYTE	= 0x3;				// compare match A toggle output
	
	/*
	// timer A 設定
	IO.PMR1.BYTE	= 1;				// output clk
	TA.TMA.BYTE		= 4 << 5;			// output 1khz
	*/
	//IO.PDR1.BYTE	= 1;
	
	// timer V 設定
	TV.TCRV0.BYTE =
		( 1 << 6 ) |	// CntV == CMFA で割り込み
		( 1 << 3 ) | 	// コンペアマッチA でクリア
		2;				// clk = φ/16 { TV.TCRV0.BYTE[2:0], TV.TCRV0.BYTE[0] }
	
	TV.TCRV1.BYTE = 0;	// clk = φ/16 { TV.TCRV0.BYTE[2:0], TV.TCRV0.BYTE[0] }
	
	// TCORA を 4KHz に設定
	TV.TCORA	= H8HZ / 16 / 4000;
	
	sci_init( 0 );
	set_imask_ccr( 0 );					/* CPU permit interrupts */
	
	Print( EOL "IR mode" EOL );
	
	for(;;) _builtin_sleep();
}
