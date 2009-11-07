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
#include "CVsdLog.h"
#include "CVsdFilter.h"

/*** macros *****************************************************************/

#define LAT_M_DEG		110949.769	// 35-36N   の距離 @ 135E
#define LNG_M_DEG		111441.812	// 135-136E の距離 @ 35N / cos(35度)
#define INVALID_POS_I	0x7FFFFFFF
#define MAX_POLY_HEIGHT	2000		// polygon 用ライン数

#define MAX_LINE_LEN	2000

#define VideoSt			( m_piParamT[ TRACK_VSt ] * 100 + m_piParamT[ TRACK_VSt2 ] )
#define VideoEd			( m_piParamT[ TRACK_VEd ] * 100 + m_piParamT[ TRACK_VEd2 ] )
#ifdef GPS_ONLY
	#define LogSt		0
	#define LogEd		0
#else
	#define LogSt		( m_piParamT[ TRACK_LSt ] * 100 + m_piParamT[ TRACK_LSt2 ] )
	#define LogEd		( m_piParamT[ TRACK_LEd ] * 100 + m_piParamT[ TRACK_LEd2 ] )
#endif
#define GPSSt			( m_piParamT[ TRACK_GSt ] * 100 + m_piParamT[ TRACK_GSt2 ] )
#define GPSEd			( m_piParamT[ TRACK_GEd ] * 100 + m_piParamT[ TRACK_GEd2 ] )
#define LineTrace		m_piParamT[ TRACK_LineTrace ]

#define DispLap			m_piParamC[ CHECK_LAP ]
#define GSnakeLen		m_piParamT[ TRACK_G_Len ]
#define GScale			( m_piParamS[ SHADOW_G_SCALE ] / 1000.0 )

#ifdef GPS_ONLY
	#define Aspect			m_piParamT[ TRACK_Aspect ]
	#define AspectRatio		(( double )m_piParamT[ TRACK_Aspect ] / 1000 )
#else
	#define Aspect			1000
	#define AspectRatio		1
#endif

#ifdef GPS_ONLY
	#define GPSPriority		FALSE
#else
	#define GPSPriority		m_piParamC[ CHECK_GPS_PRIO ]
#endif

#define MAX_MAP_SIZE	( GetWidth() * m_piParamT[ TRACK_MapSize ] / 1000.0 )

// VSD log を優先，ただしチェックボックスでオーバーライドできる
#define SelectLogVsd ( Log = ( GPSPriority || !m_VsdLog ) ? m_GPSLog : m_VsdLog )

// GPS log を優先
#define SelectLogGPS ( Log = m_GPSLog ? m_GPSLog : m_VsdLog )

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
	
	m_VsdLog 			= NULL;
	m_GPSLog 			= NULL;
	
	m_Lap	 			= new LAP_t[ MAX_LAP ];
	m_iLapNum			= 0;
	m_Lap[ 0 ].iLogNum	= 0x7FFFFFFF;	// 番犬
	m_Lap[ 0 ].iTime	= 0;			// 番犬
	
	m_iBestTime			= BESTLAP_NONE;
	m_iBestLap			= 0;
	
	m_dVideoFPS			= 30.0;
	
	m_iPreW				= 0;
	
	m_iLapIdx			= -1;
	m_iBestLogNumRunning		= 0;
	
	m_bCalcLapTimeReq	= FALSE;
	
	m_szLogFile			= NULL;
	m_szGPSLogFile		= NULL;
	
	// DrawPolygon 用バッファ
	m_Polygon = new PolygonData_t[ MAX_POLY_HEIGHT ];
}

/*** デストラクタ ***********************************************************/

CVsdFilter::~CVsdFilter () {
	delete m_VsdLog; m_VsdLog = NULL;
	delete m_GPSLog; m_GPSLog = NULL;
	
	delete m_Lap;
	m_Lap		= NULL;
	m_iLapNum	= 0;
	
	delete [] m_szLogFile;
	delete [] m_szGPSLogFile;
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
	
	int i;
	
	int	iXdiff = ABS( x1 - x2 );
	int iYdiff = ABS( y1 - y2 );
	
	int iXsign = ( x2 > x1 ) ? 1 : -1;
	int iYsign = ( y2 > y1 ) ? 1 : -1;
	
	/* 傾きが1より小さい場合 */
	if( iXdiff > iYdiff ) {
		int E = -iXdiff;
		for( i = 0 ; i <= ( iXdiff + 1 ) >> 1 ; i++ ) {
			PutPixel( x1, y1, yc, uFlag );
			PutPixel( x2, y2, yc, uFlag );
			x1 += iXsign;
			x2 -= iXsign;
			E += 2 * iYdiff;
			if( E >= 0 ) {
				y1 += iYsign;
				y2 -= iYsign;
				E -= 2 * iXdiff;
			}
		}
		/* iXdiff + 1 が奇数の場合、残った中央の点を最後に描画 */
		if(( iXdiff % 2 ) == 0 ) PutPixel( x1, y1, yc, uFlag );
	/* 傾きが1以上の場合 */
	} else {
		int E = -iYdiff;
		for( i = 0 ; i <= ( iYdiff + 1 ) >> 1 ; i++ ) {
			PutPixel( x1, y1, yc, uFlag );
			PutPixel( x2, y2, yc, uFlag );
			y1 += iYsign;
			y2 -= iYsign;
			E += 2 * iXdiff;
			if( E >= 0 ) {
				x1 += iXsign;
				x2 -= iXsign;
				E -= 2 * iYdiff;
			}
		}
		/* iYdiff + 1 が奇数の場合、残った中央の点を最後に描画 */
		if(( iYdiff % 2 ) == 0 ) PutPixel( x1, y1, yc, uFlag );
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
	
	int	i = r;
	int j = 0;
	int f = -2 * r + 3;
	
	// Polygon クリア
	if( uFlag & IMG_FILL ){
		PolygonClear();
		uFlag |= IMG_POLYGON;
	}
	
	// 円を書く
	while( i >= j ){
		PutPixel( x + i, y + j, yc, uFlag ); PutPixel( x + i, y - j, yc, uFlag );
		PutPixel( x - i, y + j, yc, uFlag ); PutPixel( x - i, y - j, yc, uFlag );
		PutPixel( x + j, y + i, yc, uFlag ); PutPixel( x - j, y + i, yc, uFlag );
		PutPixel( x + j, y - i, yc, uFlag ); PutPixel( x - j, y - i, yc, uFlag );
		
		if( f >= 0 ){
			--i;
			f -= 4 * i;
		}
		++j;
		f += 4 * j + 2;
	}
	
	// Polygon 合成
	if( uFlag & IMG_FILL ) PolygonDraw( yc, uFlag );
}

void CVsdFilter::DrawCircle( int x, int y, int r, int a, int b, const PIXEL_YC &yc, UINT uFlag ){
	int		i = ( int )(( double )r / sqrt(( double )a ));
	int		j = 0;
	double	d = sqrt(( double )a ) * ( double )r;
	int		f = ( int )( -2.0 * d ) + a + 2 * b;
	int		h = ( int )( -4.0 * d ) + 2 * a + b;
	
	// Polygon クリア
	if( uFlag & IMG_FILL ){
		PolygonClear();
		uFlag |= IMG_POLYGON;
	}
	
	while( i >= 0 ){
		PutPixel( x + i, y + j, yc, uFlag );
		PutPixel( x - i, y + j, yc, uFlag );
		PutPixel( x + i, y - j, yc, uFlag );
		PutPixel( x - i, y - j, yc, uFlag );
		
		if( f >= 0 ){
			--i;
			f -= 4 * a * i;
			h -= 4 * a * i - 2 * a;
		}
		if( h < 0 ){
			++j;
			f += 4 * b * j + 2 * b;
			h += 4 * b * j;
		}
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
						
						if( i <= TRACK_GEd ){
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
		"DirectShowSource( \"%s\", pixel_type=\"YUY2\", convertfps=true  )\n"
		"VSDFilter( \\\n"
	#ifndef GPS_ONLY
		"\tlog_file=\"%s\", \\\n"
	#endif
		"\tgps_file=\"%s\""
		,
		GetVideoFileName( szBuf ),
	#ifndef GPS_ONLY
		m_szLogFile ? m_szLogFile : "",
	#endif
		m_szGPSLogFile ? m_szGPSLogFile : ""
	);
	
	for( i = 0; i < TRACK_N; ++i ){
		if( m_szTrackbarName[ i ] == NULL ) continue;
		
		fprintf( fp, ", \\\n\t%s=%d", m_szTrackbarName[ i ],
			( i <= TRACK_GEd ) ? m_piParamT[ i ] * 100 + m_piParamT[ i + 1 ] :
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

/*** GPS ログリード ********************************************************/

BOOL CVsdFilter::GPSLogLoad( const char *szFileName ){
	
	UINT	uGPSCnt = 0;
	TCHAR	szBuf[ BUF_SIZE ];
	
	double	dLati, dLati0 = 0;
	double	dLong, dLong0 = 0;
	double	dSpeed;
	double	dBearing;
	double	dTime, dTime0;
	gzFile	fp;
	
	UINT	u;
	UINT	uSameCnt = 0;
	
	if( m_GPSLog ){
		delete m_GPSLog;
		m_GPSLog = NULL;
	}
	
	if(( fp = gzopen(( char *)szFileName, "rb" )) == NULL ) return FALSE;
	
#ifndef AVS_PLUGIN
	if( m_szGPSLogFile ) delete [] m_szGPSLogFile;
	m_szGPSLogFile = new char[ strlen( szFileName ) + 1 ];
	strcpy( m_szGPSLogFile, szFileName );
#endif
	
	GPS_LOG_t	*GPSLog = new GPS_LOG_t[ ( int )( MAX_VSD_LOG * GPS_FREQ / LOG_FREQ ) ];
	
	/*** dp3 ****************************************************************/
	
	if( IsExt(( char *)szFileName, "dp3" )){
		
		gzseek( fp, 0x100, SEEK_CUR );
		
		#define BigEndianI( p )	( \
			( *(( UCHAR *)szBuf + p + 0 ) << 24 ) | \
			( *(( UCHAR *)szBuf + p + 1 ) << 16 ) | \
			( *(( UCHAR *)szBuf + p + 2 ) <<  8 ) | \
			( *(( UCHAR *)szBuf + p + 3 )       ))
		
		#define BigEndianS( p )	( \
			( *(( UCHAR *)szBuf + p + 0 ) <<  8 ) | \
			( *(( UCHAR *)szBuf + p + 1 )       ))
		
		while( gzread( fp, szBuf, 16 )){
			
			u = BigEndianI( 0 );
			// 値補正
			// 2254460 → 22:54:46.0
			dTime =	u / 100000 * 3600 +
					u / 1000 % 100 * 60 +
					( u % 1000 ) / 10.0;
			
			dLati = BigEndianI( 8 ) / 460800.0;
			dLong = BigEndianI( 4 ) / 460800.0;
			
			if( dLati0 == 0 ){
				dLati0 = dLati;
				dLong0 = dLong;
				dTime0 = dTime;
			}
			
			if( dTime < dTime0 ) dTime += 24 * 3600;
			dTime -= dTime0;
			
			// 単位を補正
			// 緯度・経度→メートル
			GPSLog[ uGPSCnt ].fX = ( float )(( dLong - dLong0 ) * LAT_M_DEG * cos( dLati * ToRAD ));
			GPSLog[ uGPSCnt ].fY = ( float )(( dLati0 - dLati ) * LNG_M_DEG );
			
			// 速度・向き→ベクトル座標
			GPSLog[ uGPSCnt ].fSpeed	= ( float )( BigEndianS( 12 ) / 10.0 );
			GPSLog[ uGPSCnt ].fBearing	= ( float )BigEndianS( 14 );
			GPSLog[ uGPSCnt ].fTime 	= ( float )dTime;
			
			if( uGPSCnt >=2 ){
				if( GPSLog[ uGPSCnt - 1 ].fTime == GPSLog[ uGPSCnt ].fTime ){
					// 時刻が同じログが続くときそのカウントをする
					++uSameCnt;
				}else if( uSameCnt ){
					// 時刻が同じログが途切れたので，時間を補正する
					++uSameCnt;
					
					for( u = 1; u < uSameCnt; ++ u ){
						GPSLog[ uGPSCnt - uSameCnt + u ].fTime +=
							( GPSLog[ uGPSCnt ].fTime - GPSLog[ uGPSCnt - uSameCnt ].fTime )
							/ uSameCnt * u;
					}
					uSameCnt = 0;
				}
			}
			uGPSCnt++;
		}
	}
	
	/*** nmea ***************************************************************/
	
	else while( gzgets( fp, szBuf, BUF_SIZE ) != Z_NULL ){
		
		u = sscanf( szBuf,
			"$GPRMC,%lg%*[^0-9]%lg%*[^0-9]%lg%*[^0-9]%lg%*[^0-9]%lg",
			&dTime, &dLati, &dLong, &dSpeed, &dBearing
		);
		
		// $GPRMC センテンス以外はスキップ
		if( u < 5 ) continue;
		
		// 値補正
		// 225446.00 → 22:54:46.00
		dTime =	( int )dTime / 10000 * 3600 +
				( int )dTime / 100 % 100 * 60 +
				fmod( dTime, 100 );
		
		// 4916.452653 → 49度16.45分
		dLati =	( int )dLati / 100 + fmod( dLati, 100 ) / 60;
		dLong =	( int )dLong / 100 + fmod( dLong, 100 ) / 60;
			
		if( dLati0 == 0 ){
			dLati0 = dLati;
			dLong0 = dLong;
			dTime0 = dTime;
		}
		
		if( dTime < dTime0 ) dTime += 24 * 3600;
		dTime -= dTime0;
		
		// 単位を補正
		// 緯度・経度→メートル
		GPSLog[ uGPSCnt ].fX = ( float )(( dLong - dLong0 ) * LAT_M_DEG * cos( dLati * ToRAD ));
		GPSLog[ uGPSCnt ].fY = ( float )(( dLati0 - dLati ) * LNG_M_DEG );
		
		// 速度・向き→ベクトル座標
		dSpeed	 *= 1.852;	// knot/h → km/h
		GPSLog[ uGPSCnt ].fSpeed	= ( float )dSpeed;
		GPSLog[ uGPSCnt ].fBearing	= ( float )dBearing;
		GPSLog[ uGPSCnt ].fTime 	= ( float )dTime;
		
		if( uGPSCnt >=2 ){
			if( GPSLog[ uGPSCnt - 1 ].fTime == GPSLog[ uGPSCnt ].fTime ){
				// 時刻が同じログが続くときそのカウントをする
				++uSameCnt;
			}else if( uSameCnt ){
				// 時刻が同じログが途切れたので，時間を補正する
				++uSameCnt;
				
				for( u = 1; u < uSameCnt; ++ u ){
					GPSLog[ uGPSCnt - uSameCnt + u ].fTime +=
						( GPSLog[ uGPSCnt ].fTime - GPSLog[ uGPSCnt - uSameCnt ].fTime )
						/ uSameCnt * u;
				}
				uSameCnt = 0;
			}
		}
		uGPSCnt++;
	}
	
	gzclose( fp );
	
	DebugCmd( {
		FILE *fpp = fopen( "G:\\DDS\\vsd\\vsd_filter\\z_gps_raw.txt", "w" );
		for( u = 0; u < uGPSCnt; ++u ){
			fprintf( fpp, "%g\t%g\t%g\t%g\t%g\n",
				GPSLog[ u ].fX,
				GPSLog[ u ].fY,
				GPSLog[ u ].fSpeed,
				GPSLog[ u ].fBearing,
				GPSLog[ u ].fTime
			);
		}
		fclose( fpp );
	} )
	
	// アップコンバート用バッファ確保・初期化
	m_GPSLog = new CVsdLog;
	m_GPSLog->GPSLogUpConvert( GPSLog, uGPSCnt, TRUE );
	m_GPSLog->RotateMap( m_piParamT[ TRACK_MapAngle ] * ( -ToRAD / 10 ));
	
	delete [] GPSLog;
	
	DebugCmd( {
		FILE *fpp = fopen( "G:\\DDS\\vsd\\vsd_filter\\z_upcon_gps.txt", "w" );
		for( u = 0; u < ( UINT )m_GPSLog->m_iCnt; ++u ){
			fprintf( fpp, "%g\t%g\t%g\t%g\t%g\t%g\t%g\t%g\t%g\n",
				m_GPSLog->m_Log[ u ].fSpeed,
				m_GPSLog->m_Log[ u ].fTacho,
				m_GPSLog->m_Log[ u ].fMileage,
				m_GPSLog->m_Log[ u ].fX,
				m_GPSLog->m_Log[ u ].fX0,
				m_GPSLog->m_Log[ u ].fY,
				m_GPSLog->m_Log[ u ].fY0,
				m_GPSLog->m_Log[ u ].fGx,
				m_GPSLog->m_Log[ u ].fGy
			);
		}
		fclose( fpp );
	} )
	
	DeleteIfZero( m_GPSLog );
	return m_GPSLog != NULL;
}

/*** ログリード *************************************************************/

BOOL CVsdFilter::ReadLog( const char *szFileName ){
	
	TCHAR	szBuf[ BUF_SIZE ];
	gzFile	fp;
	BOOL	bCalibrating = FALSE;
	
	if( m_VsdLog ){
		delete m_VsdLog;
		m_VsdLog = NULL;
	}
	
	if(( fp = gzopen(( char *)szFileName, "rb" )) == NULL ) return FALSE;
	
#ifndef AVS_PLUGIN
	if( m_szLogFile ) delete [] m_szLogFile;
	m_szLogFile = new char[ strlen( szFileName ) + 1 ];
	strcpy( m_szLogFile, szFileName );
#endif
	
	// GPS ログ用
	UINT		uGPSCnt = 0;
	GPS_LOG_t	*GPSLog = new GPS_LOG_t[ ( int )( MAX_VSD_LOG / LOG_FREQ ) ];
	
	// 初期化
	m_iLapNum		= 0;
	m_iBestTime		= BESTLAP_NONE;
	m_iBestLap		= 0;
	m_iLapIdx		= -1;
	m_iBestLogNumRunning	= 0;
	
	m_VsdLog = new CVsdLog;
	
	// ログリード
	
	UINT	uReadCnt, uLap, uMin, uSec, uMSec;
	UINT	uLogNum = 0;
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
			uReadCnt = sscanf( p, "LAP%d%d:%d.%d", &uLap, &uMin, &uSec, &uMSec );
			
			int iTime = ( uMin * 60 + uSec ) * 1000 + uMSec;
			
			m_Lap[ m_iLapNum ].uLap		= uLap;
			m_Lap[ m_iLapNum ].iLogNum	= uLogNum;
			m_Lap[ m_iLapNum ].iTime	= ( uReadCnt == 4 ) ? iTime : 0;
			
			if(
				uReadCnt == 4 &&
				( m_iBestTime == BESTLAP_NONE || m_iBestTime > iTime )
			){
				m_iBestTime	= iTime;
				m_iBestLap	= m_iLapNum - 1;
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
			GPSLog[ uGPSCnt ].fSpeed	= ( float )dSpeed;
			GPSLog[ uGPSCnt ].fBearing	= ( float )dBearing;
			
			// LOG_FREQ は 15Hz だった時代のログでは間違いだが，等比分割だし実害なし?
			GPSLog[ uGPSCnt++ ].fTime = ( uLogNum - GPS_LOG_OFFS ) / ( float )LOG_FREQ;
		}
		
		// 普通の log
		if(( uReadCnt = sscanf( szBuf, "%g%g%g%lg%lg",
			&m_VsdLog->m_Log[ uLogNum ].fTacho,
			&m_VsdLog->m_Log[ uLogNum ].fSpeed,
			&m_VsdLog->m_Log[ uLogNum ].fMileage,
			&dGy,
			&dGx
		)) >= 3 ){
			if( uReadCnt < 5 && uLogNum ){
				// Gデータがないときは，speedから求める←廃止
				dGx = 0;
				dGy = 0;
				//dGy = ( m_VsdLog->m_Log[ uLogNum ].fSpeed - m_VsdLog->m_Log[ uLogNum - 1 ].fSpeed ) * ( 1000.0 / 3600 / 9.8 * LOG_FREQ );
			}else{
				if( dGx >= 4 ){	
					// 単位を G に変換
					dGx = -dGx / ACC_1G_Y;
					dGy =  dGy / ACC_1G_Z;
				}
				
				if( uLogNum == 0 ){
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
					uLogNum &&
					m_VsdLog->m_Log[ uLogNum - 1 ].fMileage == m_VsdLog->m_Log[ uLogNum ].fMileage &&
					( m_VsdLog->m_Log[ uLogNum - 1 ].fGy - dGy ) >= -0.02 &&
					( m_VsdLog->m_Log[ uLogNum - 1 ].fGy - dGy ) <=  0.02
				){
					dGcx += dGx / 160;
					dGcy += dGy / 160;
				}
			}
			
			m_VsdLog->m_Log[ uLogNum ].fGx = ( float )dGx;
			m_VsdLog->m_Log[ uLogNum ].fGy = ( float )dGy;
			
			// ログ開始・終了認識
			if( m_VsdLog->m_Log[ uLogNum ].fSpeed >= 300 ){
				if( !bCalibrating ){
					bCalibrating = TRUE;
					m_iLogStart  = m_iLogStop;
					m_iLogStop   = uLogNum;
				}
			}else{
				bCalibrating = FALSE;
			}
			
			// メーター補正
			m_VsdLog->m_Log[ uLogNum ].fTacho = ( float )(
				m_VsdLog->m_Log[ uLogNum ].fTacho * m_piParamS[ METER_ADJUST ] / 1000.0
			);
			m_VsdLog->m_Log[ uLogNum ].fSpeed = ( float )(
				m_VsdLog->m_Log[ uLogNum ].fSpeed * m_piParamS[ METER_ADJUST ] / 1000.0
			);
			++uLogNum;
		}
	}
	m_VsdLog->m_iCnt = uLogNum;
	
	/*** GPS ログから軌跡を求める *******************************************/
	
	if( uGPSCnt ){
		m_VsdLog->GPSLogUpConvert( GPSLog, uGPSCnt );
		m_VsdLog->RotateMap( m_piParamT[ TRACK_MapAngle ] * ( -ToRAD / 10 ));
	}
	
	delete [] GPSLog;
	
	/************************************************************************/
	
	gzclose( fp );
	
	m_Lap[ m_iLapNum ].iLogNum	= 0x7FFFFFFF;	// 番犬
	m_Lap[ m_iLapNum ].iTime	= 0;			// 番犬
	
	DeleteIfZero( m_VsdLog );
	
	return TRUE;
}

/*** ラップタイムログ → ログ番号 変換 **************************************/

double CVsdFilter::LapNum2LogNum( CVsdLog *Log, int iLapNum ){
	
	// iLapNum がおかしいときは 0 を返しとく
	if( iLapNum < 0 ) return 0;
	
	// 自動ラップ計測なら，Lap 構造体のログ番号は信用できる
	if( Log == m_VsdLog ) return m_Lap[ iLapNum ].iLogNum;
	
	// GPS のログ番号に要変換
	if( m_VsdLog ){
		// 自動計測
		return
			LogEd == LogSt ? 0 :
			( double )( m_Lap[ iLapNum ].iLogNum - LogSt ) / ( LogEd - LogSt )
			* ( GPSEd - GPSSt ) + GPSSt;
	}
	
	// 手動計測
	return
		VideoEd == VideoSt ? 0 :
		( double )( m_Lap[ iLapNum ].iLogNum - VideoSt ) / ( VideoEd - VideoSt )
		* ( GPSEd - GPSSt ) + GPSSt;
}

/*** パラメータ調整用スピードグラフ *****************************************/

void CVsdFilter::DrawSpeedGraph( CVsdLog *Log, const PIXEL_YC &yc ){
	
	int	iLogNum;
	int	x = 0;
	
	iLogNum = Log->m_iLogNum - GetWidth() / 2;
	if( iLogNum < 0 ){
		x = -iLogNum;
		iLogNum = 0;
	}
	
	for( ; x < GetWidth() - 1 && iLogNum < Log->m_iCnt - 1; ++x, ++iLogNum ){
		DrawLine(
			x,     GetHeight() - 1 - ( int )Log->Speed( iLogNum ),
			x + 1, GetHeight() - 1 - ( int )Log->Speed( iLogNum + 1 ),
			1, yc, 0
		);
	}
}

/****************************************************************************/
/*** メーター描画 ***********************************************************/
/****************************************************************************/

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
#define COLOR_G_SCALE		yc_black

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
	
	const int	iMeterR =
		m_piParamS[ SHADOW_METER_R  ] >= 0 ? m_piParamS[ SHADOW_METER_R  ] :
		50 * GetWidth() / 320;
	
	int	iMeterCx;
	if( m_piParamS[ SHADOW_METER_CX ] >= 0 ){
		iMeterCx = m_piParamS[ SHADOW_METER_CX ];
	}else if(
		#ifdef GPS_ONLY
			m_piParamC[ CHECK_METER_POS ]
		#else
			!m_piParamC[ CHECK_METER_POS ]
		#endif
	){
		iMeterCx = GetWidth() - iMeterR * Aspect / 1000 - 2;
	}else{
		iMeterCx = iMeterR * Aspect / 1000 + 1;
	}
	
	const int	iMeterCy =
		m_piParamS[ SHADOW_METER_CY ] >= 0 ? m_piParamS[ SHADOW_METER_CY ] :
		GetHeight() - iMeterR - 2;
	
	const int	iMeterMinDeg	= 135;
	const int	iMeterMaxDeg	= 45;
	const int	iMeterMaxVal	= 7000;
	const int	iMeterDegRange	= ( iMeterMaxDeg + 360 - iMeterMinDeg ) % 360;
	const int	iMeterScaleLen	= iMeterR / 8;
	const int	iMeterSMaxVal	= m_piParamT[ TRACK_SPEED ];
	
	// フォントサイズ初期化
	InitFont();
	
	CVsdLog *Log = m_VsdLog;
	
	// ログ位置の計算
	if( m_VsdLog ){
		m_VsdLog->m_dLogNum = ( VideoEd == VideoSt ) ? -1 :
			( double )( LogEd - LogSt ) / ( VideoEd - VideoSt ) * ( GetFrameCnt() - VideoSt ) + LogSt;
		m_VsdLog->m_iLogNum = ( int )m_VsdLog->m_dLogNum;
		
		// 古い log は LOG_FREQ とは限らないので，計算で求める
		m_VsdLog->m_dFreq =
			( double )(( m_Lap[ m_iLapIdx + 1 ].iLogNum - m_Lap[ m_iLapIdx ].iLogNum ) * 1000 ) /
			m_Lap[ m_iLapIdx + 1 ].iTime;
	}
	if( m_GPSLog ){
		m_GPSLog->m_dLogNum = ( VideoEd == VideoSt ) ? -1 :
			( double )( GPSEd - GPSSt ) / ( VideoEd - VideoSt ) * ( GetFrameCnt() - VideoSt ) + GPSSt;
		m_GPSLog->m_iLogNum = ( int )m_GPSLog->m_dLogNum;
		
		// GPS log は LOG_FREQ でコンバートしている
		m_GPSLog->m_dFreq = LOG_FREQ;
	}
	
	/*** Lap タイム描画 ***/
	
	if( IsHandLaptime() && DispLap && m_bCalcLapTimeReq && m_Lap ){
		m_bCalcLapTimeReq = FALSE;
		CalcLapTime();
	}
	
	// ラップインデックスを求める
	if( m_iLapNum ){
		int iLogNum = IsHandLaptime() ?
			GetFrameCnt() :		// 手動計測での m_Lap[].iLogNum はフレーム# なので
			Log->m_iLogNum;
		
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
				iTime = ( int )(( Log->m_dLogNum - m_Lap[ m_iLapIdx ].iLogNum ) * 1000 / m_VsdLog->m_dFreq );
			}
			
			sprintf( szBuf, "Time%2d'%02d.%03d", iTime / 60000, iTime / 1000 % 60, iTime % 1000 );
			DrawString( szBuf, COLOR_TIME, COLOR_TIME_EDGE, 0, GetWidth() - GetFontW() * 13, 1 );
			bInLap = TRUE;
		}else{
			// まだ開始していない
			DrawString( "Time -'--.---", COLOR_TIME, COLOR_TIME_EDGE, 0, GetWidth() - GetFontW() * 13, 1 );
		}
		
		/*** ベストとの車間距離表示 - ***/
		if( m_VsdLog || m_GPSLog ){
			if( bInLap ){
				
				SelectLogGPS;
				
				// ベストラップ開始の LogNum
				double dBestLapLogNumStart = LapNum2LogNum( Log, m_iBestLap );
				
				// この周の走行距離を求める
				double dMileage = Log->Mileage() - Log->Mileage( LapNum2LogNum( Log, m_iLapIdx ));
				
				// 最速 Lap の，同一走行距離におけるタイム (=ログ番号,整数) を求める
				// m_iBestLogNumRunning <= 最終的に求める結果 < m_iBestLogNumRunning + 1  となる
				// m_iBestLogNumRunning がおかしかったら，リセット
				if(
					m_iBestLogNumRunning < dBestLapLogNumStart ||
					m_iBestLogNumRunning >= Log->m_iCnt ||
					( Log->Mileage( m_iBestLogNumRunning ) - Log->Mileage( dBestLapLogNumStart )) > dMileage
				) m_iBestLogNumRunning = ( int )dBestLapLogNumStart;
				
				for(
					;
					( Log->Mileage( m_iBestLogNumRunning + 1 ) - Log->Mileage( dBestLapLogNumStart )) <= dMileage &&
					m_iBestLogNumRunning < Log->m_iCnt;
					++m_iBestLogNumRunning
				);
				
				// 最速 Lap の，1/15秒以下の値を求める = A / B
				double dBestLapLogNumRunning =
					( double )m_iBestLogNumRunning +
					// A: 最速ラップは，後これだけ走らないと dMileage と同じではない
					( dMileage - ( Log->Mileage( m_iBestLogNumRunning ) - Log->Mileage( dBestLapLogNumStart ))) /
					// B: 最速ラップは，1/15秒の間にこの距離を走った
					( Log->Mileage( m_iBestLogNumRunning + 1 ) - Log->Mileage( m_iBestLogNumRunning ));
				
				int iDiffTime = ( int )(
					(
						( Log->m_dLogNum - LapNum2LogNum( Log, m_iLapIdx )) -
						( dBestLapLogNumRunning - dBestLapLogNumStart )
					) * 1000.0 / Log->m_dFreq
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
	
	// フレーム表示
	
	#define Float2Time( n )	( int )( n ) / 60, fmod( n, 60 )
	
	if( m_piParamC[ CHECK_FRAME ] ){
		sprintf(
			szBuf, "Vid%5d/%5d %2d:%05.2f-%2d:%05.2f(%2d:%05.2f)",
			GetFrameCnt(), GetFrameMax() - 1,
			Float2Time( VideoSt / m_dVideoFPS ),
			Float2Time( VideoEd / m_dVideoFPS ),
			Float2Time(( VideoEd - VideoSt ) / m_dVideoFPS )
		);
		DrawString( szBuf, COLOR_STR, COLOR_TIME_EDGE, 0, 0, GetHeight() / 3 );
		
		if( m_VsdLog ){
			sprintf(
			szBuf, "Log%5d/%5d %2d:%05.2f-%2d:%05.2f(%2d:%05.2f)",
				( int )m_VsdLog->m_dLogNum, m_VsdLog->m_iCnt - 1,
				Float2Time( LogSt / m_VsdLog->m_dFreq ),
				Float2Time( LogEd / m_VsdLog->m_dFreq ),
				Float2Time(( LogEd - LogSt ) / m_VsdLog->m_dFreq )
			);
			DrawString( szBuf, COLOR_STR, COLOR_TIME_EDGE, 0 );
			DrawSpeedGraph( m_VsdLog, yc_red );
		}
		
		if( m_GPSLog ){
			sprintf( szBuf,
				"GPS%5d/%5d %2d:%05.2f-%2d:%05.2f(%2d:%05.2f)",
				( int )m_GPSLog->m_dLogNum, m_GPSLog->m_iCnt - 1,
				Float2Time( GPSSt / m_GPSLog->m_dFreq ),
				Float2Time( GPSEd / m_GPSLog->m_dFreq ),
				Float2Time(( GPSEd - GPSSt ) / m_GPSLog->m_dFreq )
			);
			DrawString( szBuf, COLOR_STR, COLOR_TIME_EDGE, 0 );
			DrawSpeedGraph( m_GPSLog, yc_cyan );
		}
		
		DrawLine(
			GetWidth() / 2, GetHeight() - 20,
			GetWidth() / 2, GetHeight() - 1,
			1, yc_cyan, 0
		);
	}
	
	if( !m_VsdLog && !m_GPSLog ) return TRUE;
	
	/*** メーターパネル ***/
	#ifdef GPS_ONLY
		DrawCircle(
			iMeterCx, iMeterCy, iMeterR * 1000,
			( 1000 * 1000 / Aspect ) * ( 1000 * 1000 / Aspect ),
			1000 * 1000,
			COLOR_PANEL, CVsdFilter::IMG_ALFA | CVsdFilter::IMG_FILL
		);
	#else
		DrawCircle(
			iMeterCx, iMeterCy, iMeterR,
			COLOR_PANEL, CVsdFilter::IMG_ALFA | CVsdFilter::IMG_FILL
		);
	#endif
	
	/*
	DrawCircle(
		iMeterCx, iMeterCy, iMeterR / 3, COLOR_G_SCALE,
		CVsdFilter::IMG_ALFA
	);
	DrawCircle(
		iMeterCx, iMeterCy, iMeterR * 2 / 3, COLOR_G_SCALE,
		CVsdFilter::IMG_ALFA
	);
	DrawLine(
		iMeterCx - iMeterR, iMeterCy,
		iMeterCx + iMeterR, iMeterCy,
		1, COLOR_G_SCALE, CVsdFilter::IMG_ALFA
	);
	DrawLine(
		iMeterCx, iMeterCy - iMeterR,
		iMeterCx, iMeterCy + iMeterR,
		1, COLOR_G_SCALE, CVsdFilter::IMG_ALFA
	);
	*/
	
	SelectLogVsd;
	
	if( Log == m_VsdLog ){
		// VSD ログがあるときはタコメータ
		for( i = 0; i <= iMeterMaxVal; i += 500 ){
			int iDeg = iMeterDegRange * i / iMeterMaxVal + iMeterMinDeg;
			
			// メーターパネル目盛り
			if( iMeterMaxVal <= 12000 || i % 1000 == 0 ){
				DrawLine(
					( int )( cos( iDeg * ToRAD ) * iMeterR * AspectRatio ) + iMeterCx,
					( int )( sin( iDeg * ToRAD ) * iMeterR ) + iMeterCy,
					( int )( cos( iDeg * ToRAD ) * ( iMeterR - iMeterScaleLen ) * AspectRatio ) + iMeterCx,
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
						( int )( cos( iDeg * ToRAD ) * iMeterR * .8 * AspectRatio ) + iMeterCx - GetFontW() / ( i >= 10000 ? 1 : 2 ),
						( int )( sin( iDeg * ToRAD ) * iMeterR * .8 ) + iMeterCy - GetFontH() / 2
					);
				}
			}
		}
	}else{
		// GPS ログ優先時はスピードメーターパネル
		int	iStep = (( iMeterSMaxVal / 20 ) + 4 ) / 5 * 5;
		
		for( i = 0; i <= iMeterSMaxVal; i += iStep ){
			int iDeg = iMeterDegRange * i / iMeterSMaxVal + iMeterMinDeg;
			
			// メーターパネル目盛り
			if( i % iStep == 0 ){
				DrawLine(
					( int )( cos( iDeg * ToRAD ) * iMeterR * AspectRatio ) + iMeterCx,
					( int )( sin( iDeg * ToRAD ) * iMeterR ) + iMeterCy,
					( int )( cos( iDeg * ToRAD ) * ( iMeterR - iMeterScaleLen ) * AspectRatio ) + iMeterCx,
					( int )( sin( iDeg * ToRAD ) * ( iMeterR - iMeterScaleLen )) + iMeterCy,
					( i % ( iStep * 2 ) == 0 ) ? 2 : 1,
					COLOR_SCALE, 0
				);
				
				// メーターパネル目盛り数値
				if( i % ( iStep * 2 ) == 0 ){
					sprintf( szBuf, "%d", i );
					DrawString(
						szBuf,
						COLOR_STR, 0,
						( int )( cos( iDeg * ToRAD ) * iMeterR * .75 * AspectRatio ) + iMeterCx - GetFontW() * strlen( szBuf ) / 2,
						( int )( sin( iDeg * ToRAD ) * iMeterR * .75 ) + iMeterCy - GetFontH() / 2
					);
				}
			}
		}
	}
	
	/*** メーターデータ描画 ***/
	
	// G スネーク
	int	iGx, iGy;
	
	if( GSnakeLen >= 0 && Log->IsDataExist()){
		if( GSnakeLen > 0 ){
			
			int iGxPrev = INVALID_POS_I, iGyPrev;
			
			for( i = -GSnakeLen; i <= 1 ; ++i ){
				
				if( Log->m_iLogNum + i >= 0 ){
					// i == 1 時は最後の中途半端な LogNum
					iGx = ( int )((( i != 1 ) ? Log->m_Log[ Log->m_iLogNum + i ].fGx : Log->Gx()) * iMeterR / GScale );
					iGy = ( int )((( i != 1 ) ? Log->m_Log[ Log->m_iLogNum + i ].fGy : Log->Gy()) * iMeterR / GScale );
					
					iGx = ( int )( iGx * AspectRatio );
					
					if( iGxPrev != INVALID_POS_I ) DrawLine(
						iMeterCx + iGx, iMeterCy - iGy, iMeterCx + iGxPrev, iMeterCy - iGyPrev,
						LINE_WIDTH, COLOR_G_HIST, 0
					);
					
					iGxPrev = iGx;
					iGyPrev = iGy;
				}
			}
		}else{
			iGx = ( int )( Log->Gx() * iMeterR / GScale * AspectRatio );
			iGy = ( int )( Log->Gy() * iMeterR / GScale );
		}
		
		// G インジケータ
		DrawCircle(
			iMeterCx + iGx, iMeterCy - iGy, iMeterR / 20,
			COLOR_G_SENSOR, CVsdFilter::IMG_FILL
		);
	}
	
	// MAP 表示
	SelectLogGPS;
	
	if( LineTrace && Log->IsDataExist()){
		double dGx, dGy;
		
		int iGxPrev = INVALID_POS_I, iGyPrev;
		
		int iLineSt = ( int )LapNum2LogNum( Log, m_iLapIdx );
		if( Log->m_iLogNum - iLineSt > ( int )( LineTrace * LOG_FREQ ))
			iLineSt = Log->m_iLogNum - ( int )( LineTrace * LOG_FREQ );
		
		int iLineEd = m_iLapIdx != m_iLapNum - 1
			? ( int )LapNum2LogNum( Log, m_iLapIdx + 1 ) : Log->m_iCnt - 1;
		
		if( iLineEd - Log->m_iLogNum > ( int )( LineTrace * LOG_FREQ ))
			iLineEd = Log->m_iLogNum + ( int )( LineTrace * LOG_FREQ );
		
		for( i = iLineSt; i <= iLineEd ; ++i ){
			#define GetMapPos( p, a ) ((( p ) - Log->m_dMapOffs ## a ) / Log->m_dMapSize * MAX_MAP_SIZE + 8 )
			dGx = GetMapPos( Log->X( i ), X );
			dGy = GetMapPos( Log->Y( i ), Y );
			
			if( !_isnan( dGx )){
				iGx = ( int )dGx;
				iGy = ( int )dGy;
				
				if( iGxPrev != INVALID_POS_I ){
					// Line の色用に G を求める
					double dG = sqrt(
						Log->Gx( i ) * Log->Gx( i ) +
						Log->Gy( i ) * Log->Gy( i )
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
						( int )( iGx     * AspectRatio ), iGy,
						( int )( iGxPrev * AspectRatio ), iGyPrev,
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
		dGx = GetMapPos( Log->X(), X );
		dGy = GetMapPos( Log->Y(), Y );
		
		if( !_isnan( dGx )) DrawCircle(
			( int )( dGx * AspectRatio ), ( int )dGy, iMeterR / 20,
			COLOR_CURRENT_POS, CVsdFilter::IMG_FILL
		);
	}
	
	// ギア表示 - VsdLog しか使用しない
	if( m_VsdLog && m_VsdLog->IsDataExist() ){
		
		UINT uGear = 0;
		
		if( m_VsdLog->Tacho() != 0 ){
			UINT uGearRatio = ( int )( m_VsdLog->Speed() * 100 * ( 1 << 8 ) / m_VsdLog->Tacho() );
			
			if     ( uGearRatio < GEAR_TH( 1 ))	uGear = 1;
			else if( uGearRatio < GEAR_TH( 2 ))	uGear = 2;
			else if( uGearRatio < GEAR_TH( 3 ))	uGear = 3;
			else if( uGearRatio < GEAR_TH( 4 ))	uGear = 4;
			else								uGear = 5;
		}
		
		sprintf( szBuf, "%d\x7F", uGear );
		DrawString(
			szBuf,
			COLOR_STR, 0,
			iMeterCx - 1 * GetFontW(), iMeterCy - iMeterR / 2
		);
	}
	
	SelectLogVsd;
	
	if( Log->IsDataExist()){
		// スピード表示
		sprintf( szBuf, "%3d\x80\x81", ( int )Log->Speed() );
		DrawString(
			szBuf,
			COLOR_STR, 0,
			iMeterCx - 3 * GetFontW(), iMeterCy + iMeterR / 2
		);
		
		// G 数値
		sprintf( szBuf, "%02dG", ( int )( sqrt( Log->Gx() * Log->Gx() + Log->Gy() * Log->Gy()) * 10 ));
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
	}
	
	if( Log == m_VsdLog ){
		if( m_VsdLog->IsDataExist()){
			// Tacho の針 - VsdLog しか使用しない
			double dTachoNeedle = iMeterDegRange / ( double )iMeterMaxVal * m_VsdLog->Tacho() + iMeterMinDeg;
			dTachoNeedle = dTachoNeedle * ToRAD;
			
			DrawLine(
				iMeterCx, iMeterCy,
				( int )( cos( dTachoNeedle ) * iMeterR * 0.95 * AspectRatio + .5 ) + iMeterCx,
				( int )( sin( dTachoNeedle ) * iMeterR * 0.95 + .5 ) + iMeterCy,
				LINE_WIDTH, COLOR_NEEDLE, 0
			);
		}
	}else{
		if( m_GPSLog->IsDataExist()){
			// Speed の針
			double dSpeedNeedle =
				iMeterDegRange / ( double )iMeterSMaxVal * m_GPSLog->Speed() + iMeterMinDeg;
			
			dSpeedNeedle = dSpeedNeedle * ToRAD;
			
			DrawLine(
				iMeterCx, iMeterCy,
				( int )( cos( dSpeedNeedle ) * iMeterR * 0.95 * AspectRatio + .5 ) + iMeterCx,
				( int )( sin( dSpeedNeedle ) * iMeterR * 0.95 + .5 ) + iMeterCy,
				LINE_WIDTH, COLOR_NEEDLE, 0
			);
		}
	}
	
	DrawCircle( iMeterCx, iMeterCy,  iMeterR / 25, COLOR_NEEDLE, CVsdFilter::IMG_FILL );
	
	return TRUE;
}
