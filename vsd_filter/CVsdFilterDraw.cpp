/*****************************************************************************
	
	VSD -- vehicle data logger system  Copyright(C) by DDS
	
	CVsdFilter.cpp - CVsdFilter class implementation
	
*****************************************************************************/

#include "StdAfx.h"

#include "dds.h"
#include "../vsd/main.h"
#include "dds_lib/dds_lib.h"

#ifndef AVS_PLUGIN
	#include "filter.h"
#endif
#include "CVsdLog.h"
#include "CVsdFont.h"
#include "CScript.h"
#include "pixel.h"
#include "CVsdImage.h"
#include "CVsdFilter.h"

/*** macros *****************************************************************/

#define INVALID_POS_I	0x7FFFFFFF
#define MAX_LINE_LEN	2000

#define LineTrace		m_piParamT[ TRACK_LineTrace ]
#define DispLap			m_piParamC[ CHECK_LAP ]
#define GSnakeLen		m_piParamT[ TRACK_G_Len ]
#define GScale			( m_piParamS[ SHADOW_G_SCALE ] * ( INVERT_G / 1000.0 ))
#define DispGraph		m_piParamC[ CHECK_GRAPH ]

#ifdef AVS_PLUGIN
	#define DispSyncInfo	0
#else
	#define DispSyncInfo	m_piParamC[ CHECK_SYNCINFO ]
#endif

#ifdef GPS_ONLY
	#define Aspect			m_piParamT[ TRACK_Aspect ]
	#define AspectRatio		(( double )m_piParamT[ TRACK_Aspect ] / 1000 )
	#define MeterPosRight	m_piParamC[ CHECK_METER_POS ]
#else
	#define Aspect			1000
	#define AspectRatio		1
	#define MeterPosRight	!m_piParamC[ CHECK_METER_POS ]
#endif

#ifdef GPS_ONLY
	#define GPSPriority		FALSE
#else
	#define GPSPriority		m_piParamC[ CHECK_GPS_PRIO ]
#endif

#define MAX_MAP_SIZE	( GetWidth() * m_piParamT[ TRACK_MapSize ] / 1000 )

// VSD log を優先，ただしチェックボックスでオーバーライドできる
#define SelectLogVsd ( Log = ( GPSPriority && m_GPSLog || !m_VsdLog ) ? m_GPSLog : m_VsdLog )

// GPS log を優先
#define SelectLogGPS ( Log = m_GPSLog ? m_GPSLog : m_VsdLog )

// Laptime 計算用
#define SelectLogForLapTime	( Log = m_iLapMode == LAPMODE_MAGNET || m_iLapMode == LAPMODE_HAND_MAGNET ? m_VsdLog : m_GPSLog )

/*** DrawLine ***************************************************************/

#define ABS( x )			(( x ) < 0 ? -( x ) : ( x ))
#define SWAP( x, y, tmp )	( tmp = x, x = y, y = tmp )

void CVsdFilter::DrawLine( int x1, int y1, int x2, int y2, const PIXEL_YCA &yc, UINT uFlag ){
	
	int i;
	
	if( y1 == y2 ){
		if( x1 > x2 ) SWAP( x1, x2, i );
		FillLine( x1, y1, x2, yc, uFlag );
		return;
	}
	
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

// http://fussy.web.fc2.com/algo/algo2-2.htm
// の，a = r / A, b = r / B と置いて両辺に ( A * B / r )^2 をかける
void CVsdFilter::DrawCircle( int x, int y, int a, int b, const PIXEL_YCA &yc, UINT uFlag ){
	
	if( a == b ){
		DrawCircle( x, y, a, yc, uFlag );
		return;
	}
	
	int		i	= a;
	int		j	= 0;
	int		a2	= b * b;
	int		b2	= a * a;
	int		d	= a * b * b;
	int		f	= -2 * d + a2 + 2 * b2;
	int		h	= -4 * d + 2 * a2 + b2;
	
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
			f -= 4 * a2 * i;
			h -= 4 * a2 * i - 2 * a2;
		}
		if( h < 0 ){
			++j;
			f += 4 * b2 * j + 2 * b2;
			h += 4 * b2 * j;
		}
	}
	
	// Polygon 合成
	if( uFlag & IMG_FILL ) PolygonDraw( yc, uFlag );
}

// iStart, iEnd は 1周が 0x10000
#define DRAWARC_ROUND 0x10000
void CVsdFilter::DrawArc(
	int x, int y,
	int a, int b,
	int iStart, int iEnd,
	const PIXEL_YCA &yc, UINT uFlag
){
	int		i	= a;
	int		j	= 0;
	int		a2	= b * b;
	int		b2	= a * a;
	int		d	= a * b * b;
	int		f	= -2 * d + a2 + 2 * b2;
	int		h	= -4 * d + 2 * a2 + b2;
	
	int		iStX = ( int )( 1024 * a * abs( cos( 2 * M_PI / DRAWARC_ROUND * iStart )));
	int		iStY = ( int )( 1024 * b * abs( sin( 2 * M_PI / DRAWARC_ROUND * iStart )));
	int		iEdX = ( int )( 1024 * a * abs( cos( 2 * M_PI / DRAWARC_ROUND * iEnd )));
	int		iEdY = ( int )( 1024 * b * abs( sin( 2 * M_PI / DRAWARC_ROUND * iEnd )));
	
	int		iStArea	= ( iStart / ( DRAWARC_ROUND / 4 )) << 4;
	int		iEdArea	= ( iEnd   / ( DRAWARC_ROUND / 4 )) << 4;
	
	// Polygon クリア
	if( uFlag & IMG_FILL ){
		PolygonClear();
		uFlag |= IMG_POLYGON;
	}
	
	int	iAreaCmpS, iAreaCmpE;
	
	while( i >= 0 ){
		// (i,j) が iStar / iEnd の角度よりも大きい / 小さい を計算しておく
		iAreaCmpS = iStX * j - iStY * i;
		iAreaCmpS = iAreaCmpS == 0 ? 0 :
					iAreaCmpS >= 0 ? 1 : -1;
		iAreaCmpS = iStArea + (( iStArea & 0x10 ) ? iAreaCmpS : -iAreaCmpS );
		
		iAreaCmpE = iEdX * j - iEdY * i;
		iAreaCmpE = iAreaCmpE == 0 ? 0 :
					iAreaCmpE >= 0 ? 1 : -1;
		iAreaCmpE = iEdArea + (( iEdArea & 0x10 ) ? iAreaCmpE : -iAreaCmpE );
		
		if( iStart < iEnd ){
			// st && ed
			if( iAreaCmpS <= 0x00 && 0x00 <= iAreaCmpE ) PutPixel( x + i, y + j, yc, uFlag );
			if( iAreaCmpS <= 0x10 && 0x10 <= iAreaCmpE ) PutPixel( x - i, y + j, yc, uFlag );
			if( iAreaCmpS <= 0x20 && 0x20 <= iAreaCmpE ) PutPixel( x - i, y - j, yc, uFlag );
			if( iAreaCmpS <= 0x30 && 0x30 <= iAreaCmpE ) PutPixel( x + i, y - j, yc, uFlag );
		}else{
			// st || ed
			if( iAreaCmpS <= 0x00 || 0x00 <= iAreaCmpE ) PutPixel( x + i, y + j, yc, uFlag );
			if( iAreaCmpS <= 0x10 || 0x10 <= iAreaCmpE ) PutPixel( x - i, y + j, yc, uFlag );
			if( iAreaCmpS <= 0x20 || 0x20 <= iAreaCmpE ) PutPixel( x - i, y - j, yc, uFlag );
			if( iAreaCmpS <= 0x30 || 0x30 <= iAreaCmpE ) PutPixel( x + i, y - j, yc, uFlag );
		}
		
		if( f >= 0 ){
			--i;
			f -= 4 * a2 * i;
			h -= 4 * a2 * i - 2 * a2;
		}
		if( h < 0 ){
			++j;
			f += 4 * b2 * j + 2 * b2;
			h += 4 * b2 * j;
		}
	}
	
	// Polygon 合成
	if( uFlag & IMG_FILL ) PolygonDraw( yc, uFlag );
}

void CVsdFilter::DrawArc(
	int x, int y,
	int a, int b,
	int c, int d,
	int iStart, int iEnd,
	const PIXEL_YCA &yc, UINT uFlag
){
	int		iStX = ( int )( 1024 * a * abs( cos( 2 * M_PI / DRAWARC_ROUND * iStart )));
	int		iStY = ( int )( 1024 * b * abs( sin( 2 * M_PI / DRAWARC_ROUND * iStart )));
	int		iEdX = ( int )( 1024 * a * abs( cos( 2 * M_PI / DRAWARC_ROUND * iEnd )));
	int		iEdY = ( int )( 1024 * b * abs( sin( 2 * M_PI / DRAWARC_ROUND * iEnd )));
	
	int		iStArea	= ( iStart / ( DRAWARC_ROUND / 4 )) << 4;
	int		iEdArea	= ( iEnd   / ( DRAWARC_ROUND / 4 )) << 4;
	
	int	iAreaCmpS, iAreaCmpE;
	
	double		a2		= pow( a + 0.5, 2 );
	double		a2_b2	= a2 / pow( b + 0.5, 2 );
	double		c2		= pow( c + 0.5, 2 );
	double		c2_d2	= c2 / pow( d + 0.5, 2 );
	
	for( int j = 0; j <= b; ++j ){
		int iScanS = ( j <= d ) ? ( int )sqrt( c2 - c2_d2 * ( j * j )) : 0;
		int iScanE = ( int )sqrt( a2 - a2_b2 * ( j * j ));
		
		for( int i = iScanS; i <= iScanE; ++i ){
			
			// (i,j) が iStar / iEnd の角度よりも大きい / 小さい を計算しておく
			iAreaCmpS = iStX * j - iStY * i;
			iAreaCmpS = iAreaCmpS == 0 ? 0 :
						iAreaCmpS >= 0 ? 1 : -1;
			iAreaCmpS = iStArea + (( iStArea & 0x10 ) ? iAreaCmpS : -iAreaCmpS );
			
			iAreaCmpE = iEdX * j - iEdY * i;
			iAreaCmpE = iAreaCmpE == 0 ? 0 :
						iAreaCmpE >= 0 ? 1 : -1;
			iAreaCmpE = iEdArea + (( iEdArea & 0x10 ) ? iAreaCmpE : -iAreaCmpE );
			
			if( iStart < iEnd ){
				// st && ed
				if(           iAreaCmpS <= 0x00 && 0x00 <= iAreaCmpE ) PutPixel( x + i, y + j, yc, uFlag );
				if( i &&      iAreaCmpS <= 0x10 && 0x10 <= iAreaCmpE ) PutPixel( x - i, y + j, yc, uFlag );
				if( i && j && iAreaCmpS <= 0x20 && 0x20 <= iAreaCmpE ) PutPixel( x - i, y - j, yc, uFlag );
				if(      j && iAreaCmpS <= 0x30 && 0x30 <= iAreaCmpE ) PutPixel( x + i, y - j, yc, uFlag );
			}else{
				// st || ed
				if(           iAreaCmpS <= 0x00 || 0x00 <= iAreaCmpE ) PutPixel( x + i, y + j, yc, uFlag );
				if( i &&      iAreaCmpS <= 0x10 || 0x10 <= iAreaCmpE ) PutPixel( x - i, y + j, yc, uFlag );
				if( i && j && iAreaCmpS <= 0x20 || 0x20 <= iAreaCmpE ) PutPixel( x - i, y - j, yc, uFlag );
				if(      j && iAreaCmpS <= 0x30 || 0x30 <= iAreaCmpE ) PutPixel( x + i, y - j, yc, uFlag );
			}
		}
	}
}

/*** DrawFont ***************************************************************/

void CVsdFilter::DrawFont( int x, int y, UCHAR c, CVsdFont *pFont, const PIXEL_YCA &yc, UINT uFlag ){
	
	int	i, j;
	PIXEL_YCA	yca;
	
	if( c < '!' || '~' < c ) return;
	
	tFontGlyph *pFontGlyph = &pFont->m_FontGlyph[ c - '!' ];
	int iBmpW = ( pFontGlyph->iW + 3 ) & ~3;
	int iOrgX = ( pFont->m_iFontW - pFontGlyph->iW ) / 2;
	
	for( j = 0; j < pFontGlyph->iH; ++j ) for( i = 0; i < pFontGlyph->iW; ++i ){
		int iAlfa = 256 - ( pFontGlyph->pBuf[ iBmpW * j + i ] << 2 );
		
		yca 		= yc;
		yca.alfa	= iAlfa;
		yca.y		= yca.y  * ( 256 - iAlfa ) / 256;
		yca.cr		= yca.cr * ( 256 - iAlfa ) / 256;
		yca.cb		= yca.cb * ( 256 - iAlfa ) / 256;
		PutPixel( x + iOrgX + i, y + pFontGlyph->iOrgY + j, yca, uFlag );
	}
}

void CVsdFilter::DrawFont( int x, int y, UCHAR c, CVsdFont *pFont, const PIXEL_YCA &yc, const PIXEL_YCA &ycEdge, UINT uFlag ){
	
	if( c == ' ' ) return;
	
	DrawFont( x + 1, y + 0, c, pFont, ycEdge, uFlag );
	DrawFont( x - 1, y + 0, c, pFont, ycEdge, uFlag );
	DrawFont( x + 0, y + 1, c, pFont, ycEdge, uFlag );
	DrawFont( x + 0, y - 1, c, pFont, ycEdge, uFlag );
	DrawFont( x + 0, y + 0, c, pFont, yc,     uFlag );
}

/*** DrawString *************************************************************/

void CVsdFilter::DrawString( char *szMsg, CVsdFont *pFont, const PIXEL_YCA &yc, UINT uFlag, int x, int y ){
	
	if( x != POS_DEFAULT ) m_iPosX = x;
	if( y != POS_DEFAULT ) m_iPosY = y;
	
	for( int i = 0; szMsg[ i ]; ++i ){
		DrawFont( m_iPosX + i * pFont->GetW(), m_iPosY, szMsg[ i ], pFont, yc, uFlag );
	}
	
	m_iPosY += pFont->GetH();
}

void CVsdFilter::DrawString( char *szMsg, CVsdFont *pFont, const PIXEL_YCA &yc, const PIXEL_YCA &ycEdge, UINT uFlag, int x, int y ){
	
	if( x != POS_DEFAULT ) m_iPosX = x;
	if( y != POS_DEFAULT ) m_iPosY = y;
	
	for( int i = 0; szMsg[ i ]; ++i ){
		DrawFont( m_iPosX + i * pFont->GetW(), m_iPosY, szMsg[ i ], pFont, yc, ycEdge, uFlag );
	}
	
	m_iPosY += pFont->GetH();
}

/*** put pixel 系 ***********************************************************/

inline void CVsdFilter::PutPixel( int x, int y, const PIXEL_YCA &yc, UINT uFlag ){
	
	if( uFlag & IMG_POLYGON ){
		// ポリゴン描画
		if( x > m_Polygon[ y ].iRight ) m_Polygon[ y ].iRight = x;
		if( x < m_Polygon[ y ].iLeft  ) m_Polygon[ y ].iLeft  = x;
	}else if( 0 <= x && x < GetWidth() && 0 <= y && y < GetHeight() && yc.alfa != 256 ){
		PutPixelLow( x, y, yc, uFlag );
	}
}

inline void CVsdFilter::FillLine( int x1, int y1, int x2, const PIXEL_YCA &yc, UINT uFlag ){
	
	if( uFlag & IMG_POLYGON ){
		// ポリゴン描画
		if( x1 > x2 ){
			if( x1 > m_Polygon[ y1 ].iRight ) m_Polygon[ y1 ].iRight = x1;
			if( x2 < m_Polygon[ y1 ].iLeft  ) m_Polygon[ y1 ].iLeft  = x2;
		}else{
			if( x2 > m_Polygon[ y1 ].iRight ) m_Polygon[ y1 ].iRight = x2;
			if( x1 < m_Polygon[ y1 ].iLeft  ) m_Polygon[ y1 ].iLeft  = x1;
		}
	}else if( 0 <= y1 && y1 < GetHeight() && yc.alfa != 256 ){
		if( x1 < 0 )         x1 = 0;
		if( x2 > GetWidth()) x2 = GetWidth();
		
		FillLineLow( x1, y1, x2, yc, uFlag );
	}
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

/****************************************************************************/
/*** メーター描画 ***********************************************************/
/****************************************************************************/

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
static const PIXEL_YCA	yc_blue2		= RGB2YCA(1024,    0, 2688, 0x80 );
static const PIXEL_YCA	yc_orange		= RGB2YC( 4095, 1024,    0 );
//static const PIXEL_YCA	yc_gray_a		= RGB2YCA( 2048, 2048, 2048, 0x80 );
static const PIXEL_YCA	yc_gray_a		= RGB2YCA( 1024, 1024, 1024, 0x80 );

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

static char g_szBuf[ BUF_SIZE ];

/*** パラメータ調整用スピードグラフ *****************************************/

#define SPEED_GRAPH_SCALE	2

void CVsdFilter::DrawSpeedGraph(
	CVsdLog *Log,
	int iX, int iY, int iW, int iH,
	const PIXEL_YCA &yc,
	int iDirection
){
	
	int	iLogNum;
	int	x = iX;
	int iCursor = 0;
	
	iLogNum = Log->m_iLogNum - iW * SPEED_GRAPH_SCALE / 2;
	if( iLogNum < 0 ){
		x = iX - iLogNum / SPEED_GRAPH_SCALE;
		iLogNum = 0;
	}
	
	for( ; x < iX + iW - 1 && iLogNum < Log->m_iCnt - 1; ++x, iLogNum += SPEED_GRAPH_SCALE ){
		DrawLine(
			x,     iY + iH - 1 - ( int )Log->Speed( iLogNum )                     * iH / Log->m_iMaxSpeed,
			x + 1, iY + iH - 1 - ( int )Log->Speed( iLogNum + SPEED_GRAPH_SCALE ) * iH / Log->m_iMaxSpeed,
			1, yc, 0
		);
		
		if( x == iX + iW / 2 ) iCursor = iLogNum;
	}
	
	int	iVal = ( int )Log->Speed( iCursor );
	iY = iY + iH - 1 - iH * iVal / Log->m_iMaxSpeed;
	x  = iX + iW / 2;
	DrawLine(
		x, iY,
		x + ( iDirection ? -10 : 10 ), iY - 10,
		1, yc, 0
	);
	
	sprintf( g_szBuf, "%d km/h", iVal );
	DrawString( g_szBuf, m_pFontS, yc, yc_black, 0,
		x + ( iDirection ? ( -10 - strlen( g_szBuf ) * m_pFontS->GetW()): 10 ),
		iY - 10 - m_pFontS->GetH() );
}

void CVsdFilter::DrawTachoGraph(
	CVsdLog *Log,
	int iX, int iY, int iW, int iH,
	const PIXEL_YCA &yc,
	int iDirection
){
	
	int	iLogNum;
	int	x = iX;
	int iCursor = 0;
	
	iLogNum = Log->m_iLogNum - iW * SPEED_GRAPH_SCALE / 2;
	if( iLogNum < 0 ){
		x = iX - iLogNum / SPEED_GRAPH_SCALE;
		iLogNum = 0;
	}
	
	for( ; x < iX + iW - 1 && iLogNum < Log->m_iCnt - 1; ++x, iLogNum += SPEED_GRAPH_SCALE ){
		DrawLine(
			x,     iY + iH - 1 - ( int )Log->Tacho( iLogNum )                     * iH / 7000,
			x + 1, iY + iH - 1 - ( int )Log->Tacho( iLogNum + SPEED_GRAPH_SCALE ) * iH / 7000,
			1, yc, 0
		);
		
		if( x == iX + iW / 2 ) iCursor = iLogNum;
	}
	
	int	iVal = ( int )Log->Tacho( iCursor );
	iY = iY + iH - 1 - iH * iVal / 7000;
	x  = iX + iW / 2;
	DrawLine(
		x, iY,
		x + ( iDirection ? -10 : 10 ), iY - 10,
		1, yc, 0
	);
	
	sprintf( g_szBuf, "%d rpm", iVal );
	DrawString( g_szBuf, m_pFontS, yc, yc_black, 0,
		x + ( iDirection ? ( -10 - strlen( g_szBuf ) * m_pFontS->GetW()): 10 ),
		iY - 10 - m_pFontS->GetH() );
}

/*** メーター等描画 *********************************************************/

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
	int	i;
	
	// スクリプトロード
	if( !m_Script ){
		m_Script = new CScript( this );
		m_Script->Load( "d:\\dds\\vsd\\vsd_filter\\z.js" );
		m_Script->Run();
		m_Script->RunFunction( "Initialize" );
	}
	m_Script->RunFunction( "Draw" );
	
	// フォントサイズ初期化
	int iFontSize = m_piParamS[ SHADOW_FONT_SIZE ] > 0 ?
		m_piParamS[ SHADOW_FONT_SIZE ] :
		GetHeight() * 21 / 480;
	
	if( m_pFontM == NULL || iFontSize != -m_logfont.lfHeight ){
		m_logfont.lfHeight = -iFontSize * 2 / 3;
		if( m_pFontS ) delete m_pFontS;
		m_pFontS = new CVsdFont( m_logfont );
		
		m_logfont.lfHeight = -iFontSize * 2;
		if( m_pFontL ) delete m_pFontL;
		m_pFontL = new CVsdFont( m_logfont );
		
		m_logfont.lfHeight = -iFontSize;
		if( m_pFontM ) delete m_pFontM;
		m_pFontM = new CVsdFont( m_logfont );
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
	
	SelectLogForLapTime;
	
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
	
	DrawLapTime();
	
	if(
		#ifdef GPS_ONLY
			DispGraph
		#else
			DispGraph || DispSyncInfo
		#endif
	){
		// グラフ
		const int iGraphW = GetWidth()  * 66 / 100;
		const int iGraphH = GetHeight() * 23 / 100;
		int       iGraphX = GetWidth()  *  2 / 100; if( !MeterPosRight ) iGraphX = GetWidth() - iGraphX - iGraphW;
		const int iGraphY = GetHeight() * 75 / 100;
		
		DrawRect(
			iGraphX, iGraphY,
			iGraphX + iGraphW - 1,
			iGraphY + iGraphH - 1,
			COLOR_PANEL, CVsdFilter::IMG_FILL
		);
		
		if( m_VsdLog ){
			DrawSpeedGraph(
				m_VsdLog,
				iGraphX, iGraphY,
				iGraphW, iGraphH,
				yc_orange, 1
			);
			if( !DispSyncInfo ) DrawTachoGraph(
				m_VsdLog,
				iGraphX, iGraphY,
				iGraphW, iGraphH,
				yc_cyan, 0
			);
		}
		
		if(( !m_VsdLog || DispSyncInfo ) && m_GPSLog ){
			DrawSpeedGraph(
				m_GPSLog,
				iGraphX, iGraphY,
				iGraphW, iGraphH,
				yc_cyan, 0
			);
		}
	}
	
	// MAP 表示
	DrawMap(
		8, 8, MAX_MAP_SIZE,
		COLOR_CURRENT_POS, yc_yellow, yc_green, yc_red
	);
	
	SelectLogVsd;
	if( Log ){
		switch( m_piParamC[ CHECK_PanelDesign ] ){
			case 0: DrawMeterPanel0(); break;
			case 1: DrawMeterPanel1(); break;
		}
	}
	
	// フレーム表示
	
	#define Float2Time( n )	( int )( n ) / 60, fmod( n, 60 )
	
	if( DispSyncInfo ){
		
		m_iPosX = 0;
		m_iPosY = GetHeight() / 3;
		
		if( m_GPSLog ){
			i = ( int )(( m_GPSLog->m_dLogStartTime + m_GPSLog->m_dLogNum / LOG_FREQ ) * 100 ) % ( 24 * 3600 * 100 );
			sprintf(
				g_szBuf, "GPS time: %02d:%02d:%02d.%02d",
				i / 360000,
				i / 6000 % 60,
				i /  100 % 60,
				i        % 100
			);
			DrawString( g_szBuf, m_pFontS, COLOR_STR, COLOR_TIME_EDGE, 0, 0 );
		}
		
		#ifndef GPS_ONLY
			DrawString( "        start       end     range cur.pos", m_pFontS, COLOR_STR, COLOR_TIME_EDGE, 0 );
			
			sprintf(
				g_szBuf, "Vid%4d:%05.2f%4d:%05.2f%4d:%05.2f%7d",
				Float2Time( VideoSt / GetFPS()),
				Float2Time( VideoEd / GetFPS()),
				Float2Time(( VideoEd - VideoSt ) / GetFPS()),
				GetFrameCnt()
			);
			DrawString( g_szBuf, m_pFontS, COLOR_STR, COLOR_TIME_EDGE, 0 );
			
			if( m_VsdLog ){
				sprintf(
					g_szBuf, "Log%4d:%05.2f%4d:%05.2f%4d:%05.2f%7d",
					Float2Time( LogSt / m_VsdLog->m_dFreq ),
					Float2Time( LogEd / m_VsdLog->m_dFreq ),
					Float2Time(( LogEd - LogSt ) / m_VsdLog->m_dFreq ),
					m_VsdLog->m_iLogNum
				);
				DrawString( g_szBuf, m_pFontS, COLOR_STR, COLOR_TIME_EDGE, 0 );
			}
			
			if( m_GPSLog ){
				sprintf(
					g_szBuf, "GPS%4d:%05.2f%4d:%05.2f%4d:%05.2f%7d",
					Float2Time( GPSSt / m_GPSLog->m_dFreq ),
					Float2Time( GPSEd / m_GPSLog->m_dFreq ),
					Float2Time(( GPSEd - GPSSt ) / m_GPSLog->m_dFreq ),
					m_GPSLog->m_iLogNum
				);
				DrawString( g_szBuf, m_pFontS, COLOR_STR, COLOR_TIME_EDGE, 0 );
			}
		#endif	// !GPS_ONLY
	}
	
	return TRUE;
}

/*** ラップタイム表示 *******************************************************/

void CVsdFilter::DrawLapTime(){
	
	BOOL	bInLap = FALSE;	// ラップタイム計測中
	int	i;
	
	if( !DispLap || !m_iLapNum ) return;
	
	CVsdLog *Log;
	SelectLogForLapTime;
	
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
		
		sprintf( g_szBuf, "Time%2d'%02d.%03d", iTime / 60000, iTime / 1000 % 60, iTime % 1000 );
		DrawString( g_szBuf, m_pFontM, COLOR_TIME, COLOR_TIME_EDGE, 0, GetWidth() - m_pFontM->GetW() * 13, 1 );
		bInLap = TRUE;
	}else{
		// まだ開始していない
		DrawString( "Time -'--.---", m_pFontM, COLOR_TIME, COLOR_TIME_EDGE, 0, GetWidth() - m_pFontM->GetW() * 13, 1 );
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
				g_szBuf, "    %c%d'%02d.%03d",
				bSign ? '-' : '+',
				iDiffTime / 60000,
				iDiffTime / 1000 % 60,
				iDiffTime % 1000
			);
			DrawString( g_szBuf, m_pFontM, bSign ? COLOR_DIFF_MINUS : COLOR_DIFF_PLUS, COLOR_TIME_EDGE, 0 );
		}else{
			m_iPosY += m_pFontM->GetH();
		}
	}
	
	m_iPosY += m_pFontM->GetH() / 4;
	
	// Best 表示
	sprintf(
		g_szBuf, "Best%2d'%02d.%03d",
		m_iBestTime / 60000,
		m_iBestTime / 1000 % 60,
		m_iBestTime % 1000
	);
	DrawString( g_szBuf, m_pFontM, COLOR_TIME, COLOR_TIME_EDGE, 0 );
	
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
					g_szBuf, "%3d%c%2d'%02d.%03d",
					m_Lap[ iLapIdxStart ].uLap,
					( iLapIdxStart == m_iLapIdx + 1 && bInLap ) ? '*' : ' ',
					m_Lap[ iLapIdxStart ].iTime / 60000,
					m_Lap[ iLapIdxStart ].iTime / 1000 % 60,
					m_Lap[ iLapIdxStart ].iTime % 1000
				);
				DrawString( g_szBuf, m_pFontM,
					m_iBestTime == m_Lap[ iLapIdxStart ].iTime ? COLOR_BEST_LAP : COLOR_TIME,
					COLOR_TIME_EDGE, 0
				);
				++i;
			}
		}
	}
}

/*** G スネーク描画 *********************************************************/

void CVsdFilter::DrawGSnake(
	int iCx, int iCy, int iR,
	const PIXEL_YCA &ycBall, const PIXEL_YCA &ycLine
){
	int	iGx, iGy;
	int	i;
	
	CVsdLog *Log;
	SelectLogVsd;
	
	if( GSnakeLen > 0 ){
		
		int iGxPrev = INVALID_POS_I, iGyPrev;
		
		for( i = -( int )( GSnakeLen * LOG_FREQ / 10.0 ) ; i <= 1 ; ++i ){
			
			if( Log->m_iLogNum + i >= 0 ){
				// i == 1 時は最後の中途半端な LogNum
				iGx = ( int )((( i != 1 ) ? Log->m_Log[ Log->m_iLogNum + i ].fGx : Log->Gx()) * iR );
				iGy = ( int )((( i != 1 ) ? Log->m_Log[ Log->m_iLogNum + i ].fGy : Log->Gy()) * iR );
				
				iGx = ( int )( iGx * AspectRatio );
				
				if( iGxPrev != INVALID_POS_I ) DrawLine(
					iCx + iGx, iCy - iGy, iCx + iGxPrev, iCy - iGyPrev,
					LINE_WIDTH, ycLine, 0
				);
				
				iGxPrev = iGx;
				iGyPrev = iGy;
			}
		}
	}else{
		iGx = ( int )( Log->Gx() * iR * AspectRatio );
		iGy = ( int )( Log->Gy() * iR );
	}
	
	// G インジケータ
	DrawCircle(
		iCx + iGx, iCy - iGy, iR / 20,
		ycBall, CVsdFilter::IMG_FILL
	);
}

/*** 走行軌跡表示 ***********************************************************/

void CVsdFilter::DrawMap(
	int iX, int iY, int iSize,
	const PIXEL_YCA &ycIndicator,
	const PIXEL_YCA &ycG0,
	const PIXEL_YCA &ycGPlus,
	const PIXEL_YCA &ycGMinus
){
	double dGx, dGy;
	int	iGx, iGy;
	int i;
	
	CVsdLog *Log;
	SelectLogGPS;
	
	if( !LineTrace || !Log || !Log->IsDataExist()) return;
	
	int iGxPrev = INVALID_POS_I, iGyPrev;
	
	int iLineSt = ( int )LapNum2LogNum( Log, m_iLapIdx );
	if( Log->m_iLogNum - iLineSt > ( int )( LineTrace * LOG_FREQ ))
		iLineSt = Log->m_iLogNum - ( int )( LineTrace * LOG_FREQ );
	
	int iLineEd = m_iLapIdx != m_iLapNum - 1
		? ( int )LapNum2LogNum( Log, m_iLapIdx + 1 ) : Log->m_iCnt - 1;
	
	if( iLineEd - Log->m_iLogNum > ( int )( LineTrace * LOG_FREQ ))
		iLineEd = Log->m_iLogNum + ( int )( LineTrace * LOG_FREQ );
	
	for( i = iLineSt; i <= iLineEd ; ++i ){
		if( !_isnan( Log->X( i ))){
			#define GetMapPos( p, a ) ((( p ) - Log->m_dMapOffs ## a ) / Log->m_dMapSize * iSize )
			iGx = iX + ( int )GetMapPos( Log->X( i ), X );
			iGy = iY + ( int )GetMapPos( Log->Y( i ), Y );
			
			if( iGxPrev != INVALID_POS_I ){
				// Line の色用に G を求める
				double dG = Log->Gy( i );
				
				PIXEL_YCA yc_line;
				
				if( dG >= 0.0 ){
					BlendColor( yc_line, ycG0, ycGPlus,  dG / Log->m_dMaxG );
				}else{
					BlendColor( yc_line, ycG0, ycGMinus, dG / Log->m_dMinG );
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
	dGx = iX + GetMapPos( Log->X(), X );
	dGy = iY + GetMapPos( Log->Y(), Y );
	
	if( !_isnan( dGx )) DrawCircle(
		( int )( dGx * AspectRatio ), ( int )dGy, 5 * GetHeight() / 480,
		ycIndicator, CVsdFilter::IMG_FILL
	);
	
	// スタートライン表示
	if( DispSyncInfo && m_iLapMode == LAPMODE_GPS ){
		double dAngle = m_piParamT[ TRACK_MapAngle ] * ( -ToRAD / 10 );
		
		int x1 = iX + ( int )((  cos( dAngle ) * m_dStartLineX1 + sin( dAngle ) * m_dStartLineY1 - Log->m_dMapOffsX ) / Log->m_dMapSize * iSize );
		int y1 = iY + ( int )(( -sin( dAngle ) * m_dStartLineX1 + cos( dAngle ) * m_dStartLineY1 - Log->m_dMapOffsY ) / Log->m_dMapSize * iSize );
		int x2 = iX + ( int )((  cos( dAngle ) * m_dStartLineX2 + sin( dAngle ) * m_dStartLineY2 - Log->m_dMapOffsX ) / Log->m_dMapSize * iSize );
		int y2 = iY + ( int )(( -sin( dAngle ) * m_dStartLineX2 + cos( dAngle ) * m_dStartLineY2 - Log->m_dMapOffsY ) / Log->m_dMapSize * iSize );
		
		DrawLine( x1, y1, x2, y2, yc_blue, 0 );
	}
}

/*** 針描画 *****************************************************************/

void CVsdFilter::DrawNeedle(
	int x, int y, int r,
	int iStart, int iEnd, double dVal,
	const PIXEL_YCA yc, int iWidth
){
	double dAngle = ( iStart + ( iEnd - iStart ) * dVal ) * ToRAD;
	
	DrawLine(
		x, y,
		( int )( cos( dAngle ) * r ) + x,
		( int )( sin( dAngle ) * r ) + y,
		iWidth, yc, 0
	);
}

/*** メーターパネル type 0 **************************************************/

void CVsdFilter::DrawMeterPanel0( void ){
	int	i;
	CVsdLog *Log;
	
	const int	iMeterR =
		m_piParamS[ SHADOW_METER_R  ] >= 0 ? m_piParamS[ SHADOW_METER_R  ] :
		100 * GetHeight() / 480;
	
	int	iMeterCx;
	if( m_piParamS[ SHADOW_METER_CX ] >= 0 ){
		iMeterCx = m_piParamS[ SHADOW_METER_CX ];
	}else if( MeterPosRight ){
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
	int	iMeterSMaxVal	= m_piParamS[ SHADOW_SPEED ];
	
	/*** メーターパネル ***/
	DrawCircle(
		iMeterCx, iMeterCy,
		#ifdef GPS_ONLY
			iMeterR * Aspect / 1000,
		#endif
		iMeterR,
		COLOR_PANEL, CVsdFilter::IMG_FILL
	);
	
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
					( int )( cos( iDeg * ToRAD ) * ( iMeterR * 95 / 100 ) * AspectRatio ) + iMeterCx,
					( int )( sin( iDeg * ToRAD ) * ( iMeterR * 95 / 100 )) + iMeterCy,
					( iMeterMaxVal <= 12000 && i % 1000 == 0 || i % 2000 == 0 ) ? 2 : 1,
					COLOR_SCALE, 0
				);
				
				// メーターパネル目盛り数値
				if( iMeterMaxVal <= 12000 && i % 1000 == 0 || i % 2000 == 0 ){
					sprintf( g_szBuf, "%d", i / 1000 );
					DrawString(
						g_szBuf, m_pFontM,
						COLOR_STR, 0,
						( int )( cos( iDeg * ToRAD ) * iMeterR * .8 * AspectRatio ) + iMeterCx - m_pFontM->GetW() / ( i >= 10000 ? 1 : 2 ),
						( int )( sin( iDeg * ToRAD ) * iMeterR * .8 ) + iMeterCy - m_pFontM->GetH() / 2
					);
				}
			}
		}
	}else{
		// GPS ログ優先時はスピードメーターパネル
		if( !iMeterSMaxVal ) iMeterSMaxVal = Log->m_iMaxSpeed;
		
		int	iStep = (( iMeterSMaxVal / 18 ) + 4 ) / 5 * 5;
		if( iStep == 0 ) iStep = 5;
		
		for( i = 0; i <= iMeterSMaxVal; i += iStep ){
			int iDeg = iMeterDegRange * i / iMeterSMaxVal + iMeterMinDeg;
			
			// メーターパネル目盛り
			if( i % iStep == 0 ){
				DrawLine(
					( int )( cos( iDeg * ToRAD ) * iMeterR * AspectRatio ) + iMeterCx,
					( int )( sin( iDeg * ToRAD ) * iMeterR ) + iMeterCy,
					( int )( cos( iDeg * ToRAD ) * ( iMeterR * 95 / 100 ) * AspectRatio ) + iMeterCx,
					( int )( sin( iDeg * ToRAD ) * ( iMeterR * 95 / 100 )) + iMeterCy,
					( i % ( iStep * 2 ) == 0 ) ? 2 : 1,
					COLOR_SCALE, 0
				);
				
				// メーターパネル目盛り数値
				if( i % ( iStep * 2 ) == 0 ){
					sprintf( g_szBuf, "%d", i );
					DrawString(
						g_szBuf, m_pFontM,
						COLOR_STR, 0,
						( int )( cos( iDeg * ToRAD ) * iMeterR * .75 * AspectRatio ) + iMeterCx - m_pFontM->GetW() * strlen( g_szBuf ) / 2,
						( int )( sin( iDeg * ToRAD ) * iMeterR * .75 ) + iMeterCy - m_pFontM->GetH() / 2
					);
				}
			}
		}
	}
	
	/*** メーターデータ描画 ***/
	
	if( GSnakeLen >= 0 && Log->IsDataExist()){
		DrawGSnake(
			iMeterCx, iMeterCy, ( int )( iMeterR / GScale ),
			COLOR_G_SENSOR, COLOR_G_HIST
		);
	}
	
	// ギア表示 - VsdLog しか使用しない
	if( m_VsdLog && m_VsdLog->IsDataExist() ){
		int iGear = 0;
		
		if( m_VsdLog->Tacho() != 0 ){
			UINT uGearRatio = ( int )( m_VsdLog->Speed() * 100 * ( 1 << 8 ) / m_VsdLog->Tacho() );
			
			if     ( uGearRatio < GEAR_TH( 1 ))	iGear = 1;
			else if( uGearRatio < GEAR_TH( 2 ))	iGear = 2;
			else if( uGearRatio < GEAR_TH( 3 ))	iGear = 3;
			else if( uGearRatio < GEAR_TH( 4 ))	iGear = 4;
			else								iGear = 5;
		}
		
		sprintf( g_szBuf, "%d", iGear );
		DrawString(
			g_szBuf, m_pFontM,
			COLOR_STR, 0,
			iMeterCx - m_pFontM->GetW() / 2, iMeterCy - iMeterR / 2
		);
	}
	
	if( Log->IsDataExist()){
		// スピード表示
		sprintf( g_szBuf, "%3d", ( int )Log->Speed() );
		DrawString(
			g_szBuf, m_pFontM,
			COLOR_STR, 0,
			iMeterCx - 3 * m_pFontM->GetW() / 2, iMeterCy + iMeterR / 2
		);
		
		// G 数値
		if( GSnakeLen >= 0 ){
			sprintf( g_szBuf, "%02dG", ( int )( sqrt( Log->Gx() * Log->Gx() + Log->Gy() * Log->Gy()) * 10 ));
			DrawString(
				g_szBuf, m_pFontM,
				COLOR_STR, 0,
				iMeterCx - 3 * m_pFontM->GetW() / 2, iMeterCy + iMeterR / 2 - m_pFontM->GetH()
			);
			
			int iDotSize = -m_logfont.lfHeight * 2 / 20 - 1;
			
			DrawString(
				".", m_pFontM,
				COLOR_STR, 0,
				iMeterCx - m_pFontM->GetW(),
				iMeterCy + iMeterR / 2 - m_pFontM->GetH()
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
	
	DrawCircle( iMeterCx, iMeterCy, iMeterR / 25, COLOR_NEEDLE, CVsdFilter::IMG_FILL );
}

/*** メーターパネル type 1 **************************************************/

void CVsdFilter::DrawMeterPanel1( void ){
	int	i;
	CVsdLog *Log;
	
	const int	iMeterR =
		m_piParamS[ SHADOW_METER_R  ] >= 0 ? m_piParamS[ SHADOW_METER_R  ] :
		100 * GetHeight() / 480;
	
	int	iMeterCx;
	if( m_piParamS[ SHADOW_METER_CX ] >= 0 ){
		iMeterCx = m_piParamS[ SHADOW_METER_CX ];
	}else if( MeterPosRight ){
		iMeterCx = GetWidth() - iMeterR * Aspect / 1000 - 2;
	}else{
		iMeterCx = iMeterR * Aspect / 1000 + 1;
	}
	
	const int	iMeterCy =
		m_piParamS[ SHADOW_METER_CY ] >= 0 ? m_piParamS[ SHADOW_METER_CY ] :
		GetHeight() - iMeterR - 2;
	
	const int	iMeterMinDeg	= 90;
	const int	iMeterMaxDeg	= 0;
	const int	iMeterMaxVal	= 7000;
	const int	iMeterRedZone	= 6500;
	const int	iMeterDegRange	= ( iMeterMaxDeg - iMeterMinDeg + 360 ) % 360;
	const int	iMeterRedZoneDeg= ( iMeterDegRange * iMeterRedZone / iMeterMaxVal + iMeterMinDeg ) % 360;
	int	iMeterSMaxVal	= m_piParamS[ SHADOW_SPEED ];
	
	/*** メーターパネル ***/
	/*
	DrawCircle(
		iMeterCx, iMeterCy,
		#ifdef GPS_ONLY
			iMeterR * Aspect / 1000,
		#endif
		iMeterR,
		COLOR_PANEL, CVsdFilter::IMG_FILL
	);*/
	
	DrawArc(
		iMeterCx, iMeterCy,
		iMeterR * Aspect * 100 / 100 / 1000, iMeterR * 100 / 100,
		iMeterR * Aspect *  70 / 100 / 1000, iMeterR *  70 / 100,
		iMeterMinDeg * DRAWARC_ROUND / 360, iMeterMaxDeg * DRAWARC_ROUND / 360,
		yc_blue2, CVsdFilter::IMG_FILL
	);
	
	SelectLogVsd;
	
	if( Log == m_VsdLog ){
		
		/*
		if( m_VsdLog->IsDataExist()){
			// Tacho の針 - VsdLog しか使用しない
			DrawArc(
				iMeterCx, iMeterCy,
				iMeterR * Aspect * 95 / 100 / 1000, iMeterR * 95 / 100,
				iMeterR * Aspect * 70 / 100 / 1000, iMeterR * 70 / 100,
				iMeterMinDeg * DRAWARC_ROUND / 360,
				( int )(( iMeterDegRange * m_VsdLog->Tacho() / iMeterMaxVal + iMeterMinDeg ) * DRAWARC_ROUND / 360 ),
				yc_cyan, CVsdFilter::IMG_FILL
			);
		}
		*/
		
		// レッドゾーン
		DrawArc(
			iMeterCx, iMeterCy,
			iMeterR * Aspect * 100 / 100 / 1000, iMeterR * 95 / 100,
			iMeterR * Aspect *  95 / 100 / 1000, iMeterR * 90 / 100,
			iMeterRedZoneDeg * DRAWARC_ROUND / 360, iMeterMaxDeg * DRAWARC_ROUND / 360,
			yc_orange, CVsdFilter::IMG_FILL
		);
		
		// VSD ログがあるときはタコメータ
		for( i = 0; i <= iMeterMaxVal; i += 500 ){
			int iDeg = iMeterDegRange * i / iMeterMaxVal + iMeterMinDeg;
			
			// メーターパネル目盛り
			if( iMeterMaxVal <= 12000 || i % 1000 == 0 ){
				DrawLine(
					( int )( cos( iDeg * ToRAD ) * ( iMeterR * 100 / 100 ) * AspectRatio ) + iMeterCx,
					( int )( sin( iDeg * ToRAD ) * ( iMeterR * 100 / 100 )) + iMeterCy,
					( int )( cos( iDeg * ToRAD ) * ( iMeterR *  95 / 100 ) * AspectRatio ) + iMeterCx,
					( int )( sin( iDeg * ToRAD ) * ( iMeterR *  95 / 100 )) + iMeterCy,
					( iMeterMaxVal <= 12000 && i % 1000 == 0 || i % 2000 == 0 ) ? 2 : 1,
					COLOR_SCALE, 0
				);
				
				// メーターパネル目盛り数値
				if( iMeterMaxVal <= 12000 && i % 1000 == 0 || i % 2000 == 0 ){
					sprintf( g_szBuf, "%d", i / 1000 );
					DrawString(
						g_szBuf, m_pFontM,
						COLOR_STR, 0,
						( int )( cos( iDeg * ToRAD ) * iMeterR * .825 * AspectRatio ) + iMeterCx - m_pFontM->GetW() / ( i >= 10000 ? 1 : 2 ),
						( int )( sin( iDeg * ToRAD ) * iMeterR * .825 ) + iMeterCy - m_pFontM->GetH() / 2
					);
				}
			}
		}
		
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
		// GPS ログ優先時はスピードメーターパネル
		if( !iMeterSMaxVal ) iMeterSMaxVal = Log->m_iMaxSpeed;
		
		int	iStep = (( iMeterSMaxVal / 26 ) + 4 ) / 5 * 5;
		if( iStep == 0 ) iStep = 5;
		
		for( i = 0; i <= iMeterSMaxVal; i += iStep ){
			int iDeg = iMeterDegRange * i / iMeterSMaxVal + iMeterMinDeg;
			
			// メーターパネル目盛り
			if( i % iStep == 0 ){
				DrawLine(
					( int )( cos( iDeg * ToRAD ) * ( iMeterR * 100 / 100 ) * AspectRatio ) + iMeterCx,
					( int )( sin( iDeg * ToRAD ) * ( iMeterR * 100 / 100 )) + iMeterCy,
					( int )( cos( iDeg * ToRAD ) * ( iMeterR *  95 / 100 ) * AspectRatio ) + iMeterCx,
					( int )( sin( iDeg * ToRAD ) * ( iMeterR *  95 / 100 )) + iMeterCy,
					( i % ( iStep * 2 ) == 0 ) ? 2 : 1,
					COLOR_SCALE, 0
				);
				
				// メーターパネル目盛り数値
				if( i % ( iStep * 2 ) == 0 ){
					sprintf( g_szBuf, "%d", i );
					DrawString(
						g_szBuf, m_pFontS,
						COLOR_STR, 0,
						( int )( cos( iDeg * ToRAD ) * iMeterR * .825 * AspectRatio ) + iMeterCx - m_pFontS->GetW() * strlen( g_szBuf ) / 2,
						( int )( sin( iDeg * ToRAD ) * iMeterR * .825 ) + iMeterCy - m_pFontS->GetH() / 2
					);
				}
			}
		}
		
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
	
	/*** G ウィンドウ ***/
	
	if( GSnakeLen >= 0 && Log->IsDataExist()){
		/*
		DrawRect(
			( int )(( iMeterCx + iMeterR * 10 / 100 ) * AspectRatio ),
			iMeterCy + iMeterR * 10 / 100,
			( int )(( iMeterCx + iMeterR ) * AspectRatio ),
			iMeterCy + iMeterR,
			COLOR_PANEL, CVsdFilter::IMG_FILL
		);
		*/
		
		DrawCircle(
			iMeterCx + ( int )(( iMeterR * 55 / 100 ) * AspectRatio ),
			iMeterCy + iMeterR * 55 / 100,
			#ifdef GPS_ONLY
				iMeterR * 45 / 100 * Aspect / 1000,
			#endif
			iMeterR * 45 / 100,
			COLOR_PANEL, CVsdFilter::IMG_FILL
		);
		
		DrawLine(
			iMeterCx + ( int )(( iMeterR * 10 / 100 ) * AspectRatio ),
			iMeterCy + iMeterR * 55 / 100,
			iMeterCx + ( int )(( iMeterR ) * AspectRatio ),
			iMeterCy + iMeterR * 55 / 100,
			1, yc_gray, 0
		);
		
		DrawLine(
			iMeterCx + ( int )(( iMeterR * 55 / 100 ) * AspectRatio ),
			iMeterCy + iMeterR * 10 / 100,
			iMeterCx + ( int )(( iMeterR * 55 / 100 ) * AspectRatio ),
			iMeterCy + iMeterR,
			1, yc_gray, 0
		);
		
		DrawGSnake(
			iMeterCx + ( int )(( iMeterR * 55 / 100 ) * AspectRatio ),
			iMeterCy + iMeterR * 55 / 100,
			( int )( iMeterR * 45 / 100 / GScale ),
			COLOR_G_SENSOR, COLOR_G_HIST
		);
	}
	
	/*** メーターデータ描画 ***/
	// ギア表示 - VsdLog しか使用しない
	if( m_VsdLog && m_VsdLog->IsDataExist() ){
		int iGear = 0;
		
		if( m_VsdLog->Tacho() != 0 ){
			UINT uGearRatio = ( int )( m_VsdLog->Speed() * 100 * ( 1 << 8 ) / m_VsdLog->Tacho() );
			
			if     ( uGearRatio < GEAR_TH( 1 ))	iGear = 1;
			else if( uGearRatio < GEAR_TH( 2 ))	iGear = 2;
			else if( uGearRatio < GEAR_TH( 3 ))	iGear = 3;
			else if( uGearRatio < GEAR_TH( 4 ))	iGear = 4;
			else								iGear = 5;
		}
		
		DrawRect(
			iMeterCx - m_pFontM->GetW() * 3 / 4,
			iMeterCy + iMeterR * 10 / 100,
			iMeterCx + m_pFontM->GetW() * 3 / 4,
			iMeterCy + iMeterR * 10 / 100 + m_pFontM->GetH(),
			yc_orange, CVsdFilter::IMG_FILL
		);
		
		sprintf( g_szBuf, "%d", iGear );
		DrawString(
			g_szBuf, m_pFontM,
			yc_black, 0,
			iMeterCx - m_pFontM->GetW() / 2,
			iMeterCy + iMeterR * 10 / 100
		);
	}
	
	if( Log->IsDataExist()){
		// スピード表示
		i = ( int )Log->Speed();
		sprintf( g_szBuf, "%d", i );
		DrawString(
			g_szBuf, m_pFontL,
			COLOR_STR, 0,
			iMeterCx - ( i >= 100 ? 3 : i >= 10 ? 2 : 1 ) * m_pFontL->GetW() / 2,
			iMeterCy - ( iMeterR * 35 / 100 ) - m_pFontL->GetH() / 2
		);
		
		DrawString(
			"km/h", m_pFontS,
			COLOR_STR, 0,
			iMeterCx - m_pFontS->GetW() * 4 / 2,
			iMeterCy - m_pFontS->GetH()
		);
		
		// G 数値
		if( GSnakeLen >= 0 ){
			sprintf( g_szBuf, "%.1fG", sqrt( Log->Gx() * Log->Gx() + Log->Gy() * Log->Gy()));
			DrawString(
				g_szBuf, m_pFontS,
				COLOR_STR, 0,
				iMeterCx + ( int )(( iMeterR ) * AspectRatio ) - 4 * m_pFontS->GetW(),
				iMeterCy + iMeterR - m_pFontS->GetH()
			);
		}
	}
}
