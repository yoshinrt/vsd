/*****************************************************************************
	
	VSD -- vehicle data logger system  Copyright(C) by DDS
	
	CVsdFont.cpp - CVsdFont class implementation
	$Id$
	
*****************************************************************************/

#include <windows.h>
#include "dds.h"
#include "CVsdFont.h"

/*** macros *****************************************************************/

/*** static member **********************************************************/

/*** コンストラクタ *********************************************************/

CVsdFont::CVsdFont( LOGFONT &logfont ){
	
	BITMAPINFO	biBMP;
	HDC			hdc;
    HFONT		hFont, hFontOld;
    int			i;
	
	ZeroMemory( &biBMP, sizeof( biBMP ));
	
	/* 256*256ピクセル、32ビットDIB用BITMAPINFO設定 */
	biBMP.bmiHeader.biSize		= sizeof( BITMAPINFOHEADER );
	biBMP.bmiHeader.biBitCount	= 32;
	biBMP.bmiHeader.biPlanes	= 1;
	biBMP.bmiHeader.biWidth		= 8;
	biBMP.bmiHeader.biHeight	= -8;
	
	/* 仮DIBSection作成 */
	HBITMAP hbmpTemp = CreateDIBSection( NULL, &biBMP, DIB_RGB_COLORS, ( void ** )( &m_pFontData ), NULL, 0 );
	
	/* ウインドウのHDC取得 */
	HWND hwnd = GetDesktopWindow();	// AviSynth で hwnd が無いので，てきとう (^^;
	hdc = GetDC( hwnd );
	
	/* DIBSection用メモリデバイスコンテキスト作成 */
	m_hdcBMP = CreateCompatibleDC( hdc );
	
	/* ウインドウのHDC解放 */
	ReleaseDC( hwnd, hdc );
	
	/* DIBSectionのHBITMAPをメモリデバイスコンテキストに選択 */
	m_hbmpOld = ( HBITMAP )SelectObject( m_hdcBMP, hbmpTemp );
	
	hFont = CreateFontIndirect( &logfont );
	
	hFontOld = ( HFONT )SelectObject( m_hdcBMP, hFont );
	
	// フォントサイズ情報初期化
	SIZE size;
	GetTextExtentPoint32( m_hdcBMP, "B", 1, &size );	// W が一番幅が広い
	
	m_iFontH = size.cy;
	m_iFontW = size.cx;
	m_iBMP_W = m_iFontW * 16 * 2;
	
	/* 256*256ピクセル、32ビットDIB用BITMAPINFO設定 */
	biBMP.bmiHeader.biWidth		= m_iBMP_W;
	biBMP.bmiHeader.biHeight	= -m_iFontH * 7;
	
	/* DIBSection作成 */
	m_hbmpBMP = CreateDIBSection( NULL, &biBMP, DIB_RGB_COLORS, ( void ** )( &m_pFontData ), NULL, 0 );
	
	/* DIBSectionのHBITMAPをメモリデバイスコンテキストに選択 */
	SelectObject( m_hdcBMP, m_hbmpBMP );
	
	/* 仮DIBSectionを削除 */
	DeleteObject( hbmpTemp );
	
	// BMP を白に初期化
	for( i = 0; i < m_iBMP_W * m_iFontH * 7; ++i ){
		m_pFontData[ i ] = 0xFFFFFF;
	}
	
	// 文字を並べる
	char szChar[ 1 ];
	
	for( i = 0x20; i < 0x80; ++i ){
		*szChar = i;
		GetTextExtentPoint32( m_hdcBMP, szChar, 1, &size );
		
		TextOut(
			m_hdcBMP,
			( i % 16 ) * m_iFontW * 2 + ( m_iFontW * 2 - size.cx ) / 2,
			(( i - 0x20 ) / 16 ) * m_iFontH, szChar, 1
		);
	}
	
	SelectObject( m_hdcBMP, hFontOld );
	DeleteObject( hFont );
}

/*** デストラクタ ***********************************************************/

CVsdFont::~CVsdFont(){
	/* DIBSectionをメモリデバイスコンテキストの選択から外す */
	SelectObject( m_hdcBMP, m_hbmpOld );
	
	/* DIBSectionを削除 */
	DeleteObject( m_hbmpBMP );
	
	/* メモリデバイスコンテキストを削除 */
	DeleteDC( m_hdcBMP );
}
