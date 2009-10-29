/*****************************************************************************
	
	VSD -- vehicle data logger system  Copyright(C) by DDS
	
	CVsdFilter.cpp - CVsdFilter class implementation
	$Id$
	
*****************************************************************************/

#include <windows.h>
#include <stdio.h>
#include <math.h>
#include <stddef.h>
#include <float.h>
#include <ctype.h>

#include "zlib/zlib.h"
#include "zlib/zconf.h"

#include "dds.h"
#include "../vsd/main.h"
#include "dds_lib/dds_lib.h"

#ifndef AVS_PLUGIN
	#include "filter.h"
#endif
#include "CVsdFilter.h"

/*** macros *****************************************************************/

#define PI			3.14159265358979323
#define ToRAD		( PI / 180 )

#define LAT_M_DEG	110863.95	// 緯度1度の距離
#define LNG_M_DEG	111195.10	// 経度1度の距離 @ 0N

#define INVALID_POS_I	0x7FFFFFFF
#define INVALID_POS_D	NaN

#define PTD_LOG_FREQ	11025.0

#define MAX_POLY_HEIGHT		2000		// polygon 用ライン数

/*** static member **********************************************************/

const UCHAR CVsdFilter::m_Font9p[] = {
	#include "font_9p.h"
};

const UCHAR CVsdFilter::m_Font18p[] = {
	#include "font_18p.h"
};

/*** tarckbar / checkbox conf_name 名 ***/

const char *CVsdFilter::m_szTrackbarName[] = {
	#define DEF_TRACKBAR( id, init, min, max, name, conf_name ) conf_name,
	#include "def_trackbar.h"
};

const char *CVsdFilter::m_szCheckboxName[] = {
	#define DEF_CHECKBOX( id, init, name, conf_name ) conf_name,
	#include "def_checkbox.h"
};

const char *CVsdFilter::m_szShadowParamName[] = {
	#define DEF_SHADOW( id, init, conf_name ) conf_name,
	#include "def_shadow.h"
};

/*** コンストラクタ *********************************************************/

CVsdFilter::CVsdFilter () {
	
	m_VsdLog 			= new VSD_LOG_t[ MAX_VSD_LOG ];;
	m_iVsdLogNum		= 0;
	
	m_Lap	 			= new LAP_t[ MAX_LAP ];
	m_iLapNum			= 0;
	m_Lap[ 0 ].iLogNum	= 0x7FFFFFFF;	// 番犬
	m_Lap[ 0 ].iTime	= 0;			// 番犬
	
	m_iBestTime			= BESTLAP_NONE;
	m_iBestLapLogNum	= 0;
	
	m_dMapSize			= 0;
	m_dMapOffsX			= 0;
	m_dMapOffsY			= 0;
	
	m_dVideoFPS			= 30.0;
	
	m_iPreW				= 0;
	
	m_iLapIdx			= -1;
	m_iBestLogNum		= 0;
	
	m_bCalcLapTimeReq	= FALSE;
	
	m_szLogFile			= NULL;
	
	// DrawPolygon 用バッファ
	m_Polygon = new PolygonData_t[ MAX_POLY_HEIGHT ];
}

/*** デストラクタ ***********************************************************/

CVsdFilter::~CVsdFilter () {
	delete m_VsdLog;
	m_VsdLog	= NULL;
	m_iVsdLogNum= 0;
	
	delete m_Lap;
	m_Lap		= NULL;
	m_iLapNum	= 0;
	
	delete [] m_szLogFile;
}

/*** フォントデータ初期化 ***************************************************/

void CVsdFilter::InitFont( void ){
	
	if( m_iPreW != GetWidth()){
		m_iPreW = GetWidth();
		
		const UCHAR *p = GetWidth() >= HIREZO_TH ? m_Font18p : m_Font9p;
		
		m_iBMP_H = *( int *)( p + 0x16 );
		m_iFontW = *( int *)( p + 0x12 ) / 16;
		m_iFontH = m_iBMP_H / 7;
		
		m_pFontData = p +
			0x36 +	// パレットデータ先頭 addr
			( 1 << *( USHORT *)( p + 0x1A )) *	// color depth
			4; // 1色あたりのバイト数
		
		m_iBMP_BytesPerLine = *( int *)( p + 0x12 ) / 8;
	}
}

/*** DrawLine ***************************************************************/

#define ABS( x )			(( x ) < 0 ? -( x ) : ( x ))
#define SWAP( x, y, tmp )	( tmp = x, x = y, y = tmp )

void CVsdFilter::DrawLine( int x1, int y1, int x2, int y2, const PIXEL_YC &yc, UINT uFlag ){
	
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

void CVsdFilter::DrawLine( int x1, int y1, int x2, int y2, int width, const PIXEL_YC &yc, UINT uFlag ){
	for( int y = 0; y < width; ++y ) for( int x = 0; x < width; ++x ){
		DrawLine(
			x1 + x - width / 2, y1 + y - width / 2,
			x2 + x - width / 2, y2 + y - width / 2,
			yc, uFlag
		);
	}
}

inline void CVsdFilter::FillLine( int x1, int y1, int x2, const PIXEL_YC &yc, UINT uFlag ){
	
	int	i;
	
	// x 基準で描画
	for( i = x1; i <= x2; ++i ) PutPixel( i, y1, yc, uFlag );
}

/*** DrawRect ***************************************************************/

void CVsdFilter::DrawRect( int x1, int y1, int x2, int y2, const PIXEL_YC &yc, UINT uFlag ){
	int	y;
	
	if( y1 > y2 ) SWAP( y1, y2, y );
	if( x1 > x2 ) SWAP( x1, x2, y );
	
	for( y = y1; y <= y2; ++y ){
		FillLine( x1, y, x2, yc, uFlag );
	}
}

/*** DrawCircle *************************************************************/

void CVsdFilter::DrawCircle( int x, int y, int r, const PIXEL_YC &yc, UINT uFlag ){
	
	int			i, j;
	
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

/*** DrawFont ***************************************************************/

void CVsdFilter::DrawFont( int x, int y, UCHAR c, const PIXEL_YC &yc, UINT uFlag ){
	
	int	i, j;
	
	if( c -= ' ' ) for( j = 0; j < GetFontH(); ++j ) for( i = 0; i < GetFontW(); ++i ){
		if( GetBMPPix(
			( c % 16 ) * GetFontW() + i,
			( c / 16 ) * GetFontH() + j
		) == 0 ) PutPixel( x + i, y + j, yc, uFlag );
	}
}

void CVsdFilter::DrawFont( int x, int y, UCHAR c, const PIXEL_YC &yc, const PIXEL_YC &ycEdge, UINT uFlag ){
	
	int	i, j;
	
	UCHAR cc = c - ' ';
	
	if( !cc ) return;
	
	for( j = 0; j < GetFontH(); ++j ) for( i = 0; i < GetFontW(); ++i ){
		if( GetBMPPix(
			( cc % 16 ) * GetFontW() + i,
			( cc / 16 ) * GetFontH() + j
		) == 0 ){
			PutPixel( x + i - 1, y + j,		ycEdge, uFlag );
			PutPixel( x + i + 1, y + j,		ycEdge, uFlag );
			PutPixel( x + i,	 y + j - 1, ycEdge, uFlag );
			PutPixel( x + i,	 y + j + 1, ycEdge, uFlag );
		}
	}
	
	DrawFont( x, y, c, yc, uFlag );
}

/*** DrawString *************************************************************/

void CVsdFilter::DrawString( char *szMsg, const PIXEL_YC &yc, UINT uFlag, int x, int y ){
	
	if( x != POS_DEFAULT ) m_iPosX = x;
	if( y != POS_DEFAULT ) m_iPosY = y;
	
	for( int i = 0; szMsg[ i ]; ++i ){
		DrawFont( m_iPosX + i * GetFontW(), m_iPosY, szMsg[ i ], yc, uFlag );
	}
	
	m_iPosY += GetFontH();
}

void CVsdFilter::DrawString( char *szMsg, const PIXEL_YC &yc, const PIXEL_YC &ycEdge, UINT uFlag, int x, int y ){
	
	if( x != POS_DEFAULT ) m_iPosX = x;
	if( y != POS_DEFAULT ) m_iPosY = y;
	
	for( int i = 0; szMsg[ i ]; ++i ){
		DrawFont( m_iPosX + i * GetFontW(), m_iPosY, szMsg[ i ], yc, ycEdge, uFlag );
	}
	
	m_iPosY += GetFontH();
}

/*** ポリゴン描画 ***********************************************************/

inline void CVsdFilter::PolygonClear( void ){
	for( int y = 0; y < GetHeight(); ++y ){
		m_Polygon[ y ].iRight	= 0;		// right
		m_Polygon[ y ].iLeft	= 0x7FFF;	// left
	}
}

inline void CVsdFilter::PolygonDraw( const PIXEL_YC &yc, UINT uFlag ){
	for( int y = 0; y < GetHeight(); ++y ) if( m_Polygon[ y ].iLeft <= m_Polygon[ y ].iRight ){
		FillLine( m_Polygon[ y ].iLeft, y, m_Polygon[ y ].iRight, yc, uFlag & ~IMG_POLYGON );
	}
}

/*** 設定ロード・セーブ *****************************************************/

char *CVsdFilter::IsConfigParam( const char *szParamName, char *szBuf, int &iVal ){
	
	int	iLen;
	
	while( isspace( *szBuf )) ++szBuf;
	
	if(
		strncmp( szBuf, szParamName, iLen = strlen( szParamName )) == 0 &&
		szBuf[ iLen ] == '='
	){
		iVal = atoi( szBuf + iLen + 1 );
		return szBuf + iLen + 1;
	}
	
	return NULL;
}

BOOL CVsdFilter::ParseMarkStr( const char *szMark ){
	
	while( szMark && *szMark ){
		SetFrameMark( atoi( szMark ));
		if( szMark = strchr( szMark, ',' )) ++szMark;	// 次のパラメータ
	}
	m_bCalcLapTimeReq = TRUE;
	return TRUE;
}

BOOL CVsdFilter::ConfigLoad( const char *szFileName ){
	
	int 	i, iVal;
	FILE	*fp;
	char	szBuf[ BUF_SIZE ];
	
	if(( fp = fopen( szFileName, "r" )) != NULL ){
		m_bCalcLapTimeReq = TRUE;
		
		while( fgets( szBuf, BUF_SIZE, fp )){
			if( char *p = IsConfigParam( "mark", szBuf, iVal )){
				// ラップタイムマーク
				ParseMarkStr( p + 1 );
			}else{
				// Mark 以外のパラメータ
				for( i = 0; i < TRACK_N; ++i ){
					if(
						m_szTrackbarName[ i ] &&
						IsConfigParam( m_szTrackbarName[ i ], szBuf, iVal )
					){
						m_piParamT[ i ] = iVal;
						
						if( i <= TRACK_LEd ){
							m_piParamT[ i + 1 ] = m_piParamT[ i ] % 100;
							m_piParamT[ i ] /= 100;
						}
						goto Next;
					}
				}
				
				for( i = 0; i < CHECK_N; ++i ){
					if(
						m_szCheckboxName[ i ] &&
						IsConfigParam( m_szCheckboxName[ i ], szBuf, iVal )
					){
						m_piParamC[ i ] = iVal;
						goto Next;
					}
				}
				
				for( i = 0; i < SHADOW_N; ++i ){
					if( IsConfigParam( m_szShadowParamName[ i ], szBuf, iVal )){
						m_piParamS[ i ] = iVal;
						goto Next;
					}
				}
			}
		  Next: ;
		}
		fclose( fp );
	}
	return TRUE;
}

#ifndef AVS_PLUGIN
BOOL CVsdFilter::ConfigSave( const char *szFileName ){
	FILE	*fp;
	int		i;
	
	if(( fp = fopen( szFileName, "w" )) == NULL ) return FALSE;
	
	char szBuf[ BUF_SIZE ];
	
	fprintf( fp,
		"DirectShowSource( \"%s\", pixel_type=\"YUY2\", convertfps = true  )\n"
		"VSDFilter( \\\n\tlog_file=\"%s\"",
		GetVideoFileName( szBuf ),
		m_szLogFile ? m_szLogFile : ""
	);
	
	for( i = 0; i < TRACK_N; ++i ){
		if( m_szTrackbarName[ i ] == NULL ) continue;
		
		fprintf( fp, ", \\\n\t%s=%d", m_szTrackbarName[ i ],
			( i <= TRACK_LEd ) ? m_piParamT[ i ] * 100 + m_piParamT[ i + 1 ] :
			m_piParamT[ i ]
		);
	}
	
	for( i = 0; i < CHECK_N; ++i ){
		if( m_szCheckboxName[ i ] == NULL ) continue;
		
		fprintf(
			fp, ", \\\n\t%s=%d", m_szCheckboxName[ i ], m_piParamC[ i ]
		);
	}
	
	// 手動ラップ計測マーク出力
	if( IsHandLaptime() && m_iLapNum ){
		for( i = 0; i < m_iLapNum; ++i ){
			fprintf( fp, "%s%u", i ? "," : ", \\\n\tmark=\"", m_Lap[ i ].iLogNum );
		}
		fputc( '"', fp );
	}
	
	fprintf( fp, " \\\n)\n" );
	
	fclose( fp );
	return TRUE;
}
#endif

/*** GPS ログの up-convert **************************************************/

void UpConvertGPSLog( VSD_LOG_t *Out, GPS_LOG_t *In, UINT uCnt ){
	
	int		iLogNum;
	int		i		= -1;
	
	double	dX3, dX2, dX1, dX0;
	double	dY3, dY2, dY1, dY0;
	double	t;
	
	In[ uCnt ].fTime = 9999999; // 番犬
	
	iLogNum = ( int )( In[ 0 ].fTime * LOG_FREQ + 1 );

	for(;; ++iLogNum ){
		
		// In[ i ].fTime <= iLogNum / LOG_FREQ < In[ i + 1 ].fTime
		// の範囲になければ，正しい i を探す
		if( i == -1 || ( float )iLogNum / LOG_FREQ >= In[ i + 1 ].fTime ){
			
			for( ++i; ( float )iLogNum / LOG_FREQ >= In[ i + 1 ].fTime; ++i );
			
			// GPS ログ範囲を超えたので return
			if( i > ( int )uCnt - 2 ) return;
			
			// パラメータを計算
			#define X0	In[ i ].fX
			#define Y0	In[ i ].fY
			#define VX0	In[ i ].fVX
			#define VY0	In[ i ].fVY
			#define X1	In[ i + 1 ].fX
			#define Y1	In[ i + 1 ].fY
			#define VX1	In[ i + 1 ].fVX
			#define VY1	In[ i + 1 ].fVY
			
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
		}
		
		// 5秒以上 GPS ログがあいていれば，補完情報の計算をしない
		//if( In[ i + 1 ].fTime - In[ i ].fTime > 5 ) continue;
		
		t =
			( double )(( float )iLogNum / LOG_FREQ - In[ i ].fTime ) /
			( In[ i + 1 ].fTime - In[ i ].fTime );
		
		Out[ iLogNum ].fX0 = ( float )((( dX3 * t + dX2 ) * t + dX1 ) * t + dX0 );
		Out[ iLogNum ].fY0 = ( float )((( dY3 * t + dY2 ) * t + dY1 ) * t + dY0 );
	}
}

/*** ログリード *************************************************************/

BOOL CVsdFilter::ReadLog( const char *szFileName ){
	TCHAR	szBuf[ BUF_SIZE ];
	gzFile	fp;
	BOOL	bCalibrating = FALSE;
	
	float		NaN = 0;
	NaN /= *( volatile float *)&NaN;
	
	if(( fp = gzopen(( char *)szFileName, "rb" )) == NULL ) return FALSE;
	
#ifndef AVS_PLUGIN
	if( m_szLogFile ) delete [] m_szLogFile;
	m_szLogFile = new char[ strlen( szFileName ) + 1 ];
	strcpy( m_szLogFile, szFileName );
#endif
	
	// GPS ログ用
	UINT		uGPSCnt = 0;
	GPS_LOG_t	*GPSLog = new GPS_LOG_t[ ( int )( MAX_VSD_LOG * GPS_FREQ / LOG_FREQ ) ];
	
	// 初期化
	m_iVsdLogNum	= 0;
	m_iLapNum		= 0;
	m_iBestTime		= BESTLAP_NONE;
	m_iBestLapLogNum= 0;
	m_iLapIdx		= -1;
	m_iBestLogNum	= 0;
	
	// ログリード
	
	UINT	uCnt, uLap, uMin, uSec, uMSec;
	double	dGcx = 0;
	double	dGcy = 0;
	double	dGx, dGy;
	
	double	dLati;
	double	dLong;
	double	dLati0 = 0;
	double	dLong0 = 0;
	double	dSpeed;
	double	dBearing;
	
	m_iLogStart = m_iLogStop = 0;
	
	TCHAR	*p;
	
	while( gzgets( fp, szBuf, BUF_SIZE ) != Z_NULL ){
		if(( p = strstr( szBuf, "LAP" )) != NULL ){ // ラップタイム記録を見つけた
			uCnt = sscanf( p, "LAP%d%d:%d.%d", &uLap, &uMin, &uSec, &uMSec );
			
			int iTime = ( uMin * 60 + uSec ) * 1000 + uMSec;
			
			m_Lap[ m_iLapNum ].uLap		= uLap;
			m_Lap[ m_iLapNum ].iLogNum	= m_iVsdLogNum;
			m_Lap[ m_iLapNum ].iTime	= ( uCnt == 4 ) ? iTime : 0;
			
			if(
				uCnt == 4 &&
				( m_iBestTime == BESTLAP_NONE || m_iBestTime > iTime )
			){
				m_iBestTime			= iTime;
				m_iBestLapLogNum	= m_Lap[ m_iLapNum - 1 ].iLogNum;
			}
			++m_iLapNum;
		}
		
		if(( p = strstr( szBuf, "GPS" )) != NULL ){ // GPS記録を見つけた
			sscanf( p, "GPS%lg%lg%lg%lg", &dLati, &dLong, &dSpeed, &dBearing );
			
			if( dLati0 == 0 ){
				dLati0 = dLati;
				dLong0 = dLong;
			}
			
			// 単位を補正
			// 緯度・経度→メートル
			GPSLog[ uGPSCnt ].fX = ( float )(( dLong - dLong0 ) * LAT_M_DEG * cos( dLati * ToRAD ));
			GPSLog[ uGPSCnt ].fY = ( float )(( dLati0 - dLati ) * LNG_M_DEG );
			
			// 速度・向き→ベクトル座標
			dBearing *= ToRAD;
			dSpeed	 *= ( 1000.0 / 3600 );
			GPSLog[ uGPSCnt ].fVX	= ( float )(  sin( dBearing ) * dSpeed );
			GPSLog[ uGPSCnt ].fVY	= ( float )( -cos( dBearing ) * dSpeed );
			
			GPSLog[ uGPSCnt++ ].fTime = ( m_iVsdLogNum - GPS_LOG_OFFS ) / ( float )LOG_FREQ;
		}
		
		// 普通の log
		if(( uCnt = sscanf( szBuf, "%g%g%g%lg%lg",
			&m_VsdLog[ m_iVsdLogNum ].fTacho,
			&m_VsdLog[ m_iVsdLogNum ].fSpeed,
			&m_VsdLog[ m_iVsdLogNum ].fMileage,
			&dGy,
			&dGx
		)) >= 3 ){
			if( uCnt < 5 && m_iVsdLogNum ){
				// Gデータがないときは，speedから求める←廃止
				dGx = 0;
				dGy = 0;
				//dGy = ( m_VsdLog[ m_iVsdLogNum ].fSpeed - m_VsdLog[ m_iVsdLogNum - 1 ].fSpeed ) * ( 1000.0 / 3600 / 9.8 * LOG_FREQ );
			}else{
				if( dGx >= 4 ){	
					// 単位を G に変換
					dGx = -dGx / ACC_1G_Y;
					dGy =  dGy / ACC_1G_Z;
				}
				
				if( m_iVsdLogNum == 0 ){
					// G センターの初期値
					dGcx = dGx;
					dGcy = dGy;
				}
				
				// G センターだし
				dGx -= dGcx;
				dGy -= dGcy;
				
				// 静止していると思しきときは，G センターを補正する
				// 走行距離 == 0 && ±0.02G
				if(
					m_iVsdLogNum &&
					m_VsdLog[ m_iVsdLogNum - 1 ].fMileage == m_VsdLog[ m_iVsdLogNum ].fMileage &&
					( m_VsdLog[ m_iVsdLogNum - 1 ].fGy - dGy ) >= -0.02 &&
					( m_VsdLog[ m_iVsdLogNum - 1 ].fGy - dGy ) <=  0.02
				){
					dGcx += dGx / 160;
					dGcy += dGy / 160;
				}
			}
			
			m_VsdLog[ m_iVsdLogNum ].fGx = ( float )dGx;
			m_VsdLog[ m_iVsdLogNum ].fGy = ( float )dGy;
			
			m_VsdLog[ m_iVsdLogNum ].fX0 = m_VsdLog[ m_iVsdLogNum ].fY0 = INVALID_POS_D;
			
			// ログ開始・終了認識
			if( m_VsdLog[ m_iVsdLogNum ].fSpeed >= 300 ){
				if( !bCalibrating ){
					bCalibrating = TRUE;
					m_iLogStart  = m_iLogStop;
					m_iLogStop   = m_iVsdLogNum;
				}
			}else{
				bCalibrating = FALSE;
			}
			
			// メーター補正
			m_VsdLog[ m_iVsdLogNum ].fTacho = ( float )(
				m_VsdLog[ m_iVsdLogNum ].fTacho * m_piParamS[ METER_ADJUST ] / 1000.0
			);
			m_VsdLog[ m_iVsdLogNum ].fSpeed = ( float )(
				m_VsdLog[ m_iVsdLogNum ].fSpeed * m_piParamS[ METER_ADJUST ] / 1000.0
			);
			++m_iVsdLogNum;
		}
	}
	
	/*** GPS ログから軌跡を求める ***************************************/
	
	if( uGPSCnt ){
		UpConvertGPSLog( m_VsdLog, GPSLog, uGPSCnt );
		RotateMap();	// Map 回転
	}
	
	delete [] GPSLog;
	
	/********************************************************************/
	
	gzclose( fp );
	
	m_Lap[ m_iLapNum ].iLogNum	= 0x7FFFFFFF;	// 番犬
	m_Lap[ m_iLapNum ].iTime	= 0;			// 番犬
	
	return TRUE;
}

/*** MAP 回転処理 ***********************************************************/

void CVsdFilter::RotateMap( void ){
	
	int i;
	double dMaxX, dMinX, dMaxY, dMinY;
	double dMapAngle = ( m_piParamT[ TRACK_MapAngle ] * ( -ToRAD / 10 ));
	
	dMaxX = dMinX = dMaxY = dMinY = 0;
	
	for( i = 0; i < m_iVsdLogNum; ++i ){
		m_VsdLog[ i ].fX = ( float )(  cos( dMapAngle ) * m_VsdLog[ i ].fX0 + sin( dMapAngle ) * m_VsdLog[ i ].fY0 );
		m_VsdLog[ i ].fY = ( float )( -sin( dMapAngle ) * m_VsdLog[ i ].fX0 + cos( dMapAngle ) * m_VsdLog[ i ].fY0 );
		
		if     ( dMaxX < m_VsdLog[ i ].fX ) dMaxX = m_VsdLog[ i ].fX;
		else if( dMinX > m_VsdLog[ i ].fX ) dMinX = m_VsdLog[ i ].fX;
		if     ( dMaxY < m_VsdLog[ i ].fY ) dMaxY = m_VsdLog[ i ].fY;
		else if( dMinY > m_VsdLog[ i ].fY ) dMinY = m_VsdLog[ i ].fY;
	}
	
	dMaxX -= dMinX;
	dMaxY -= dMinY;
	
	m_dMapSize	= dMaxX > dMaxY ? dMaxX : dMaxY;
	m_dMapOffsX		= dMinX;
	m_dMapOffsY		= dMinY;
}

/****************************************************************************/
/*** メーター描画 ***********************************************************/
/****************************************************************************/

/*** macros *****************************************************************/

#define VideoSt			( m_piParamT[ TRACK_VSt ] * 100 + m_piParamT[ TRACK_VSt2 ] )
#define VideoEd			( m_piParamT[ TRACK_VEd ] * 100 + m_piParamT[ TRACK_VEd2 ] )
#define LogSt			( m_piParamT[ TRACK_LSt ] * 100 + m_piParamT[ TRACK_LSt2 ] )
#define LogEd			( m_piParamT[ TRACK_LEd ] * 100 + m_piParamT[ TRACK_LEd2 ] )
#define LineTrace		m_piParamT[ TRACK_LineTrace ]

#define DispLap			m_piParamC[ CHECK_LAP ]
#define GSnakeLen		m_piParamT[ TRACK_G_Len ]
#define GScale			( m_piParamS[ SHADOW_G_SCALE ] / 1000.0 )

#define MAX_MAP_SIZE	( GetWidth() * m_piParamT[ TRACK_MapSize ] / 1000.0 )

/*** メーター等描画 *********************************************************/

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
	m_VsdLog[ ( UINT )dLogNum     ].p * ( 1 - ( dLogNum - ( UINT )dLogNum )) + \
	m_VsdLog[ ( UINT )dLogNum + 1 ].p * (       dLogNum - ( UINT )dLogNum ))

// ログ番号からフレームカウントを得る
#define GetFrameNum( i ) ( \
	( LogEd == LogSt ) ? 0 : \
	((( i ) - LogSt ) * ( VideoEd - VideoSt ) / ( double )( LogEd - LogSt ) + VideoSt ) \
)

BOOL CVsdFilter::DrawVSD( void ){
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
	
	char	szBuf[ 128 ];
	int	i;
	
	BOOL	bInLap = FALSE;	// ラップタイム計測中
	
	const int	iMeterR			= m_piParamS[ SHADOW_METER_R  ] >= 0 ? m_piParamS[ SHADOW_METER_R  ] : 50 * GetWidth() / 320;
	const int	iMeterCx		= m_piParamS[ SHADOW_METER_CX ] >= 0 ? m_piParamS[ SHADOW_METER_CX ] : GetWidth()  - iMeterR - 2;
	const int	iMeterCy		= m_piParamS[ SHADOW_METER_CY ] >= 0 ? m_piParamS[ SHADOW_METER_CY ] : GetHeight() - iMeterR - 2;
	const int	iMeterMinDeg	= 135;
	const int	iMeterMaxDeg	= 45;
	const int	iMeterMaxVal	= 7000;
	const int	iMeterDegRange	= ( iMeterMaxDeg + 360 - iMeterMinDeg ) % 360;
	const int	iMeterScaleLen	= iMeterR / 8;
	
	// フォントサイズ初期化
	InitFont();
	
	// ログ位置の計算
	double	dLogNum = ( VideoEd == VideoSt ) ? -1 :
						( double )( LogEd - LogSt ) / ( VideoEd - VideoSt ) * ( GetFrameCnt() - VideoSt ) + LogSt;
	int		iLogNum = ( int )dLogNum;
	
	double	dLogFreq;	// 自動計測モード時の，実測 Log Freq
	
	// フレーム表示
	if( m_piParamC[ CHECK_FRAME ] ){
		sprintf( szBuf, "V%6d/%6d", GetFrameCnt(), GetFrameMax() - 1 );
		DrawString( szBuf, COLOR_STR, COLOR_TIME_EDGE, 0, GetWidth() / 2, GetHeight() / 2 );
		sprintf( szBuf, "L%6d/%6d", ( int )dLogNum, m_iVsdLogNum - 1 );
		DrawString( szBuf, COLOR_STR, COLOR_TIME_EDGE, 0 );
	}
	
	/*** Lap タイム描画 ***/
	
	if( IsHandLaptime() && DispLap && m_bCalcLapTimeReq && m_Lap ){
		m_bCalcLapTimeReq = FALSE;
		CalcLapTime();
	}
	
	// ラップインデックスを求める
	if( m_iLapNum ){
		if( IsHandLaptime() ){
			// CIRCUIT_TOMO での m_Lap[].iLogNum はフレーム# なので
			iLogNum = GetFrameCnt();
		}
		
		// カレントポインタがおかしいときは，-1 にリセット
		if(
			m_iLapIdx >= m_iLapNum ||
			m_iLapIdx >= 0 && m_Lap[ m_iLapIdx ].iLogNum > iLogNum
		) m_iLapIdx = -1;
		
		for( ; m_Lap[ m_iLapIdx + 1 ].iLogNum <= iLogNum; ++m_iLapIdx );
	}
	
	if( DispLap && m_iLapNum ){
		// 時間表示
		if( m_iLapIdx >= 0 && m_Lap[ m_iLapIdx + 1 ].iTime != 0 ){
			int iTime;
			if( IsHandLaptime()){
				// 手動計測モードのときは，フレーム数から計算
				iTime = ( int )(( GetFrameCnt() - m_Lap[ m_iLapIdx ].iLogNum ) * 1000.0 / m_dVideoFPS );
			}else{
				// 自動計測時は，タイム / ログ数 から計算
				dLogFreq =
					( double )(( m_Lap[ m_iLapIdx + 1 ].iLogNum - m_Lap[ m_iLapIdx ].iLogNum ) * 1000 ) /
					m_Lap[ m_iLapIdx + 1 ].iTime;
				
				iTime = ( int )(( dLogNum - m_Lap[ m_iLapIdx ].iLogNum ) * 1000 / dLogFreq );
			}
			
			sprintf( szBuf, "Time%2d'%02d.%03d", iTime / 60000, iTime / 1000 % 60, iTime % 1000 );
			DrawString( szBuf, COLOR_TIME, COLOR_TIME_EDGE, 0, GetWidth() - GetFontW() * 13, 1 );
			bInLap = TRUE;
		}else{
			// まだ開始していない
			DrawString( "Time -'--.---", COLOR_TIME, COLOR_TIME_EDGE, 0, GetWidth() - GetFontW() * 13, 1 );
		}
		
		/*** ベストとの車間距離表示 ***/
		if( !IsHandLaptime() ){
			if( bInLap ){
				// この周の走行距離を求める
				double dMileage = GetVsdLog( fMileage ) - m_VsdLog[ m_Lap[ m_iLapIdx ].iLogNum ].fMileage;
				
				// 最速 Lap の，同一走行距離におけるタイム (=ログ番号,整数) を求める
				// m_iBestLogNum <= 最終的に求める結果 < m_iBestLogNum + 1  となる
				// m_iBestLogNum がおかしかったら，リセット
				if(
					m_iBestLogNum < m_iBestLapLogNum ||
					m_iBestLogNum >= m_iVsdLogNum ||
					( m_VsdLog[ m_iBestLogNum ].fMileage - m_VsdLog[ m_iBestLapLogNum ].fMileage ) > dMileage
				) m_iBestLogNum = m_iBestLapLogNum;
				
				for(
					;
					( m_VsdLog[ m_iBestLogNum + 1 ].fMileage - m_VsdLog[ m_iBestLapLogNum ].fMileage ) <= dMileage &&
					m_iBestLogNum < m_iVsdLogNum;
					++m_iBestLogNum
				);
				
				// 最速 Lap の，1/15秒以下の値を求める = A / B
				double dBestLogNum =
					( double )m_iBestLogNum +
					// A: 最速ラップは，後これだけ走らないと dMileage と同じではない
					( dMileage - ( m_VsdLog[ m_iBestLogNum ].fMileage - m_VsdLog[ m_iBestLapLogNum ].fMileage )) /
					// B: 最速ラップは，1/15秒の間にこの距離を走った
					( m_VsdLog[ m_iBestLogNum + 1 ].fMileage - m_VsdLog[ m_iBestLogNum ].fMileage );
				
				int iDiffTime = ( int )(
					(
						( dLogNum - m_Lap[ m_iLapIdx ].iLogNum ) -
						( dBestLogNum - m_iBestLapLogNum )
					) * 1000.0 / dLogFreq
				);
				
				BOOL bSign = iDiffTime <= 0;
				if( iDiffTime < 0 ) iDiffTime = -iDiffTime;
				
				sprintf(
					szBuf, "    %c%d'%02d.%03d",
					bSign ? '-' : '+',
					iDiffTime / 60000,
					iDiffTime / 1000 % 60,
					iDiffTime % 1000
				);
				DrawString( szBuf, bSign ? COLOR_DIFF_MINUS : COLOR_DIFF_PLUS, COLOR_TIME_EDGE, 0 );
			}else{
				m_iPosY += GetFontH();
			}
		}
		
		m_iPosY += GetFontH() / 4;
		
		// Best 表示
		sprintf(
			szBuf, "Best%2d'%02d.%03d",
			m_iBestTime / 60000,
			m_iBestTime / 1000 % 60,
			m_iBestTime % 1000
		);
		DrawString( szBuf, COLOR_TIME, COLOR_TIME_EDGE, 0 );
		
		// Lapタイム表示
		i = 0;
		for( int iLapIdxTmp = m_iLapIdx + 1; iLapIdxTmp >= 0 && i < 3; --iLapIdxTmp ){
			if( m_Lap[ iLapIdxTmp ].iTime != 0 ){
				sprintf(
					szBuf, "%3d%c%2d'%02d.%03d",
					m_Lap[ iLapIdxTmp ].uLap,
					( i == 0 && bInLap ) ? '*' : ' ',
					m_Lap[ iLapIdxTmp ].iTime / 60000,
					m_Lap[ iLapIdxTmp ].iTime / 1000 % 60,
					m_Lap[ iLapIdxTmp ].iTime % 1000
				);
				DrawString( szBuf,
					m_iBestTime == m_Lap[ iLapIdxTmp ].iTime ? COLOR_BEST_LAP : COLOR_TIME,
					COLOR_TIME_EDGE, 0 );
				++i;
			}
		}
	}
	
	if( m_iVsdLogNum == 0 ) return TRUE;
	
	/*** メーターパネル ***/
	DrawCircle(
		iMeterCx, iMeterCy, iMeterR, COLOR_PANEL,
		CVsdFilter::IMG_ALFA | CVsdFilter::IMG_FILL
	);
	
	// タコメータ
	for( i = 0; i <= iMeterMaxVal; i += 500 ){
		int iDeg = iMeterDegRange * i / iMeterMaxVal + iMeterMinDeg;
		if( iDeg >= 360 ) iDeg -= 360;
		
		// メーターパネル目盛り
		if( iMeterMaxVal <= 12000 || i % 1000 == 0 ){
			DrawLine(
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
				DrawString(
					szBuf,
					COLOR_STR, 0,
					( int )( cos( iDeg * ToRAD ) * iMeterR * .8 ) + iMeterCx - GetFontW() / ( i >= 10000 ? 1 : 2 ),
					( int )( sin( iDeg * ToRAD ) * iMeterR * .8 ) + iMeterCy - GetFontH() / 2
				);
			}
		}
	}
	
	/*** メーター描画 ***/
	
	if( dLogNum < 0 || dLogNum > m_iVsdLogNum - 1 ) return TRUE;
	
	double	dSpeed	= GetVsdLog( fSpeed );
	double	dTacho	= GetVsdLog( fTacho );
	
	// G スネーク
	int	iGx, iGy;
	
	if( GSnakeLen >= 0 ){
		if( GSnakeLen > 0 ){
			
			int iGxPrev = 0, iGyPrev;
			
			for( i = -GSnakeLen; i <= 1 ; ++i ){
				
				if( iLogNum + i >= 0 ){
					// i == 1 時は最後の中途半端な LogNum
					iGx = iMeterCx + ( int )((( i != 1 ) ? m_VsdLog[ iLogNum + i ].fGx : GetVsdLog( fGx )) * iMeterR / GScale );
					iGy = iMeterCy - ( int )((( i != 1 ) ? m_VsdLog[ iLogNum + i ].fGy : GetVsdLog( fGy )) * iMeterR / GScale );
					
					if( iGxPrev ) DrawLine(
						iGx, iGy, iGxPrev, iGyPrev,
						LINE_WIDTH, COLOR_G_HIST, 0
					);
					
					iGxPrev = iGx;
					iGyPrev = iGy;
				}
			}
		}else{
			iGx = iMeterCx + ( int )( GetVsdLog( fGx ) * iMeterR / GScale );
			iGy = iMeterCy - ( int )( GetVsdLog( fGy ) * iMeterR / GScale );
		}
		
		// G インジケータ
		DrawCircle(
			iGx, iGy, iMeterR / 20,
			COLOR_G_SENSOR, CVsdFilter::IMG_FILL
		);
	}
	
	// MAP 表示
	if( LineTrace ){
		double dGx, dGy;
		
		int iGxPrev = INVALID_POS_I, iGyPrev;
		
		int	iLineSt = m_iLapIdx >= 0 ? m_Lap[ m_iLapIdx ].iLogNum : 0;
		if( iLogNum - iLineSt > ( int )( LineTrace * LOG_FREQ ))
			iLineSt = iLogNum - ( int )( LineTrace * LOG_FREQ );
		
		int iLineEd = m_iLapIdx != m_iLapNum - 1 ? m_Lap[ m_iLapIdx + 1 ].iLogNum : m_iVsdLogNum - 1;
		if( iLineEd - iLogNum > ( int )( LineTrace * LOG_FREQ ))
			iLineEd = iLogNum + ( int )( LineTrace * LOG_FREQ );
		
		for( i = iLineSt; i <= iLineEd ; ++i ){
			#define GetMapPos( p, a ) ( (( p ) - m_dMapOffs ## a ) / m_dMapSize * MAX_MAP_SIZE + 8 )
			dGx = GetMapPos( m_VsdLog[ i ].fX, X );
			dGy = GetMapPos( m_VsdLog[ i ].fY, Y );
			
			if( !_isnan( dGx )){
				iGx = ( int )dGx;
				iGy = ( int )dGy;
				
				if( iGxPrev != INVALID_POS_I ){
					// Line の色用に G を求める
					double dG = sqrt(
						m_VsdLog[ i ].fGx * m_VsdLog[ i ].fGx +
						m_VsdLog[ i ].fGy * m_VsdLog[ i ].fGy
					) / MAP_G_MAX;
					
					PIXEL_YC yc_line;
					
					if( dG < 0.5 ){
						dG *= 2;
						yc_line.y  = ( PIXEL_t )( MAP_LINE2.y  * dG + MAP_LINE1.y  * ( 1 - dG ));
						yc_line.cb = ( PIXEL_t )( MAP_LINE2.cb * dG + MAP_LINE1.cb * ( 1 - dG ));
						yc_line.cr = ( PIXEL_t )( MAP_LINE2.cr * dG + MAP_LINE1.cr * ( 1 - dG ));
					#ifdef AVS_PLUGIN
						yc_line.y1 = yc_line.y;
					#endif
					}else if( dG < 1.0 ){
						dG = ( dG - 0.5 ) * 2;
						yc_line.y  = ( PIXEL_t )( MAP_LINE3.y  * dG + MAP_LINE2.y  * ( 1 - dG ));
						yc_line.cb = ( PIXEL_t )( MAP_LINE3.cb * dG + MAP_LINE2.cb * ( 1 - dG ));
						yc_line.cr = ( PIXEL_t )( MAP_LINE3.cr * dG + MAP_LINE2.cr * ( 1 - dG ));
					#ifdef AVS_PLUGIN
						yc_line.y1 = yc_line.y;
					#endif
					}else{
						yc_line = MAP_LINE3;
					}
					
					// Line を引く
					DrawLine(
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
		
		if( !_isnan( dGx )) DrawCircle(
			( int )dGx, ( int )dGy, iMeterR / 20,
			COLOR_CURRENT_POS, CVsdFilter::IMG_FILL
		);
	}
	
	// スピード / ギア
	UINT uGear = 0;
	
	if( dTacho ){
		UINT uGearRatio = ( int )( dSpeed * 100 * ( 1 << 8 ) / dTacho );
		
		if     ( uGearRatio < GEAR_TH( 1 ))	uGear = 1;
		else if( uGearRatio < GEAR_TH( 2 ))	uGear = 2;
		else if( uGearRatio < GEAR_TH( 3 ))	uGear = 3;
		else if( uGearRatio < GEAR_TH( 4 ))	uGear = 4;
		else								uGear = 5;
	}
	
	// スピード表示
	sprintf( szBuf, "%d\x7F", uGear );
	DrawString(
		szBuf,
		COLOR_STR, 0,
		iMeterCx - 1 * GetFontW(), iMeterCy - iMeterR / 2
	);
	
	sprintf( szBuf, "%3d\x80\x81", ( int )dSpeed );
	DrawString(
		szBuf,
		COLOR_STR, 0,
		iMeterCx - 3 * GetFontW(), iMeterCy + iMeterR / 2
	);
	
	sprintf( szBuf, "%02dG", ( int )( sqrt( GetVsdLog( fGx ) * GetVsdLog( fGx ) + GetVsdLog( fGy ) * GetVsdLog( fGy )) * 10 ));
	DrawString(
		szBuf,
		COLOR_STR, 0,
		iMeterCx - 2 * GetFontW(), iMeterCy + iMeterR / 2 - GetFontH()
	);
	DrawRect(
		m_iPosX + GetFontW() - 1, m_iPosY - 4,
		m_iPosX + GetFontW()    , m_iPosY - 3,
		COLOR_STR, 0
	);
	
	// Tacho の針
	double dTachoNeedle = iMeterDegRange / ( double )iMeterMaxVal * dTacho + iMeterMinDeg;
	if( dTachoNeedle >= 360 ) dTachoNeedle -= 360;
	dTachoNeedle = dTachoNeedle * ToRAD;
	
	DrawLine(
		iMeterCx, iMeterCy,
		( int )( cos( dTachoNeedle ) * iMeterR * 0.95 + .5 ) + iMeterCx,
		( int )( sin( dTachoNeedle ) * iMeterR * 0.95 + .5 ) + iMeterCy,
		LINE_WIDTH, COLOR_NEEDLE, 0
	);
	
	DrawCircle( iMeterCx, iMeterCy,  iMeterR / 25, COLOR_NEEDLE, CVsdFilter::IMG_FILL );
	
	return TRUE;
}
