/*****************************************************************************
	
	VSD -- vehicle data logger system  Copyright(C) by DDS
	
	CVsdImage.cpp - Image file operation
	
*****************************************************************************/

#include "StdAfx.h"

#include "dds.h"
#include "pixel.h"
#include "CVsdImage.h"

/*** コンストラクタ・デストラクタ *******************************************/

CVsdImage::CVsdImage(){
	m_pRGBA_Buf	= NULL;
	m_pPixelBuf	= NULL;
	
	m_iWidth  = 0;	// 画像の幅
	m_iHeight = 0;	//  〃  高さ
}

CVsdImage::~CVsdImage(){
	delete [] m_pRGBA_Buf;
	delete [] m_pPixelBuf;
}

/*** イメージのロード *******************************************************/

BOOL CVsdImage::Load( const char *szFileName ){
	// 文字コードをワイド文字列に変換
	// 【注意】本来はこの箇所は文字列バッファ長の考慮の他に文字列終端コードを処理するよりセキュアな対応が好ましいです。
	wchar_t	path[ MAX_PATH ];
	size_t	pathLength = 0;
	
	if(
		mbstowcs_s(
			&pathLength,	// [out]	変換された文字数
			&path[0],		// [out]	変換されたワイド文字列を格納するバッファのアドレス(変換先)
			MAX_PATH,		// [in]	 出力側のバッファのサイズ(単位:文字数)
			szFileName,		// [in]	 マルチバイト文字列のアドレス(変換元)
			_TRUNCATE		// [in]	 出力先に格納するワイド文字の最大数
		) != 0
	){
		return FALSE;
	}
	
	// GDI+オブジェクト（画像展開に必要）
	Gdiplus::GdiplusStartupInput	gdiplusStartupInput;
	ULONG_PTR						gdiplusToken;
	
	//---- GDI+の初期設定
	if( Gdiplus::GdiplusStartup( &gdiplusToken, &gdiplusStartupInput, NULL ) != Gdiplus::Ok ){
		return FALSE;
	}
	
	//-------------------------------------------------------------
	// 画像の読み込み
	//-------------------------------------------------------------
	BOOL	result = TRUE;
	m_iWidth  = 0;	// 画像の幅
	m_iHeight = 0;	//  〃  高さ
	
	//--- 画像ファイルを開く
	//  【対応画像形式】  BMP, JPEG, PNG, GIF, TIFF, WMF, EMF
	Gdiplus::Bitmap*	pBitmap = Gdiplus::Bitmap::FromFile( path );
	if( pBitmap && pBitmap->GetLastStatus() == Gdiplus::Ok ){
		//---- 画像サイズ分の領域確保
		m_iWidth  = pBitmap->GetWidth();
		m_iHeight = pBitmap->GetHeight();
		
		// ■ロードする画像のメモリの解像度を変更/設定（この位置に任意に記述して下さい）
		m_pRGBA_Buf = new PIXEL_RGBA[ m_iWidth * m_iHeight ];
		
		//---- 画像イメージの読み込み
		for( int y = 0; y < m_iHeight; ++y ){
			for( int x = 0; x < m_iWidth; ++x ){
				Gdiplus::Color srcColor;
				pBitmap->GetPixel( x, y, &srcColor );
				
				m_pRGBA_Buf[ x + y * m_iWidth ].r = srcColor.GetR();
				m_pRGBA_Buf[ x + y * m_iWidth ].g = srcColor.GetG();
				m_pRGBA_Buf[ x + y * m_iWidth ].b = srcColor.GetB();
				m_pRGBA_Buf[ x + y * m_iWidth ].a = 255 - srcColor.GetA();
			}
		}
	}
	
	delete pBitmap;
	
	//---- GDI+の解放
	Gdiplus::GdiplusShutdown( gdiplusToken );
	
	return result;
}

/*** PIXEL_RGBA -> PIXEL_YCA 変換 *******************************************/

BOOL CVsdImage::ConvRGBA2YCA( void ){
	
	if( m_pPixelBuf || !m_pRGBA_Buf ) return FALSE;
	
	// メモリ確保
	if( !( m_pPixelBuf = new PIXEL_YCA[ m_iWidth * m_iHeight ] )){
		return FALSE;
	}
	
	for( int y = 0; y < m_iHeight; ++y ){
		for( int x = 0; x < m_iWidth; ++x ){
			int iIdx = x + y * m_iWidth;
			Color2YCA( m_pPixelBuf[ iIdx ], m_pRGBA_Buf[ iIdx ].argb );
		}
	}
	
	delete [] m_pRGBA_Buf;
	m_pRGBA_Buf = NULL;
	
	return TRUE;
}
