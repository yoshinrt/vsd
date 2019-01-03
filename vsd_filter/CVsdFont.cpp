/*****************************************************************************
	
	VSD -- vehicle data logger system  Copyright(C) by DDS
	
	CVsdFont.cpp - CVsdFont class implementation
	
*****************************************************************************/

#include "StdAfx.h"
#include "CVsdFont.h"

/*** macros *****************************************************************/

/*** static member **********************************************************/

#ifdef DEBUG
int CVsdFont::m_iObjCnt = 0;
#endif

/*** コンストラクタ *********************************************************/

CVsdFont::CVsdFont( const char *szFontName, int iSize, UINT uAttr ){
	//DebugMsgD( "new CFont %d:%X\n", ++m_iObjCnt, this );
	CreateFont( szFontName, iSize, uAttr );
}

CVsdFont::CVsdFont( LPCWSTR szFontName, int iSize, UINT uAttr ){
	//DebugMsgD( "new CFont %d:%X\n", ++m_iObjCnt, this );
	char szFont[ LF_FACESIZE ];
	
	WideCharToMultiByte(
		CP_ACP,				// コードページ
		0,					// 処理速度とマッピング方法を決定するフラグ
		szFontName,			// ワイド文字列のアドレス
		-1,					// ワイド文字列の文字数
		szFont,				// 新しい文字列を受け取るバッファのアドレス
		sizeof( szFont ),	// 新しい文字列を受け取るバッファのサイズ
		NULL,				// マップできない文字の既定値のアドレス
		NULL				// 既定の文字を使ったときにセットするフラグのアドレス
	);
	
	CreateFont( szFont, iSize, uAttr );
}

/*** フォント作成 ***********************************************************/

void CVsdFont::CreateFont( const char *szFontName, int iSize, UINT uAttr ){
	m_uAttr = uAttr;
	
	m_LogFont.lfHeight			= iSize;						// 文字セルまたは文字の高さ
	m_LogFont.lfWidth			= 0;							// 平均文字幅
	m_LogFont.lfEscapement		= 0;							// 文字送りの方向とX軸との角度
	m_LogFont.lfOrientation		= 0;							// ベースラインとX軸との角度
	m_LogFont.lfWeight			= uAttr & ATTR_BOLD ? FW_BOLD : FW_REGULAR;	// フォントの太さ
	m_LogFont.lfItalic			= uAttr & ATTR_ITALIC ? TRUE : FALSE;		// イタリック体指定
	m_LogFont.lfUnderline		= FALSE;						// 下線付き指定
	m_LogFont.lfStrikeOut		= FALSE;						// 打ち消し線付き指定
	m_LogFont.lfCharSet			= DEFAULT_CHARSET;				// キャラクタセット
	m_LogFont.lfOutPrecision	= OUT_DEFAULT_PRECIS;			// 出力精度
	m_LogFont.lfClipPrecision	= CLIP_DEFAULT_PRECIS;			// クリッピングの精度
	m_LogFont.lfQuality			= PROOF_QUALITY;				// 出力品質
	m_LogFont.lfPitchAndFamily	= FIXED_PITCH | FF_DONTCARE;	// ピッチとファミリ
	strcpy( m_LogFont.lfFaceName, szFontName );					// フォント名
	
	CreateFont();
}

const MAT2 CVsdFont::mat = {{ 0, 1 }, { 0, 0 }, { 0, 0 }, { 0, 1 }};

void CVsdFont::CreateFont( void ){
	
	// DC, FONT ハンドル取得
	HDC		hdc			= GetDC( NULL );
	HFONT	hFont		= CreateFontIndirect( &m_LogFont );
	HFONT	hFontOld	= ( HFONT )SelectObject( hdc, hFont );
	
	// tmAscent 取得
	TEXTMETRIC tm;
	GetTextMetrics( hdc, &tm );
    GLYPHMETRICS	gm;
	
	int	iBitmapDepth = IsNoAntialias() ? GGO_BITMAP : GGO_GRAY8_BITMAP;
	
	// プロポーショナルの Space 幅取得
	GetGlyphOutlineW( hdc, ' ', iBitmapDepth, &gm, 0, NULL, &mat );
	m_iFontW_Space = gm.gmCellIncX;
	
	GetGlyphOutlineW( hdc, 'B', iBitmapDepth, &gm, 0, NULL, &mat );
	m_iFontW = gm.gmCellIncX;	// 'W' が一番幅が広い
	m_iFontH = tm.tmHeight;
	
	SelectObject( hdc, hFontOld );
	DeleteObject( hFont );
	ReleaseDC( NULL, hdc );			/* ウインドウのHDC解放 */
}

// 漢字 glyph 作成
CFontGlyph& CVsdFont::CreateFontGlyph( WCHAR c ){
	
	// DC, FONT ハンドル取得
	HDC		hdc			= GetDC( NULL );
	HFONT	hFont		= CreateFontIndirect( &m_LogFont );
	HFONT	hFontOld	= ( HFONT )SelectObject( hdc, hFont );
	
	// tmAscent 取得
	TEXTMETRIC tm;
	GetTextMetrics( hdc, &tm );
    GLYPHMETRICS	gm;
	
	int	iBitmapDepth = IsNoAntialias() ? GGO_BITMAP : GGO_GRAY8_BITMAP;
	
	// 必要配列サイズ取得
	int iSize = GetGlyphOutlineW( hdc, c, iBitmapDepth, &gm, 0, NULL, &mat );
	
	CFontGlyph	*pGlyph;
	if( c <= FONT_CHAR_LAST ){
		pGlyph = &m_FontGlyph[ c - FONT_CHAR_FIRST ];
	}else{
		pGlyph = new CFontGlyph;
	}
	
	if( iSize > 0 ){
		pGlyph->pBuf = new BYTE[ iSize ];
		
		// フォントデータ取得
		GetGlyphOutlineW( hdc, c, iBitmapDepth, &gm, iSize, pGlyph->pBuf, &mat );
		pGlyph->iW			= gm.gmBlackBoxX;
		pGlyph->iH			= gm.gmBlackBoxY;
		pGlyph->iOrgY		= ( short )( tm.tmAscent - gm.gmptGlyphOrigin.y );
		pGlyph->iCellIncX	= ( short )gm.gmCellIncX;
	}else{
		pGlyph->pBuf		= NULL;
		pGlyph->iW			=
		pGlyph->iH			=
		pGlyph->iOrgY		= 0;
		pGlyph->iCellIncX	= m_iFontW_Space;
	}
	
	if( c > FONT_CHAR_LAST ) m_FontGlyphK[ c ] = pGlyph;
	
	SelectObject( hdc, hFontOld );
	DeleteObject( hFont );
	ReleaseDC( NULL, hdc );			/* ウインドウのHDC解放 */
	
	return *pGlyph;
}
