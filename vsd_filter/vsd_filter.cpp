//----------------------------------------------------------------------------------
//		サンプルビデオフィルタ(フィルタプラグイン)  for AviUtl ver0.96e以降
//----------------------------------------------------------------------------------
// $Id$

#define _CRT_SECURE_NO_DEPRECATE 1

#include <windows.h>
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <float.h>

#include "dds.h"
#include "filter.h"
#include "../vsd/main.h"
#include "dds_lib/dds_lib.h"

#define	FILE_TXT		"LogFile (*.log)\0*.log\0AllFile (*.*)\0*.*\0"

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

#define VideoSt		( fp->track[ TRACK_VSt ] * 100 + fp->track[ TRACK_VSt2 ] )
#define VideoEd		( fp->track[ TRACK_VEd ] * 100 + fp->track[ TRACK_VEd2 ] )
#define LogSt		( fp->track[ TRACK_LSt ] * 100 + fp->track[ TRACK_LSt2 ] )
#define LogEd		( fp->track[ TRACK_LEd ] * 100 + fp->track[ TRACK_LEd2 ] )
#define LineTrace	fp->track[ TRACK_LineTrace ]

#define G_CX_CNT		30
#define G_HIST			(( int )( LOG_FREQ * 3 ))
#define MAX_G_SCALE		1.5

#define MAX_MAP_SIZE	( Img.w / 2.5 )
#define INVALID_POS_I	0x7FFFFFFF
#define INVALID_POS_D	NaN

#define HIREZO_TH		600
#define LINE_WIDTH		( Img.w / HIREZO_TH + 1 )

#define GPS_LOG_OFFS	15

/*** CAviUtlImage class *****************************************************/

const UCHAR g_Font9p[] = {
	#include "font_9p.h"
};

const UCHAR g_Font18p[] = {
	#include "font_18p.h"
};

#define POS_DEFAULT	0x80000000

class CAviUtlImage : public FILTER_PROC_INFO {
	
  public:
	//void PutPixel( int x, int y, short iY, short iCr, short iCb );
	void PutPixel( int x, int y, const PIXEL_YC &yc, UINT uFlag );
	PIXEL_YC &GetPixel( int x, int y, UINT uFlag );
	void DrawLine( int x1, int y1, int x2, int y2, const PIXEL_YC &yc, UINT uFlag );
	void DrawLine( int x1, int y1, int x2, int y2, int width, const PIXEL_YC &yc, UINT uFlag );
	void FillLine( int x1, int y1, int x2,         const PIXEL_YC &yc, UINT uFlag );
	void DrawRect( int x1, int y1, int x2, int y2, const PIXEL_YC &yc, UINT uFlag );
	void DrawCircle( int x, int y, int r, const PIXEL_YC &yc, UINT uFlag );
	int GetIndex( int x, int y ){ return max_w * y + x; }
	
	PIXEL_YC &GetYC( PIXEL_YC &yc, int r, int g, int b ){
		yc.y  = ( int )( 0.299 * r + 0.587 * g + 0.114 * b + .5 );
		yc.cr = ( int )( 0.500 * r - 0.419 * g - 0.081 * b + .5 );
		yc.cb = ( int )(-0.169 * r - 0.332 * g + 0.500 * b + .5 );
		
		return yc;
	}
	
	void CAviUtlImage::CopyRect(
		int	Sx1, int Sy1,
		int	Sx2, int Sy2,
		int Dx,  int Dy,
		const PIXEL_YC &yc, UINT uFlag
	);
	
	void DrawFont( int x, int y, char c, const PIXEL_YC &yc, UINT uFlag );
	void DrawFont( int x, int y, char c, const PIXEL_YC &yc, const PIXEL_YC &ycEdge, UINT uFlag );
	void DrawString( char *szMsg, const PIXEL_YC &yc, UINT uFlag, int x = POS_DEFAULT, int y = POS_DEFAULT );
	void DrawString( char *szMsg, const PIXEL_YC &yc, const PIXEL_YC &ycEdge, UINT uFlag, int x = POS_DEFAULT, int y = POS_DEFAULT );
	
	// ポリゴン描写
	void PolygonClear( void );
	void PolygonDraw( const PIXEL_YC &yc, UINT uFlag );
	
	// フォント
	const UCHAR *GetFontBase( void ){ return w >= HIREZO_TH ? g_Font18p : g_Font9p; }
	int GetFontBMP_W( void ){ return *( int *)( GetFontBase() + 0x12 ); }
	int GetFontBMP_H( void ){ return *( int *)( GetFontBase() + 0x16 ); }
	UCHAR GetBMPPix( int x, int y ){ return GetFontBase()[ 0x436 + GetFontBMP_W() * ( GetFontBMP_H() - ( y ) - 1 ) + ( x )]; }
	int GetFontW( void ){ return GetFontBMP_W() / 16; }
	int GetFontH( void ){ return GetFontBMP_H() / 7; }
	
	enum {
		IMG_ALFA	= ( 1 << 0 ),
		IMG_TMP		= ( 1 << 1 ),
		IMG_FILL	= ( 1 << 2 ),
		IMG_TMP_DST	= ( 1 << 3 ),
		IMG_POLYGON	= ( 1 << 4 ),
	};
};

/*** PutPixel ***************************************************************/

/* 変換式
Y  =  0.299R+0.587G+0.114B
Cr =  0.500R-0.419G-0.081B
Cb = -0.169R-0.332G+0.500B

R = Y+1.402Cr 
G = Y-0.714Cr-0.344Cb 
B = Y+1.772Cb 
*/

/*
inline void CAviUtlImage::PutPixel( int x, int y, short iY, short iCr, short iCb ){
	int	iIndex = GetIndex( x, y );
	
	ycp_edit[ iIndex ].y  = iY;
	ycp_edit[ iIndex ].cr = iCr;
	ycp_edit[ iIndex ].cb = iCb;
}
*/

inline void CAviUtlImage::PutPixel( int x, int y, const PIXEL_YC &yc, UINT uFlag ){
	
	if( uFlag & IMG_POLYGON ){
		// ポリゴン描画
		if( x > ycp_temp[ y ].cr ) ycp_temp[ y ].cr = x;
		if( x < ycp_temp[ y ].cb ) ycp_temp[ y ].cb = x;
	}else{
		PIXEL_YC	*ycp = ( uFlag & IMG_TMP ) ? ycp_temp : ycp_edit;
		
		if( uFlag & IMG_ALFA && yc.y == -1 ) return;
		
		if( 0 <= x && x < max_w && 0 <= y && y < max_h ){
			if( uFlag & IMG_ALFA ){
				int	iIndex = GetIndex( x, y );
				
				ycp[ iIndex ].y  = ( yc.y  + ycp[ iIndex ].y  ) / 2;
				ycp[ iIndex ].cr = ( yc.cr + ycp[ iIndex ].cr ) / 2;
				ycp[ iIndex ].cb = ( yc.cb + ycp[ iIndex ].cb ) / 2;
			}else{
				ycp[ GetIndex( x, y ) ] = yc;
			}
		}
	}
}

inline PIXEL_YC &CAviUtlImage::GetPixel( int x, int y, UINT uFlag ){
	PIXEL_YC	*ycp = ( uFlag & IMG_TMP ) ? ycp_temp : ycp_edit;
	return	ycp[ GetIndex( x, y ) ];
}

/*** DrawLine ***************************************************************/

#define ABS( x )			(( x ) < 0 ? -( x ) : ( x ))
#define SWAP( x, y, tmp )	( tmp = x, x = y, y = tmp )

void CAviUtlImage::DrawLine( int x1, int y1, int x2, int y2, const PIXEL_YC &yc, UINT uFlag ){
	
	int	i;
	
	if( x1 == x2 && y1 == y2 ){
		PutPixel( x1, y1, yc, uFlag );
	}else if( ABS( x1 - x2 ) >= ABS( y1 - y2 )){
		// x 基準で描画
		if( x1 > x2 ){
			SWAP( x1, x2, i );
			SWAP( y1, y2, i );
		}
		
		int iYDiff = y2 - y1 + (( y2 > y1 ) ? 1 : ( y2 < y1 ) ? -1 : 0 );
		
		for( i = x1; i <= x2; ++i ){
			PutPixel( i, ( int )(( double )iYDiff * ( i - x1 + .5 ) / ( x2 - x1 + 1 ) /*+ .5*/ ) + y1, yc, uFlag );
		}
	}else{
		// y 基準で描画
		if( y1 > y2 ){
			SWAP( y1, y2, i );
			SWAP( x1, x2, i );
		}
		
		int iXDiff = x2 - x1 + (( x2 > x1 ) ? 1 : ( x2 < x1 ) ? -1 : 0 );
		
		for( i = y1; i <= y2; ++i ){
			PutPixel(( int )(( double )iXDiff * ( i - y1 + .5 ) / ( y2 - y1 + 1 ) /*+ .5*/ ) + x1, i, yc, uFlag );
		}
	}
}

void CAviUtlImage::DrawLine( int x1, int y1, int x2, int y2, int width, const PIXEL_YC &yc, UINT uFlag ){
	for( int y = 0; y < width; ++y ) for( int x = 0; x < width; ++x ){
		DrawLine(
			x1 + x - width / 2, y1 + y - width / 2,
			x2 + x - width / 2, y2 + y - width / 2,
			yc, uFlag
		);
	}
}

inline void CAviUtlImage::FillLine( int x1, int y1, int x2, const PIXEL_YC &yc, UINT uFlag ){
	
	int	i;
	
	// x 基準で描画
	for( i = x1; i <= x2; ++i ) PutPixel( i, y1, yc, uFlag );
}

/*** DrawRect ***************************************************************/

void CAviUtlImage::DrawRect( int x1, int y1, int x2, int y2, const PIXEL_YC &yc, UINT uFlag ){
	int	y;
	
	if( y1 > y2 ) SWAP( y1, y2, y );
	if( x1 > x2 ) SWAP( x1, x2, y );
	
	for( y = y1; y <= y2; ++y ){
		FillLine( x1, y, x2, yc, uFlag );
	}
}

/*** DrawCircle *************************************************************/

void CAviUtlImage::DrawCircle( int x, int y, int r, const PIXEL_YC &yc, UINT uFlag ){
	
	int			i, j;
	PIXEL_YC	yc_void = { -1, 0, 0 };
	
	// Polygon クリア
	if( uFlag & IMG_FILL ){
		PolygonClear();
		uFlag |= IMG_POLYGON;
	}
	
	// 円を書く
	for( i = 0, j = r; i < j; ++i ){
		
		j = ( int )( sqrt(( double )( r * r - i * i )) + .5 );
		
		PutPixel( x + i, y + j, yc, uFlag ); PutPixel( x + i, y - j, yc, uFlag );
		PutPixel( x - i, y + j, yc, uFlag ); PutPixel( x - i, y - j, yc, uFlag );
		PutPixel( x + j, y + i, yc, uFlag ); PutPixel( x - j, y + i, yc, uFlag );
		PutPixel( x + j, y - i, yc, uFlag ); PutPixel( x - j, y - i, yc, uFlag );
	}
	
	// Polygon 合成
	if( uFlag & IMG_FILL ) PolygonDraw( yc, uFlag );
}

/*** CopyRect ***************************************************************/

void CAviUtlImage::CopyRect(
	int	Sx1, int Sy1,
	int	Sx2, int Sy2,
	int Dx,  int Dy,
	const PIXEL_YC &yc, UINT uFlag
){
	int	x, y;
	UINT	uFlagDst = uFlag & ~IMG_TMP | ( uFlag & IMG_TMP_DST ? IMG_TMP : 0 );
	
	for( y = Sy1; y <= Sy2; ++y ) for( x = Sx1; x <= Sx2; ++x ){
		PutPixel( Dx + x - Sx1, Dy + y - Sy1, GetPixel( x, y, uFlag ), uFlagDst );
	}
}

/*** DrawFont ***************************************************************/

void CAviUtlImage::DrawFont( int x, int y, char c, const PIXEL_YC &yc, UINT uFlag ){
	
	int	i, j;
	
	c -= ' ';
	
	for( j = 0; j < GetFontH(); ++j ) for( i = 0; i < GetFontW(); ++i ){
		if( GetBMPPix(
			( c % 16 ) * GetFontW() + i,
			( c / 16 ) * GetFontH() + j
		) == 0 ) PutPixel( x + i, y + j, yc, uFlag );
	}
}

void CAviUtlImage::DrawFont( int x, int y, char c, const PIXEL_YC &yc, const PIXEL_YC &ycEdge, UINT uFlag ){
	
	int	i, j;
	
	char cc = c - ' ';
	
	for( j = 0; j < GetFontH(); ++j ) for( i = 0; i < GetFontW(); ++i ){
		if( GetBMPPix(
			( cc % 16 ) * GetFontW() + i,
			( cc / 16 ) * GetFontH() + j
		) == 0 ){
			PutPixel( x + i - 1, y + j, ycEdge, uFlag );
			PutPixel( x + i + 1, y + j, ycEdge, uFlag );
			PutPixel( x + i, y + j - 1, ycEdge, uFlag );
			PutPixel( x + i, y + j + 1, ycEdge, uFlag );
		}
	}
	
	DrawFont( x, y, c, yc, uFlag );
}

/*** DrawString *************************************************************/

void CAviUtlImage::DrawString( char *szMsg, const PIXEL_YC &yc, UINT uFlag, int x, int y ){
	
	static int iPosX, iPosY;
	
	if( x != POS_DEFAULT ) iPosX = x;
	if( y != POS_DEFAULT ) iPosY = y;
	
	for( int i = 0; szMsg[ i ]; ++i ){
		DrawFont( iPosX + i * GetFontW(), iPosY, szMsg[ i ], yc, uFlag );
	}
	
	iPosY += GetFontH();
}

void CAviUtlImage::DrawString( char *szMsg, const PIXEL_YC &yc, const PIXEL_YC &ycEdge, UINT uFlag, int x, int y ){
	
	static int iPosX, iPosY;
	
	if( x != POS_DEFAULT ) iPosX = x;
	if( y != POS_DEFAULT ) iPosY = y;
	
	for( int i = 0; szMsg[ i ]; ++i ){
		DrawFont( iPosX + i * GetFontW(), iPosY, szMsg[ i ], yc, ycEdge, uFlag );
	}
	
	iPosY += GetFontH();
}

/*** ポリゴン描画 ***********************************************************/

inline void CAviUtlImage::PolygonClear( void ){
	for( int y = 0; y < h; ++y ){
		ycp_temp[ y ].cr = 0;	// right
		ycp_temp[ y ].cb = w;	// left
	}
}

inline void CAviUtlImage::PolygonDraw( const PIXEL_YC &yc, UINT uFlag ){
	for( int y = 0; y < h; ++y ) if( ycp_temp[ y ].cb <= ycp_temp[ y ].cr ){
		FillLine( ycp_temp[ y ].cb, y, ycp_temp[ y ].cr, yc, uFlag & ~IMG_POLYGON );
	}
}

/*** new type ***************************************************************/

typedef struct {
	float	fSpeed;
	float	fTacho;
	float	fMileage;
	float	fGx;
	float	fGy;
	float	fX;
	float	fY;
} VSD_LOG_t;

typedef struct {
	USHORT	uLap;
	int		iLogNum;
	float	fTime;
} LAP_t;

typedef struct {
	union {	float	fLong;		float	fX;		};
	union {	float	fLati;		float	fY;		};
	union {	float	fSpeed;		float	fVX;	};
	union {	float	fBearing;	float	fVY;	};
	int		iLogNum;
} GPS_LOG_t;

/*** gloval var *************************************************************/

HINSTANCE	g_hInst = NULL;

VSD_LOG_t	*g_VsdLog 		= NULL;
int			g_iVsdLogNum	= 0;
LAP_t		*g_Lap	 		= NULL;
int			g_iLapNum		= 0;
double		g_dBestTime		= -1;
int			g_iBestLapLogNum= 0;

double		g_dMaxMapSize = 0;

int			g_iLogStart;
int			g_iLogStop;

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
	FILTER_FLAG_EX_INFORMATION | FILTER_FLAG_IMPORT,	
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
	func_exit,					//	終了時に呼ばれる関数へのポインタ (NULLなら呼ばれません)
	func_update,				//	設定が変更されたときに呼ばれる関数へのポインタ (NULLなら呼ばれません)
	func_WndProc,				//	設定ウィンドウにウィンドウメッセージが来た時に呼ばれる関数へのポインタ (NULLなら呼ばれません)
	NULL,NULL,					//	システムで使いますので使用しないでください
	NULL,						//  拡張データ領域へのポインタ (FILTER_FLAG_EX_DATAが立っている時に有効)
	NULL,						//  拡張データサイズ (FILTER_FLAG_EX_DATAが立っている時に有効)
	"VSDメーター合成 v0.00 by DDS",
								//  フィルタ情報へのポインタ (FILTER_FLAG_EX_INFORMATIONが立っている時に有効)
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

BOOL func_exit( FILTER *fp ){
	
	delete [] g_VsdLog;
	delete [] g_Lap;
	return TRUE;
}

//---------------------------------------------------------------------
//		フィルタ処理関数
//---------------------------------------------------------------------

const PIXEL_YC	yc_black		= RGB2YC(    0,    0,    0 );
const PIXEL_YC	yc_white		= RGB2YC( 4095, 4095, 4095 );
const PIXEL_YC	yc_gray			= RGB2YC( 2048, 2048, 2048 );
const PIXEL_YC	yc_red			= RGB2YC( 4095,    0,    0 );
const PIXEL_YC	yc_green		= RGB2YC(    0, 4095,    0 );
const PIXEL_YC	yc_dark_green	= RGB2YC(    0, 2048,    0 );
const PIXEL_YC	yc_blue			= RGB2YC(    0,    0, 4095 );
const PIXEL_YC	yc_cyan			= RGB2YC(    0, 4095, 4095 );
const PIXEL_YC	yc_dark_blue	= RGB2YC(    0,    0, 2048 );
const PIXEL_YC	yc_orange		= RGB2YC( 4095, 1024,    0 );

#define COLOR_PANEL			yc_gray
#define COLOR_NEEDLE		yc_red
#define COLOR_SCALE			yc_white
#define COLOR_STR			COLOR_SCALE
#define COLOR_TIME			yc_white
#define COLOR_TIME_EDGE		yc_black
#define COLOR_G_SENSOR		yc_green
#define COLOR_G_HIST		yc_dark_green
#define COLOR_DIFF_MINUS	yc_cyan
#define COLOR_DIFF_PLUS		yc_red

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
	CAviUtlImage	&Img = *( CAviUtlImage *)fpip;
	
	const int	iMeterR			= 50 * Img.w / 320;
	const int	iMeterCx		= Img.w - iMeterR - 2;
	const int	iMeterCy		= Img.h - iMeterR - 2;
	const int	iMeterMinDeg	= 135;
	const int	iMeterMaxDeg	= 45;
	const int	iMeterMaxVal	= 6000;
	const int	iMeterDegRange	= (( iMeterMaxDeg < iMeterMinDeg ? iMeterMaxDeg + 360 : iMeterMaxDeg ) - iMeterMinDeg );
	const int	iMeterScaleLen	= iMeterR / 8;
	
	// ログ位置の計算
	double	dLogNum = ( double )( LogEd - LogSt ) / ( VideoEd - VideoSt ) * ( Img.frame - VideoSt ) + LogSt;
	
	// メーターパネル
	Img.DrawCircle(
		iMeterCx, iMeterCy, iMeterR, COLOR_PANEL,
		CAviUtlImage::IMG_ALFA | CAviUtlImage::IMG_FILL
	);
	
	for( i = 0; i <= iMeterMaxVal; i += 500 ){
		int iDeg = iMeterDegRange * i / iMeterMaxVal + iMeterMinDeg;
		if( iDeg >= 360 ) iDeg -= 360;
		
		// メーターパネル目盛り
		Img.DrawLine(
			( int )( cos( iDeg * ToRAD ) * iMeterR ) + iMeterCx,
			( int )( sin( iDeg * ToRAD ) * iMeterR ) + iMeterCy,
			( int )( cos( iDeg * ToRAD ) * ( iMeterR - iMeterScaleLen )) + iMeterCx,
			( int )( sin( iDeg * ToRAD ) * ( iMeterR - iMeterScaleLen )) + iMeterCy,
			COLOR_SCALE, 0
		);
		
		// メーターパネル目盛り数値
		if( i % 1000 == 0 ) Img.DrawFont(
			( int )( cos( iDeg * ToRAD ) * iMeterR * .8 ) + iMeterCx - Img.GetFontW() / 2,
			( int )( sin( iDeg * ToRAD ) * iMeterR * .8 ) + iMeterCy - Img.GetFontH() / 2,
			'0' + i / 1000 + ( i >= 1000 ? 1 : 0 ),
			COLOR_STR, 0
		);
	}
	
	// フレーム表示
	if( fp->check[ CHECK_FRAME ] ){
		sprintf( szBuf, "V%6d/%6d", Img.frame, Img.frame_n - 1 );
		Img.DrawString( szBuf, COLOR_STR, COLOR_TIME_EDGE, 0, Img.w / 2, Img.h / 2 );
		sprintf( szBuf, "L%6d/%6d", ( int )dLogNum, g_iVsdLogNum - 1 );
		Img.DrawString( szBuf, COLOR_STR, COLOR_TIME_EDGE, 0 );
	}
	
	/*** Lap タイム描画 ***/
	
	if( fp->check[ CHECK_LAP ] && g_iLapNum ){
		/*
		Img.DrawRect(
			iLapX, iLapY, Img.w - 1, iLapY + Img.GetFontH() * 5 - 1,
			COLOR_PANEL,
			CAviUtlImage::IMG_ALFA | CAviUtlImage::IMG_FILL
		);
		*/
		
		// カレントポインタがおかしいときは，-1 にリセット
		if(
			iLapIdx >= g_iLapNum ||
			iLapIdx >= 0 && g_Lap[ iLapIdx ].iLogNum > ( int )dLogNum
		) iLapIdx = -1;
		
		for( ; g_Lap[ iLapIdx + 1 ].iLogNum <= ( int )dLogNum; ++iLapIdx );
		
		// Best 表示
		sprintf( szBuf, "Bst%3d'%02d.%03d", ( int )g_dBestTime / 60, ( int )g_dBestTime % 60, ( int )( g_dBestTime * 1000 + .5 ) % 1000 );
		Img.DrawString( szBuf, COLOR_TIME, COLOR_TIME_EDGE, 0, Img.w - Img.GetFontW() * 13, 1 );
		
		// Lapタイム表示
		i = 0;
		for( int iLapIdxTmp = iLapIdx; iLapIdxTmp >= 0; --iLapIdxTmp ){
			if( g_Lap[ iLapIdxTmp ].fTime != 0 ){
				sprintf( szBuf, "%3d%3d'%02d.%03d", g_Lap[ iLapIdxTmp ].uLap, ( int )g_Lap[ iLapIdxTmp ].fTime / 60, ( int )g_Lap[ iLapIdxTmp ].fTime % 60, ( int )( g_Lap[ iLapIdxTmp ].fTime * 1000 + .5 ) % 1000 );
				Img.DrawString( szBuf, COLOR_TIME, COLOR_TIME_EDGE, 0 );
				if( ++i >= 3 ) break;
			}
		}
		
		// 時間表示
		if(
			iLapIdx >= 0 &&
			g_Lap[ iLapIdx + 1 ].iLogNum != 0x7FFFFFFF &&
			g_Lap[ iLapIdx + 1 ].fTime  != 0
		){
			double dTime = ( dLogNum - g_Lap[ iLapIdx ].iLogNum ) / LOG_FREQ;
			sprintf( szBuf, "%2d'%02d.%03d", ( int )dTime / 60, ( int )dTime % 60, ( int )( dTime * 1000 + .5 ) % 1000 );
			Img.DrawString( szBuf, COLOR_TIME, COLOR_TIME_EDGE, 0, ( Img.w - Img.GetFontW() * 9 ) / 2, 1 );
			bInLap = TRUE;
		}else{
			// まだ開始していない
			Img.DrawString( "--'--.---", COLOR_TIME, COLOR_TIME_EDGE, 0, ( Img.w - Img.GetFontW() * 9 ) / 2, 1 );
		}
		
		/*** ベストとの車間距離表示 ***/
		
		if( bInLap ){
			// この周の走行距離を求める
			double dMileage = GetVsdLog( fMileage ) - g_VsdLog[ g_Lap[ iLapIdx ].iLogNum ].fMileage;
			
			// 最速 Lap の，同一走行距離におけるタイム (=ログ番号,整数) を求める
			// iBestLogNum - 1 <= 最終的に求める結果 < iBestLogNum   となる
			int iBestLogNum;
			for(
				iBestLogNum = g_iBestLapLogNum;
				(
					g_VsdLog[ iBestLogNum ].fMileage -
					g_VsdLog[ g_iBestLapLogNum ].fMileage
				) <= dMileage &&
				iBestLogNum < g_iVsdLogNum;
				++iBestLogNum
			);
			
			// 最速 Lap の，1/15秒以下の値を求める = A / B
			double dBestLogNum =
				( double )( iBestLogNum - 1 ) +
				// A: 最速ラップは，後これだけ走らないと dMileage と同じではない
				( dMileage - ( g_VsdLog[ iBestLogNum - 1 ].fMileage - g_VsdLog[ g_iBestLapLogNum ].fMileage )) /
				// B: 最速ラップは，1/15秒の間にこの距離を走った
				( g_VsdLog[ iBestLogNum ].fMileage - g_VsdLog[ iBestLogNum - 1 ].fMileage );
			
			double dDiffTime =
				(
					( dLogNum - g_Lap[ iLapIdx ].iLogNum ) -
					( dBestLogNum - g_iBestLapLogNum )
				) / LOG_FREQ;
			
			if( -0.0005 < dDiffTime && dDiffTime < 0 ) dDiffTime = 0;
			
			sprintf(
				szBuf, "%c%d'%06.3f",
				dDiffTime < 0 ? '-' : '+',
				( int )( fabs( dDiffTime )) / 60,
				fmod( fabs( dDiffTime ), 60 )
			);
			Img.DrawString( szBuf, dDiffTime < 0 ? COLOR_DIFF_MINUS : COLOR_DIFF_PLUS, COLOR_TIME_EDGE, 0 );
		}
	}
	
	/*** メーター描画 ***/
	
	if( dLogNum < 0 || dLogNum > g_iVsdLogNum - 1 ) return TRUE;
	
	double	dSpeed	= GetVsdLog( fSpeed );
	double	dTacho	= GetVsdLog( fTacho );
	
	// G スネーク
	int	iGx, iGy;
	
	if( fp->check[ CHECK_SNAKE ] ){
		
		int iGxPrev = 0, iGyPrev;
		int iLogNum = ( int )dLogNum;
		
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
		COLOR_G_SENSOR, CAviUtlImage::IMG_FILL
	);
	
	// MAP 表示
	if( LineTrace ){
		
		int iGxPrev = INVALID_POS_I, iGyPrev;
		int iLogNum = ( int )dLogNum;
		
		for( i = -( int )( LineTrace * LOG_FREQ ); i <= 1 ; ++i ){
			if( iLogNum + i >= 0 ){
				// i == 1 時は最後の中途半端な LogNum
				double dGx = (( i != 1 ) ? g_VsdLog[ iLogNum + i ].fX : GetVsdLog( fX )) / g_dMaxMapSize * MAX_MAP_SIZE + 8;
				double dGy = (( i != 1 ) ? g_VsdLog[ iLogNum + i ].fY : GetVsdLog( fY )) / g_dMaxMapSize * MAX_MAP_SIZE + 8;
				
				if( !_isnan( dGx )){
					iGx = ( int )dGx;
					iGy = ( int )dGy;
					
					if( iGxPrev != INVALID_POS_I ) Img.DrawLine(
						iGx, iGy, iGxPrev, iGyPrev,
						LINE_WIDTH, COLOR_G_HIST, 0
					);
				}else{
					iGx = INVALID_POS_I;
				}
				
				iGxPrev = iGx;
				iGyPrev = iGy;
			}
		}
		
		// MAP インジケータ
		if( iGx != INVALID_POS_I ) Img.DrawCircle(
			iGx, iGy, iMeterR / 20,
			COLOR_G_SENSOR, CAviUtlImage::IMG_FILL
		);
	}
	
	// Tacho の針
	double dTachoNeedle =
		iMeterDegRange / ( double )iMeterMaxVal *
		( dTacho <= 2000 ? dTacho / 2 : dTacho - 1000 )
		+ iMeterMinDeg;
	if( dTachoNeedle >= 360 ) dTachoNeedle -= 360;
	dTachoNeedle = dTachoNeedle * ToRAD;
	
	Img.DrawLine(
		iMeterCx, iMeterCy,
		( int )( cos( dTachoNeedle ) * iMeterR * 0.95 + .5 ) + iMeterCx,
		( int )( sin( dTachoNeedle ) * iMeterR * 0.95 + .5 ) + iMeterCy,
		LINE_WIDTH, COLOR_NEEDLE, 0
	);
	
	Img.DrawCircle( iMeterCx, iMeterCy,  iMeterR / 25, COLOR_NEEDLE, CAviUtlImage::IMG_FILL );
	
	// スピード / ギア
	UINT uGear = 0;
	UINT uGearRatio = 0;
	
	if( dTacho ){
		UINT uGearRatio = ( int )( dSpeed * 100 * ( 1 << 8 ) / dTacho );
		
		if     ( uGearRatio < GEAR_TH( 1 ))	uGear = 1;
		else if( uGearRatio < GEAR_TH( 2 ))	uGear = 2;
		else if( uGearRatio < GEAR_TH( 3 ))	uGear = 3;
		else if( uGearRatio < GEAR_TH( 4 ))	uGear = 4;
		else								uGear = 5;
	}
	
	// スピード表示
	/*
	sprintf( szBuf, "%d\x7F%4d\x80\x81", uGear, ( int )dSpeed );
	Img.DrawString(
		iMeterCx - 4 * Img.GetFontW(), iMeterCy + iMeterR / 2,
		szBuf,
		COLOR_STR, 0
	);
	*/
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
	
	sprintf( szBuf, "%2d\x82", ( int )( sqrt( GetVsdLog( fGx ) * GetVsdLog( fGx ) + GetVsdLog( fGy ) * GetVsdLog( fGy )) * 10 ));
	Img.DrawString(
		szBuf,
		COLOR_STR, 0,
		iMeterCx - 2 * Img.GetFontW(), iMeterCy + iMeterR / 2 - Img.GetFontH()
	);
	
	return TRUE;
}

//---------------------------------------------------------------------
//		WndProc
//---------------------------------------------------------------------

BOOL func_WndProc( HWND hwnd,UINT message,WPARAM wparam,LPARAM lparam,void *editp,FILTER *filter ){
	//	TRUEを返すと全体が再描画される
	
	static TCHAR	szBuf[ BUF_SIZE ];
	TCHAR	*p;
	FILE	*fp;
	BOOL	bCalibrating = FALSE;
	
	// GPS ログ用
	UINT		uGPSCnt = 0;
	GPS_LOG_t	*GPSLog;
	double		dLatiMin = 1000, dLatiMax = 0;
	double		dLongMin = 1000, dLongMax = 0;
	
	float		NaN = 0;
	NaN /= *( volatile float *)&NaN;
	
	//	編集中でなければ何もしない
	if( filter->exfunc->is_editing(editp) != TRUE ) return FALSE;
	
	switch( message ) {
	  case WM_FILTER_IMPORT:
		
		if( g_VsdLog == NULL )	g_VsdLog 	= new VSD_LOG_t[ MAX_VSD_LOG ];
		if( g_Lap    == NULL )	g_Lap		= new LAP_t[ MAX_LAP ];
		
		GPSLog = new GPS_LOG_t[ ( int )( MAX_VSD_LOG / LOG_FREQ ) ];
		
		if( filter->exfunc->dlg_get_load_name(szBuf,FILE_TXT,NULL) != TRUE ) break;
		if(( fp = fopen( szBuf, "r" )) == NULL ) return FALSE;
		
		// 20070814 以降のログは，横 G が反転している
		BOOL bReverseGy	= ( strcmp( StrTokFile( NULL, szBuf, STF_NAME ), "vsd20070814" ) >= 0 );
		
		g_iVsdLogNum	= 0;
		g_iLapNum		= 0;
		g_dBestTime		= -1;
		
		// ログリード
		
		UINT	uCnt, uLap, uMin, uSec, uMSec;
		double	dGcx = 0;
		double	dGcy = 0;
		double	dGx, dGy;
		
		g_iLogStart = g_iLogStop = 0;
		
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
					( g_dBestTime == -1 || g_dBestTime > dTime )
				){
					g_dBestTime			= dTime;
					g_iBestLapLogNum	= g_Lap[ g_iLapNum - 1 ].iLogNum;
				}
				++g_iLapNum;
			}
			
			if(( p = strstr( szBuf, "GPS" )) != NULL ){ // GPS記録を見つけた
				sscanf( p, "GPS%g%g%g%g",
					&GPSLog[ uGPSCnt ].fLati,
					&GPSLog[ uGPSCnt ].fLong,
					&GPSLog[ uGPSCnt ].fSpeed,
					&GPSLog[ uGPSCnt ].fBearing
				);
				
				if( dLongMin > GPSLog[ uGPSCnt ].fLong ) dLongMin = GPSLog[ uGPSCnt ].fLong;
				if( dLongMax < GPSLog[ uGPSCnt ].fLong ) dLongMax = GPSLog[ uGPSCnt ].fLong;
				if( dLatiMin > GPSLog[ uGPSCnt ].fLati ) dLatiMin = GPSLog[ uGPSCnt ].fLati;
				if( dLatiMax < GPSLog[ uGPSCnt ].fLati ) dLatiMax = GPSLog[ uGPSCnt ].fLati;
				
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
				
				g_VsdLog[ g_iVsdLogNum ].fX = g_VsdLog[ g_iVsdLogNum ].fY = INVALID_POS_D;
				
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
			
			// 単位を補正
			for( u = 0; u < uGPSCnt; ++u ){
				// 緯度・経度→メートル
				GPSLog[ u ].fX = ( float )(( GPSLog[ u ].fLong - dLongMin ) * LAT_M_DEG * cos( GPSLog[ u ].fLati * ToRAD ));
				GPSLog[ u ].fY = ( float )(( dLatiMax - GPSLog[ u ].fLati ) * LNG_M_DEG );
				
				// 速度・向き→ベクトル座標
				double dBearing	= GPSLog[ u ].fBearing * ToRAD;
				double dSpeed	= GPSLog[ u ].fSpeed * ( 1000.0 / 3600 );
				GPSLog[ u ].fVX	= ( float )(  sin( dBearing ) * dSpeed );
				GPSLog[ u ].fVY	= ( float )( -cos( dBearing ) * dSpeed );
			}
			
			// 補完点を計算
			for( u = 0; u < uGPSCnt - 1; ++u ){
				// 2秒以上 GPS ログがあいていれば，補完情報の計算をしない
				if( GPSLog[ u + 1 ].iLogNum - GPSLog[ u ].iLogNum > ( int )( LOG_FREQ * 2 )) continue;
				
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
					
					g_VsdLog[ i ].fX = ( float )((( dX3 * t + dX2 ) * t + dX1 ) * t + dX0 );
					g_VsdLog[ i ].fY = ( float )((( dY3 * t + dY2 ) * t + dY1 ) * t + dY0 );
				}
			}
			
			// マップの倍率を求める
			double dSizeX = (( dLongMax - dLongMin ) * LAT_M_DEG * cos(( dLatiMax + dLatiMin ) / 2 * ToRAD ));
			double dSizeY = (( dLatiMax - dLatiMin ) * LNG_M_DEG );
			
			g_dMaxMapSize = dSizeX > dSizeY ? dSizeX : dSizeY;
			
			/*
			FILE *fp = fopen( "c:\\dds\\hoge.log", "w" );
			for( i = 0; i < g_iVsdLogNum; ++i )
				fprintf( fp, "%g\t%g\n", g_VsdLog[ i ].fX, g_VsdLog[ i ].fY );
			fclose( fp );
			*/
		}
		
		delete [] GPSLog;
		
		/********************************************************************/
		
		fclose( fp );
		
		g_Lap[ g_iLapNum ].iLogNum = 0x7FFFFFFF;	// 番犬
		
		// trackbar 設定
		track_e[ TRACK_VSt ] =
		track_e[ TRACK_VEd ] = ( filter->exfunc->get_frame_n( editp ) + 99 ) / 100;
		track_e[ TRACK_LSt ] =
		track_e[ TRACK_LEd ] = ( g_iVsdLogNum + 99 ) / 100;
		
		// 設定再描画
		filter->exfunc->filter_window_update( filter );
		
		// log pos 更新
		func_update( filter, FILTER_UPDATE_STATUS_CHECK + CHECK_LOGPOS );
		
		return TRUE;
	}
	
	return FALSE;
}

/*** スライダバー連動 *******************************************************/

BOOL func_update( FILTER *fp, int status ){
	static	BOOL	bReEnter = FALSE;
	
	if( bReEnter ) return TRUE;
	
	bReEnter = TRUE;
	
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
