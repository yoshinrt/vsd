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

#define DEFAULT_FONT	"ＭＳ ゴシック"
// #define DEFAULT_FONT	"FixedSys"

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

void CVsdFilter::DrawLine( int x1, int y1, int x2, int y2, UINT uColor, UINT uFlag ){
	PIXEL_YCA yc( uColor );
	DrawLine( x1, y1, x2, y2, yc, uFlag );
}

void CVsdFilter::DrawLine(
	int x1, int y1, int x2, int y2,
	int width,
	UINT uColor,
	UINT uFlag
){
	
	PIXEL_YCA yc( uColor );
	
	for( int y = 0; y < width; ++y ) for( int x = 0; x < width; ++x ){
		DrawLine(
			x1 + x - width / 2, y1 + y - width / 2,
			x2 + x - width / 2, y2 + y - width / 2,
			yc, uFlag
		);
	}
}

/*** DrawRect ***************************************************************/

void CVsdFilter::DrawRect(
	int x1, int y1, int x2, int y2,
	UINT uColor, UINT uFlag
){
	int	y;
	
	if( y1 > y2 ) SWAP( y1, y2, y );
	if( x1 > x2 ) SWAP( x1, x2, y );
	
	for( y = y1; y <= y2; ++y ){
		FillLine( x1, y, x2, uColor, uFlag );
	}
}

/*** DrawCircle *************************************************************/

void CVsdFilter::DrawCircle(
	int x, int y, int r,
	UINT uColor,
	UINT uFlag
){
	
	int	i = r;
	int j = 0;
	int f = -2 * r + 3;
	
	PIXEL_YCA yc( uColor );
	
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
void CVsdFilter::DrawCircle( int x, int y, int a, int b, UINT uColor, UINT uFlag ){
	
	PIXEL_YCA yc( uColor );
	
	if( a == b ){
		DrawCircle( x, y, a, uColor, uFlag );
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

void CVsdFilter::DrawArc(
	int x, int y,
	int a, int b,
	double dStart, double dEnd,
	UINT uColor, UINT uFlag
){
	int		i	= a;
	int		j	= 0;
	int		a2	= b * b;
	int		b2	= a * a;
	int		d	= a * b * b;
	int		f	= -2 * d + a2 + 2 * b2;
	int		h	= -4 * d + 2 * a2 + b2;
	
	int		iStX = ( int )( 1024 * a * abs( cos( ToRAD * dStart )));
	int		iStY = ( int )( 1024 * b * abs( sin( ToRAD * dStart )));
	int		iEdX = ( int )( 1024 * a * abs( cos( ToRAD * dEnd )));
	int		iEdY = ( int )( 1024 * b * abs( sin( ToRAD * dEnd )));
	
	int		iStart	= ( int )dStart;
	int		iEnd	= ( int )dEnd;
	int		iStArea	= ( iStart / ( 360 / 4 )) << 4;
	int		iEdArea	= ( iEnd   / ( 360 / 4 )) << 4;
	
	
	PIXEL_YCA yc( uColor );
	
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
	double dStart, double dEnd,
	UINT uColor, UINT uFlag
){
	int		iStX = ( int )( 1024 * a * abs( cos( ToRAD * dStart )));
	int		iStY = ( int )( 1024 * b * abs( sin( ToRAD * dStart )));
	int		iEdX = ( int )( 1024 * a * abs( cos( ToRAD * dEnd )));
	int		iEdY = ( int )( 1024 * b * abs( sin( ToRAD * dEnd )));
	
	int		iStart	= ( int )dStart;
	int		iEnd	= ( int )dEnd;
	int		iStArea	= ( iStart / ( 360 / 4 )) << 4;
	int		iEdArea	= ( iEnd   / ( 360 / 4 )) << 4;
	
	int	iAreaCmpS, iAreaCmpE;
	
	double		a2		= pow( a + 0.5, 2 );
	double		a2_b2	= a2 / pow( b + 0.5, 2 );
	double		c2		= pow( c + 0.5, 2 );
	double		c2_d2	= c2 / pow( d + 0.5, 2 );
	
	PIXEL_YCA yc( uColor );
	
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

int CVsdFilter::DrawFont0( int x, int y, UCHAR c, CVsdFont &Font, UINT uColor ){
	
	int	i, j;
	
	// 文字幅を得る
	CFontGlyph &FontGlyph = Font.FontGlyph( c );
	
	int iBmpW = ( FontGlyph.iW + 3 ) & ~3;
	int iCellIncX = Font.IsFixed() ? Font.GetW() : FontGlyph.iCellIncX;
	int iOrgX = ( iCellIncX - FontGlyph.iW ) / 2;
	
	for( j = 0; j < FontGlyph.iH; ++j ) for( i = 0; i < FontGlyph.iW; ++i ){
		int iDensity = FontGlyph.pBuf[ iBmpW * j + i ];	// 0～64
		
		if( iDensity ){
			UINT uColorTmp = ( uColor & 0xFFFFFF ) | (
				(
					(
						( 0x3FFF << 6 ) - iDensity * ( 0xFF - ( uColor >> 24 ))
					) & ~0x3F
				) << ( 24 - 6 )
			);
			PutPixel( x + iOrgX + i, y + FontGlyph.iOrgY + j, uColorTmp, 0 );
		}
	}
	
	return iCellIncX;
}

int CVsdFilter::DrawFont( int x, int y, UCHAR c, CVsdFont &Font, UINT uColor, UINT uColorOutline ){
	
	// フォントが存在しない文字なら，space の文字幅を返す
	if( !CVsdFont::ExistFont( c ))
		return ( Font.IsFixed()) ? Font.GetW() : Font.GetW_Space();
	
	if( Font.IsOutline()){
		DrawFont0( x + 1, y + 0, c, Font, uColorOutline );
		DrawFont0( x - 1, y + 0, c, Font, uColorOutline );
		DrawFont0( x + 0, y + 1, c, Font, uColorOutline );
		DrawFont0( x + 0, y - 1, c, Font, uColorOutline );
	}
	return DrawFont0( x, y, c, Font, uColor );
}

/*** DrawText *************************************************************/

void CVsdFilter::DrawText( int x, int y, char *szMsg, CVsdFont &Font, UINT uColor, UINT uColorOutline ){
	
	if( x != POS_DEFAULT ) m_iPosX = x;
	if( y != POS_DEFAULT ) m_iPosY = y;
	
	x = m_iPosX;
	
	for( int i = 0; szMsg[ i ]; ++i ){
		x += DrawFont( x, m_iPosY, szMsg[ i ], Font, uColor, uColorOutline );
	}
	
	m_iPosY += Font.GetH();
}

/*** put pixel 系 ***********************************************************/

void CVsdFilter::PutPixel( int x, int y, UINT uColor, UINT uFlag ){
	PIXEL_YCA	yc( uColor );
	PutPixel( x, y, yc, uFlag );
}

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

inline UINT CVsdFilter::BlendColor(
	UINT uColor0,
	UINT uColor1,
	double	dAlfa
){
	if     ( dAlfa < 0.0 ) dAlfa = 0.0;
	else if( dAlfa > 1.0 ) dAlfa = 1.0;
	
	return
		(( UINT )(( uColor1 & 0xFF0000 ) * dAlfa + ( uColor0 & 0xFF0000 ) * ( 1 - dAlfa )) & 0xFF0000 ) +
		(( UINT )(( uColor1 & 0x00FF00 ) * dAlfa + ( uColor0 & 0x00FF00 ) * ( 1 - dAlfa )) & 0x00FF00 ) +
		(( UINT )(( uColor1 & 0x0000FF ) * dAlfa + ( uColor0 & 0x0000FF ) * ( 1 - dAlfa )) & 0x0000FF );
}

/****************************************************************************/
/*** メーター描画 ***********************************************************/
/****************************************************************************/
							//  AARRGGBB
static UINT	color_black			= 0x00000000;
static UINT	color_white			= 0x00FFFFFF;
static UINT	color_red			= 0x00FF0000;
static UINT	color_blue			= 0x000000FF;
static UINT	color_cyan			= 0x0000FFFF;
static UINT	color_orange		= 0x00FF4000;
static UINT	color_gray_a		= 0x80404040;

#define COLOR_PANEL			color_gray_a
#define COLOR_SCALE			color_white
#define COLOR_STR			COLOR_SCALE

static char g_szBuf[ BUF_SIZE ];

/*** パラメータ調整用スピードグラフ *****************************************/

#define SPEED_GRAPH_SCALE	2

void CVsdFilter::DrawSpeedGraph(
	CVsdLog *Log,
	int iX, int iY, int iW, int iH,
	UINT uColor,
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
			1, uColor, 0
		);
		
		if( x == iX + iW / 2 ) iCursor = iLogNum;
	}
	
	int	iVal = ( int )Log->Speed( iCursor );
	iY = iY + iH - 1 - iH * iVal / Log->m_iMaxSpeed;
	x  = iX + iW / 2;
	DrawLine(
		x, iY,
		x + ( iDirection ? -10 : 10 ), iY - 10,
		1, uColor, 0
	);
	
	sprintf( g_szBuf, "%d km/h", iVal );
	DrawText(
		x + ( iDirection ? ( -10 - m_pFont->GetTextWidth( g_szBuf )) : 10 ),
		iY - 10 - m_pFont->GetH(),
		g_szBuf, *m_pFont, uColor, color_black
	);
}

void CVsdFilter::DrawTachoGraph(
	CVsdLog *Log,
	int iX, int iY, int iW, int iH,
	UINT uColor,
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
			1, uColor, 0
		);
		
		if( x == iX + iW / 2 ) iCursor = iLogNum;
	}
	
	int	iVal = ( int )Log->Tacho( iCursor );
	iY = iY + iH - 1 - iH * iVal / 7000;
	x  = iX + iW / 2;
	DrawLine(
		x, iY,
		x + ( iDirection ? -10 : 10 ), iY - 10,
		1, uColor, 0
	);
	
	sprintf( g_szBuf, "%d rpm", iVal );
	DrawText(
		x + ( iDirection ? ( -10 - m_pFont->GetTextWidth( g_szBuf )): 10 ),
		iY - 10 - m_pFont->GetH(),
		g_szBuf, *m_pFont, uColor, color_black
	);
}

/*** G スネーク描画 *********************************************************/

void CVsdFilter::DrawGSnake(
	int iCx, int iCy, int iR, int iIndicatorR, int iWidth,
	UINT uColorBall, UINT uColorLine
){
	int	iGx, iGy;
	int	i;
	
	CVsdLog *Log;
	SelectLogVsd;
	
	if( Log ){
		if( GSnakeLen > 0 ){
			
			int iGxPrev = INVALID_POS_I, iGyPrev;
			
			for( i = -( int )( GSnakeLen * LOG_FREQ / 10.0 ) ; i <= 1 ; ++i ){
				
				if( Log->m_iLogNum + i >= 0 ){
					// i == 1 時は最後の中途半端な LogNum
					iGx = ( int )((( i != 1 ) ? Log->m_Log[ Log->m_iLogNum + i ].fGx : Log->Gx()) * iR );
					iGy = ( int )((( i != 1 ) ? Log->m_Log[ Log->m_iLogNum + i ].fGy : Log->Gy()) * iR );
					
					iGx = ( int )( iGx );
					
					if( iGxPrev != INVALID_POS_I ) DrawLine(
						iCx + iGx, iCy - iGy, iCx + iGxPrev, iCy - iGyPrev,
						iWidth, uColorLine, 0
					);
					
					iGxPrev = iGx;
					iGyPrev = iGy;
				}
			}
		}else{
			iGx = ( int )( Log->Gx() * iR );
			iGy = ( int )( Log->Gy() * iR );
		}
	}else{
		iGx = iGy = 0;
	}
	
	// G インジケータ
	DrawCircle(
		iCx + iGx, iCy - iGy, iIndicatorR,
		uColorBall, CVsdFilter::IMG_FILL
	);
}

/*** 走行軌跡表示 ***********************************************************/

void CVsdFilter::DrawMap(
	int iX, int iY, int iSize, int iWidth,
	int iIndicatorR,
	UINT uColorIndicator,
	UINT uColorG0,
	UINT uColorGPlus,
	UINT uColorGMinus
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
				
				UINT uColorLine;
				
				if( dG >= 0.0 ){
					uColorLine = BlendColor( uColorG0, uColorGPlus,  dG / Log->m_dMaxG );
				}else{
					uColorLine = BlendColor( uColorG0, uColorGMinus, dG / Log->m_dMinG );
				}
				
				// Line を引く
				DrawLine(
					( int )( iGx     ), iGy,
					( int )( iGxPrev ), iGyPrev,
					iWidth, uColorLine, 0
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
		( int )( dGx ), ( int )dGy, iIndicatorR,
		uColorIndicator, CVsdFilter::IMG_FILL
	);
	
	// スタートライン表示
	if( DispSyncInfo && m_iLapMode == LAPMODE_GPS ){
		double dAngle = m_piParamT[ TRACK_MapAngle ] * ( -ToRAD / 10 );
		
		int x1 = iX + ( int )((  cos( dAngle ) * m_dStartLineX1 + sin( dAngle ) * m_dStartLineY1 - Log->m_dMapOffsX ) / Log->m_dMapSize * iSize );
		int y1 = iY + ( int )(( -sin( dAngle ) * m_dStartLineX1 + cos( dAngle ) * m_dStartLineY1 - Log->m_dMapOffsY ) / Log->m_dMapSize * iSize );
		int x2 = iX + ( int )((  cos( dAngle ) * m_dStartLineX2 + sin( dAngle ) * m_dStartLineY2 - Log->m_dMapOffsX ) / Log->m_dMapSize * iSize );
		int y2 = iY + ( int )(( -sin( dAngle ) * m_dStartLineX2 + cos( dAngle ) * m_dStartLineY2 - Log->m_dMapOffsY ) / Log->m_dMapSize * iSize );
		
		DrawLine( x1, y1, x2, y2, color_blue, 0 );
	}
}

/*** 針描画 *****************************************************************/

void CVsdFilter::DrawNeedle(
	int x, int y, int r,
	int iStart, int iEnd, double dVal,
	UINT uColor,
	int iWidth
){
	iStart %= 360;
	iEnd   %= 360;
	double dAngle = ( iStart + (( iStart > iEnd ? 360 : 0 ) + ( iEnd - iStart )) * dVal ) * ToRAD;
	
	DrawLine(
		x, y,
		( int )( cos( dAngle ) * r ) + x,
		( int )( sin( dAngle ) * r ) + y,
		iWidth, uColor, 0
	);
}

/*** ラップタイム表示 *******************************************************/

void CVsdFilter::DrawLapTime(
	int x, int y, CVsdFont &Font,
	UINT uColor, UINT uColorOutline, UINT uColorBest, UINT uColorPlus
){
	
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
		DrawText( x, y, g_szBuf, Font, uColor, uColorOutline );
		bInLap = TRUE;
	}else{
		// まだ開始していない
		DrawText( x, y, "Time -'--.---", Font, uColor, uColorOutline );
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
			DrawText( POS_DEFAULT, POS_DEFAULT, g_szBuf, Font, bSign ? uColorBest : uColorPlus, uColorOutline );
		}else{
			m_iPosY += Font.GetH();
		}
	}
	
	m_iPosY += Font.GetH() / 4;
	
	// Best 表示
	sprintf(
		g_szBuf, "Best%2d'%02d.%03d",
		m_iBestTime / 60000,
		m_iBestTime / 1000 % 60,
		m_iBestTime % 1000
	);
	DrawText( POS_DEFAULT, POS_DEFAULT, g_szBuf, Font, uColor, uColorOutline );
	
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
				DrawText(
					POS_DEFAULT, POS_DEFAULT, g_szBuf, Font,
					m_iBestTime == m_Lap[ iLapIdxStart ].iTime ? uColorBest : uColor,
					uColorOutline
				);
				++i;
			}
		}
	}
}

/*** メーターパネル type 0 **************************************************/

void CVsdFilter::DrawMeterPanel0(
	int	iMeterCx,
	int	iMeterCy,
	int	iMeterR,
	int	iMaxSpeed,
	CVsdFont &Font
){
	int	i;
	
	const int	iMeterMinDeg	= 135;
	const int	iMeterMaxDeg	= 45;
	const int	iMeterDegRange	= ( iMeterMaxDeg + 360 - iMeterMinDeg ) % 360;
	const int	iMeterScaleLen	= iMeterR / 8;
	
	/*** メーターパネル ***/
	
	int	iStep = (( iMaxSpeed / 18 ) + 4 ) / 5 * 5;
	if( iStep == 0 ) iStep = 5;
	
	for( i = 0; i <= iMaxSpeed; i += iStep ){
		int iDeg = iMeterDegRange * i / iMaxSpeed + iMeterMinDeg;
		
		// メーターパネル目盛り
		if( i % iStep == 0 ){
			DrawLine(
				( int )( cos( iDeg * ToRAD ) * iMeterR ) + iMeterCx,
				( int )( sin( iDeg * ToRAD ) * iMeterR ) + iMeterCy,
				( int )( cos( iDeg * ToRAD ) * ( iMeterR * 95 / 100 )) + iMeterCx,
				( int )( sin( iDeg * ToRAD ) * ( iMeterR * 95 / 100 )) + iMeterCy,
				( i % ( iStep * 2 ) == 0 ) ? 2 : 1,
				COLOR_SCALE, 0
			);
			
			// メーターパネル目盛り数値
			if( i % ( iStep * 2 ) == 0 ){
				sprintf( g_szBuf, "%d", i );
				DrawText(
					( int )( cos( iDeg * ToRAD ) * iMeterR * .75 ) + iMeterCx - Font.GetTextWidth( g_szBuf ) / 2,
					( int )( sin( iDeg * ToRAD ) * iMeterR * .75 ) + iMeterCy - Font.GetH() / 2,
					g_szBuf, Font, COLOR_STR
				);
			}
		}
	}
}

/*** メーターパネル type 1 **************************************************/

void CVsdFilter::DrawMeterPanel1(
	int	iMeterCx,
	int	iMeterCy,
	int	iMeterR,
	int	iMaxSpeed,
	CVsdFont &Font
){
	int	i;
	
	const int	iMeterMinDeg	= 90;
	const int	iMeterMaxDeg	= 0;
	const int	iMeterDegRange	= ( iMeterMaxDeg - iMeterMinDeg + 360 ) % 360;
	
	/*** メーターパネル ***/
	
	// GPS ログ優先時はスピードメーターパネル
	
	int	iStep = (( iMaxSpeed / 26 ) + 4 ) / 5 * 5;
	if( iStep == 0 ) iStep = 5;
	
	for( i = 0; i <= iMaxSpeed; i += iStep ){
		int iDeg = iMeterDegRange * i / iMaxSpeed + iMeterMinDeg;
		
		// メーターパネル目盛り
		if( i % iStep == 0 ){
			DrawLine(
				( int )( cos( iDeg * ToRAD ) * ( iMeterR * 100 / 100 )) + iMeterCx,
				( int )( sin( iDeg * ToRAD ) * ( iMeterR * 100 / 100 )) + iMeterCy,
				( int )( cos( iDeg * ToRAD ) * ( iMeterR *  95 / 100 )) + iMeterCx,
				( int )( sin( iDeg * ToRAD ) * ( iMeterR *  95 / 100 )) + iMeterCy,
				( i % ( iStep * 2 ) == 0 ) ? 2 : 1,
				COLOR_SCALE, 0
			);
			
			// メーターパネル目盛り数値
			if( i % ( iStep * 2 ) == 0 ){
				sprintf( g_szBuf, "%d", i );
				DrawText(
					( int )( cos( iDeg * ToRAD ) * iMeterR * .825 ) + iMeterCx - Font.GetTextWidth( g_szBuf ) / 2,
					( int )( sin( iDeg * ToRAD ) * iMeterR * .825 ) + iMeterCy - Font.GetH() / 2,
					g_szBuf, Font, COLOR_STR
				);
			}
		}
	}
}

/*** メーター等描画 *********************************************************/

BOOL CVsdFilter::DrawVSD( void ){
	
	int	i;
	
	// フォントサイズ初期化
	if( m_pFont == NULL ){
		m_pFont = new CVsdFont( DEFAULT_FONT, 20, CVsdFont::ATTR_OUTLINE );
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
	
	// JavaScript 用ログデータ計算
	SelectLogVsd;
	if( Log ){
		m_dSpeed	= Log->Speed( Log->m_dLogNum );
		m_dTacho	= Log->Tacho( Log->m_dLogNum );
		m_dGx		= Log->Gx( Log->m_dLogNum );
		m_dGy		= Log->Gy( Log->m_dLogNum );
		m_iMaxSpeed	= Log->m_iMaxSpeed;
	}else{
		m_dSpeed	=
		m_dTacho	=
		m_dGx		=
		m_dGy		= 0;
		m_iMaxSpeed	= 180;
	}
	
	// スクリプト実行
	
	if( !m_Script && *m_szSkinFile ){
		m_Script = new CScript( this );
		m_Script->Initialize( m_szSkinFile );
		m_Script->Run( "Initialize" );
	}
	
	if( m_Script ){
		m_Script->Run( "Draw" );
	}else{
		DrawText( 0, 0, "Skin not loaded.", *m_pFont, COLOR_STR, color_black );
	}
	
#if 0
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
				color_orange, 1
			);
			if( !DispSyncInfo ) DrawTachoGraph(
				m_VsdLog,
				iGraphX, iGraphY,
				iGraphW, iGraphH,
				color_cyan, 0
			);
		}
		
		if(( !m_VsdLog || DispSyncInfo ) && m_GPSLog ){
			DrawSpeedGraph(
				m_GPSLog,
				iGraphX, iGraphY,
				iGraphW, iGraphH,
				color_cyan, 0
			);
		}
	}
#endif
	
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
			DrawText( POS_DEFAULT, POS_DEFAULT, g_szBuf, *m_pFont, COLOR_STR );
		}
		
		#ifndef GPS_ONLY
			DrawText( POS_DEFAULT, POS_DEFAULT, "        start       end     range cur.pos", *m_pFont, COLOR_STR );
			
			sprintf(
				g_szBuf, "Vid%4d:%05.2f%4d:%05.2f%4d:%05.2f%7d",
				Float2Time( VideoSt / GetFPS()),
				Float2Time( VideoEd / GetFPS()),
				Float2Time(( VideoEd - VideoSt ) / GetFPS()),
				GetFrameCnt()
			);
			DrawText( POS_DEFAULT, POS_DEFAULT, g_szBuf, *m_pFont, COLOR_STR );
			
			if( m_VsdLog ){
				sprintf(
					g_szBuf, "Log%4d:%05.2f%4d:%05.2f%4d:%05.2f%7d",
					Float2Time( LogSt / m_VsdLog->m_dFreq ),
					Float2Time( LogEd / m_VsdLog->m_dFreq ),
					Float2Time(( LogEd - LogSt ) / m_VsdLog->m_dFreq ),
					m_VsdLog->m_iLogNum
				);
				DrawText( POS_DEFAULT, POS_DEFAULT, g_szBuf, *m_pFont, COLOR_STR );
			}
			
			if( m_GPSLog ){
				sprintf(
					g_szBuf, "GPS%4d:%05.2f%4d:%05.2f%4d:%05.2f%7d",
					Float2Time( GPSSt / m_GPSLog->m_dFreq ),
					Float2Time( GPSEd / m_GPSLog->m_dFreq ),
					Float2Time(( GPSEd - GPSSt ) / m_GPSLog->m_dFreq ),
					m_GPSLog->m_iLogNum
				);
				DrawText( POS_DEFAULT, POS_DEFAULT, g_szBuf, *m_pFont, COLOR_STR );
			}
		#endif	// !GPS_ONLY
	}
	
	return TRUE;
}
