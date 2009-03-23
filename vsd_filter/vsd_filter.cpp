//----------------------------------------------------------------------------------
//		サンプルビデオフィルタ(フィルタプラグイン)  for AviUtl ver0.96e以降
//----------------------------------------------------------------------------------
// $Id$

#define _CRT_SECURE_NO_DEPRECATE 1
//#define CIRCUIT_TOMO

#include <windows.h>
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <float.h>
#include <stddef.h>

#include "dds.h"
#include "filter.h"
#include "../vsd/main.h"
#include "dds_lib/dds_lib.h"
#include "CVsdImg.h"

#ifdef CIRCUIT_TOMO
	#define	FILE_EXT		"Pulse-Time Data (*.ptd)\0*.ptd\0Config File (*.cfg)\0*.cfg\0AllFile (*.*)\0*.*\0"
#else
	#define	FILE_EXT		"LogFile (*.log)\0*.log\0Config File (*.cfg)\0*.cfg\0AllFile (*.*)\0*.*\0"
#endif

#define	FILE_CFG_EXT		"Config File (*.cfg)\0*.cfg\0AllFile (*.*)\0*.*\0"

/****************************************************************************/

#define BUF_SIZE	1024

#define PI			3.14159265358979323
#define ToRAD		( PI / 180 )
#define LAT_M_DEG	110863.95	// 緯度1度の距離
#define LNG_M_DEG	111195.10	// 経度1度の距離 @ 0N

#define RGB2YC( r, g, b ) { \
	( int )( 0.299 * r + 0.587 * g + 0.114 * b + .5 ), \
	( int )(-0.169 * r - 0.332 * g + 0.500 * b + .5 ), \
	( int )( 0.500 * r - 0.419 * g - 0.081 * b + .5 ) \
}

#define MAX_VSD_LOG		(( int )( LOG_FREQ * 3600 * 2 ))
#define MAX_LAP			200

#define VideoSt			( fp->track[ TRACK_VSt ] * 100 + fp->track[ TRACK_VSt2 ] )
#define VideoEd			( fp->track[ TRACK_VEd ] * 100 + fp->track[ TRACK_VEd2 ] )
#ifdef CIRCUIT_TOMO
	#define LogSt		(( fp->track[ TRACK_LSt ] * 1000 + fp->track[ TRACK_LSt2 ] ) / 1000 * LOG_FREQ )
	#define LogEd		(( fp->track[ TRACK_LEd ] * 1000 + fp->track[ TRACK_LEd2 ] ) / 1000 * LOG_FREQ )
#else
	#define LogSt		( fp->track[ TRACK_LSt ] * 100 + fp->track[ TRACK_LSt2 ] )
	#define LogEd		( fp->track[ TRACK_LEd ] * 100 + fp->track[ TRACK_LEd2 ] )
#endif
#define LineTrace		fp->track[ TRACK_LineTrace ]

#define DispLap			fp->check[ CHECK_LAP ]
#define DispGSnake		fp->check[ CHECK_SNAKE ]

#define G_CX_CNT		30
#define G_HIST			(( int )( LOG_FREQ * 3 ))
#define MAX_G_SCALE		1.5

#define MAX_MAP_SIZE	( Img.w * fp->track[ TRACK_MapSize ] / 1000.0 )
#define MAP_ANGLE		( fp->track[ TRACK_MapAngle ] * ( -ToRAD / 10 ))
#define INVALID_POS_I	0x7FFFFFFF
#define INVALID_POS_D	NaN

#define LINE_WIDTH		( Img.w / HIREZO_TH + 1 )

#define GPS_LOG_OFFS	15

#define MAP_LINE1		yc_green
#define MAP_LINE2		yc_yellow
#define MAP_LINE3		yc_red
#define MAP_G_MAX		1.2

#define PTD_LOG_FREQ	11025.0

// 手動ラップタイム計測モードかどうか
#ifdef CIRCUIT_TOMO
	#define IS_HAND_LAPTIME	TRUE
#else
	#define IS_HAND_LAPTIME	( g_iVsdLogNum == 0 )
#endif

/*** new type ***************************************************************/

typedef struct {
	float	fSpeed;
	float	fTacho;
#ifndef CIRCUIT_TOMO
	float	fMileage;
	float	fGx, fGy;
	float	fX, fX0;
	float	fY, fY0;
#endif
} VSD_LOG_t;

typedef struct {
	USHORT	uLap;
	int		iLogNum;
	float	fTime;
} LAP_t;

typedef struct {
	float	fX;
	float	fY;
	float	fVX;
	float	fVY;
	int		iLogNum;
} GPS_LOG_t;

/*** gloval var *************************************************************/

HINSTANCE	g_hInst = NULL;

VSD_LOG_t	*g_VsdLog 		= NULL;
int			g_iVsdLogNum	= 0;
LAP_t		*g_Lap	 		= NULL;
int			g_iLapNum		= 0;
float		g_fBestTime		= -1;
int			g_iBestLapLogNum= 0;

double		g_dMapSize		= 0;
double		g_dMapOffsX		= 0;
double		g_dMapOffsY		= 0;

int			g_iLogStart;
int			g_iLogStop;

double		g_dVideoFPS  = 30.0;

/*** tarckbar / checkbox id 名 ***/

const char *g_szTrackbarName[] = {
	#define DEF_TRACKBAR( id, init, min, max, name ) #id,
	#include "def_trackbar.h"
};

const char *g_szCheckboxName[] = {
	#define DEF_CHECKBOX( id, init, name ) #id,
	#include "def_checkbox.h"
};

/****************************************************************************/
//---------------------------------------------------------------------
//		フィルタ構造体定義
//---------------------------------------------------------------------

// トラックバーの名前
TCHAR	*track_name[] = {
	#define DEF_TRACKBAR( id, init, min, max, name )	name,
	#include "def_trackbar.h"
};
// トラックバーの初期値
int		track_default[] = {
	#define DEF_TRACKBAR( id, init, min, max, name )	init,
	#include "def_trackbar.h"
};
// トラックバーの下限値
int		track_s[] = {
	#define DEF_TRACKBAR( id, init, min, max, name )	min,
	#include "def_trackbar.h"
};
// トラックバーの上限値
int		track_e[] = {
	#define DEF_TRACKBAR( id, init, min, max, name )	max,
	#include "def_trackbar.h"
};

enum {
	#define DEF_TRACKBAR( id, init, min, max, name )	id,
	#include "def_trackbar.h"
	TRACK_N
};

// チェックボックスの名前
TCHAR	*check_name[] = {
	#define DEF_CHECKBOX( id, init, name )	name,
	#include "def_checkbox.h"
};

// チェックボックスの初期値 (値は0か1)
int		check_default[] = {
	#define DEF_CHECKBOX( id, init, name )	init,
	#include "def_checkbox.h"
};

enum {
	#define DEF_CHECKBOX( id, init, name )	id,
	#include "def_checkbox.h"
	CHECK_N
};

FILTER_DLL filter = {
	FILTER_FLAG_EX_INFORMATION | FILTER_FLAG_IMPORT | FILTER_FLAG_EXPORT | FILTER_FLAG_MAIN_MESSAGE,
								//	フィルタのフラグ
								//	FILTER_FLAG_ALWAYS_ACTIVE		: フィルタを常にアクティブにします
								//	FILTER_FLAG_CONFIG_POPUP		: 設定をポップアップメニューにします
								//	FILTER_FLAG_CONFIG_CHECK		: 設定をチェックボックスメニューにします
								//	FILTER_FLAG_CONFIG_RADIO		: 設定をラジオボタンメニューにします
								//	FILTER_FLAG_EX_DATA				: 拡張データを保存出来るようにします。
								//	FILTER_FLAG_PRIORITY_HIGHEST	: フィルタのプライオリティを常に最上位にします
								//	FILTER_FLAG_PRIORITY_LOWEST		: フィルタのプライオリティを常に最下位にします
								//	FILTER_FLAG_WINDOW_THICKFRAME	: サイズ変更可能なウィンドウを作ります
								//	FILTER_FLAG_WINDOW_SIZE			: 設定ウィンドウのサイズを指定出来るようにします
								//	FILTER_FLAG_DISP_FILTER			: 表示フィルタにします
								//	FILTER_FLAG_EX_INFORMATION		: フィルタの拡張情報を設定できるようにします
								//	FILTER_FLAG_NO_CONFIG			: 設定ウィンドウを表示しないようにします
								//	FILTER_FLAG_AUDIO_FILTER		: オーディオフィルタにします
								//	FILTER_FLAG_RADIO_BUTTON		: チェックボックスをラジオボタンにします
								//	FILTER_FLAG_WINDOW_HSCROLL		: 水平スクロールバーを持つウィンドウを作ります
								//	FILTER_FLAG_WINDOW_VSCROLL		: 垂直スクロールバーを持つウィンドウを作ります
								//	FILTER_FLAG_IMPORT				: インポートメニューを作ります
								//	FILTER_FLAG_EXPORT				: エクスポートメニューを作ります
	0,0,						//	設定ウインドウのサイズ (FILTER_FLAG_WINDOW_SIZEが立っている時に有効)
	"VSDメーター合成",			//	フィルタの名前
	TRACK_N,					//	トラックバーの数 (0なら名前初期値等もNULLでよい)
	track_name,					//	トラックバーの名前郡へのポインタ
	track_default,				//	トラックバーの初期値郡へのポインタ
	track_s, track_e,			//	トラックバーの数値の下限上限 (NULLなら全て0～256)
	CHECK_N,					//	チェックボックスの数 (0なら名前初期値等もNULLでよい)
	check_name,					//	チェックボックスの名前郡へのポインタ
	check_default,				//	チェックボックスの初期値郡へのポインタ
	func_proc,					//	フィルタ処理関数へのポインタ (NULLなら呼ばれません)
	NULL /*func_init*/,			//	開始時に呼ばれる関数へのポインタ (NULLなら呼ばれません)
	NULL /*func_exit*/,			//	終了時に呼ばれる関数へのポインタ (NULLなら呼ばれません)
	func_update,				//	設定が変更されたときに呼ばれる関数へのポインタ (NULLなら呼ばれません)
	func_WndProc,				//	設定ウィンドウにウィンドウメッセージが来た時に呼ばれる関数へのポインタ (NULLなら呼ばれません)
	NULL,NULL,					//	システムで使いますので使用しないでください
	NULL,						//  拡張データ領域へのポインタ (FILTER_FLAG_EX_DATAが立っている時に有効)
	NULL,						//  拡張データサイズ (FILTER_FLAG_EX_DATAが立っている時に有効)
	NULL,						//  フィルタ情報へのポインタ (FILTER_FLAG_EX_INFORMATIONが立っている時に有効)
	NULL,						//	セーブが開始される直前に呼ばれる関数へのポインタ (NULLなら呼ばれません)
	NULL,						//	セーブが終了した直前に呼ばれる関数へのポインタ (NULLなら呼ばれません)
};


//---------------------------------------------------------------------
//		フィルタ構造体のポインタを渡す関数
//---------------------------------------------------------------------

EXTERN_C FILTER_DLL __declspec(dllexport) * __stdcall GetFilterTable( void ){
	return &filter;
}

//---------------------------------------------------------------------
//		初期化
//---------------------------------------------------------------------

/*
BOOL func_init( FILTER *fp ){
	
	return TRUE;
}
*/

//---------------------------------------------------------------------
//		終了
//---------------------------------------------------------------------

/*
BOOL func_exit( FILTER *fp ){
	
	delete [] g_VsdLog;
	delete [] g_Lap;
	return TRUE;
}
*/

//---------------------------------------------------------------------
//		フィルタ処理関数
//---------------------------------------------------------------------

/*** ラップタイム再計算 *****************************************************/

BOOL	g_bCalcLapTimeReq;

#ifdef AVS_PLUGIN
void CalcLapTime( void ){
	// ★ under construction ...
}
#else
void CalcLapTime( FILTER *fp, FILTER_PROC_INFO *fpip ){
	
	FRAME_STATUS	fsp;
	int				i;
	double			dTime, dPrevTime;
	
	g_iLapNum	= 0;
	g_fBestTime	= -1;
	
	for( i = 0; i < fpip->frame_n; ++i ){
		fp->exfunc->get_frame_status( fpip->editp, i, &fsp );
		
		if( fsp.edit_flag & EDIT_FRAME_EDIT_FLAG_MARKFRAME ){
			// ラップ検出
			dTime = i / g_dVideoFPS;
			
			g_Lap[ g_iLapNum ].uLap		= g_iLapNum;
			g_Lap[ g_iLapNum ].iLogNum	= i;
			g_Lap[ g_iLapNum ].fTime	= g_iLapNum ? ( float )( dTime - dPrevTime ) : 0;
			
			if(
				g_iLapNum &&
				( g_fBestTime == -1 || g_fBestTime > g_Lap[ g_iLapNum ].fTime )
			){
				g_fBestTime			= g_Lap[ g_iLapNum ].fTime;
				g_iBestLapLogNum	= g_Lap[ g_iLapNum - 1 ].iLogNum;
			}
			
			dPrevTime = dTime;
			++g_iLapNum;
		}
	}
	g_Lap[ g_iLapNum ].iLogNum	= 0x7FFFFFFF;	// 番犬
	g_Lap[ g_iLapNum ].fTime	= 0;			// 番犬
}
#endif

/****************************************************************************/

static const PIXEL_YC	yc_black		= RGB2YC(    0,    0,    0 );
static const PIXEL_YC	yc_white		= RGB2YC( 4095, 4095, 4095 );
static const PIXEL_YC	yc_gray			= RGB2YC( 2048, 2048, 2048 );
static const PIXEL_YC	yc_red			= RGB2YC( 4095,    0,    0 );
static const PIXEL_YC	yc_green		= RGB2YC(    0, 4095,    0 );
static const PIXEL_YC	yc_yellow		= RGB2YC( 4095, 4095,    0 );
static const PIXEL_YC	yc_dark_green	= RGB2YC(    0, 2048,    0 );
static const PIXEL_YC	yc_blue			= RGB2YC(    0,    0, 4095 );
static const PIXEL_YC	yc_cyan			= RGB2YC(    0, 4095, 4095 );
static const PIXEL_YC	yc_dark_blue	= RGB2YC(    0,    0, 2048 );
static const PIXEL_YC	yc_orange		= RGB2YC( 4095, 1024,    0 );

#define COLOR_PANEL			yc_gray
#define COLOR_NEEDLE		yc_red
#define COLOR_SCALE			yc_white
#define COLOR_STR			COLOR_SCALE
#define COLOR_TIME			yc_white
#define COLOR_TIME_EDGE		yc_black
#define COLOR_BEST_LAP		yc_cyan
#define COLOR_G_SENSOR		yc_green
#define COLOR_G_HIST		yc_dark_green
#define COLOR_DIFF_MINUS	yc_cyan
#define COLOR_DIFF_PLUS		yc_red
#define COLOR_CURRENT_POS	yc_red
#define COLOR_FASTEST_POS	yc_green

// 半端な dLogNum 値からログの中間値を求める
#define GetVsdLog( p ) ( \
	g_VsdLog[ ( UINT )dLogNum     ].p * ( 1 - ( dLogNum - ( UINT )dLogNum )) + \
	g_VsdLog[ ( UINT )dLogNum + 1 ].p * (       dLogNum - ( UINT )dLogNum ))


BOOL func_proc( FILTER *fp,FILTER_PROC_INFO *fpip ){
//
//	fp->track[n]			: トラックバーの数値
//	fp->check[n]			: チェックボックスの数値
//	fpip->w 				: 実際の画像の横幅
//	fpip->h 				: 実際の画像の縦幅
//	fpip->w					: 画像領域の横幅
//	fpip->h					: 画像領域の縦幅
//	fpip->ycp_edit			: 画像領域へのポインタ
//	fpip->ycp_temp			: テンポラリ領域へのポインタ
//	fpip->ycp_edit[n].y		: 画素(輝度    )データ (     0 ～ 4095 )
//	fpip->ycp_edit[n].cb	: 画素(色差(青))データ ( -2048 ～ 2047 )
//	fpip->ycp_edit[n].cr	: 画素(色差(赤))データ ( -2048 ～ 2047 )
//
//  画素データは範囲外に出ていることがあります。
//  また範囲内に収めなくてもかまいません。
//
//	画像サイズを変えたいときは fpip->w や fpip->h を変えます。
//
//	テンポラリ領域に処理した画像を格納したいときは
//	fpip->ycp_edit と fpip->ycp_temp を入れ替えます。
//
	
	static char	szBuf[ 128 ];
	static	int	iLapIdx	= -1;
	int	i;
	
	BOOL	bInLap = FALSE;	// ラップタイム計測中
	
	// クラスに変換
	CVsdImg	&Img = *( CVsdImg *)fpip;
	
#ifdef CIRCUIT_TOMO
	const int	iMeterR			= 50 * Img.w / 320;
	const int	iMeterCx		= iMeterR * 3 + 4;
	const int	iMeterCy		= ( int )( Img.h - iMeterR * 0.75 - 2 );
	const int	iMeterMinDeg	= 135;
	const int	iMeterMaxDeg	= 45;
	const int	iMeterMaxVal	= fp->track[ TRACK_TACHO ] * 1000;
	const int	iMeterDegRange	= ( iMeterMaxDeg + 360 - iMeterMinDeg ) % 360;
	const int	iMeterScaleLen	= iMeterR / 8;
	
	const int	iMeterSCx		= iMeterR + 2;
	const int	iMeterSMaxVal	= fp->track[ TRACK_SPEED ];
#else
	const int	iMeterR			= 50 * Img.w / 320;
	const int	iMeterCx		= Img.w - iMeterR - 2;
	const int	iMeterCy		= Img.h - iMeterR - 2;
	const int	iMeterMinDeg	= 135;
	const int	iMeterMaxDeg	= 45;
	const int	iMeterMaxVal	= 7000;
	const int	iMeterDegRange	= ( iMeterMaxDeg + 360 - iMeterMinDeg ) % 360;
	const int	iMeterScaleLen	= iMeterR / 8;
#endif
	
	// フォントサイズ初期化
	Img.InitFont();
	
	// ログ位置の計算
	double	dLogNum = ( VideoEd == VideoSt ) ? -1 :
						( double )( LogEd - LogSt ) / ( VideoEd - VideoSt ) * ( Img.frame - VideoSt ) + LogSt;
	int		iLogNum = ( int )dLogNum;
	
	// フレーム表示
	if( fp->check[ CHECK_FRAME ] ){
		sprintf( szBuf, "V%6d/%6d", Img.frame, Img.frame_n - 1 );
		Img.DrawString( szBuf, COLOR_STR, COLOR_TIME_EDGE, 0, Img.w / 2, Img.h / 2 );
	#ifdef CIRCUIT_TOMO
		sprintf( szBuf, "L%.3f/%.3f", ( double )dLogNum / LOG_FREQ, g_iVsdLogNum / LOG_FREQ );
	#else
		sprintf( szBuf, "L%6d/%6d", ( int )dLogNum, g_iVsdLogNum - 1 );
	#endif
		Img.DrawString( szBuf, COLOR_STR, COLOR_TIME_EDGE, 0 );
	}
	
	/*** Lap タイム描画 ***/
	
	if( IS_HAND_LAPTIME && DispLap && g_bCalcLapTimeReq && g_Lap ){
		g_bCalcLapTimeReq = FALSE;
		#ifdef AVS_PLUGIN
			CalcLapTime();
		#else
			CalcLapTime( fp, fpip );
		#endif
	}
	
	if( DispLap && g_iLapNum ){
		/*
		Img.DrawRect(
			iLapX, iLapY, Img.w - 1, iLapY + Img.GetFontH() * 5 - 1,
			COLOR_PANEL,
			CVsdImg::IMG_ALFA | CVsdImg::IMG_FILL
		);
		*/
		
		if( IS_HAND_LAPTIME ){
			// CIRCUIT_TOMO での g_Lap[].iLogNum はフレーム# なので
			iLogNum = Img.frame;
		}
		
		// カレントポインタがおかしいときは，-1 にリセット
		if(
			iLapIdx >= g_iLapNum ||
			iLapIdx >= 0 && g_Lap[ iLapIdx ].iLogNum > iLogNum
		) iLapIdx = -1;
		
		for( ; g_Lap[ iLapIdx + 1 ].iLogNum <= iLogNum; ++iLapIdx );
		
		// 時間表示
		if( iLapIdx >= 0 && g_Lap[ iLapIdx + 1 ].fTime != 0 ){
			double dTime = IS_HAND_LAPTIME ?
				( double )( Img.frame - g_Lap[ iLapIdx ].iLogNum ) / g_dVideoFPS :
				( dLogNum - g_Lap[ iLapIdx ].iLogNum ) / LOG_FREQ;
			
			sprintf( szBuf, "Time%2d'%02d.%03d", ( int )dTime / 60, ( int )dTime % 60, ( int )( dTime * 1000 ) % 1000 );
			Img.DrawString( szBuf, COLOR_TIME, COLOR_TIME_EDGE, 0, Img.w - Img.GetFontW() * 13, 1 );
			bInLap = TRUE;
		}else{
			// まだ開始していない
			Img.DrawString( "Time -'--.---", COLOR_TIME, COLOR_TIME_EDGE, 0, Img.w - Img.GetFontW() * 13, 1 );
		}
		
	#ifndef CIRCUIT_TOMO
		/*** ベストとの車間距離表示 ***/
		if( !IS_HAND_LAPTIME ){
			if( bInLap ){
				// この周の走行距離を求める
				double dMileage = GetVsdLog( fMileage ) - g_VsdLog[ g_Lap[ iLapIdx ].iLogNum ].fMileage;
				
				// 最速 Lap の，同一走行距離におけるタイム (=ログ番号,整数) を求める
				// iBestLogNum <= 最終的に求める結果 < iBestLogNum + 1  となる
				static int iBestLogNum = 0;
				
				// iBestLogNum がおかしかったら，リセット
				if(
					iBestLogNum < g_iBestLapLogNum ||
					iBestLogNum >= g_iVsdLogNum ||
					( g_VsdLog[ iBestLogNum ].fMileage - g_VsdLog[ g_iBestLapLogNum ].fMileage ) > dMileage
				) iBestLogNum = g_iBestLapLogNum;
				
				for(
					;
					( g_VsdLog[ iBestLogNum + 1 ].fMileage - g_VsdLog[ g_iBestLapLogNum ].fMileage ) <= dMileage &&
					iBestLogNum < g_iVsdLogNum;
					++iBestLogNum
				);
				
				// 最速 Lap の，1/15秒以下の値を求める = A / B
				double dBestLogNum =
					( double )iBestLogNum +
					// A: 最速ラップは，後これだけ走らないと dMileage と同じではない
					( dMileage - ( g_VsdLog[ iBestLogNum ].fMileage - g_VsdLog[ g_iBestLapLogNum ].fMileage )) /
					// B: 最速ラップは，1/15秒の間にこの距離を走った
					( g_VsdLog[ iBestLogNum + 1 ].fMileage - g_VsdLog[ iBestLogNum ].fMileage );
				
				double dDiffTime =
					(
						( dLogNum - g_Lap[ iLapIdx ].iLogNum ) -
						( dBestLogNum - g_iBestLapLogNum )
					) / LOG_FREQ;
				
				if( -0.001 < dDiffTime && dDiffTime < 0.001 ) dDiffTime = 0;
				
				sprintf(
					szBuf, "    %c%d'%06.3f",
					dDiffTime <= 0 ? '-' : '+',
					( int )( fabs( dDiffTime )) / 60,
					fmod( fabs( dDiffTime ), 60 )
				);
				Img.DrawString( szBuf, dDiffTime <= 0 ? COLOR_DIFF_MINUS : COLOR_DIFF_PLUS, COLOR_TIME_EDGE, 0 );
			}else{
				Img.m_iPosY += Img.GetFontH();
			}
		}
	#endif
		
		Img.m_iPosY += Img.GetFontH() / 4;
		
		// Best 表示
		sprintf(
			szBuf, "Best%2d'%02d.%03d",
			( int )g_fBestTime / 60,
			( int )g_fBestTime % 60,
			( int )( g_fBestTime * 1000 ) % 1000
		);
		Img.DrawString( szBuf, COLOR_TIME, COLOR_TIME_EDGE, 0 );
		
		// Lapタイム表示
		i = 0;
		for( int iLapIdxTmp = iLapIdx + 1; iLapIdxTmp >= 0 && i < 3; --iLapIdxTmp ){
			if( g_Lap[ iLapIdxTmp ].fTime != 0 ){
				sprintf(
					szBuf, "%3d%c%2d'%02d.%03d",
					g_Lap[ iLapIdxTmp ].uLap,
					( i == 0 && bInLap ) ? '*' : ' ',
					( int )g_Lap[ iLapIdxTmp ].fTime / 60,
					( int )g_Lap[ iLapIdxTmp ].fTime % 60,
					( int )( g_Lap[ iLapIdxTmp ].fTime * 1000 ) % 1000
				);
				Img.DrawString( szBuf,
					g_fBestTime == g_Lap[ iLapIdxTmp ].fTime ? COLOR_BEST_LAP : COLOR_TIME,
					COLOR_TIME_EDGE, 0 );
				++i;
			}
		}
	}
	
	if( g_iVsdLogNum == 0 ) return TRUE;
	
	/*** メーターパネル ***/
	Img.DrawCircle(
		iMeterCx, iMeterCy, iMeterR, COLOR_PANEL,
		CVsdImg::IMG_ALFA | CVsdImg::IMG_FILL
	);
	
	// タコメータ
	for( i = 0; i <= iMeterMaxVal; i += 500 ){
		int iDeg = iMeterDegRange * i / iMeterMaxVal + iMeterMinDeg;
		if( iDeg >= 360 ) iDeg -= 360;
		
		// メーターパネル目盛り
		if( iMeterMaxVal <= 12000 || i % 1000 == 0 ){
			Img.DrawLine(
				( int )( cos( iDeg * ToRAD ) * iMeterR ) + iMeterCx,
				( int )( sin( iDeg * ToRAD ) * iMeterR ) + iMeterCy,
				( int )( cos( iDeg * ToRAD ) * ( iMeterR - iMeterScaleLen )) + iMeterCx,
				( int )( sin( iDeg * ToRAD ) * ( iMeterR - iMeterScaleLen )) + iMeterCy,
				( iMeterMaxVal <= 12000 && i % 1000 == 0 || i % 2000 == 0 ) ? 2 : 1,
				COLOR_SCALE, 0
			);
			
			// メーターパネル目盛り数値
			if( iMeterMaxVal <= 12000 && i % 1000 == 0 || i % 2000 == 0 ){
				sprintf( szBuf, "%d", i / 1000 );
				Img.DrawString(
					szBuf,
					COLOR_STR, 0,
					( int )( cos( iDeg * ToRAD ) * iMeterR * .8 ) + iMeterCx - Img.GetFontW() / ( i >= 10000 ? 1 : 2 ),
					( int )( sin( iDeg * ToRAD ) * iMeterR * .8 ) + iMeterCy - Img.GetFontH() / 2
				);
			}
		}
	}
	
#ifdef CIRCUIT_TOMO
	// スピードメーターパネル
	Img.DrawCircle(
		iMeterSCx, iMeterCy, iMeterR, COLOR_PANEL,
		CVsdImg::IMG_ALFA | CVsdImg::IMG_FILL
	);
	
	int	iStep = (( iMeterSMaxVal / 20 ) + 4 ) / 5 * 5;
	
	for( i = 0; i <= iMeterSMaxVal; i += iStep ){
		int iDeg = iMeterDegRange * i / iMeterSMaxVal + iMeterMinDeg;
		if( iDeg >= 360 ) iDeg -= 360;
		
		// メーターパネル目盛り
		if( i % iStep == 0 ){
			Img.DrawLine(
				( int )( cos( iDeg * ToRAD ) * iMeterR ) + iMeterSCx,
				( int )( sin( iDeg * ToRAD ) * iMeterR ) + iMeterCy,
				( int )( cos( iDeg * ToRAD ) * ( iMeterR - iMeterScaleLen )) + iMeterSCx,
				( int )( sin( iDeg * ToRAD ) * ( iMeterR - iMeterScaleLen )) + iMeterCy,
				( i % ( iStep * 2 ) == 0 ) ? 2 : 1,
				COLOR_SCALE, 0
			);
			
			// メーターパネル目盛り数値
			if( i % ( iStep * 2 ) == 0 ){
				sprintf( szBuf, "%d", i );
				Img.DrawString(
					szBuf,
					COLOR_STR, 0,
					( int )( cos( iDeg * ToRAD ) * iMeterR * .75 ) + iMeterSCx - Img.GetFontW() * strlen( szBuf ) / 2,
					( int )( sin( iDeg * ToRAD ) * iMeterR * .75 ) + iMeterCy - Img.GetFontH() / 2
				);
			}
		}
	}
#endif // CIRCUIT_TOMO
	
	/*** メーター描画 ***/
	
	if( dLogNum < 0 || dLogNum > g_iVsdLogNum - 1 ) return TRUE;
	
	double	dSpeed	= GetVsdLog( fSpeed );
	double	dTacho	= GetVsdLog( fTacho );
	
#ifdef CIRCUIT_TOMO
	// スピード・タコ修正
	
	dSpeed *= 3600.0 / fp->track[ TRACK_PULSE_SPEED ];
	dTacho *= 1000.0 * 60 / fp->track[ TRACK_PULSE_TACHO ];
	
#else // CIRCUIT_TOMO
	// G スネーク
	int	iGx, iGy;
	
	if( DispGSnake ){
		
		int iGxPrev = 0, iGyPrev;
		
		for( i = -G_HIST; i <= 1 ; ++i ){
			
			if( iLogNum + i >= 0 ){
				// i == 1 時は最後の中途半端な LogNum
				iGx = iMeterCx + ( int )((( i != 1 ) ? g_VsdLog[ iLogNum + i ].fGx : GetVsdLog( fGx )) * iMeterR / MAX_G_SCALE );
				iGy = iMeterCy - ( int )((( i != 1 ) ? g_VsdLog[ iLogNum + i ].fGy : GetVsdLog( fGy )) * iMeterR / MAX_G_SCALE );
				
				if( iGxPrev ) Img.DrawLine(
					iGx, iGy, iGxPrev, iGyPrev,
					LINE_WIDTH, COLOR_G_HIST, 0
				);
				
				iGxPrev = iGx;
				iGyPrev = iGy;
			}
		}
	}else{
		iGx = iMeterCx + ( int )( GetVsdLog( fGx ) * iMeterR / MAX_G_SCALE );
		iGy = iMeterCy - ( int )( GetVsdLog( fGy ) * iMeterR / MAX_G_SCALE );
	}
	
	// G インジケータ
	Img.DrawCircle(
		iGx, iGy, iMeterR / 20,
		COLOR_G_SENSOR, CVsdImg::IMG_FILL
	);
	
	// MAP 表示
	if( LineTrace ){
		double dGx, dGy;
		
		int iGxPrev = INVALID_POS_I, iGyPrev;
		
		int	iLineSt = iLapIdx >= 0 ? g_Lap[ iLapIdx ].iLogNum : 0;
		if( iLogNum - iLineSt > ( int )( LineTrace * LOG_FREQ ))
			iLineSt = iLogNum - ( int )( LineTrace * LOG_FREQ );
		
		int iLineEd = iLapIdx != g_iLapNum - 1 ? g_Lap[ iLapIdx + 1 ].iLogNum : g_iVsdLogNum - 1;
		if( iLineEd - iLogNum > ( int )( LineTrace * LOG_FREQ ))
			iLineEd = iLogNum + ( int )( LineTrace * LOG_FREQ );
		
		for( i = iLineSt; i <= iLineEd ; ++i ){
			#define GetMapPos( p, a ) ( (( p ) - g_dMapOffs ## a ) / g_dMapSize * MAX_MAP_SIZE + 8 )
			dGx = GetMapPos( g_VsdLog[ i ].fX, X );
			dGy = GetMapPos( g_VsdLog[ i ].fY, Y );
			
			if( !_isnan( dGx )){
				iGx = ( int )dGx;
				iGy = ( int )dGy;
				
				if( iGxPrev != INVALID_POS_I ){
					// Line の色用に G を求める
					double dG = sqrt(
						g_VsdLog[ i ].fGx * g_VsdLog[ i ].fGx +
						g_VsdLog[ i ].fGy * g_VsdLog[ i ].fGy
					) / MAP_G_MAX;
					
					PIXEL_YC yc_line;
					
					if( dG < 0.5 ){
						dG *= 2;
						yc_line.y  = ( short )( MAP_LINE2.y  * dG + MAP_LINE1.y  * ( 1 - dG ));
						yc_line.cb = ( short )( MAP_LINE2.cb * dG + MAP_LINE1.cb * ( 1 - dG ));
						yc_line.cr = ( short )( MAP_LINE2.cr * dG + MAP_LINE1.cr * ( 1 - dG ));
					}else if( dG < 1.0 ){
						dG = ( dG - 0.5 ) * 2;
						yc_line.y  = ( short )( MAP_LINE3.y  * dG + MAP_LINE2.y  * ( 1 - dG ));
						yc_line.cb = ( short )( MAP_LINE3.cb * dG + MAP_LINE2.cb * ( 1 - dG ));
						yc_line.cr = ( short )( MAP_LINE3.cr * dG + MAP_LINE2.cr * ( 1 - dG ));
					}else{
						yc_line = MAP_LINE3;
					}
					
					// Line を引く
					Img.DrawLine(
						iGx, iGy, iGxPrev, iGyPrev,
						LINE_WIDTH, yc_line, 0
					);
				}
			}else{
				iGx = INVALID_POS_I;
			}
			
			iGxPrev = iGx;
			iGyPrev = iGy;
		}
		
		// MAP インジケータ (自車)
		dGx = GetMapPos( GetVsdLog( fX ), X );
		dGy = GetMapPos( GetVsdLog( fY ), Y );
		
		if( !_isnan( dGx )) Img.DrawCircle(
			( int )dGx, ( int )dGy, iMeterR / 20,
			COLOR_CURRENT_POS, CVsdImg::IMG_FILL
		);
	}
#endif // CIRCUIT_TOMO
	
	// スピード / ギア
	UINT uGear = 0;
	
	if( dTacho ){
	#ifdef CIRCUIT_TOMO
		UINT	u;
		uGear = 6;
		for( u = 1; u <= 5; ++u ){
			if(
				( fp->track[ TRACK_GEAR1 + u - 1 ] + fp->track[ TRACK_GEAR1 + u ] ) / 20000.0 >
				dSpeed / 3600 * 1000 * 60 / dTacho
			){
				uGear = u;
				break;
			}
		}
	#else
		UINT uGearRatio = ( int )( dSpeed * 100 * ( 1 << 8 ) / dTacho );
		
		if     ( uGearRatio < GEAR_TH( 1 ))	uGear = 1;
		else if( uGearRatio < GEAR_TH( 2 ))	uGear = 2;
		else if( uGearRatio < GEAR_TH( 3 ))	uGear = 3;
		else if( uGearRatio < GEAR_TH( 4 ))	uGear = 4;
		else								uGear = 5;
	#endif
	}
	
	// スピード表示
	sprintf( szBuf, "%d\x7F", uGear );
	Img.DrawString(
		szBuf,
		COLOR_STR, 0,
		iMeterCx - 1 * Img.GetFontW(), iMeterCy - iMeterR / 2
	);
	
	sprintf( szBuf, "%3d\x80\x81", ( int )dSpeed );
	Img.DrawString(
		szBuf,
		COLOR_STR, 0,
		iMeterCx - 3 * Img.GetFontW(), iMeterCy + iMeterR / 2
	);
	
#ifndef CIRCUIT_TOMO
	sprintf( szBuf, "%2d\x82", ( int )( sqrt( GetVsdLog( fGx ) * GetVsdLog( fGx ) + GetVsdLog( fGy ) * GetVsdLog( fGy )) * 10 ));
	Img.DrawString(
		szBuf,
		COLOR_STR, 0,
		iMeterCx - 2 * Img.GetFontW(), iMeterCy + iMeterR / 2 - Img.GetFontH()
	);
#else
	// Speed の針
	double dSpeedNeedle =
		iMeterDegRange / ( double )iMeterSMaxVal * dSpeed + iMeterMinDeg;
	if( dSpeedNeedle >= 360 ) dSpeedNeedle -= 360;
	dSpeedNeedle = dSpeedNeedle * ToRAD;
	
	Img.DrawLine(
		iMeterSCx, iMeterCy,
		( int )( cos( dSpeedNeedle ) * iMeterR * 0.95 + .5 ) + iMeterSCx,
		( int )( sin( dSpeedNeedle ) * iMeterR * 0.95 + .5 ) + iMeterCy,
		LINE_WIDTH, COLOR_NEEDLE, 0
	);
	
	Img.DrawCircle( iMeterSCx, iMeterCy,  iMeterR / 25, COLOR_NEEDLE, CVsdImg::IMG_FILL );
#endif
	
	// Tacho の針
	double dTachoNeedle = iMeterDegRange / ( double )iMeterMaxVal * dTacho + iMeterMinDeg;
	if( dTachoNeedle >= 360 ) dTachoNeedle -= 360;
	dTachoNeedle = dTachoNeedle * ToRAD;
	
	Img.DrawLine(
		iMeterCx, iMeterCy,
		( int )( cos( dTachoNeedle ) * iMeterR * 0.95 + .5 ) + iMeterCx,
		( int )( sin( dTachoNeedle ) * iMeterR * 0.95 + .5 ) + iMeterCy,
		LINE_WIDTH, COLOR_NEEDLE, 0
	);
	
	Img.DrawCircle( iMeterCx, iMeterCy,  iMeterR / 25, COLOR_NEEDLE, CVsdImg::IMG_FILL );
	
	return TRUE;
}

//---------------------------------------------------------------------
//		WndProc
//---------------------------------------------------------------------

#ifdef CIRCUIT_TOMO
static UINT ReadPTD( FILE *fp, UINT uOffs ){
	
	UINT	uCnt		= 0;
	UINT	uPulseCnt	= 0;
	UINT	uOutCnt		= 0;
	UINT	uPrevTime	= 0;
	UINT	u;
	
	fscanf( fp, "%u", &uCnt );
	
	while( uCnt ){
		do{
			fscanf( fp, "%u", &u );
			
			--uCnt;
			++uPulseCnt;
		}while(( double )u / PTD_LOG_FREQ < ( double )uOutCnt / LOG_FREQ && uCnt );
		
		while(( double )uOutCnt / LOG_FREQ < ( double )u / PTD_LOG_FREQ ){
			*( float *)(( char *)( &g_VsdLog[ uOutCnt ] ) + uOffs ) =
				( float )(( double )uPulseCnt / ( u - uPrevTime ) * PTD_LOG_FREQ );
			++uOutCnt;
		}
		uPrevTime = u;
		uPulseCnt = 0;
	}
	return( uOutCnt );
}
#endif

/*** MAP 回転処理 ***********************************************************/

#ifndef CIRCUIT_TOMO
void RotateMap( FILTER *fp ){
	
	int i;
	double dMaxX, dMinX, dMaxY, dMinY;
	
	dMaxX = dMinX = dMaxY = dMinY = 0;
	
	for( i = 0; i < g_iVsdLogNum; ++i ){
		g_VsdLog[ i ].fX = ( float )(  cos( MAP_ANGLE ) * g_VsdLog[ i ].fX0 + sin( MAP_ANGLE ) * g_VsdLog[ i ].fY0 );
		g_VsdLog[ i ].fY = ( float )( -sin( MAP_ANGLE ) * g_VsdLog[ i ].fX0 + cos( MAP_ANGLE ) * g_VsdLog[ i ].fY0 );
		
		if     ( dMaxX < g_VsdLog[ i ].fX ) dMaxX = g_VsdLog[ i ].fX;
		else if( dMinX > g_VsdLog[ i ].fX ) dMinX = g_VsdLog[ i ].fX;
		if     ( dMaxY < g_VsdLog[ i ].fY ) dMaxY = g_VsdLog[ i ].fY;
		else if( dMinY > g_VsdLog[ i ].fY ) dMinY = g_VsdLog[ i ].fY;
	}
	
	dMaxX -= dMinX;
	dMaxY -= dMinY;
	
	g_dMapSize	= dMaxX > dMaxY ? dMaxX : dMaxY;
	g_dMapOffsX		= dMinX;
	g_dMapOffsY		= dMinY;
}
#endif

/*** ログリード *************************************************************/

BOOL ReadLog( void *editp, FILTER *filter ){
	TCHAR	szBuf[ BUF_SIZE ];
	TCHAR	szBuf2[ BUF_SIZE ];
	FILE	*fp;
	BOOL	bCalibrating = FALSE;
	
	float		NaN = 0;
	NaN /= *( volatile float *)&NaN;
	
	if( filter->exfunc->dlg_get_load_name( szBuf, FILE_EXT, NULL ) != TRUE ) return FALSE;
	
	/*** cfg リード ***/
	
	if(( fp = fopen( ChangeExt( szBuf2, szBuf, "cfg" ), "r" )) != NULL ){
		int i, iLen;
		BOOL	bMark = FALSE;
		FRAME_STATUS	fsp;
		
		g_bCalcLapTimeReq = TRUE;
		
		while( fgets( szBuf2, BUF_SIZE, fp )){
			if( bMark ){
				// マークセット
				i = atoi( szBuf2 );
				filter->exfunc->get_frame_status( editp, i, &fsp );
				fsp.edit_flag |= EDIT_FRAME_EDIT_FLAG_MARKFRAME;
				filter->exfunc->set_frame_status( editp, i, &fsp );
				
			}else if( strncmp( szBuf2, "MARK:", 5 ) == 0 ){
				// マークモードに入る
				bMark = TRUE;
				
			}else{
				// Mark 以外のパラメータ
				for( i = 0; i < TRACK_N; ++i ){
					if(
						strncmp( szBuf2, g_szTrackbarName[ i ], iLen = strlen( g_szTrackbarName[ i ] )) == 0 &&
						szBuf2[ iLen ] == '='
					){
						filter->track[ i ] = atoi( szBuf2 + iLen + 1 );
						goto Next;
					}
				}
				
				for( i = 0; i < CHECK_N; ++i ){
					if(
						strncmp( szBuf2, g_szCheckboxName[ i ], iLen = strlen( g_szCheckboxName[ i ] )) == 0 &&
						szBuf2[ iLen ] == '='
					){
						filter->check[ i ] = atoi( szBuf2 + iLen + 1 );
						goto Next;
					}
				}
			}
		  Next: ;
		}
		fclose( fp );
	}
	
	/******************/
	
	if( IsExt( szBuf, "cfg" ) || ( fp = fopen( szBuf, "r" )) == NULL ) return FALSE;
	
	g_iVsdLogNum	= 0;
	g_iLapNum		= 0;
	g_fBestTime		= -1;
	
	if( g_VsdLog == NULL )	g_VsdLog 	= new VSD_LOG_t[ MAX_VSD_LOG ];
	if( g_Lap    == NULL )	g_Lap		= new LAP_t[ MAX_LAP ];
	
#ifdef CIRCUIT_TOMO
	int i;
	for( i = 0; i < MAX_VSD_LOG; ++i ){
		g_VsdLog[ i ].fSpeed = g_VsdLog[ i ].fTacho = 0;
	}
	
	g_iVsdLogNum	= ReadPTD( fp, offsetof( VSD_LOG_t, fTacho ));
	i				= ReadPTD( fp, offsetof( VSD_LOG_t, fSpeed ));
	
	if( i > g_iVsdLogNum ) g_iVsdLogNum = i;
	
#else // CIRCUIT_TOMO
	
	// GPS ログ用
	UINT		uGPSCnt = 0;
	GPS_LOG_t	*GPSLog = new GPS_LOG_t[ ( int )( MAX_VSD_LOG / LOG_FREQ ) ];
	
	// 20070814 以降のログは，横 G が反転している
	BOOL bReverseGy	= ( strcmp( StrTokFile( NULL, szBuf, STF_NAME ), "vsd20070814" ) >= 0 );
	
	// ログリード
	
	UINT	uCnt, uLap, uMin, uSec, uMSec;
	double	dGcx = 0;
	double	dGcy = 0;
	double	dGx, dGy;
	
	float	fLati;
	float	fLong;
	double	dLati0 = 0;
	double	dLong0 = 0;
	float	fSpeed;
	float	fBearing;
	
	g_iLogStart = g_iLogStop = 0;
	
	TCHAR	*p;
	
	while( fgets( szBuf, BUF_SIZE, fp ) != NULL ){
		if(( p = strstr( szBuf, "LAP" )) != NULL ){ // ラップタイム記録を見つけた
			uCnt = sscanf( p, "LAP%d%d:%d.%d", &uLap, &uMin, &uSec, &uMSec );
			
			double dTime = uMin * 60 + uSec + ( double )uMSec / 1000;
			
			g_Lap[ g_iLapNum ].uLap		= uLap;
			g_Lap[ g_iLapNum ].iLogNum	= g_iVsdLogNum;
			g_Lap[ g_iLapNum ].fTime	=
				( uCnt == 4 ) ? ( float )dTime : 0;
			
			if(
				uCnt == 4 &&
				( g_fBestTime == -1 || g_fBestTime > dTime )
			){
				g_fBestTime			= ( float )dTime;
				g_iBestLapLogNum	= g_Lap[ g_iLapNum - 1 ].iLogNum;
			}
			++g_iLapNum;
		}
		
		if(( p = strstr( szBuf, "GPS" )) != NULL ){ // GPS記録を見つけた
			sscanf( p, "GPS%g%g%g%g", &fLati, &fLong, &fSpeed, &fBearing );
			
			if( dLati0 == 0 ){
				dLati0 = fLati;
				dLong0 = fLong;
			}
			
			// 単位を補正
			// 緯度・経度→メートル
			GPSLog[ uGPSCnt ].fX = ( float )(( fLong - dLong0 ) * LAT_M_DEG * cos( fLati * ToRAD ));
			GPSLog[ uGPSCnt ].fY = ( float )(( dLati0 - fLati ) * LNG_M_DEG );
			
			// 速度・向き→ベクトル座標
			double dBearing	= fBearing * ToRAD;
			double dSpeed	= fSpeed * ( 1000.0 / 3600 );
			GPSLog[ uGPSCnt ].fVX	= ( float )(  sin( dBearing ) * dSpeed );
			GPSLog[ uGPSCnt ].fVY	= ( float )( -cos( dBearing ) * dSpeed );
			
			GPSLog[ uGPSCnt++ ].iLogNum =
				( g_iVsdLogNum - GPS_LOG_OFFS ) >= 0 ?
					( g_iVsdLogNum - GPS_LOG_OFFS ) : 0;
		}
		
		// 普通の log
		if(( uCnt = sscanf( szBuf, "%g%g%g%lg%lg",
			&g_VsdLog[ g_iVsdLogNum ].fTacho,
			&g_VsdLog[ g_iVsdLogNum ].fSpeed,
			&g_VsdLog[ g_iVsdLogNum ].fMileage,
			&dGy,
			&dGx
		)) >= 3 ){
			if( uCnt < 5 && g_iVsdLogNum ){
				// Gデータがないときは，speedから求める←廃止
				dGx = 0;
				dGy = 0;
				//dGy = ( g_VsdLog[ g_iVsdLogNum ].fSpeed - g_VsdLog[ g_iVsdLogNum - 1 ].fSpeed ) * ( 1000.0 / 3600 / 9.8 * LOG_FREQ );
			}else if( dGx >= 4 ){	// 4 以下なら，すでに G が計算済み
				if( bReverseGy ) dGx = -dGx;
				
				if( g_iVsdLogNum < G_CX_CNT ){
					// G センター検出
					dGcx += dGx;
					dGcy += dGy;
					dGx = dGy = 0;
					
					if( g_iVsdLogNum == G_CX_CNT - 1 ){
						dGcx /= G_CX_CNT;
						dGcy /= G_CX_CNT;
					}
				}else{
					// 単位を G に変換
					dGx = ( dGx - dGcx ) / ACC_1G_Y;
					dGy = ( dGy - dGcy ) / ( bReverseGy ? ACC_1G_Z : ACC_1G_X );
				}
			}
			g_VsdLog[ g_iVsdLogNum ].fGx = ( float )dGx;
			g_VsdLog[ g_iVsdLogNum ].fGy = ( float )dGy;
			
			g_VsdLog[ g_iVsdLogNum ].fX0 = g_VsdLog[ g_iVsdLogNum ].fY0 = INVALID_POS_D;
			
			// ログ開始・終了認識
			if( g_VsdLog[ g_iVsdLogNum ].fSpeed >= 300 ){
				if( !bCalibrating ){
					bCalibrating = TRUE;
					g_iLogStart  = g_iLogStop;
					g_iLogStop   = g_iVsdLogNum;
				}
			}else{
				bCalibrating = FALSE;
			}
			
			++g_iVsdLogNum;
		}
	}
	
	/*** GPS ログから軌跡を求める ***************************************/
	
	if( uGPSCnt ){
		UINT	u;
		
		// 補完点を計算
		for( u = 0; u < uGPSCnt - 1; ++u ){
			// 2秒以上 GPS ログがあいていれば，補完情報の計算をしない
			if( GPSLog[ u + 1 ].iLogNum - GPSLog[ u ].iLogNum > ( int )( LOG_FREQ * 5 )) continue;
			
			double dX3, dX2, dX1, dX0;
			double dY3, dY2, dY1, dY0;
			
			#define X0	GPSLog[ u ].fX
			#define Y0	GPSLog[ u ].fY
			#define VX0	GPSLog[ u ].fVX
			#define VY0	GPSLog[ u ].fVY
			#define X1	GPSLog[ u + 1 ].fX
			#define Y1	GPSLog[ u + 1 ].fY
			#define VX1	GPSLog[ u + 1 ].fVX
			#define VY1	GPSLog[ u + 1 ].fVY
			
			dX3 = 2 * ( X0 - X1 ) + VX0 + VX1;
			dX2 = 3 * ( -X0 + X1 ) - 2 * VX0 - VX1;
			dX1 = VX0;
			dX0 = X0;
			dY3 = 2 * ( Y0 - Y1 ) + VY0 + VY1;
			dY2 = 3 * ( -Y0 + Y1 ) - 2 * VY0 - VY1;
			dY1 = VY0;
			dY0 = Y0;
			
			#undef X0
			#undef Y0
			#undef VX0
			#undef VY0
			#undef X1
			#undef Y1
			#undef VX1
			#undef VY1
			
			for( int i = GPSLog[ u ].iLogNum; i <= GPSLog[ u + 1 ].iLogNum; ++i ){
				double t =
					( double )( i - GPSLog[ u ].iLogNum ) /
					( GPSLog[ u + 1 ].iLogNum - GPSLog[ u ].iLogNum );
				
				g_VsdLog[ i ].fX0 = ( float )((( dX3 * t + dX2 ) * t + dX1 ) * t + dX0 );
				g_VsdLog[ i ].fY0 = ( float )((( dY3 * t + dY2 ) * t + dY1 ) * t + dY0 );
			}
		}
		
		// Map 回転
		RotateMap( filter );
		
		/*
		FILE *fp = fopen( "c:\\dds\\hoge.log", "w" );
		for( i = 0; i < g_iVsdLogNum; ++i )
			fprintf( fp, "%g\t%g\n", g_VsdLog[ i ].fX, g_VsdLog[ i ].fY );
		fclose( fp );
		*/
	}
	
	delete [] GPSLog;
#endif // CIRCUIT_TOMO
	
	/********************************************************************/
	
	fclose( fp );
	
	g_Lap[ g_iLapNum ].iLogNum	= 0x7FFFFFFF;	// 番犬
	g_Lap[ g_iLapNum ].fTime	= 0;			// 番犬
	
	// trackbar 設定
	track_e[ TRACK_LSt ] =
	track_e[ TRACK_LEd ] =
		#ifdef CIRCUIT_TOMO
			( int )( g_iVsdLogNum / LOG_FREQ + 1 );
		#else
			( g_iVsdLogNum + 99 ) / 100;
		#endif
	
	// 設定再描画
	filter->exfunc->filter_window_update( filter );
	
#ifndef CIRCUIT_TOMO
	// log pos 更新
	func_update( filter, FILTER_UPDATE_STATUS_CHECK + CHECK_LOGPOS );
#endif
	
	return TRUE;
}

/*** 設定セーブ *************************************************************/

BOOL SaveConfig( void *editp, FILTER *filter ){
	TCHAR	szBuf[ BUF_SIZE ];
	FILE	*fp;
	int		i;
	
	if( filter->exfunc->dlg_get_save_name( szBuf, FILE_CFG_EXT, NULL ) != TRUE ) return FALSE;
	if(( fp = fopen( szBuf, "w" )) == NULL ) return FALSE;
	
	for( i = 0; i < TRACK_N; ++i ){
		fprintf( fp, "%s=%u\n", g_szTrackbarName[ i ], filter->track[ i ] );
	}
	
	for( i = 0; i < CHECK_N; ++i ){
		if(
			i == CHECK_FRAME
			#ifndef CIRCUIT_TOMO
			|| i == CHECK_LOGPOS
			#endif
		) continue;
		
		fprintf( fp, "%s=%u\n", g_szCheckboxName[ i ], filter->check[ i ] );
	}
	
	// 手動ラップ計測マーク出力
	if( IS_HAND_LAPTIME && g_iLapNum ){
		fputs( "MARK:\n", fp );
		
		for( i = 0; i < g_iLapNum; ++i ){
			fprintf( fp, "%u\n", g_Lap[ i ].iLogNum );
		}
	}
	
	fclose( fp );
	return 0;
}

/****************************************************************************/

BOOL func_WndProc( HWND hwnd,UINT message,WPARAM wparam,LPARAM lparam,void *editp,FILTER *filter ){
	//	TRUEを返すと全体が再描画される
	
	//	編集中でなければ何もしない
	if( filter->exfunc->is_editing(editp) != TRUE ) return FALSE;
	
	int	iFrame;
	
	switch( message ) {
	  case WM_FILTER_IMPORT:
		return ReadLog( editp, filter );
		
	  Case WM_FILTER_EXPORT:
		return SaveConfig( editp, filter );
		
	  Case WM_FILTER_FILE_OPEN:
		// fps 取得
		FILE_INFO fi;
		filter->exfunc->get_file_info( editp, &fi );
		g_dVideoFPS  = ( double )fi.video_rate / fi.video_scale;
		
		// g_Lap 初期化
		if( g_Lap == NULL )	g_Lap = new LAP_t[ MAX_LAP ];
		g_iLapNum = 0;
		g_Lap[ 0 ].iLogNum	= 0x7FFFFFFF;	// 番犬
		g_Lap[ 0 ].fTime	= 0;			// 番犬
		
		g_bCalcLapTimeReq = TRUE;
		
		// trackbar 設定
		track_e[ TRACK_VSt ] =
		track_e[ TRACK_VEd ] = ( filter->exfunc->get_frame_n( editp ) + 99 ) / 100;
		
		// 設定再描画
		filter->exfunc->filter_window_update( filter );
		
	  Case WM_FILTER_FILE_CLOSE:
		delete [] g_VsdLog;
		g_VsdLog		= NULL;
		g_iVsdLogNum	= 0;
		
		delete [] g_Lap;
		g_Lap		= NULL;
		g_iLapNum	= 0;
		
	  Case WM_FILTER_MAIN_KEY_DOWN:
		switch( wparam ){
			
		  case 'M':
			// マーク
			FRAME_STATUS	fsp;
			iFrame = filter->exfunc->get_frame( editp );
			
			filter->exfunc->get_frame_status( editp, iFrame, &fsp );
			fsp.edit_flag ^= EDIT_FRAME_EDIT_FLAG_MARKFRAME;
			filter->exfunc->set_frame_status( editp, iFrame, &fsp );
			
			g_bCalcLapTimeReq = TRUE;
			return TRUE;
			
		  Case 'S':
			// フレーム数セット
			iFrame = filter->exfunc->get_frame( editp );
			
			filter->track[ TRACK_VSt ]	= filter->track[ TRACK_VEd ];
			filter->track[ TRACK_VSt2 ]	= filter->track[ TRACK_VEd2 ];
			
			filter->track[ TRACK_VEd ]	= iFrame / 100;
			filter->track[ TRACK_VEd2 ]	= iFrame % 100;
			
			// 設定再描画
			filter->exfunc->filter_window_update( filter );
		}
	}
	
	return FALSE;
}

/*** スライダバー連動 *******************************************************/

BOOL func_update( FILTER *fp, int status ){
	static	BOOL	bReEnter = FALSE;
	
	if( bReEnter ) return TRUE;
	
	bReEnter = TRUE;
	
#ifndef CIRCUIT_TOMO
	if(
		status == ( FILTER_UPDATE_STATUS_CHECK + CHECK_LOGPOS ) &&
		fp->check[ CHECK_LOGPOS ]
	){
		fp->track[ TRACK_LSt  ] = g_iLogStart / 100;
		fp->track[ TRACK_LSt2 ] = g_iLogStart % 100;
		fp->track[ TRACK_LEd  ] = g_iLogStop  / 100;
		fp->track[ TRACK_LEd2 ] = g_iLogStop  % 100;
		
		// 設定再描画
		fp->exfunc->filter_window_update( fp );
	}
	
	// マップ回転
	if( status == ( FILTER_UPDATE_STATUS_TRACK + TRACK_MapAngle )){
		RotateMap( fp );
	}
#endif
	
	bReEnter = FALSE;
	
	return TRUE;
}

/****************************************************************************/

BOOL WINAPI DllMain(
	HINSTANCE	hinstDLL,	// handle to DLL module
	DWORD		fdwReason,	// reason for calling function
	LPVOID		lpvReserved	// reserved
){
	if( fdwReason == DLL_PROCESS_ATTACH ){
		g_hInst = hinstDLL;
	}
	return TRUE;
}
