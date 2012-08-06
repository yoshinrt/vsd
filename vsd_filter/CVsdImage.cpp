/*****************************************************************************
	
	VSD -- vehicle data logger system  Copyright(C) by DDS
	
	CVsdImage.cpp - Image file operation
	
*****************************************************************************/

#include "StdAfx.h"

#include "dds.h"
#include "pixel.h"
#include "CVsdImage.h"
#include "error_code.h"

/*** コンストラクタ・デストラクタ *******************************************/

CVsdImage::CVsdImage(){
	m_pBuf	= NULL;
	
	m_iWidth  = 0;	// 画像の幅
	m_iHeight = 0;	//  〃  高さ
}

CVsdImage::CVsdImage( CVsdImage &Org ){
	*this = Org;
	m_pBuf = new PIXEL_RABY[ m_iWidth * m_iHeight ];
	memcpy( m_pBuf, Org.m_pBuf, sizeof( PIXEL_RABY ) * m_iWidth * m_iHeight );
}

CVsdImage::~CVsdImage(){
	delete [] m_pBuf;
}

/*** イメージのロード *******************************************************/

UINT CVsdImage::Load( const char *szFileName ){
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
		return ERR_MBSTOWCS;
	}
	
	// GDI+オブジェクト（画像展開に必要）
	Gdiplus::GdiplusStartupInput	gdiplusStartupInput;
	ULONG_PTR						gdiplusToken;
	
	//---- GDI+の初期設定
	if( Gdiplus::GdiplusStartup( &gdiplusToken, &gdiplusStartupInput, NULL ) != Gdiplus::Ok ){
		return ERR_GDIPLUS;
	}
	
	//-------------------------------------------------------------
	// 画像の読み込み
	//-------------------------------------------------------------
	UINT	result = ERR_OK;
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
		m_pBuf = new PIXEL_RABY[ m_iWidth * m_iHeight ];
		
		if( !m_pBuf ){
			result = ERR_NOT_ENOUGH_MEMORY;
		}else{
			//---- 画像イメージの読み込み
			for( int y = 0; y < m_iHeight; ++y ){
				for( int x = 0; x < m_iWidth; ++x ){
					Gdiplus::Color srcColor;
					pBitmap->GetPixel( x, y, &srcColor );
					
					m_pBuf[ x + y * m_iWidth ].raby = PIXEL_RABY::Argb2Raby(
						255 - srcColor.GetA(),
						srcColor.GetR(),
						srcColor.GetG(),
						srcColor.GetB()
					);
				}
			}
		}
	}else{
		result = ERR_FILE_NOT_FOUND;
	}
	
	delete pBitmap;
	
	//---- GDI+の解放
	Gdiplus::GdiplusShutdown( gdiplusToken );
	
	return result;
}

/*** リサンプリング *********************************************************/

template<typename Tx, typename Ty>
UINT CVsdImage::Resampling( Tx x, Ty y ){
	
	UINT uColorA;
	UINT uColorB;
	UINT uColorC;
	UINT uColorD;
	
	// 範囲外なら透明を返す
	if( x <= -1 || m_iWidth <= x || y <= -1 || m_iHeight <= y ){
		return RABY_TRANSPARENT;
	}
	
	// ( int )-0.5 が 0 になってしまうので
	int x0 = ( int )( x + 1 ) - 1; int x1 = x0 + 1;
	int y0 = ( int )( y + 1 ) - 1; int y1 = y0 + 1;
	
	uColorA = GetPixel( x0, y0 );
	uColorB = GetPixel( x1, y0 );
	uColorC = GetPixel( x0, y1 );
	uColorD = GetPixel( x1, y1 );
	
	Tx alfa = x - x0; Tx alfa_ = 1 - alfa;
	Ty beta = y - y0; Ty beta_ = 1 - beta;
	
	return
		// cr
		(( int )(
			(( uColorA >> 24 ) & 0xFF ) * alfa_ * beta_ +
			(( uColorB >> 24 ) & 0xFF ) * alfa  * beta_ +
			(( uColorC >> 24 ) & 0xFF ) * alfa_ * beta  +
			(( uColorD >> 24 ) & 0xFF ) * alfa  * beta
		) << 24 ) |
		// a
		(( int )(
			(( uColorA >> 16 ) & 0xFF ) * alfa_ * beta_ +
			(( uColorB >> 16 ) & 0xFF ) * alfa  * beta_ +
			(( uColorC >> 16 ) & 0xFF ) * alfa_ * beta  +
			(( uColorD >> 16 ) & 0xFF ) * alfa  * beta
		) << 16 ) |
		// cb
		(( int )(
			(( uColorA >>  8 ) & 0xFF ) * alfa_ * beta_ +
			(( uColorB >>  8 ) & 0xFF ) * alfa  * beta_ +
			(( uColorC >>  8 ) & 0xFF ) * alfa_ * beta  +
			(( uColorD >>  8 ) & 0xFF ) * alfa  * beta
		) <<  8 ) |
		// y
		(( int )(
			(( uColorA >>  0 ) & 0xFF ) * alfa_ * beta_ +
			(( uColorB >>  0 ) & 0xFF ) * alfa  * beta_ +
			(( uColorC >>  0 ) & 0xFF ) * alfa_ * beta  +
			(( uColorD >>  0 ) & 0xFF ) * alfa  * beta
		) <<  0 );
}

/*** 縮小リサイズ ***********************************************************/

// x 方向縮小
UINT CVsdImage::Resampling( double x0, double x1, int y ){
	// 1pix まるまる部分の計算
	int ir, ia, ib, iy;
	ir = ia = ib = iy = 0;
	
	for( int i = ( int )ceil( x0 ); i < ( int )ceil( x1 ); ++i ){
		UINT u = GetPixel0( i, y );
		ir += ( u >> 24 )       ;
		ia += ( u >> 16 ) & 0xFF;
		ib += ( u >>  8 ) & 0xFF;
		iy += ( u >>  0 ) & 0xFF;
	}
	
	double alfa;
	double a = ir;
	double r = ia;
	double g = ib;
	double b = iy;
	double sum = ceil( x1 ) - ceil( x0 );
	
	// 先頭の半端部分
	alfa = ceil( x0 ) - x0;
	if( alfa > 0 ){
		UINT u = GetPixel0(( int )x0, y );
		a += (( u >> 24 )        ) * alfa;
		r += (( u >> 16 ) & 0xFF ) * alfa;
		g += (( u >>  8 ) & 0xFF ) * alfa;
		b += (( u >>  0 ) & 0xFF ) * alfa;
		
		sum += alfa;
	}
	
	// 最後の半端部分
	alfa = x1 - floor( x1 );
	if( alfa > 0 ){
		UINT u = GetPixel0(( int )x1, y );
		a += (( u >> 24 )        ) * alfa;
		r += (( u >> 16 ) & 0xFF ) * alfa;
		g += (( u >>  8 ) & 0xFF ) * alfa;
		b += (( u >>  0 ) & 0xFF ) * alfa;
		
		sum += alfa;
	}
	
	return
		(( int )( a / sum ) << 24 ) |
		(( int )( r / sum ) << 16 ) |
		(( int )( g / sum ) <<  8 ) |
		(( int )( b / sum ) <<  0 );
}

// y 方向縮小
UINT CVsdImage::Resampling( int x, double y0, double y1 ){
	// 1pix まるまる部分の計算
	int ir, ia, ib, iy;
	ir = ia = ib = iy = 0;
	
	for( int i = ( int )ceil( y0 ); i < ( int )ceil( y1 ); ++i ){
		UINT u = GetPixel0( x, i );
		ir += ( u >> 24 )       ;
		ia += ( u >> 16 ) & 0xFF;
		ib += ( u >>  8 ) & 0xFF;
		iy += ( u >>  0 ) & 0xFF;
	}
	
	double alfa;
	double r = ir;
	double a = ia;
	double b = ib;
	double y = iy;
	double sum = ceil( y1 ) - ceil( y0 );
	
	// 先頭の半端部分
	alfa = ceil( y0 ) - y0;
	if( alfa > 0 ){
		UINT u = GetPixel0( x, ( int )y0 );
		r += (( u >> 24 )        ) * alfa;
		a += (( u >> 16 ) & 0xFF ) * alfa;
		b += (( u >>  8 ) & 0xFF ) * alfa;
		y += (( u >>  0 ) & 0xFF ) * alfa;
		
		sum += alfa;
	}
	
	// 最後の半端部分
	alfa = y1 - floor( y1 );
	if( alfa > 0 ){
		UINT u = GetPixel0( x, ( int )y1 );
		r += (( u >> 24 )        ) * alfa;
		a += (( u >> 16 ) & 0xFF ) * alfa;
		b += (( u >>  8 ) & 0xFF ) * alfa;
		y += (( u >>  0 ) & 0xFF ) * alfa;
		
		sum += alfa;
	}
	
	return
		(( int )( r / sum ) << 24 ) |
		(( int )( a / sum ) << 16 ) |
		(( int )( b / sum ) <<  8 ) |
		(( int )( y / sum ) <<  0 );
}

/*** リサイズ ***************************************************************/

UINT CVsdImage::Resize( int iWidth, int iHeight ){
	
	PIXEL_RABY *pNewBuf;
	
	// x, y 両方拡大の場合のみ，一気に拡大
	if( iWidth > m_iWidth && iHeight > m_iHeight ){
		
		pNewBuf = new PIXEL_RABY[ iWidth * iHeight ];
		
		if( !pNewBuf ) return ERR_NOT_ENOUGH_MEMORY;
		
		#ifdef _OPENMP
			#pragma omp parallel for
		#endif
		for( int y = 0; y < iHeight; ++y ) for( int x = 0; x < iWidth; ++x ){
			pNewBuf[ x + iWidth * y ].raby = Resampling(
				m_iWidth  * x / ( double )iWidth,
				m_iHeight * y / ( double )iHeight
			);
		}
		
		delete [] m_pBuf;
		m_pBuf = pNewBuf;
		
		m_iWidth  = iWidth;
		m_iHeight = iHeight;
	}else{
		/*** x 方向の処理 ***/
		
		if( iWidth != m_iWidth ){
			pNewBuf = new PIXEL_RABY[ iWidth * m_iHeight ];
			if( !pNewBuf ) return ERR_NOT_ENOUGH_MEMORY;
			
			if( iWidth > m_iWidth ){
				// x 拡大
				#ifdef _OPENMP
					#pragma omp parallel for
				#endif
				for( int y = 0; y < m_iHeight; ++y ) for( int x = 0; x < iWidth; ++x ){
					pNewBuf[ x + iWidth * y ].raby = Resampling(
						m_iWidth  * x / ( double )iWidth, y
					);
				}
			}else{
				// x 縮小
				#ifdef _OPENMP
					#pragma omp parallel for
				#endif
				for( int y = 0; y < m_iHeight; ++y ) for( int x = 0; x < iWidth; ++x ){
					pNewBuf[ x + iWidth * y ].raby = Resampling(
						m_iWidth * x         / ( double )iWidth,
						m_iWidth * ( x + 1 ) / ( double )iWidth,
						y
					);
				}
			}
			
			delete [] m_pBuf;
			m_pBuf = pNewBuf;
			m_iWidth = iWidth;
		}
		
		/*** y 方向の処理 ***/
		
		if( iHeight != m_iHeight ){
			pNewBuf = new PIXEL_RABY[ iWidth * iHeight ];
			if( !pNewBuf ) return ERR_NOT_ENOUGH_MEMORY;
			
			if( iHeight > m_iHeight ){
				// y 拡大
				#ifdef _OPENMP
					#pragma omp parallel for
				#endif
				for( int y = 0; y < iHeight; ++y ) for( int x = 0; x < iWidth; ++x ){
					pNewBuf[ x + iWidth * y ].raby = Resampling(
						x, m_iHeight * y / ( double )iHeight
					);
				}
			}else{
				// y 縮小
				#ifdef _OPENMP
					#pragma omp parallel for
				#endif
				for( int y = 0; y < iHeight; ++y ) for( int x = 0; x < iWidth; ++x ){
					pNewBuf[ x + iWidth * y ].raby = Resampling(
						x,
						m_iHeight * y         / ( double )iHeight,
						m_iHeight * ( y + 1 ) / ( double )iHeight
					);
				}
			}
			
			delete [] m_pBuf;
			m_pBuf = pNewBuf;
			m_iHeight = iHeight;
		}
	}
	
	return ERR_OK;
}

/*** 回転 *******************************************************************/

UINT CVsdImage::Rotate( int cx, int cy, double dAngle ){
	
	PIXEL_RABY *pNewBuf = new PIXEL_RABY[ m_iWidth * m_iHeight ];
	if( !pNewBuf ) return ERR_NOT_ENOUGH_MEMORY;
	
	double dSin = sin( dAngle * ToRAD );
	double dCos = cos( dAngle * ToRAD );
	
	#ifdef _OPENMP
		#pragma omp parallel for
	#endif
	for( int y = 0; y < m_iHeight; ++y ) for( int x = 0; x < m_iWidth; ++x ){
		pNewBuf[ x + m_iWidth * y ].raby = Resampling(
			cx + ( x - cx ) * dCos + ( y - cy ) * dSin,
			cy - ( x - cx ) * dSin + ( y - cy ) * dCos
		);
	}
	
	delete [] m_pBuf;
	m_pBuf = pNewBuf;
	
	return ERR_OK;
}
