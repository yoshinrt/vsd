/*****************************************************************************
	
	VSD -- vehicle data logger system  Copyright(C) by DDS
	
	CVsdFont.cpp - CVsdFont class implementation
	
*****************************************************************************/

#include "StdAfx.h"

#include "dds.h"
#include "CVsdFont.h"

/*** macros *****************************************************************/

/*** static member **********************************************************/

/*** コンストラクタ *********************************************************/

CVsdFont::CVsdFont( LOGFONT &logfont ){
	
	const MAT2		mat = {{ 0, 1 }, { 0, 0 }, { 0, 0 }, { 0, 1 }};
    GLYPHMETRICS	gm;
    int	i;
	
	// DC, FONT ハンドル取得
	HDC		hdc			= GetDC( NULL );
	HFONT	hFont		= CreateFontIndirect( &logfont );
	HFONT	hFontOld	= ( HFONT )SelectObject( hdc, hFont );
	
	// tmAscent 取得
	TEXTMETRIC tm;
	GetTextMetrics( hdc, &tm );
	
	int iSize;
	m_FontGlyph = new tFontGlyph[ '~' - '!' + 1 ];
	
	for( i = 0; i <= '~' - '!'; ++i ){
		// 必要配列サイズ取得
		iSize = GetGlyphOutline( hdc, '!' + i, GGO_GRAY8_BITMAP, &gm, 0, NULL, &mat );
		if( iSize > 0 ){
			m_FontGlyph[ i ].pBuf = new BYTE[ iSize ];
			
			// フォントデータ取得
			GetGlyphOutline( hdc, '!' + i, GGO_GRAY8_BITMAP, &gm, iSize, m_FontGlyph[ i ].pBuf, &mat );
			m_FontGlyph[ i ].iW		= gm.gmBlackBoxX;
			m_FontGlyph[ i ].iH		= gm.gmBlackBoxY;
			m_FontGlyph[ i ].iOrgY	= tm.tmAscent - gm.gmptGlyphOrigin.y;
			
			if( i == 'B' - '!' ){	// 'W' が一番幅が広い
				m_iFontW = gm.gmCellIncX;
			}
		}else{
			m_FontGlyph[ i ].pBuf	= NULL;
			m_FontGlyph[ i ].iW		=
			m_FontGlyph[ i ].iH		=
			m_FontGlyph[ i ].iOrgY	= 0;
		}
	}
	
	m_iFontH = tm.tmHeight;
	
	SelectObject( hdc, hFontOld );
	DeleteObject( hFont );
	ReleaseDC( NULL, hdc );			/* ウインドウのHDC解放 */
}

/*** デストラクタ ***********************************************************/

CVsdFont::~CVsdFont(){
	int i;

	for( i = 0; i <= '~' - '!'; ++i ){
		delete [] m_FontGlyph[ i ].pBuf;
	}
	delete [] m_FontGlyph;
	m_FontGlyph = NULL;
}
