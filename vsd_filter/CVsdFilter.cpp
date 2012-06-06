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
#include "CVsdFont.h"
#include "CVsdFilter.h"

/*** macros *****************************************************************/

#define INVALID_POS_I	0x7FFFFFFF
#define MAX_POLY_HEIGHT	2000		// polygon 用ライン数

#define MAX_LINE_LEN	2000

#define LineTrace		m_piParamT[ TRACK_LineTrace ]

#define DispLap			m_piParamC[ CHECK_LAP ]
#define GSnakeLen		m_piParamT[ TRACK_G_Len ]
#define GScale			( m_piParamS[ SHADOW_G_SCALE ] * ( INVERT_G / 1000.0 ))
#define SLineWidth		( m_piParamT[ TRACK_SLineWidth ] / 10.0 )

#ifdef AVS_PLUGIN
	#define DispFrameInfo	0
#else
	#define DispFrameInfo	m_piParamC[ CHECK_FRAME ]
#endif

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
#define SelectLogVsd ( Log = ( GPSPriority && m_GPSLog || !m_VsdLog ) ? m_GPSLog : m_VsdLog )

// GPS log を優先
#define SelectLogGPS ( Log = m_GPSLog ? m_GPSLog : m_VsdLog )

// パラメータを変換
#define ConvParam( p, from, to ) ( \
	from##Ed == from##St ? 0 : \
	( double )( to##Ed - to##St ) * (( p ) - from##St ) \
	/ ( from##Ed - from##St ) \
	+ to##St \
)

/*** static member **********************************************************/

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

CVsdFilter::CVsdFilter (){
	
	m_VsdLog 			= NULL;
	m_GPSLog 			= NULL;
	
	m_Lap	 			= new LAP_t[ MAX_LAP ];
	m_iLapNum			= 0;
	m_Lap[ 0 ].fLogNum	= FLT_MAX;	// 番犬
	m_Lap[ 0 ].iTime	= 0;		// 番犬
	
	m_iBestTime			= BESTLAP_NONE;
	m_iBestLap			= 0;
	m_iLapMode			= LAPMODE_HAND_VIDEO;
	
	m_iLapIdx			= -1;
	m_iBestLogNumRunning= 0;
	
	m_bCalcLapTimeReq	= FALSE;
	
	m_szLogFile			= new char[ MAX_PATH + 1 ];
	m_szGPSLogFile		= new char[ MAX_PATH + 1 ];
	
	// DrawPolygon 用バッファ
	m_Polygon			= new PolygonData_t[ MAX_POLY_HEIGHT ];
	
	m_pFont				= NULL;
	
	m_logfont.lfHeight			= 0;							// 文字セルまたは文字の高さ
	m_logfont.lfWidth			= 0;							// 平均文字幅
	m_logfont.lfEscapement		= 0;							// 文字送りの方向とX軸との角度
	m_logfont.lfOrientation		= 0;							// ベースラインとX軸との角度
	m_logfont.lfWeight			= FW_REGULAR;					// フォントの太さ
	m_logfont.lfItalic			= FALSE;						// イタリック体指定
	m_logfont.lfUnderline		= FALSE;						// 下線付き指定
	m_logfont.lfStrikeOut		= FALSE;						// 打ち消し線付き指定
	m_logfont.lfCharSet			= DEFAULT_CHARSET;				// キャラクタセット
	m_logfont.lfOutPrecision	= OUT_DEFAULT_PRECIS;			// 出力精度
	m_logfont.lfClipPrecision	= CLIP_DEFAULT_PRECIS;			// クリッピングの精度
	m_logfont.lfQuality			= PROOF_QUALITY;				// 出力品質
	m_logfont.lfPitchAndFamily	= FIXED_PITCH | FF_DONTCARE;	// ピッチとファミリ
	//m_logfont.lfFaceName[LF_FACESIZE];   						// フォント名
	
	// str param に初期値設定
	#define DEF_STR_PARAM( id, var, init, conf_name ) strcpy( var, init );
	#include "def_str_param.h"
}

/*** デストラクタ ***********************************************************/

CVsdFilter::~CVsdFilter (){
	delete m_VsdLog;
	delete m_GPSLog;
	delete [] m_Lap;
	delete [] m_szLogFile;
	delete [] m_szGPSLogFile;
	delete [] m_Polygon;
	delete m_pFont;
}

/*** DrawLine ***************************************************************/

#define ABS( x )			(( x ) < 0 ? -( x ) : ( x ))
#define SWAP( x, y, tmp )	( tmp = x, x = y, y = tmp )

void CVsdFilter::DrawLine( int x1, int y1, int x2, int y2, const PIXEL_YCA &yc, UINT uFlag ){
	
	int i;
	
	int	iXdiff = ABS( x1 - x2 ) + 1;
	int iYdiff = ABS( y1 - y2 ) + 1;
	
	int iXsign = ( x2 > x1 ) ? 1 : -1;
	int iYsign = ( y2 > y1 ) ? 1 : -1;
	
	/* 傾きが1より小さい場合 */
	if( iXdiff > iYdiff ){
		int E = iYdiff - 2 * iXdiff;
		for( i = 0; i < iXdiff; i++ ){
			PutPixel( x1, y1, yc, uFlag );
			x1 += iXsign;
			E += 2 * iYdiff;
			if( E > 0 ){
				y1 += iYsign;
				E -= 2 * iXdiff;
			}
		}
	/* 傾きが1以上の場合 */
	} else {
		int E = iXdiff - 2 * iYdiff;
		for( i = 0; i < iYdiff; i++ ){
			PutPixel( x1, y1, yc, uFlag );
			y1 += iYsign;
			E += 2 * iXdiff;
			if( E > 0 ){
				x1 += iXsign;
				E -= 2 * iYdiff;
			}
		}
	}
}

void CVsdFilter::DrawLine( int x1, int y1, int x2, int y2, int width, const PIXEL_YCA &yc, UINT uFlag ){
	for( int y = 0; y < width; ++y ) for( int x = 0; x < width; ++x ){
		DrawLine(
			x1 + x - width / 2, y1 + y - width / 2,
			x2 + x - width / 2, y2 + y - width / 2,
			yc, uFlag
		);
	}
}

inline void CVsdFilter::FillLine( int x1, int y1, int x2, const PIXEL_YCA &yc, UINT uFlag ){
	
	int	i;
	
	// x 基準で描画
	for( i = x1; i <= x2; ++i ) PutPixel( i, y1, yc, uFlag );
}

/*** DrawRect ***************************************************************/

void CVsdFilter::DrawRect( int x1, int y1, int x2, int y2, const PIXEL_YCA &yc, UINT uFlag ){
	int	y;
	
	if( y1 > y2 ) SWAP( y1, y2, y );
	if( x1 > x2 ) SWAP( x1, x2, y );
	
	for( y = y1; y <= y2; ++y ){
		FillLine( x1, y, x2, yc, uFlag );
	}
}

/*** DrawCircle *************************************************************/

void CVsdFilter::DrawCircle( int x, int y, int r, const PIXEL_YCA &yc, UINT uFlag ){
	
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

void CVsdFilter::DrawCircle( int x, int y, int r, int a, int b, const PIXEL_YCA &yc, UINT uFlag ){
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

void CVsdFilter::DrawFont( int x, int y, UCHAR c, const PIXEL_YCA &yc, UINT uFlag ){
	
	int	i, j;
	PIXEL_YCA	yca;
	
	if( c < '!' || '~' < c ) return;
	
	tFontGlyph *pFont = &m_pFont->m_FontGlyph[ c - '!' ];
	int iBmpW = ( pFont->iW + 3 ) & ~3;
	int iOrgX = ( m_pFont->m_iFontW - pFont->iW ) / 2;
	
	for( j = 0; j < pFont->iH; ++j ) for( i = 0; i < pFont->iW; ++i ){
		int iAlfa = 256 - ( pFont->pBuf[ iBmpW * j + i ] << 2 );
		
		if( iAlfa ){
			yca 		= yc;
			yca.alfa	= iAlfa;
			PutPixel( x + iOrgX + i, y + pFont->iOrgY + j, yca, uFlag );
		}else{
			PutPixel( x + iOrgX + i, y + pFont->iOrgY + j, yc, uFlag );
		}
	}
}

void CVsdFilter::DrawFont( int x, int y, UCHAR c, const PIXEL_YCA &yc, const PIXEL_YCA &ycEdge, UINT uFlag ){
	
	if( c == ' ' ) return;
	
	DrawFont( x + 1, y + 0, c, ycEdge, uFlag );
	DrawFont( x - 1, y + 0, c, ycEdge, uFlag );
	DrawFont( x + 0, y + 1, c, ycEdge, uFlag );
	DrawFont( x + 0, y - 1, c, ycEdge, uFlag );
	DrawFont( x + 0, y + 0, c, yc,     uFlag );
}

/*** DrawString *************************************************************/

void CVsdFilter::DrawString( char *szMsg, const PIXEL_YCA &yc, UINT uFlag, int x, int y ){
	
	if( x != POS_DEFAULT ) m_iPosX = x;
	if( y != POS_DEFAULT ) m_iPosY = y;
	
	for( int i = 0; szMsg[ i ]; ++i ){
		DrawFont( m_iPosX + i * m_pFont->GetW(), m_iPosY, szMsg[ i ], yc, uFlag );
	}
	
	m_iPosY += m_pFont->GetH();
}

void CVsdFilter::DrawString( char *szMsg, const PIXEL_YCA &yc, const PIXEL_YCA &ycEdge, UINT uFlag, int x, int y ){
	
	if( x != POS_DEFAULT ) m_iPosX = x;
	if( y != POS_DEFAULT ) m_iPosY = y;
	
	for( int i = 0; szMsg[ i ]; ++i ){
		DrawFont( m_iPosX + i * m_pFont->GetW(), m_iPosY, szMsg[ i ], yc, ycEdge, uFlag );
	}
	
	m_iPosY += m_pFont->GetH();
}

/*** ポリゴン描画 ***********************************************************/

inline void CVsdFilter::PolygonClear( void ){
	for( int y = 0; y < GetHeight(); ++y ){
		m_Polygon[ y ].iRight	= 0;		// right
		m_Polygon[ y ].iLeft	= 0x7FFF;	// left
	}
}

inline void CVsdFilter::PolygonDraw( const PIXEL_YCA &yc, UINT uFlag ){
	for( int y = 0; y < GetHeight(); ++y ) if( m_Polygon[ y ].iLeft <= m_Polygon[ y ].iRight ){
		FillLine( m_Polygon[ y ].iLeft, y, m_Polygon[ y ].iRight, yc, uFlag & ~IMG_POLYGON );
	}
}

/*** カラーを混ぜる *********************************************************/

inline PIXEL_YCA *CVsdFilter::BlendColor(
	PIXEL_YCA		&ycDst,
	const PIXEL_YCA	&ycColor0,
	const PIXEL_YCA	&ycColor1,
	double	dAlfa
){
	if     ( dAlfa < 0.0 ) dAlfa = 0.0;
	else if( dAlfa > 1.0 ) dAlfa = 1.0;
	
	ycDst.y  = ( PIXEL_t )( ycColor1.y  * dAlfa + ycColor0.y  * ( 1 - dAlfa ));
	ycDst.cb = ( PIXEL_t )( ycColor1.cb * dAlfa + ycColor0.cb * ( 1 - dAlfa ));
	ycDst.cr = ( PIXEL_t )( ycColor1.cr * dAlfa + ycColor0.cr * ( 1 - dAlfa ));
#ifdef AVS_PLUGIN
	ycDst.y1 = ycDst.y;
#endif
	ycDst.alfa = 0;
	return &ycDst;
}

/*** 設定ロード *************************************************************/

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

char *CVsdFilter::IsConfigParamStr( const char *szParamName, char *szBuf, char *szDst ){
	
	int		iLen;
	char	*p;
	
	while( isspace( *szBuf )) ++szBuf;
	
	if(
		strncmp( szBuf, szParamName, iLen = strlen( szParamName )) == 0 &&
		szBuf[ iLen ] == '='
	){
		szBuf += iLen + 1;	// " を指しているはず
		
		// 文字列先頭
		if( p = strchr( szBuf, '"' )){
			szBuf = p + 1;
		}
		
		strcpy( szDst, szBuf );
		
		// 文字列終端
		if(( p = strchr( szDst, '"' )) || ( p = strchr( szDst, ',' ))){
			*p = '\0';
		}
		
		return szDst;
	}
	
	return NULL;
}

BOOL CVsdFilter::ParseMarkStr( const char *szMark ){
	
	do{
		while( *szMark && !isdigit( *szMark )) ++szMark;	// 数字までスキップ
		if( !*szMark ) break;
		
		SetFrameMark( atoi( szMark ));
		if( szMark = strchr( szMark, ',' )) ++szMark;	// 次のパラメータ
	}while( szMark );
	
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
			}
			
			// str param のリード
			#define DEF_STR_PARAM( id, var, init, conf_name ) else if( IsConfigParamStr( conf_name, szBuf, var ));
			#include "def_str_param.h"
			
			else{
				// Mark 以外のパラメータ
				for( i = 0; i < TRACK_N; ++i ){
					if(
						m_szTrackbarName[ i ] &&
						IsConfigParam( m_szTrackbarName[ i ], szBuf, iVal )
					){
						m_piParamT[ i ] = iVal;
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
		
		// フォント再設定
		m_logfont.lfWeight	= m_piParamS[ SHADOW_FONT_ATTR ] & 0x1 ? FW_BOLD : FW_REGULAR;
		m_logfont.lfItalic	= m_piParamS[ SHADOW_FONT_ATTR ] & 0x2 ? 1 : 0;
		
		delete m_pFont; m_pFont = NULL;
	}
	return TRUE;
}

/*** GPS ログリード ********************************************************/

double CVsdFilter::GPSLogGetLength(
	double dLong0, double dLati0,
	double dLong1, double dLati1
){
	// ヒュベニの公式 http://yamadarake.jp/trdi/report000001.html
	const double a	= 6378137.000;
	const double b	= 6356752.314245;
	const double e2	= ( a * a - b * b ) / ( a * a );
	
	double dx	= ( dLong1 - dLong0 ) * ToRAD;
	double dy	= ( dLati1 - dLati0 ) * ToRAD;
	double uy	= ( dLati0 + dLati1 ) / 2 * ToRAD;
	double W	= sqrt( 1 - e2 * sin( uy ) * sin( uy ));
	double M	= a * ( 1 - e2 ) / pow( W, 3 );
	double N	= a / W;
	
	return	sqrt( dy * dy * M * M + pow( dx * N * cos( uy ), 2 ));
}

BOOL CVsdFilter::GPSLogLoad( const char *szFileName ){
	
	UINT	uGPSCnt = 0;
	TCHAR	szBuf[ BUF_SIZE ];
	
	double	dLati, dLati0 = 0;
	double	dLong, dLong0 = 0;
	double	dLati2Meter, dLong2Meter;
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
			
			u = BigEndianI( 0 ) & 0x3FFFFF;
			// 値補正
			// 2254460 → 22:54:46.0
			dTime =	u / 100000 * 3600 +
					u / 1000 % 100 * 60 +
					( u % 1000 ) / 10.0;
			
			dLati = BigEndianI( 8 ) / 460800.0;
			dLong = BigEndianI( 4 ) / 460800.0;
			
			if( uGPSCnt == 0 ){
				dLati0 = dLati;
				dLong0 = dLong;
				dTime0 = dTime;
				
				dLong2Meter = GPSLogGetLength( dLong, dLati, dLong + 1.0 / 3600, dLati ) * 3600;
				dLati2Meter = GPSLogGetLength( dLong, dLati, dLong, dLati + 1.0 / 3600 ) * 3600;
			}
			
			if( dTime < dTime0 ) dTime += 24 * 3600;
			dTime -= dTime0;
			
			// 単位を補正
			// 緯度・経度→メートル
			GPSLog[ uGPSCnt ].fX = ( float )(( dLong - dLong0 ) * dLong2Meter );
			GPSLog[ uGPSCnt ].fY = ( float )(( dLati0 - dLati ) * dLati2Meter );
			
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
	
	/*** dp3x ***************************************************************/
	
	if( IsExt(( char *)szFileName, "dp3x" )){
		
		// 原点取得
		gzread( fp, szBuf, 0x78 );
		dLati0 = *( int *)( szBuf + 0x54 ) / 460800.0;
		dLong0 = *( int *)( szBuf + 0x50 ) / 460800.0;
		dTime = 0;
		
		while( gzread( fp, szBuf, 18 )){
			
			dLati = *( short int *)( szBuf + 0x2 ) / 460800.0 + dLati0;
			dLong = *( short int *)( szBuf + 0x0 ) / 460800.0 + dLong0;
			
			if( uGPSCnt == 0 ){
				dLong2Meter = GPSLogGetLength( dLong, dLati, dLong + 1.0 / 3600, dLati ) * 3600;
				dLati2Meter = GPSLogGetLength( dLong, dLati, dLong, dLati + 1.0 / 3600 ) * 3600;
			}
			
			// 単位を補正
			// 緯度・経度→メートル
			GPSLog[ uGPSCnt ].fX = ( float )(( dLong - dLong0 ) * dLong2Meter );
			GPSLog[ uGPSCnt ].fY = ( float )(( dLati0 - dLati ) * dLati2Meter );
			
			// 速度・向き→ベクトル座標
			GPSLog[ uGPSCnt ].fSpeed	= ( float )( *( short int *)( szBuf + 0x4 ) / 10.0 );
			
			GPSLog[ uGPSCnt ].fBearing	= FLT_MAX;
			GPSLog[ uGPSCnt ].fTime 	= ( float )dTime;
			
			uGPSCnt++;
			dTime += 0.2;
		}
	}
	
	/*** nmea ***************************************************************/
	
	else while( gzgets( fp, szBuf, BUF_SIZE ) != Z_NULL ){
		
		char	cNorthSouth;
		char	cEastWest;
		UINT	uParamCnt;
		
		uParamCnt = sscanf( szBuf,
			"$GPRMC,"
			"%lg,%*c,"	// time
			"%lg,%c,"	// lat
			"%lg,%c,"	// long
			"%lg,%lg,",	// speed, bearing
			// 1	2		3				4		5			6		7
			&dTime, &dLati, &cNorthSouth, &dLong, &cEastWest, &dSpeed, &dBearing
		);
		
		// $GPRMC センテンス以外はスキップ
		if( uParamCnt < 5 ) continue;
		
		// 海外対応w
		if( cNorthSouth == 'S' ) dLati = -dLati;
		if( cEastWest   == 'W' ) dLong = -dLong;
		
		// 値補正
		// 225446.00 → 22:54:46.00
		dTime =	( int )dTime / 10000 * 3600 +
				( int )dTime / 100 % 100 * 60 +
				fmod( dTime, 100 );
		
		// 4916.452653 → 49度16.45分
		dLati =	( int )dLati / 100 + fmod( dLati, 100 ) / 60;
		dLong =	( int )dLong / 100 + fmod( dLong, 100 ) / 60;
		
		if( uGPSCnt == 0 ){
			dLati0 = dLati;
			dLong0 = dLong;
			dTime0 = dTime;
			
			dLong2Meter = GPSLogGetLength( dLong, dLati, dLong + 1.0 / 3600, dLati ) * 3600;
			dLati2Meter = GPSLogGetLength( dLong, dLati, dLong, dLati + 1.0 / 3600 ) * 3600;
		}
		
		if( dTime < dTime0 ) dTime += 24 * 3600;
		dTime -= dTime0;
		
		// 単位を補正
		// 緯度・経度→メートル
		GPSLog[ uGPSCnt ].fX = ( float )(( dLong - dLong0 ) * dLong2Meter );
		GPSLog[ uGPSCnt ].fY = ( float )(( dLati0 - dLati ) * dLati2Meter );
		
		// 速度・向き→ベクトル座標
		GPSLog[ uGPSCnt ].fSpeed	= uParamCnt < 6 ? FLT_MAX : ( float )( dSpeed * 1.852 ); // knot/h → km/h
		GPSLog[ uGPSCnt ].fBearing	= uParamCnt < 7 ? FLT_MAX : ( float )dBearing;
		GPSLog[ uGPSCnt ].fTime		= ( float )dTime;
		
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
		
		// 20km/h 以下で 3秒以上 log の間隔が開くとき，0km/h の log を補完する
		if(
			uGPSCnt >= 2 &&
			GPSLog[ uGPSCnt ].fTime - GPSLog[ uGPSCnt - 1 ].fTime >= 3 &&
			GPSLog[ uGPSCnt ].fSpeed <= 20
		){
			// -1 +0 +1 +2
			// A  B
			//  ↓
			// A  A' B' B
			
			// データ B のコピー
			GPSLog[ uGPSCnt + 1 ] =
			GPSLog[ uGPSCnt + 2 ] = GPSLog[ uGPSCnt ];
			
			// データ A のコピー
			GPSLog[ uGPSCnt ] = GPSLog[ uGPSCnt - 1 ];
			
			// スピードを 0 に
			GPSLog[ uGPSCnt ].fSpeed = GPSLog[ uGPSCnt + 1 ].fSpeed = 0;
			
			// 時間調整
			float fDiff = GPSLog[ uGPSCnt - 1 ].fTime - GPSLog[ uGPSCnt - 2 ].fTime;
			GPSLog[ uGPSCnt     ].fTime += fDiff;
			GPSLog[ uGPSCnt + 1 ].fTime -= fDiff;
			
			uGPSCnt += 2;
		}
		
		uGPSCnt++;
	}
	
	gzclose( fp );
	
	/************************************************************************/
	
	// アップコンバート用バッファ確保・初期化
	m_GPSLog = new CVsdLog;
	m_GPSLog->GPSLogUpConvert( GPSLog, uGPSCnt, TRUE );
	m_GPSLog->RotateMap( m_piParamT[ TRACK_MapAngle ] * ( -ToRAD / 10 ));
	
	DebugCmd( {
		FILE *fpp = fopen( "C:\\DDS\\vsd\\vsd_filter\\z_gps_raw.txt", "w" );
		for( u = 0; u < uGPSCnt; ++u ){
			fprintf( fpp, "%g\t%g\t%g\t%g\t%g\t%g\t%g\n",
				GPSLog[ u ].fX,
				GPSLog[ u ].fY,
				GPSLog[ u ].fSpeed,
				GPSLog[ u ].fBearing,
				GPSLog[ u ].fGx,
				GPSLog[ u ].fGy,
				GPSLog[ u ].fTime
			);
		}
		fclose( fpp );
	} )
	
	delete [] GPSLog;
	
	DebugCmd( {
		FILE *fpp = fopen( "C:\\DDS\\vsd\\vsd_filter\\z_upcon_gps.txt", "w" );
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
	double	dLati2Meter, dLong2Meter;
	double	dSpeed;
	double	dBearing;
	
	m_iLogStart = m_iLogStop = 0;
	
	TCHAR	*p;
	
	int	iLogFreqLog		= 0;
	int	iLogFreqTime	= 0;
	
	while( gzgets( fp, szBuf, BUF_SIZE ) != Z_NULL ){
		if(( p = strstr( szBuf, "LAP" )) != NULL ){ // ラップタイム記録を見つけた
			m_iLapMode = LAPMODE_MAGNET;
			
			uReadCnt = sscanf( p, "LAP%d%d:%d.%d", &uLap, &uMin, &uSec, &uMSec );
			
			int iTime = ( uMin * 60 + uSec ) * 1000 + uMSec;
			
			m_Lap[ m_iLapNum ].uLap		= uLap;
			m_Lap[ m_iLapNum ].fLogNum	= ( float )uLogNum;
			m_Lap[ m_iLapNum ].iTime	= ( uReadCnt == 4 ) ? iTime : 0;
			
			if(
				uReadCnt == 4 &&
				( m_iBestTime == BESTLAP_NONE || m_iBestTime > iTime )
			){
				m_iBestTime	= iTime;
				m_iBestLap	= m_iLapNum - 1;
				
				iLogFreqLog	 += uLogNum - ( int )m_Lap[ m_iLapNum - 1 ].fLogNum;
				iLogFreqTime += iTime;
			}
			++m_iLapNum;
		}
		
		if(( p = strstr( szBuf, "GPS" )) != NULL ){ // GPS記録を見つけた
			sscanf( p, "GPS%lg%lg%lg%lg", &dLati, &dLong, &dSpeed, &dBearing );
			
			if( uGPSCnt == 0 ){
				dLati0 = dLati;
				dLong0 = dLong;
				
				dLong2Meter = GPSLogGetLength( dLong, dLati, dLong + 1.0 / 3600, dLati ) * 3600;
				dLati2Meter = GPSLogGetLength( dLong, dLati, dLong, dLati + 1.0 / 3600 ) * 3600;
			}
			
			// 単位を補正
			// 緯度・経度→メートル
			GPSLog[ uGPSCnt ].fX = ( float )(( dLong - dLong0 ) * dLong2Meter );
			GPSLog[ uGPSCnt ].fY = ( float )(( dLati0 - dLati ) * dLati2Meter );
			GPSLog[ uGPSCnt ].fSpeed	= ( float )dSpeed;
			GPSLog[ uGPSCnt ].fBearing	= ( float )dBearing;
			
			// LOG_FREQ は 15Hz だった時代のログでは間違いだが，等比分割だし実害なし?
			GPSLog[ uGPSCnt++ ].fTime = (( int )uLogNum - GPS_LOG_OFFS ) / ( float )LOG_FREQ;
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
	
	// 古い log は LOG_FREQ とは限らないので，計算で求める
	if( iLogFreqTime ){
		m_VsdLog->m_dFreq = iLogFreqLog / ( iLogFreqTime / 1000.0 );
	}
	
	/*** GPS ログから軌跡を求める *******************************************/
	
	if( uGPSCnt ){
		m_VsdLog->GPSLogUpConvert( GPSLog, uGPSCnt );
		m_VsdLog->RotateMap( m_piParamT[ TRACK_MapAngle ] * ( -ToRAD / 10 ));
	}
	
	delete [] GPSLog;
	
	/************************************************************************/
	
	gzclose( fp );
	
	m_Lap[ m_iLapNum ].fLogNum	= FLT_MAX;	// 番犬
	m_Lap[ m_iLapNum ].iTime	= 0;		// 番犬
	
	DeleteIfZero( m_VsdLog );
	
	return TRUE;
}

/*** ラップ番号 → ログ番号 変換 ********************************************/

double CVsdFilter::LapNum2LogNum( CVsdLog *Log, int iLapNum ){
	
	double a;
	
	// iLapNum がおかしいときは 0 を返しとく
	if( iLapNum < 0 ) return 0;
	
	if( m_iLapMode == LAPMODE_MAGNET || m_iLapMode == LAPMODE_HAND_MAGNET ){
		// iLogNum は VSD ログ番号
		if( Log == m_VsdLog ) return m_Lap[ iLapNum ].fLogNum;
		if( LogSt == LogEd )  return 0;
		a = ( m_Lap[ iLapNum ].fLogNum - LogSt ) / ( LogEd - LogSt );
		
	}else if( m_iLapMode == LAPMODE_GPS || m_iLapMode == LAPMODE_HAND_GPS ){
		// iLogNum は GPS ログ番号
		if( Log == m_GPSLog ) return m_Lap[ iLapNum ].fLogNum;
		if( GPSSt == GPSEd )  return 0;
		a = ( m_Lap[ iLapNum ].fLogNum - GPSSt ) / ( GPSEd - GPSSt );
		
	}else{
		// iLogNum はビデオフレーム番号
		if( VideoSt == VideoEd ) return 0;
		a = ( m_Lap[ iLapNum ].fLogNum - VideoSt ) / ( VideoEd - VideoSt );
	}
	
	return Log == m_VsdLog ?
		a * ( LogEd - LogSt ) + LogSt :
		a * ( GPSEd - GPSSt ) + GPSSt;
}

/*** パラメータ調整用スピードグラフ *****************************************/

#define SPEED_GRAPH_SCALE	2

void CVsdFilter::DrawSpeedGraph( CVsdLog *Log, const PIXEL_YCA &yc ){
	
	int	iLogNum;
	int	x = 0;
	int iCursor = GetHeight() - 1;
	
	iLogNum = Log->m_iLogNum - GetWidth() * SPEED_GRAPH_SCALE / 2;
	if( iLogNum < 0 ){
		x = -iLogNum / SPEED_GRAPH_SCALE;
		iLogNum = 0;
	}
	
	for( ; x < GetWidth() - 1 && iLogNum < Log->m_iCnt - 1; ++x, iLogNum += SPEED_GRAPH_SCALE ){
		DrawLine(
			x,     GetHeight() - 1 - ( int )Log->Speed( iLogNum ),
			x + 1, GetHeight() - 1 - ( int )Log->Speed( iLogNum + SPEED_GRAPH_SCALE ),
			1, yc, 0
		);
		
		if( x == GetWidth() / 2 ) iCursor = GetHeight() - 1 - ( int )Log->Speed( iLogNum );
	}
	
	DrawLine(
		GetWidth() / 2, iCursor - 10,
		GetWidth() / 2, iCursor + 10,
		1, yc, 0
	);
}

/*** ラップタイム再計算 (手動) **********************************************/

void CVsdFilter::CalcLapTime( void ){
	
	int		iTime, iPrevTime;
	int		iFrame = 0;
	double	dLogNum;
	
	m_iLapNum	= 0;
	m_iBestTime	= BESTLAP_NONE;
	
	if( m_VsdLog ){
		m_iLapMode = LAPMODE_HAND_MAGNET;
	}else if( m_GPSLog ){
		m_iLapMode = LAPMODE_HAND_GPS;
	}else{
		m_iLapMode = LAPMODE_HAND_VIDEO;
	}
	
	while(( iFrame = GetFrameMark( iFrame )) >= 0 ){
		
		if( m_iLapMode == LAPMODE_HAND_MAGNET ){
			dLogNum	= ConvParam( iFrame, Video, Log );
			iTime	= ( int )( dLogNum / m_VsdLog->m_dFreq * 1000 );
		}else if( m_iLapMode == LAPMODE_HAND_GPS ){
			dLogNum	= ConvParam( iFrame, Video, GPS );
			iTime	= ( int )( dLogNum / LOG_FREQ * 1000 );
		}else{
			iTime	= ( int )( iFrame * 1000.0 / GetFPS());
		}
		
		m_Lap[ m_iLapNum ].fLogNum	= m_iLapMode == LAPMODE_HAND_VIDEO ? iFrame : ( float )dLogNum;
		
		if( m_piParamT[ TRACK_SLineWidth ] < 0 ){
			// ジムカーナモード
			m_Lap[ m_iLapNum ].uLap		= ( m_iLapNum / 2 ) + 1;
			m_Lap[ m_iLapNum ].iTime	= ( m_iLapNum & 1 ) ? iTime - iPrevTime : 0;
		}else{
			m_Lap[ m_iLapNum ].uLap		= m_iLapNum;
			m_Lap[ m_iLapNum ].iTime	= m_iLapNum ? iTime - iPrevTime : 0;
		}
		
		if(
			m_iLapNum &&
			m_Lap[ m_iLapNum ].iTime &&
			( m_iBestTime == BESTLAP_NONE || m_iBestTime > m_Lap[ m_iLapNum ].iTime )
		){
			m_iBestTime	= m_Lap[ m_iLapNum ].iTime;
			m_iBestLap	= m_iLapNum - 1;
		}
		
		iPrevTime = iTime;
		++m_iLapNum;
		++iFrame;
	}
	m_Lap[ m_iLapNum ].fLogNum	= FLT_MAX;	// 番犬
	m_Lap[ m_iLapNum ].iTime	= 0;		// 番犬
}

/*** ラップタイム再計算 (GPS auto) ******************************************/

void CVsdFilter::CalcLapTimeAuto( void ){
	
	int iFrame;
	
	if(( iFrame = GetFrameMark( 0 )) < 0 ) return;
	
	m_iLapMode = LAPMODE_GPS;
	
	/*** スタートラインの位置を取得 ***/
	// iFrame に対応する GPS ログ番号取得
	double dLogNum = ConvParam( iFrame, Video, GPS );
	
	int iLogNum = ( int )dLogNum;
	
	// iLogNum ～ iLogNum + 1 の方位を算出
	
	double dAngle = atan2(
		( m_GPSLog->m_Log[ iLogNum + 1 ].fY0 - m_GPSLog->m_Log[ iLogNum ].fY0 ),
		( m_GPSLog->m_Log[ iLogNum + 1 ].fX0 - m_GPSLog->m_Log[ iLogNum ].fX0 )
	);
	
	#define x1 m_dStartLineX1
	#define y1 m_dStartLineY1
	#define x2 m_dStartLineX2
	#define y2 m_dStartLineY2
	#define x3 m_GPSLog->m_Log[ i ].fX0
	#define y3 m_GPSLog->m_Log[ i ].fY0
	#define x4 m_GPSLog->m_Log[ i + 1 ].fX0
	#define y4 m_GPSLog->m_Log[ i + 1 ].fY0
	
	// 仮想光電管の位置を求める
	x2 = m_GPSLog->X0( dLogNum );	// スタート地点
	y2 = m_GPSLog->Y0( dLogNum );
	
	x1 = x2 + cos( dAngle + 90 * ToRAD ) * SLineWidth / 2;
	y1 = y2 + sin( dAngle + 90 * ToRAD ) * SLineWidth / 2;
	x2 = x2 + cos( dAngle - 90 * ToRAD ) * SLineWidth / 2;
	y2 = y2 + sin( dAngle - 90 * ToRAD ) * SLineWidth / 2;
	
	/*****/
	
	m_iLapNum	= 0;
	m_iBestTime	= BESTLAP_NONE;
	
	int iTime, iPrevTime;
	int	iLapNum = 0;
	
	for( int i = 0; i < m_GPSLog->m_iCnt - 1; ++i ){
		
		/*** 交差判定，交点判定 ***/
		double s1, s2, a;
		
		// 交点がスタートライン線分上かの判定
		s1 = ( x4 - x3 ) * ( y1 - y3 ) - ( x1 - x3 ) * ( y4 - y3 );
		s2 = ( x4 - x3 ) * ( y3 - y2 ) - ( x3 - x2 ) * ( y4 - y3 );
		a = ( s1 + s2 == 0 ) ? -1 : s1 / ( s1 + s2 );
		if( !( 0 <= a && a <= 1 )) continue;
		
		// 交点が iLogNum ～ +1 線分上かの判定
		s1 = ( x2 - x1 ) * ( y3 - y1 ) - ( y2 - y1 ) * ( x3 - x1 );
		s2 = ( x2 - x1 ) * ( y1 - y4 ) - ( y2 - y1 ) * ( x1 - x4 );
		a = ( s1 + s2 == 0 ) ? -1 : s1 / ( s1 + s2 );
		if( !( 0 <= a && a < 1 )) continue;
		
		// 進行方向の判定，dAngle ±45度
		double dAngle2 = dAngle - atan2(
			( m_GPSLog->m_Log[ i + 1 ].fY0 - m_GPSLog->m_Log[ i ].fY0 ),
			( m_GPSLog->m_Log[ i + 1 ].fX0 - m_GPSLog->m_Log[ i ].fX0 )
		);
		if     ( dAngle2 < -180 * ToRAD ) dAngle2 += 360 * ToRAD;
		else if( dAngle2 >  180 * ToRAD ) dAngle2 -= 360 * ToRAD;
		if( dAngle2 < -45 * ToRAD || dAngle2 > 45 * ToRAD ) continue;
		
		#undef x1
		#undef y1
		#undef x2
		#undef y2
		#undef x3
		#undef y3
		#undef x4
		#undef y4
		
		// 半端な LogNum
		dLogNum = i + a;
		iTime = ( int )( dLogNum / LOG_FREQ * 1000 );
		iPrevTime;
		
		if( m_piParamS[ SHADOW_LAP_START ] - 1 <= iLapNum && iLapNum <= m_piParamS[ SHADOW_LAP_END ] ){
			m_Lap[ m_iLapNum ].uLap		= m_iLapNum;
			m_Lap[ m_iLapNum ].fLogNum	= ( float )dLogNum;
			m_Lap[ m_iLapNum ].iTime	= m_iLapNum ? iTime - iPrevTime : 0;
			
			if(
				m_iLapNum &&
				( m_iBestTime == BESTLAP_NONE || m_iBestTime > m_Lap[ m_iLapNum ].iTime )
			){
				m_iBestTime	= m_Lap[ m_iLapNum ].iTime;
				m_iBestLap	= m_iLapNum - 1;
			}
			
			iPrevTime = iTime;
			++m_iLapNum;
		}
		++iLapNum;
	}
	m_Lap[ m_iLapNum ].fLogNum	= FLT_MAX;	// 番犬
	m_Lap[ m_iLapNum ].iTime	= 0;		// 番犬
}

/****************************************************************************/
/*** メーター描画 ***********************************************************/
/****************************************************************************/

/*** メーター等描画 *********************************************************/

static const PIXEL_YCA	yc_black		= RGB2YC(    0,    0,    0 );
static const PIXEL_YCA	yc_white		= RGB2YC( 4095, 4095, 4095 );
static const PIXEL_YCA	yc_gray			= RGB2YC( 2048, 2048, 2048 );
static const PIXEL_YCA	yc_red			= RGB2YC( 4095,    0,    0 );
static const PIXEL_YCA	yc_green		= RGB2YC(    0, 4095,    0 );
static const PIXEL_YCA	yc_yellow		= RGB2YC( 4095, 4095,    0 );
static const PIXEL_YCA	yc_dark_green	= RGB2YC(    0, 2048,    0 );
static const PIXEL_YCA	yc_blue			= RGB2YC(    0,    0, 4095 );
static const PIXEL_YCA	yc_cyan			= RGB2YC(    0, 4095, 4095 );
static const PIXEL_YCA	yc_dark_blue	= RGB2YC(    0,    0, 2048 );
static const PIXEL_YCA	yc_orange		= RGB2YC( 4095, 1024,    0 );
static const PIXEL_YCA	yc_gray_a		= RGB2YCA( 2048, 2048, 2048, 0x80 );

#define COLOR_PANEL			yc_gray_a
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
		50 * GetHeight() / 240;
	
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
	int iFontSize = m_piParamS[ SHADOW_FONT_SIZE ] > 0 ?
		m_piParamS[ SHADOW_FONT_SIZE ] :
		iMeterR * 21 / 100;
	
	if( m_pFont == NULL || iFontSize != -m_logfont.lfHeight ){
		m_logfont.lfHeight = -iFontSize;
		if( m_pFont ) delete m_pFont;
		m_pFont = new CVsdFont( m_logfont );
	}
	
	CVsdLog *Log;
	
	// ログ位置の計算
	if( m_VsdLog ){
		m_VsdLog->m_dLogNum = ConvParam( GetFrameCnt(), Video, Log );
		m_VsdLog->m_iLogNum = ( int )m_VsdLog->m_dLogNum;
	}
	if( m_GPSLog ){
		m_GPSLog->m_dLogNum = ConvParam( GetFrameCnt(), Video, GPS );
		m_GPSLog->m_iLogNum = ( int )m_GPSLog->m_dLogNum;
	}
	
	/*** Lap タイム描画 ***/
	
	// ラップタイムの再計算
	if( m_iLapMode != LAPMODE_MAGNET && DispLap && m_bCalcLapTimeReq ){
		m_bCalcLapTimeReq	= FALSE;
		m_iLapMode			= LAPMODE_HAND_VIDEO;
		
		if( m_GPSLog && m_piParamT[ TRACK_SLineWidth ] > 0 ){
			CalcLapTimeAuto();
		}
		if( m_iLapMode != LAPMODE_GPS ){
			CalcLapTime();
		}
	}
	
	Log = m_iLapMode == LAPMODE_MAGNET || m_iLapMode == LAPMODE_HAND_MAGNET ? m_VsdLog : m_GPSLog;
	
	// ラップインデックスを求める
	if( m_iLapNum ){
		// VSD/GPS 両方のログがなければ，手動計測での m_Lap[].fLogNum はフレーム# なので
		// m_Lap[].fLogNum と精度をあわせるため，m_dLogNum はいったん float に落とす
		float fLogNum = m_iLapMode != LAPMODE_HAND_VIDEO ? ( float )Log->m_dLogNum : GetFrameCnt();
		
		// カレントポインタがおかしいときは，-1 にリセット
		if(
			m_iLapIdx >= m_iLapNum ||
			m_iLapIdx >= 0 && m_Lap[ m_iLapIdx ].fLogNum > fLogNum
		) m_iLapIdx = -1;
		
		for( ; m_Lap[ m_iLapIdx + 1 ].fLogNum <= fLogNum; ++m_iLapIdx );
	}else{
		m_iLapIdx = -1;
	}
	
	if( DispLap && m_iLapNum ){
		// 時間表示
		if( m_iLapIdx >= 0 && m_Lap[ m_iLapIdx + 1 ].iTime != 0 ){
			int iTime;
			if( m_iLapMode != LAPMODE_HAND_VIDEO ){
				// 自動計測時は，タイム / ログ数 から計算
				iTime = ( int )(( Log->m_dLogNum - m_Lap[ m_iLapIdx ].fLogNum ) * 1000 / Log->m_dFreq );
			}else{
				// 手動計測モードのときは，フレーム数から計算
				iTime = ( int )(( GetFrameCnt() - m_Lap[ m_iLapIdx ].fLogNum ) * 1000.0 / GetFPS());
			}
			
			sprintf( szBuf, "Time%2d'%02d.%03d", iTime / 60000, iTime / 1000 % 60, iTime % 1000 );
			DrawString( szBuf, COLOR_TIME, COLOR_TIME_EDGE, 0, GetWidth() - m_pFont->GetW() * 13, 1 );
			bInLap = TRUE;
		}else{
			// まだ開始していない
			DrawString( "Time -'--.---", COLOR_TIME, COLOR_TIME_EDGE, 0, GetWidth() - m_pFont->GetW() * 13, 1 );
		}
		
		/*** ベストとの車間距離表示 - ***/
		if( m_VsdLog || m_GPSLog ){
			if( bInLap ){
				
				SelectLogGPS;
				
				// ベストラップ開始の LogNum
				double dBestLapLogNumStart = LapNum2LogNum( Log, m_iBestLap );
				
				// この周の走行距離を求める
				double dMileage = Log->Mileage() - Log->Mileage( LapNum2LogNum( Log, m_iLapIdx ));
				
				// この周の 1周の走行距離から，現在の走行距離を補正する
				dMileage =
					dMileage
					* ( Log->Mileage( LapNum2LogNum( Log, m_iBestLap + 1 )) - Log->Mileage( dBestLapLogNumStart ))
					/ ( Log->Mileage( LapNum2LogNum( Log, m_iLapIdx  + 1 )) - Log->Mileage( LapNum2LogNum( Log, m_iLapIdx )));
				
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
				m_iPosY += m_pFont->GetH();
			}
		}
		
		m_iPosY += m_pFont->GetH() / 4;
		
		// Best 表示
		sprintf(
			szBuf, "Best%2d'%02d.%03d",
			m_iBestTime / 60000,
			m_iBestTime / 1000 % 60,
			m_iBestTime % 1000
		);
		DrawString( szBuf, COLOR_TIME, COLOR_TIME_EDGE, 0 );
		
		// Lapタイム表示
		// 3つタイム表示する分の，最後の LapIdx を求める．
		// 通常は m_iLapIdx + 1 だが，m_Lap[ iLapIdxEnd ].iTime == 0 の時は
		// 周回モードでは最後のラップを走り終えた
		// ジムカモードでは 1周走り終えたことを示しているので
		// LapIdx を -1 する
		int iLapIdxEnd = m_iLapIdx + 1;
		if( m_Lap[ iLapIdxEnd ].iTime == 0 ) --iLapIdxEnd;
		
		// iLapIdxEnd から有効なラップタイムが 2個見つかるまで遡る
		int iLapIdxStart = iLapIdxEnd - 1;
		for( i = 0; iLapIdxStart > 0; --iLapIdxStart ){
			if( m_Lap[ iLapIdxStart ].iTime ){
				if( ++i >= 2 ) break;
			}
		}
		
		if( iLapIdxStart >= 0 ){
			for( ; iLapIdxStart <= iLapIdxEnd; ++iLapIdxStart ){
				if( m_Lap[ iLapIdxStart ].iTime != 0 ){
					sprintf(
						szBuf, "%3d%c%2d'%02d.%03d",
						m_Lap[ iLapIdxStart ].uLap,
						( iLapIdxStart == m_iLapIdx + 1 && bInLap ) ? '*' : ' ',
						m_Lap[ iLapIdxStart ].iTime / 60000,
						m_Lap[ iLapIdxStart ].iTime / 1000 % 60,
						m_Lap[ iLapIdxStart ].iTime % 1000
					);
					DrawString( szBuf,
						m_iBestTime == m_Lap[ iLapIdxStart ].iTime ? COLOR_BEST_LAP : COLOR_TIME,
						COLOR_TIME_EDGE, 0 );
					++i;
				}
			}
		}
	}
	
	// フレーム表示
	
	#define Float2Time( n )	( int )( n ) / 60, fmod( n, 60 )
	
	#ifndef AVS_PLUGIN
		if( DispFrameInfo ){
			
			#ifndef GPS_ONLY
				DrawString( "        start       end     range cur.pos", COLOR_STR, COLOR_TIME_EDGE, 0, 0, GetHeight() / 3 );
				
				sprintf(
					szBuf, "Vid%4d:%05.2f%4d:%05.2f%4d:%05.2f%7d",
					Float2Time( VideoSt / GetFPS()),
					Float2Time( VideoEd / GetFPS()),
					Float2Time(( VideoEd - VideoSt ) / GetFPS()),
					GetFrameCnt()
				);
				DrawString( szBuf, COLOR_STR, COLOR_TIME_EDGE, 0, 0 );
				
				if( m_VsdLog ){
					sprintf(
						szBuf, "Log%4d:%05.2f%4d:%05.2f%4d:%05.2f%7d",
						Float2Time( LogSt / m_VsdLog->m_dFreq ),
						Float2Time( LogEd / m_VsdLog->m_dFreq ),
						Float2Time(( LogEd - LogSt ) / m_VsdLog->m_dFreq ),
						m_VsdLog->m_iLogNum
					);
					DrawString( szBuf, COLOR_STR, COLOR_TIME_EDGE, 0 );
					DrawSpeedGraph( m_VsdLog, yc_red );
				}
				
				if( m_GPSLog ){
					sprintf(
						szBuf, "GPS%4d:%05.2f%4d:%05.2f%4d:%05.2f%7d",
						Float2Time( GPSSt / m_GPSLog->m_dFreq ),
						Float2Time( GPSEd / m_GPSLog->m_dFreq ),
						Float2Time(( GPSEd - GPSSt ) / m_GPSLog->m_dFreq ),
						m_GPSLog->m_iLogNum
					);
					DrawString( szBuf, COLOR_STR, COLOR_TIME_EDGE, 0 );
				}
			#endif	// !GPS_ONLY
			
			if( m_GPSLog ){
				DrawSpeedGraph( m_GPSLog, yc_cyan );
			}
		}
	#endif // !AVS_PLUGIN
	
	if( !m_VsdLog && !m_GPSLog ) return TRUE;
	
	/*** メーターパネル ***/
	#ifdef GPS_ONLY
		DrawCircle(
			iMeterCx, iMeterCy, iMeterR * 1000,
			( 1000 * 1000 / Aspect ) * ( 1000 * 1000 / Aspect ),
			1000 * 1000,
			COLOR_PANEL, CVsdFilter::IMG_FILL
		);
	#else
		DrawCircle(
			iMeterCx, iMeterCy, iMeterR,
			COLOR_PANEL, CVsdFilter::IMG_FILL
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
						( int )( cos( iDeg * ToRAD ) * iMeterR * .8 * AspectRatio ) + iMeterCx - m_pFont->GetW() / ( i >= 10000 ? 1 : 2 ),
						( int )( sin( iDeg * ToRAD ) * iMeterR * .8 ) + iMeterCy - m_pFont->GetH() / 2
					);
				}
			}
		}
	}else{
		// GPS ログ優先時はスピードメーターパネル
		int	iStep = (( iMeterSMaxVal / 18 ) + 4 ) / 5 * 5;
		if( iStep == 0 ) iStep = 5;
		
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
						( int )( cos( iDeg * ToRAD ) * iMeterR * .75 * AspectRatio ) + iMeterCx - m_pFont->GetW() * strlen( szBuf ) / 2,
						( int )( sin( iDeg * ToRAD ) * iMeterR * .75 ) + iMeterCy - m_pFont->GetH() / 2
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
			
			for( i = -( int )( GSnakeLen * LOG_FREQ / 10.0 ) ; i <= 1 ; ++i ){
				
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
					double dG = Log->Gy( i );
					
					PIXEL_YCA yc_line;
					
					if( dG >= 0.0 ){
						BlendColor( yc_line, yc_yellow, yc_green, dG / Log->m_dMaxG );
					}else{
						BlendColor( yc_line, yc_yellow, yc_red, dG / Log->m_dMinG );
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
		
		// スタートライン表示
		#ifndef AVS_PLUGIN
			if( DispFrameInfo && m_iLapMode == LAPMODE_GPS ){
				double dAngle = m_piParamT[ TRACK_MapAngle ] * ( -ToRAD / 10 );
				
				int x1 = ( int )((  cos( dAngle ) * m_dStartLineX1 + sin( dAngle ) * m_dStartLineY1 - Log->m_dMapOffsX ) / Log->m_dMapSize * MAX_MAP_SIZE + 8 );
				int y1 = ( int )(( -sin( dAngle ) * m_dStartLineX1 + cos( dAngle ) * m_dStartLineY1 - Log->m_dMapOffsY ) / Log->m_dMapSize * MAX_MAP_SIZE + 8 );
				int x2 = ( int )((  cos( dAngle ) * m_dStartLineX2 + sin( dAngle ) * m_dStartLineY2 - Log->m_dMapOffsX ) / Log->m_dMapSize * MAX_MAP_SIZE + 8 );
				int y2 = ( int )(( -sin( dAngle ) * m_dStartLineX2 + cos( dAngle ) * m_dStartLineY2 - Log->m_dMapOffsY ) / Log->m_dMapSize * MAX_MAP_SIZE + 8 );
				
				DrawLine( x1, y1, x2, y2, yc_blue, 0 );
			}
		#endif
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
		
		sprintf( szBuf, "%d", uGear );
		DrawString(
			szBuf,
			COLOR_STR, 0,
			iMeterCx - m_pFont->GetW() / 2, iMeterCy - iMeterR / 2
		);
	}
	
	SelectLogVsd;
	
	if( Log->IsDataExist()){
		// スピード表示
		sprintf( szBuf, "%3d", ( int )Log->Speed() );
		DrawString(
			szBuf,
			COLOR_STR, 0,
			iMeterCx - 3 * m_pFont->GetW() / 2, iMeterCy + iMeterR / 2
		);
		
		// G 数値
		if( GSnakeLen >= 0 ){
			sprintf( szBuf, "%02dG", ( int )( sqrt( Log->Gx() * Log->Gx() + Log->Gy() * Log->Gy()) * 10 ));
			DrawString(
				szBuf,
				COLOR_STR, 0,
				iMeterCx - 3 * m_pFont->GetW() / 2, iMeterCy + iMeterR / 2 - m_pFont->GetH()
			);
			
			int iDotSize = -m_logfont.lfHeight * 2 / 20 - 1;
			
			// 小数点
			/*
			DrawRect(
				m_iPosX + m_pFont->GetW(),            m_iPosY - ( int )( m_pFont->GetH() * 0.15 ),
				m_iPosX + m_pFont->GetW() + iDotSize, m_iPosY - ( int )( m_pFont->GetH() * 0.15 ) + iDotSize,
				COLOR_STR, 0
			);
			*/
			DrawString(
				".",
				COLOR_STR, 0,
				iMeterCx - m_pFont->GetW(),
				iMeterCy + iMeterR / 2 - m_pFont->GetH()
			);
		}
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
