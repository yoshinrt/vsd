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
#include "error_code.h"

/*** macros *****************************************************************/

#define SPRINTF_BUF		128

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

#define DEFAULT_FONT	"ＭＳ ゴシック"

/*** DrawLine ***************************************************************/

#define ABS( x )			(( x ) < 0 ? -( x ) : ( x ))
#define SWAP( x, y, tmp )	( tmp = x, x = y, y = tmp )

void CVsdFilter::DrawLine( int x1, int y1, int x2, int y2, const PIXEL_YCA_ARG yc, UINT uFlag ){
	
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
	}else{
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

void CVsdFilter::DrawLine( int x1, int y1, int x2, int y2, tRABY uColor, UINT uFlag ){
	PIXEL_YCA yc( uColor );
	DrawLine( x1, y1, x2, y2, yc, uFlag );
}

void CVsdFilter::DrawLine(
	int x1, int y1, int x2, int y2,
	int width,
	tRABY uColor,
	UINT uFlag
){
	
	PIXEL_YCA yc( uColor );
	
	#ifdef _OPENMP
		#pragma omp parallel for
		for( int i = 0; i < width * width; ++i ){
			int x = i % width - width / 2;
			int y = i / width - width / 2;
			
			DrawLine(
				x1 + x - width / 2, y1 + y - width / 2,
				x2 + x - width / 2, y2 + y - width / 2,
				yc, uFlag
			);
		}
	#else
		for( int y = 0; y < width; ++y ) for( int x = 0; x < width; ++x ){
			DrawLine(
				x1 + x - width / 2, y1 + y - width / 2,
				x2 + x - width / 2, y2 + y - width / 2,
				yc, uFlag
			);
		}
	#endif
}

/*** DrawRect ***************************************************************/

void CVsdFilter::DrawRect(
	int x1, int y1, int x2, int y2,
	tRABY uColor, UINT uFlag
){
	int	y;
	
	if( y1 > y2 ) SWAP( y1, y2, y );
	if( x1 > x2 ) SWAP( x1, x2, y );
	
	if( uFlag & IMG_FILL ){
		#ifdef _OPENMP
			#pragma omp parallel for
		#endif
		for( y = y1; y <= y2; ++y ){
			FillLine( x1, y, x2, uColor, 0 );
		}
	}else{
		DrawLine( x1, y1, x2, y1, uColor, 0 );
		DrawLine( x1, y2, x2, y2, uColor, 0 );
		DrawLine( x1, y1, x1, y2, uColor, 0 );
		DrawLine( x2, y1, x2, y2, uColor, 0 );
	}
}

/*** DrawCircle *************************************************************/

void CVsdFilter::DrawCircle(
	int x, int y, int r,
	tRABY uColor,
	UINT uFlag
){
	
	int	i = r;
	int j = 0;
	int f = -2 * r + 3;
	
	PIXEL_YCA yc( uColor );
	
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
	if( uFlag & IMG_FILL ) DrawPolygon( yc );
}

// http://fussy.web.fc2.com/algo/algo2-2.htm
// の，a = r / A, b = r / B と置いて両辺に ( A * B / r )^2 をかける
void CVsdFilter::DrawCircle( int x, int y, int a, int b, tRABY uColor, UINT uFlag ){
	
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
	if( uFlag & IMG_FILL ) DrawPolygon( yc );
}

void CVsdFilter::DrawArc(
	int x, int y,
	int a, int b,
	double dStart, double dEnd,
	tRABY uColor, UINT uFlag
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
	if( uFlag & IMG_FILL ) DrawPolygon( yc );
}

void CVsdFilter::DrawArc(
	int x, int y,
	int a, int b,
	int c, int d,
	double dStart, double dEnd,
	tRABY uColor, UINT uFlag
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
				if(           iAreaCmpS <= 0x00 && 0x00 <= iAreaCmpE ) PutPixel( x + i, y + j, yc, 0 );
				if( i &&      iAreaCmpS <= 0x10 && 0x10 <= iAreaCmpE ) PutPixel( x - i, y + j, yc, 0 );
				if( i && j && iAreaCmpS <= 0x20 && 0x20 <= iAreaCmpE ) PutPixel( x - i, y - j, yc, 0 );
				if(      j && iAreaCmpS <= 0x30 && 0x30 <= iAreaCmpE ) PutPixel( x + i, y - j, yc, 0 );
			}else{
				// st || ed
				if(           iAreaCmpS <= 0x00 || 0x00 <= iAreaCmpE ) PutPixel( x + i, y + j, yc, 0 );
				if( i &&      iAreaCmpS <= 0x10 || 0x10 <= iAreaCmpE ) PutPixel( x - i, y + j, yc, 0 );
				if( i && j && iAreaCmpS <= 0x20 || 0x20 <= iAreaCmpE ) PutPixel( x - i, y - j, yc, 0 );
				if(      j && iAreaCmpS <= 0x30 || 0x30 <= iAreaCmpE ) PutPixel( x + i, y - j, yc, 0 );
			}
		}
	}
}

/*** DrawFont ***************************************************************/

int CVsdFilter::DrawFont0( int x, int y, WCHAR c, CVsdFont &Font, tRABY uColor ){
	
	// 文字幅を得る
	CFontGlyph &FontGlyph = Font.FontGlyph( c );
	
	int iCellIncX = Font.IsFixed() ? Font.GetWidth() : FontGlyph.iCellIncX;
	int iOrgX = ( iCellIncX - FontGlyph.iW ) / 2;
	
	if( !Font.IsNoAntialias()){
		int iBmpW = ( FontGlyph.iW + 3 ) & ~3;
		
		#ifdef _OPENMP
			#pragma omp parallel for
		#endif
		for( int j = 0; j < FontGlyph.iH; ++j ) for( int i = 0; i < FontGlyph.iW; ++i ){
			int iDensity = FontGlyph.pBuf[ iBmpW * j + i ];	// 0〜64
			
			if( iDensity ){
				if( iDensity == 64 ){
					PutPixel( x + iOrgX + i, y + FontGlyph.iOrgY + j, uColor, 0 );
				}else{
					int ir = ( uColor >> 24 ) - 0x80;
					int ia = ( 0xFF << 6 ) - (( 0xFF - (( uColor >> 16 ) & 0xFF )) * iDensity );
					int ib = (( uColor >>  8 ) & 0xFF ) - 0x80;
					int iy = uColor & 0xFF;
					
					UINT uColorTmp = (
						(( ir * iDensity ) & ( 0xFF << 6 )) << ( 24 - 6 ) |
						(( ia            ) & ( 0xFF << 6 )) << ( 16 - 6 ) |
						(( ib * iDensity ) & ( 0xFF << 6 )) << (  8 - 6 ) |
						(( iy * iDensity ) & ( 0xFF << 6 )) >> (      6 )
					) ^ 0x80008000;
					
					PutPixel( x + iOrgX + i, y + FontGlyph.iOrgY + j, uColorTmp, 0 );
				}
			}
		}
	}else{
		int iBmpW = (( FontGlyph.iW + 31 ) & ~31 ) / 8;
		
		#ifdef _OPENMP
			#pragma omp parallel
		#endif
		{
			UINT uBitmap;
			#ifdef _OPENMP
				#pragma omp for
			#endif
			for( int j = 0; j < FontGlyph.iH; ++j ) for( int i = 0; i < FontGlyph.iW; ++i ){
				if(( i & 0x7 ) == 0 ) uBitmap = FontGlyph.pBuf[ iBmpW * j + ( i >> 3 ) ];
				
				if( uBitmap & 0x80 ){
					PutPixel( x + iOrgX + i, y + FontGlyph.iOrgY + j, uColor, 0 );
				}
				uBitmap <<= 1;
			}
		}
	}
	
	return iCellIncX;
}

int CVsdFilter::DrawFont( int x, int y, WCHAR c, CVsdFont &Font, tRABY uColor, tRABY uColorOutline ){
	
	// フォントが存在しない文字なら，space の文字幅を返す
	if( !CVsdFont::ExistFont( c ))
		return ( Font.IsFixed()) ? Font.GetWidth() : Font.GetW_Space();
	
	if( Font.IsOutline()){
		DrawFont0( x + 1, y + 0, c, Font, uColorOutline );
		DrawFont0( x - 1, y + 0, c, Font, uColorOutline );
		DrawFont0( x + 0, y + 1, c, Font, uColorOutline );
		DrawFont0( x + 0, y - 1, c, Font, uColorOutline );
	}
	return DrawFont0( x, y, c, Font, uColor );
}

/*** DrawText *************************************************************/

void CVsdFilter::DrawText( int x, int y, LPCWSTR szMsg, CVsdFont &Font, tRABY uColor, tRABY uColorOutline ){
	
	if( x != POS_DEFAULT ) m_iPosX = x;
	if( y != POS_DEFAULT ) m_iPosY = y;
	
	x = m_iPosX;
	
	for( int i = 0; szMsg[ i ]; ++i ){
		x += DrawFont( x, m_iPosY, szMsg[ i ], Font, uColor, uColorOutline );
	}
	
	m_iPosY += Font.GetHeight();
}

void CVsdFilter::DrawTextAlign( int x, int y, UINT uAlign, LPCWSTR szMsg, CVsdFont &Font, tRABY uColor, tRABY uColorOutline ){
	
	if( uAlign & ALIGN_HCENTER ){
		x -= Font.GetTextWidth( szMsg ) / 2;
	}else if( uAlign & ALIGN_RIGHT ){
		x -= Font.GetTextWidth( szMsg );
	}
	
	if( uAlign & ALIGN_VCENTER ){
		y -= Font.GetHeight() / 2;
	}else if( uAlign & ALIGN_BOTTOM ){
		y -= Font.GetHeight();
	}
	
	for( int i = 0; szMsg[ i ]; ++i ){
		x += DrawFont( x, y, szMsg[ i ], Font, uColor, uColorOutline );
	}
	
	m_iPosY += Font.GetHeight();
}

/*** put pixel 系 ***********************************************************/

inline void CVsdFilter::PutPixel( int x, int y, tRABY uColor, UINT uFlag ){
	PIXEL_YCA	yc( uColor );
	PutPixel( x, y, yc, uFlag );
}

inline void CVsdFilter::PutPixel( int x, int y, const PIXEL_YCA_ARG yc, UINT uFlag ){
	
	if( !( 0 <= y && y < GetHeight())) return;
	
	if( uFlag & IMG_FILL ){
		// ポリゴン描画
		if( x > m_Polygon[ y ].iRight ){
			m_Polygon[ y ].iRight = ( x >= GetWidth()) ? GetWidth() : x;
		}
		if( x < m_Polygon[ y ].iLeft  ){
			m_Polygon[ y ].iLeft  = ( x < 0 ) ? 0 : x;
		}
	}else if( 0 <= x && x < GetWidth() && yc.alfa < 255 ){
		PutPixel( x, y, yc );
	}
}

inline void CVsdFilter::FillLine( int x1, int y1, int x2, const PIXEL_YCA_ARG yc, UINT uFlag ){
	
	if( uFlag & IMG_FILL ){
		// ポリゴン描画
		if( x1 > x2 ){
			if( x1 > m_Polygon[ y1 ].iRight ) m_Polygon[ y1 ].iRight = x1;
			if( x2 < m_Polygon[ y1 ].iLeft  ) m_Polygon[ y1 ].iLeft  = x2;
		}else{
			if( x2 > m_Polygon[ y1 ].iRight ) m_Polygon[ y1 ].iRight = x2;
			if( x1 < m_Polygon[ y1 ].iLeft  ) m_Polygon[ y1 ].iLeft  = x1;
		}
	}else if( 0 <= y1 && y1 < GetHeight() && yc.alfa < 255 ){
		if( x1 < 0 )         x1 = 0;
		if( x2 > GetWidth()) x2 = GetWidth();
		
		FillLine( x1, y1, x2, yc );
	}
}

/*** ポリゴン描画 ***********************************************************/

inline void CVsdFilter::InitPolygon( void ){
	#ifdef _OPENMP
		#pragma omp parallel for
	#endif
	for( int y = 0; y < GetHeight(); ++y ){
		m_Polygon[ y ].iRight	= 0;		// right
		m_Polygon[ y ].iLeft	= 0x7FFF;	// left
	}
}

inline void CVsdFilter::DrawPolygon( const PIXEL_YCA_ARG yc ){
	#ifdef _OPENMP
		#pragma omp parallel for
	#endif
	for( int y = 0; y < GetHeight(); ++y ) if( m_Polygon[ y ].iLeft <= m_Polygon[ y ].iRight ){
		FillLine( m_Polygon[ y ].iLeft, y, m_Polygon[ y ].iRight, yc );
	}
	
	InitPolygon();
}

void CVsdFilter::DrawPolygon( tRABY uColor ){
	PIXEL_YCA	yc( uColor );
	DrawPolygon( yc );
}

/*** カラーを混ぜる *********************************************************/

inline UINT CVsdFilter::BlendColor(
	tRABY uColor0,
	tRABY uColor1,
	double	dAlfa
){
	if     ( dAlfa < 0.0 ) dAlfa = 0.0;
	else if( dAlfa > 1.0 ) dAlfa = 1.0;
	
	return
		(( UINT )(( uColor1 & 0xFF000000 ) * dAlfa + ( uColor0 & 0xFF000000 ) * ( 1 - dAlfa )) & 0xFF000000 ) +
		(( UINT )(( uColor1 & 0x0000FF00 ) * dAlfa + ( uColor0 & 0x0000FF00 ) * ( 1 - dAlfa )) & 0x0000FF00 ) +
		(( UINT )(( uColor1 & 0x000000FF ) * dAlfa + ( uColor0 & 0x000000FF ) * ( 1 - dAlfa )) & 0x000000FF );
}

/****************************************************************************/
/*** メーター描画 ***********************************************************/
/****************************************************************************/

/*** パラメータ調整用スピードグラフ *****************************************/

#define GRAPH_SCALE	2

static double GetSpeedLog( CVsdLog& Log, int iIndex ){
	return Log.Speed( iIndex );
}

static double GetTachoLog( CVsdLog& Log, int iIndex ){
	return Log.Tacho( iIndex );
}

void CVsdFilter::DrawGraph(
	int x1, int y1, int x2, int y2,
	LPCWSTR szFormat,
	CVsdFont &Font,
	tRABY uColor,
	CVsdLog& Log,
	double ( *GetDataFunc )( CVsdLog&, int ),
	double dMaxVal
){
	int	iWidth  = x2 - x1 + 1;
	int iHeight = y2 - y1 + 1;
	
	int		iPrevY = INVALID_POS_I;
	int		iCursorPos;
	double	dCursorVal;
	double	dVal;
	
	WCHAR	szBuf[ SPRINTF_BUF ];
	
	for( int x = 0; x < iWidth; ++x ){
		int iLogNum = Log.m_iLogNum + ( x - iWidth / 2 ) * GRAPH_SCALE;
		dVal = Log.IsDataExist( iLogNum ) ? GetDataFunc( Log, iLogNum ) : 0;
		
		int iPosY = y2 - ( int )( dVal * iHeight / dMaxVal );
		if( iPrevY != INVALID_POS_I )
			DrawLine( x1 + x - 1, iPrevY, x1 + x, iPosY, 1, uColor, 0 );
		
		iPrevY = iPosY;
		
		if( x == iWidth / 2 ){
			iCursorPos = iPosY;
			dCursorVal = dVal;
		}
	}
	
	int x = x1 + iWidth / 2;
	DrawLine(
		x, iCursorPos,
		x + 10, iCursorPos - 10,
		1, uColor, 0
	);
	
	swprintf( szBuf, sizeof( szBuf ), szFormat, dCursorVal );
	DrawText(
		x + 10,
		iCursorPos - 10 - Font.GetHeight(),
		szBuf, Font, uColor
	);
}

// スピード・タコグラフ
void CVsdFilter::DrawGraph(
	int x1, int y1, int x2, int y2,
	CVsdFont &Font
){
	if(
		#ifdef GPS_ONLY
			DispGraph
		#else
			DispGraph || DispSyncInfo
		#endif
	){
		if( m_VsdLog ){
			DrawGraph(
				x1, y1, x2, y2,
				L"%.0f km/h", Font, color_orange,
				*m_VsdLog,
				GetSpeedLog, m_VsdLog->m_iMaxSpeed
			);
			if( !DispSyncInfo ) DrawGraph(
				x1, y1, x2, y2,
				L"%.0f rpm", Font, color_cyan,
				*m_VsdLog,
				GetTachoLog, m_VsdLog->m_iMaxTacho
			);
		}
		
		if(( !m_VsdLog || DispSyncInfo ) && m_GPSLog ){
			DrawGraph(
				x1, y1, x2, y2,
				L"%.0f km/h", Font, color_cyan,
				*m_GPSLog,
				GetSpeedLog, m_VsdLog->m_iMaxSpeed
			);
		}
	}
}

/*** G スネーク描画 *********************************************************/

void CVsdFilter::DrawGSnake(
	int iCx, int iCy, int iR, int iIndicatorR, int iWidth,
	tRABY uColorBall, tRABY uColorLine
){
	int	iGx, iGy;
	int	i;
	
	SelectLogVsd;
	
	if( m_CurLog && m_CurLog->IsDataExist( m_CurLog->m_iLogNum )){
		if( GSnakeLen > 0 ){
			
			int iGxPrev = INVALID_POS_I, iGyPrev;
			
			for( i = -( int )( GSnakeLen * LOG_FREQ / 10.0 ) ; i <= 1 ; ++i ){
				
				if( m_CurLog->m_iLogNum + i >= 0 ){
					// i == 1 時は最後の中途半端な LogNum
					iGx = ( int )((( i != 1 ) ? m_CurLog->m_Log[ m_CurLog->m_iLogNum + i ].Gx() : m_CurLog->Gx()) * iR );
					iGy = ( int )((( i != 1 ) ? m_CurLog->m_Log[ m_CurLog->m_iLogNum + i ].Gy() : m_CurLog->Gy()) * iR );
					
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
			iGx = ( int )( m_CurLog->Gx() * iR );
			iGy = ( int )( m_CurLog->Gy() * iR );
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
	int x1, int y1, int x2, int y2,
	UINT uAlign,
	int iLineWidth,
	int iIndicatorR,
	tRABY uColorIndicator,
	tRABY uColorG0,
	tRABY uColorGPlus,
	tRABY uColorGMinus
){
	double dGx, dGy;
	int	iGx, iGy;
	int i;
	
	SelectLogGPS;
	
	if( !LineTrace || !m_CurLog || !m_CurLog->IsDataExist()) return;
	
	int iWidth  = x2 - x1 + 1;
	int iHeight = y2 - y1 + 1;
	double dScaleX = iWidth  / m_CurLog->m_dMapSizeX;
	double dScaleY = iHeight / m_CurLog->m_dMapSizeY;
	double dScale;
	
	if( dScaleX < dScaleY ){
		// 幅律速なので y1 を再計算
		dScale = dScaleX;
		if( uAlign & ALIGN_HCENTER ){
			y1 = y1 + ( iHeight - ( int )( m_CurLog->m_dMapSizeY * dScale )) / 2;
		}else if( uAlign & ALIGN_BOTTOM ){
			y1 = y1 + ( iHeight - ( int )( m_CurLog->m_dMapSizeY * dScale ));
		}
	}else{
		// 高さ律速なので x1 を再計算
		dScale = dScaleY;
		if( uAlign & ALIGN_HCENTER ){
			x1 = x1 + ( iWidth - ( int )( m_CurLog->m_dMapSizeX * dScale )) / 2;
		}else if( uAlign & ALIGN_BOTTOM ){
			x1 = x1 + ( iWidth - ( int )( m_CurLog->m_dMapSizeX * dScale ));
		}
	}
	
	int iGxPrev = INVALID_POS_I, iGyPrev;
	
	int iLineSt, iLineEd;
	
	if( m_LapLog ){
		iLineSt = ( int )LapNum2LogNum( m_CurLog, m_LapLog->m_iLapIdx );
		iLineEd = m_LapLog->m_iLapIdx < m_LapLog->m_iLapNum - 1 ?
			( int )LapNum2LogNum( m_CurLog, m_LapLog->m_iLapIdx + 1 ) : 0;
	}else{
		iLineSt = 0;
		iLineEd = m_CurLog->m_iCnt - 1;
	}
	
	if( m_CurLog->m_iLogNum - iLineSt > ( int )( LineTrace * LOG_FREQ ))
		iLineSt = m_CurLog->m_iLogNum - ( int )( LineTrace * LOG_FREQ );
	
	if( iLineEd - m_CurLog->m_iLogNum > ( int )( LineTrace * LOG_FREQ ))
		iLineEd = m_CurLog->m_iLogNum + ( int )( LineTrace * LOG_FREQ );
	
	for( i = iLineSt; i <= iLineEd ; ++i ){
		if( !_isnan( m_CurLog->X( i ))){
			#define GetMapPos( p, a ) ((( p ) - m_CurLog->m_dMapOffs ## a ) * dScale )
			iGx = x1 + ( int )GetMapPos( m_CurLog->X( i ), X );
			iGy = y1 + ( int )GetMapPos( m_CurLog->Y( i ), Y );
			
			if( iGxPrev != INVALID_POS_I ){
				if(
					( iGx - iGxPrev ) * ( iGx - iGxPrev ) +
					( iGy - iGyPrev ) * ( iGy - iGyPrev ) >= ( 25 )
				){
					// Line の色用に G を求める
					
					double dG = m_CurLog->Gy( i );
					
					tRABY uColorLine;
					
					if( dG >= 0.0 ){
						uColorLine = BlendColor( uColorG0, uColorGPlus,  dG / m_CurLog->m_dMaxG );
					}else{
						uColorLine = BlendColor( uColorG0, uColorGMinus, dG / m_CurLog->m_dMinG );
					}
					
					// Line を引く
					DrawLine(
						iGx,     iGy,
						iGxPrev, iGyPrev,
						iLineWidth, uColorLine, 0
					);
					iGxPrev = iGx;
					iGyPrev = iGy;
				}
			}else{
				iGxPrev = iGx;
				iGyPrev = iGy;
			}
		}else{
			iGxPrev = INVALID_POS_I;
		}
	}
	
	// MAP インジケータ (自車)
	dGx = x1 + GetMapPos( m_CurLog->X(), X );
	dGy = y1 + GetMapPos( m_CurLog->Y(), Y );
	
	if( !_isnan( dGx )) DrawCircle(
		( int )( dGx ), ( int )dGy, iIndicatorR,
		uColorIndicator, CVsdFilter::IMG_FILL
	);
	
	// スタートライン表示
	if( DispSyncInfo && m_LapLog && m_LapLog->m_iLapMode == LAPMODE_GPS ){
		double dAngle = m_piParamT[ TRACK_MapAngle ] * ( -ToRAD / 10 );
		
		int xs1 = x1 + ( int )((  cos( dAngle ) * m_dStartLineX1 + sin( dAngle ) * m_dStartLineY1 - m_CurLog->m_dMapOffsX ) * dScale );
		int ys1 = y1 + ( int )(( -sin( dAngle ) * m_dStartLineX1 + cos( dAngle ) * m_dStartLineY1 - m_CurLog->m_dMapOffsY ) * dScale );
		int xs2 = x1 + ( int )((  cos( dAngle ) * m_dStartLineX2 + sin( dAngle ) * m_dStartLineY2 - m_CurLog->m_dMapOffsX ) * dScale );
		int ys2 = y1 + ( int )(( -sin( dAngle ) * m_dStartLineX2 + cos( dAngle ) * m_dStartLineY2 - m_CurLog->m_dMapOffsY ) * dScale );
		
		DrawLine( xs1, ys1, xs2, ys2, color_blue, 0 );
	}
}

/*** 針描画 *****************************************************************/

void CVsdFilter::DrawNeedle(
	int x, int y, int r1, int r2,
	int iStart, int iEnd, double dVal,
	tRABY uColor,
	int iWidth
){
	iStart %= 360;
	iEnd   %= 360;
	double dAngle = ( iStart + (( iStart > iEnd ? 360 : 0 ) + ( iEnd - iStart )) * dVal ) * ToRAD;
	
	DrawLine(
		( int )( cos( dAngle ) * r1 ) + x,
		( int )( sin( dAngle ) * r1 ) + y,
		( int )( cos( dAngle ) * r2 ) + x,
		( int )( sin( dAngle ) * r2 ) + y,
		iWidth, uColor, 0
	);
}

/*** ラップタイム format ****************************************************/

LPCWSTR CVsdFilter::FormatTime( int iTime ){
	static WCHAR szBuf[ 32 ];
	
	if( iTime == TIME_NONE ) return L"-'--.---";
	
	swprintf( szBuf, sizeof( szBuf ), L"%d'%02d.%03d", iTime / 60000, iTime / 1000 % 60, iTime % 1000 );
	return szBuf;
}

/*** ラップタイム表示 *******************************************************/

void CVsdFilter::DrawLapTime(
	int x, int y, UINT uAlign, CVsdFont &Font,
	tRABY uColor, tRABY uColorOutline, tRABY uColorBest, tRABY uColorPlus
){
	int	i;
	WCHAR	szBuf[ SPRINTF_BUF ];
	
	if( !DispLap || !m_LapLog ) return;
	
	SelectLogForLapTime;
	
	if( uAlign & ALIGN_HCENTER ){
		x -= Font.GetTextWidth( L"Time 0'00.000" ) / 2;
	}else if( uAlign & ALIGN_RIGHT ){
		x -= Font.GetTextWidth( L"Time 0'00.000" );
	}
	
	if( uAlign & ALIGN_VCENTER ){
		y -= Font.GetHeight() / 2;
	}else if( uAlign & ALIGN_BOTTOM ){
		y -= Font.GetHeight();
	}
	
	// 時間表示
	BOOL	bInLap = FALSE;
	
	if( m_LapLog->m_iCurTime != TIME_NONE ){
		swprintf(
			szBuf, sizeof( szBuf ), L"Time%2d'%02d.%03d",
			m_LapLog->m_iCurTime / 60000,
			m_LapLog->m_iCurTime / 1000 % 60,
			m_LapLog->m_iCurTime % 1000
		);
		DrawText( x, y, szBuf, Font, uColor, uColorOutline );
		
		/*** ベストとの車間距離表示 - ***/
		BOOL bSign = m_LapLog->m_iDiffTime <= 0;
		if( m_LapLog->m_iDiffTime < 0 ) m_LapLog->m_iDiffTime = -m_LapLog->m_iDiffTime;
		
		swprintf(
			szBuf, sizeof( szBuf ), L"    %c%d'%02d.%03d",
			bSign ? '-' : '+',
			m_LapLog->m_iDiffTime / 60000,
			m_LapLog->m_iDiffTime / 1000 % 60,
			m_LapLog->m_iDiffTime % 1000
		);
		DrawText( POS_DEFAULT, POS_DEFAULT, szBuf, Font, bSign ? uColorBest : uColorPlus, uColorOutline );
		
		bInLap = TRUE;
	}else{
		// まだ開始していない
		DrawText( x, y, L"Time -'--.---", Font, uColor, uColorOutline );
		m_iPosY += Font.GetHeight();
	}
	
	m_iPosY += Font.GetHeight() / 4;
	
	// Best 表示
	swprintf(
		szBuf, sizeof( szBuf ), L"Best%2d'%02d.%03d",
		m_LapLog->m_iBestTime / 60000,
		m_LapLog->m_iBestTime / 1000 % 60,
		m_LapLog->m_iBestTime % 1000
	);
	DrawText( POS_DEFAULT, POS_DEFAULT, szBuf, Font, uColor, uColorOutline );
	
	// Lapタイム表示
	// 3つタイム表示する分の，最後の LapIdx を求める．
	// 通常は m_iLapIdx + 1 だが，m_LapLog[ iLapIdxEnd ].iTime == 0 の時は
	// 周回モードでは最後のラップを走り終えた
	// ジムカモードでは 1周走り終えたことを示しているので
	// LapIdx を -1 する
	int iLapIdxEnd = m_LapLog->m_iLapIdx + 1;
	if( m_LapLog->m_Lap[ iLapIdxEnd ].iTime == 0 ) --iLapIdxEnd;
	
	// iLapIdxEnd から有効なラップタイムが 2個見つかるまで遡る
	int iLapIdxStart = iLapIdxEnd - 1;
	for( i = 0; iLapIdxStart > 0; --iLapIdxStart ){
		if( m_LapLog->m_Lap[ iLapIdxStart ].iTime ){
			if( ++i >= 2 ) break;
		}
	}
	
	if( iLapIdxStart >= 0 ){
		for( ; iLapIdxStart <= iLapIdxEnd; ++iLapIdxStart ){
			if( m_LapLog->m_Lap[ iLapIdxStart ].iTime != 0 ){
				swprintf(
					szBuf, sizeof( szBuf ), L"%3d%c%2d'%02d.%03d",
					m_LapLog->m_Lap[ iLapIdxStart ].uLap,
					( iLapIdxStart == m_LapLog->m_iLapIdx + 1 && bInLap ) ? '*' : ' ',
					m_LapLog->m_Lap[ iLapIdxStart ].iTime / 60000,
					m_LapLog->m_Lap[ iLapIdxStart ].iTime / 1000 % 60,
					m_LapLog->m_Lap[ iLapIdxStart ].iTime % 1000
				);
				DrawText(
					POS_DEFAULT, POS_DEFAULT, szBuf, Font,
					m_LapLog->m_iBestTime == m_LapLog->m_Lap[ iLapIdxStart ].iTime ? uColorBest : uColor,
					uColorOutline
				);
				++i;
			}
		}
	}
}

/*** メーターパネル目盛り ***************************************************/

void CVsdFilter::DrawMeterScale(
	int iCx, int iCy, int iR,
	int iLineLen1, int iLineWidth1, tRABY uColorLine1,
	int iLineLen2, int iLineWidth2, tRABY uColorLine2,
	int iLine2Cnt,
	int iMinDeg, int iMaxDeg,
	int iRNum,
	int iMaxVal, int iMaxNumCnt, tRABY uColorNum,
	CVsdFont &Font
){
	int	i;
	WCHAR	szBuf[ SPRINTF_BUF ];
	
	const int iDegRange	= ( iMaxDeg + 360 - iMinDeg ) % 360;
	
	/*** メーターパネル ***/
	
	// iStep は切り上げ
	int	iStep = ( iMaxVal + iMaxNumCnt - 1 ) / iMaxNumCnt;
	
	if( iStep == 0 ){
		iStep = 1;
	}else if( iMaxVal >= 50 ){
		// 50以上では，10単位に切り上げ
		iStep = ( iStep + 9 ) / 10 * 10;
	}
	
	for( i = 0; i <= iMaxVal * iLine2Cnt / iStep; ++i ){
		double dAngle = ( iDegRange * i * iStep / ( double )iLine2Cnt / iMaxVal + iMinDeg ) * ToRAD;
		
		// メーターパネル目盛り
		if( i % iLine2Cnt == 0 ){
			DrawLine(
				( int )( cos( dAngle ) * iR ) + iCx,
				( int )( sin( dAngle ) * iR ) + iCy,
				( int )( cos( dAngle ) * ( iR - iLineLen1 )) + iCx,
				( int )( sin( dAngle ) * ( iR - iLineLen1 )) + iCy,
				iLineWidth1,
				uColorLine1, 0
			);
			
			// メーターパネル目盛り数値
			swprintf( szBuf, sizeof( szBuf ), L"%d", iStep * i / iLine2Cnt );
			DrawTextAlign(
				( int )( cos( dAngle ) * iRNum ) + iCx,
				( int )( sin( dAngle ) * iRNum ) + iCy,
				ALIGN_HCENTER | ALIGN_VCENTER,
				szBuf, Font, uColorNum
			);
		}else{
			// 小目盛り
			DrawLine(
				( int )( cos( dAngle ) * iR ) + iCx,
				( int )( sin( dAngle ) * iR ) + iCy,
				( int )( cos( dAngle ) * ( iR - iLineLen2 )) + iCx,
				( int )( sin( dAngle ) * ( iR - iLineLen2 )) + iCy,
				iLineWidth2,
				uColorLine2, 0
			);
		}
	}
}

/*** エラーメッセージ *******************************************************/

void CVsdFilter::DispErrorMessage( LPCWSTR szMsg ){
	DrawRect( 0, 0, GetWidth() - 1, GetHeight() - 1, color_black_a, IMG_FILL );
	
	int x = 0; int y = 0;
	
	for( ; *szMsg; ++szMsg ){
		if( *szMsg == '\n' ){
			// 改行
			x = 0;
			y += m_pFont->GetHeight();
		}else{
			x += DrawFont( x, y, *szMsg, *m_pFont, color_red );
		}
	}
}

/*** メーター等描画 *********************************************************/

BOOL CVsdFilter::DrawVSD( void ){
	
	WCHAR	szBuf[ MAX_PATH + 1 ];
	
	// 解像度変更
	if( m_iWidth != GetWidth() || m_iHeight != GetHeight()){
		m_iWidth  = GetWidth();
		m_iHeight = GetHeight();
		
		// ポリゴン用バッファリサイズ
		if( m_Polygon ) delete [] m_Polygon;
		m_Polygon = new PolygonData_t[ m_iHeight ];
		InitPolygon();
		
		// JavaScript 再起動用に削除
		if( m_Script ){
			delete m_Script;
			m_Script = NULL;
		}
		// フォントサイズ初期化
		if( m_pFont ) delete m_pFont;
		m_pFont = new CVsdFont( DEFAULT_FONT, 18, CVsdFont::ATTR_OUTLINE | CVsdFont::ATTR_NOANTIALIAS );
	}
	
	// ログ位置の計算
	if( m_VsdLog ){
		m_VsdLog->m_dLogNum = ConvParam( GetFrameCnt(), Video, Log );
		m_VsdLog->m_iLogNum = ( int )m_VsdLog->m_dLogNum;
	}
	if( m_GPSLog ){
		m_GPSLog->m_dLogNum = ConvParam( GetFrameCnt(), Video, GPS );
		m_GPSLog->m_iLogNum = ( int )m_GPSLog->m_dLogNum;
	}
	
	// ラップタイムの再生成
	if(
		DispLap && m_bCalcLapTimeReq &&
		( m_LapLog == NULL || m_LapLog->m_iLapMode != LAPMODE_MAGNET )
	){
		m_bCalcLapTimeReq	= FALSE;
		
		if( m_LapLog ) delete m_LapLog;
		
		// GPS からラップタイム計算してみる
		if( m_GPSLog && m_piParamT[ TRACK_SLineWidth ] > 0 ){
			m_LapLog = CreateLapTimeAuto();
		}
		
		// できなかったので手動で
		if( !m_LapLog ){
			m_LapLog = CreateLapTime(
				m_VsdLog ? LAPMODE_HAND_MAGNET :
				m_GPSLog ? LAPMODE_HAND_GPS :
						   LAPMODE_HAND_VIDEO
			);
		}
	}
	
	// ラップタイム等再計算
	if( m_LapLog ) CalcLapTime();
	
	// JavaScript 用ログデータ計算
	SelectLogVsd;
	if( m_CurLog && m_CurLog->IsDataExist()){
		m_dSpeed	= m_CurLog->Speed( m_CurLog->m_dLogNum );
		m_dTacho	= m_CurLog->Tacho( m_CurLog->m_dLogNum );
		m_dGx		= m_CurLog->Gx( m_CurLog->m_dLogNum );
		m_dGy		= m_CurLog->Gy( m_CurLog->m_dLogNum );
	}else{
		m_dSpeed	=
		m_dTacho	=
		m_dGx		=
		m_dGy		= 0;
	}
	
	// スクリプト実行
	DebugMsgD( ":DrawVSD():Running script... %X\n", GetCurrentThreadId());
	if( !m_Script && m_szSkinFile ){
		m_Script = new CScript( this );
		
		wcscat( wcscpy( szBuf, m_szPluginDirW ), L"initialize.js" );
		m_Script->Initialize();
		if( m_Script->RunFile( szBuf ) == ERR_OK ){
			LPWSTR p = NULL;
			StringNew( p, m_szSkinFile );
			m_Script->RunFile( p );
			delete [] p;
		}
	}
	
	if( m_Script ){
		if( !m_Script->m_uError ) m_Script->Run( L"Draw" );
		if( m_Script->m_uError ) DispErrorMessage( m_Script->GetErrorMessage());
	}else{
		DrawText( 0, 0, L"Skin not loaded.", *m_pFont, color_white );
	}
	
	// フレーム表示
	
	#define Float2Time( n )	( int )( n ) / 60, fmod( n, 60 )
	
	if( DispSyncInfo ){
		
		m_iPosX = 0;
		m_iPosY = GetHeight() / 3;
		
		if( m_GPSLog ){
			int i = ( int )(( m_GPSLog->m_dLogStartTime + m_GPSLog->m_dLogNum / LOG_FREQ ) * 100 ) % ( 24 * 3600 * 100 );
			swprintf(
				szBuf, sizeof( szBuf ), L"GPS time: %02d:%02d:%02d.%02d",
				i / 360000,
				i / 6000 % 60,
				i /  100 % 60,
				i        % 100
			);
			DrawText( POS_DEFAULT, POS_DEFAULT, szBuf, *m_pFont, color_white );
		}
		
		#ifndef GPS_ONLY
			DrawText( POS_DEFAULT, POS_DEFAULT, L"        start       end     range cur.pos", *m_pFont, color_white );
			
			swprintf(
				szBuf, sizeof( szBuf ), L"Vid%4d:%05.2f%4d:%05.2f%4d:%05.2f%7d",
				Float2Time( VideoSt / GetFPS()),
				Float2Time( VideoEd / GetFPS()),
				Float2Time(( VideoEd - VideoSt ) / GetFPS()),
				GetFrameCnt()
			);
			DrawText( POS_DEFAULT, POS_DEFAULT, szBuf, *m_pFont, color_white );
			
			if( m_VsdLog ){
				swprintf(
					szBuf, sizeof( szBuf ), L"Log%4d:%05.2f%4d:%05.2f%4d:%05.2f%7d",
					Float2Time( LogSt / m_VsdLog->m_dFreq ),
					Float2Time( LogEd / m_VsdLog->m_dFreq ),
					Float2Time(( LogEd - LogSt ) / m_VsdLog->m_dFreq ),
					m_VsdLog->m_iLogNum
				);
				DrawText( POS_DEFAULT, POS_DEFAULT, szBuf, *m_pFont, color_white );
			}
			
			if( m_GPSLog ){
				swprintf(
					szBuf, sizeof( szBuf ), L"GPS%4d:%05.2f%4d:%05.2f%4d:%05.2f%7d",
					Float2Time( GPSSt / m_GPSLog->m_dFreq ),
					Float2Time( GPSEd / m_GPSLog->m_dFreq ),
					Float2Time(( GPSEd - GPSSt ) / m_GPSLog->m_dFreq ),
					m_GPSLog->m_iLogNum
				);
				DrawText( POS_DEFAULT, POS_DEFAULT, szBuf, *m_pFont, color_white );
			}
		#endif	// !GPS_ONLY
	}
	
	return TRUE;
}
