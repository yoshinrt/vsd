/*****************************************************************************
	
	$Id$
	
	VSD - poor VSD
	Copyright(C) by DDS
	
	main.h -- main / main2 common header
	
*****************************************************************************/

/*** common macros **********************************************************/

#define H8HZ			16030000
#define SERIAL_DIVCNT	16		// シリアル出力を行う周期
#define LOG_FREQ		16

// 現在ギアを求めるときのスレッショルド設定値
// 10%マージンは廃止
#define GEAR_TH( g )	(( UINT )( GEAR_RATIO ## g * 256 + 0.5 ))

// スピード * 100/Taco 比
// ギア比 * マージンじゃなくて，ave( ギアn, ギアn+1 ) に変更
#define KPH_GEAR1 0.981168441
#define KPH_GEAR2 1.637149627
#define KPH_GEAR3 2.187031944
#define KPH_GEAR4 2.7815649
#define KPH_GEAR5 3.370602172

#define GEAR_RATIO1 (( KPH_GEAR1 + KPH_GEAR2 ) / 2 )
#define GEAR_RATIO2 (( KPH_GEAR2 + KPH_GEAR3 ) / 2 )
#define GEAR_RATIO3 (( KPH_GEAR3 + KPH_GEAR4 ) / 2 )
#define GEAR_RATIO4 (( KPH_GEAR4 + KPH_GEAR5 ) / 2 )

// シフトダウン警告時，下のギアでもこの回転数を下回ったら警告
#define SHIFTDOWN_WARN	6500

#define REV_LIMIT		6500
#define SH_DOWN_TH( g )	(( UINT )( SHIFTDOWN_WARN * KPH_GEAR ## g ))


// たぶん，ホイル一周が30パルス
#define PULSE_PAR_1KM	(( double )68774.48913 / 4.597593609 )

#define ITOA_RADIX_BIT	7
#define ITOA_DIGIT_NUM	(( 32 + ITOA_RADIX_BIT - 1 ) / ITOA_RADIX_BIT )

#define ACC_1G_X	6762.594337
#define ACC_1G_Y	6591.556755
#define ACC_1G_Z	6659.691379

/*** macros *****************************************************************/

#ifndef _WIN32
//#define RISE_EDGE_CAR_SIGNAL

//#define IR_FLASHER
//#define TEST1SEC

// ベクタセットアップ
#define SetVector( v, f )	( *( void **)(( v ) * 2 + 0xFF4C ) = ( f ))

// モニター起動
#define MONITOR_ENTRY	( *(( void (*)( void ))0x10E ))
//#define GoMonitor	INT000

// ソフトリセット
#define SoftReset	( *(( void (*)( void ))0x100 ))

// LED アニメーションデータ定義用
#define LED_ANIME_PARAM_NUM	6
#define LED_ANIME( a, b, c, d, bar, time ) \
	( time ) / ( CALC_DIVCNT ), FONT_ ## a, FONT_ ## b, FONT_ ## c, FONT_ ## d, bar

// VRAM 用データ一括生成
#define MakeDisp( a, b, c, d ) \
	(( FONT_ ## a << 24 ) | ( FONT_ ## b << 16 ) | ( FONT_ ## c << 8 ) | FONT_ ## d )

// AD 変換機レジスタ
#define	G_SENSOR_X	AD.ADDRC
#define	G_SENSOR_Y	AD.ADDRB
#define	G_SENSOR_Z	AD.ADDRA

#define BEEP_OFF	0xFFFF

#define BZero( v )			bzero(( UCHAR *)( &v ), sizeof( v ))
#define VOLATILE( t, v )	( *( volatile t *)&v )

/*** const ******************************************************************/

#define BLINK_RATE	( 1 << 4 )	// ブリンクRate

#define CALC_DIVCNT		8		// Taco 計算等を行う周期
#define DISP_DIVCNT		1		// speed 表示を行う周期

#define SPEED_ADJ		(( ULONG )(( double )H8HZ * 3600 * 100 / PULSE_PAR_1KM ))
#define TACO_ADJ		(( ULONG )(( double )H8HZ * 60 / 2 ))

// 直前のNewLapからこれだけ空かないとNewLapとして認めない(3秒)
#define NEWLAP_MIN_INTERVAL	( 3 * 256 )

#define EOL		"\r\n"

// LED 表示関係
enum {
	MODE_LAPTIME,
	MODE_ZERO_FOUR,
	MODE_ZERO_ONE,
	MODE_ZERO_ONE_WAIT,	// 0-100 ゴール待ち
};

enum {	// g_Flags の bit幅に注意
	DISPMODE_OPENING,
	DISPMODE_MSG,
	DISPMODE_MSG_LOOP,
	DISPMODE_MSG_PC,
	DISPMODE_ANIME_PC,
	DISPMODE_SPEED,
	DISPMODE_TACHO,
	DISPMODE_MILEAGE,
	DISPMODE_IR,
};

// ギア・タコ表示関係
enum {
	GM_TOWN,			// 0 town モードのタコバー
	GM_CIRCUIT,			// 1 サーキットモードのタコバー
	GM_BL_MAIN,			// 2 +レブリミット時に Main ブリンク
	GM_GEAR,			// 3 +ギア警告
	GM_DESIRED_GEAR,	// 4 +最適ギア表示
};

// タッチパッドのクリック数
enum {
	SWCMD_TACHO_SPD		= 1,
	SWCMD_AUTOMODE,
	SWCMD_VCARIB,
};

enum {
	AM_OFF,
	AM_TBAR,	// タコバー自動切換え
	AM_DISP,	// TBAR + Tacho⇔Speed の自動切換え
	AM_NUM
};

#define DISP_FINISH		0xFFFF
#define DBL_CLICK_TIME	8

/*** new type ***************************************************************/

// 上詰めなので下に追記する
typedef struct {
	UCHAR	uDispModeNext	:4;
	UCHAR	uLapMode		:2;
	BOOL	bBlinkMain		:1;
	BOOL	bBlinkSub		:1;
	BOOL	bBeep			:1;
	BOOL	bNewLap			:1;
	UCHAR	uGearMode		:5;
	UCHAR	uDispMode		:5;
	UCHAR	uAutoMode		:2;
} Flags_t;

typedef struct {
	UCHAR	uPrev		:4;
	UCHAR	uTrig		:4;
} PushSW_t;

typedef struct {
	union {
		ULONG	dw;
		struct	{ UINT	h, l; } w;
	} Time;					// timer W のカウント
	
	union {
		ULONG	dw;
		struct	{ UINT	h, l; } w;
	} PrevTime;				// 前回の timer W のカウント
	
	UINT	uPulseCnt;		// パルス入力回数
	UINT	uVal;			// スピード等の計算値
} PULSE;

typedef union {
	ULONG	dw;
	struct	{ UINT	h, l; } w;
} UNI_LONG;

typedef union {
	ULONG	lDisp;
	UCHAR	cDisp[ 4 ];
} VRAM;

typedef struct {
	UCHAR	uPushElapsed;
	UCHAR	uPushCnt;
} TouchPanel_t;

typedef struct{
	ULONG	uTacho;
	ULONG	uSpeed;
	ULONG	uGx, uGy;
	UINT	uPrevGx;
	UINT	uCnt;
} DispVal_t;

/*** extern *****************************************************************/
#endif
