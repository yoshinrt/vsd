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

CVsdFont::CVsdFont( const char *szFontName, int iSize, UINT uAttr ){
	m_uAttr = uAttr;
	
	LOGFONT	logfont;
	
	logfont.lfHeight			= iSize;						// 文字セルまたは文字の高さ
	logfont.lfWidth				= 0;							// 平均文字幅
	logfont.lfEscapement		= 0;							// 文字送りの方向とX軸との角度
	logfont.lfOrientation		= 0;							// ベースラインとX軸との角度
	logfont.lfWeight			= uAttr & ATTR_BOLD ? FW_BOLD : FW_REGULAR;	// フォントの太さ
	logfont.lfItalic			= uAttr & ATTR_ITALIC ? TRUE : FALSE;		// イタリック体指定
	logfont.lfUnderline			= FALSE;						// 下線付き指定
	logfont.lfStrikeOut			= FALSE;						// 打ち消し線付き指定
	logfont.lfCharSet			= DEFAULT_CHARSET;				// キャラクタセット
	logfont.lfOutPrecision		= OUT_DEFAULT_PRECIS;			// 出力精度
	logfont.lfClipPrecision		= CLIP_DEFAULT_PRECIS;			// クリッピングの精度
	logfont.lfQuality			= PROOF_QUALITY;				// 出力品質
	logfont.lfPitchAndFamily	= FIXED_PITCH | FF_DONTCARE;	// ピッチとファミリ
	strcpy( logfont.lfFaceName, szFontName );					// フォント名
	
	CreateFont( logfont );
}

/*** フォント作成 ***********************************************************/

void CVsdFont::CreateFont( LOGFONT &logfont ){
	
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
	
	// プロポーショナルの Space 幅取得
	GetGlyphOutline( hdc, ' ', GGO_GRAY8_BITMAP, &gm, 0, NULL, &mat );
	m_iFontW_Space = gm.gmCellIncX;
	
	for( i = FONT_CHAR_FIRST; i <= FONT_CHAR_LAST; ++i ){
		// 必要配列サイズ取得
		iSize = GetGlyphOutline( hdc, i, GGO_GRAY8_BITMAP, &gm, 0, NULL, &mat );
		
		if( iSize > 0 ){
			FontGlyph( i ).pBuf = new BYTE[ iSize ];
			
			// フォントデータ取得
			GetGlyphOutline( hdc, i, GGO_GRAY8_BITMAP, &gm, iSize, FontGlyph( i ).pBuf, &mat );
			FontGlyph( i ).iW			= gm.gmBlackBoxX;
			FontGlyph( i ).iH			= gm.gmBlackBoxY;
			FontGlyph( i ).iOrgY		= tm.tmAscent - gm.gmptGlyphOrigin.y;
			FontGlyph( i ).iCellIncX	= gm.gmCellIncX;
		}else{
			FontGlyph( i ).pBuf			= NULL;
			FontGlyph( i ).iW			=
			FontGlyph( i ).iH			=
			FontGlyph( i ).iOrgY		= 0;
			FontGlyph( i ).iCellIncX	= m_iFontW_Space;
		}
	}
	
	m_iFontW = FontGlyph( 'B' ).iCellIncX;	// 'W' が一番幅が広い
	m_iFontH = tm.tmHeight;
	
	SelectObject( hdc, hFontOld );
	DeleteObject( hFont );
	ReleaseDC( NULL, hdc );			/* ウインドウのHDC解放 */
}
