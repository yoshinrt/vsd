/*****************************************************************************
	
	VSD -- vehicle data logger system  Copyright(C) by DDS
	
	CVsdImage.cpp - Image file operation
	
*****************************************************************************/

#include "StdAfx.h"

#include "CVsdImage.h"
#include "error_code.h"

/*** コンストラクタ・デストラクタ *******************************************/

CVsdImage::CVsdImage(){
	DebugMsgD( "new CVsdImage %X\n", this );
	m_pBuf	= NULL;
	
	m_iWidth  = 0;	// 画像の幅
	m_iHeight = 0;	//  〃  高さ
	m_iOffsX = m_iOffsY = 0;
	
	m_iStatus	= IMG_STATUS_LOAD_INCOMPLETE;
	m_pFileName	= NULL;
}

CVsdImage::CVsdImage( CVsdImage &Org ){
	DebugMsgD( "new CVsdImage %X\n", this );
	*this = Org;
	m_pBuf = new PIXEL_RABY[ m_iRawWidth * m_iRawHeight ];
	memcpy( m_pBuf, Org.m_pBuf, sizeof( PIXEL_RABY ) * m_iRawWidth * m_iRawHeight );
}

CVsdImage::~CVsdImage(){
	DebugMsgD( "delete CVsdImage %X\n", this );
	
	if( m_pFileName ) delete [] m_pFileName;
	delete [] m_pBuf;
}

/*** イメージのロード *******************************************************/

DWORD WINAPI CVsdImage_LoadAsync(
	LPVOID lpParameter   // スレッドのデータ
){
	CVsdImage *pImg = reinterpret_cast<CVsdImage *>( lpParameter );
	pImg->Load( pImg->m_pFileName );
	delete [] pImg->m_pFileName;
	pImg->m_pFileName = NULL;

	return 0;
}

UINT CVsdImage::Load( LPCWSTR szFileName, UINT uFlag ){
	
	//-------------------------------------------------------------
	// 画像の読み込み
	//-------------------------------------------------------------
	UINT	result = ERR_OK;
	m_iWidth =	// 画像の幅
	m_iHeight =	//  〃  高さ
	m_iRawWidth =
	m_iRawHeight = 0;
	
	int	iMinX = MAXINT;
	int iMinY = MAXINT;
	int iMaxX = MININT;
	int iMaxY = MININT;
	
	UINT	raby;
	Gdiplus::Bitmap*	pBitmap = NULL;
	
	if(
		wcsncmp( szFileName, L"http://",  7 ) == 0 ||
		wcsncmp( szFileName, L"https://", 8 ) == 0
	){
		if( uFlag & IMG_INET_ASYNC ){
			// 非同期読み込みなのでスレッド起動
			UINT uSize = wcslen( szFileName ) + 1;
			m_pFileName = new WCHAR[ uSize ];
			wcscpy_s( m_pFileName, uSize, szFileName );
			
			CreateThread(
				NULL,					// セキュリティ記述子
				0,						// 初期のスタックサイズ
				CVsdImage_LoadAsync,	// スレッドの機能
				this,					// スレッドの引数
				0,						// 作成オプション
				NULL					// スレッド識別子
			);
			
			return result;
		}
		
		// URL で開く
		HINTERNET	hInternet	= NULL;
		HINTERNET	hFile		= NULL;
		HGLOBAL		hBuffer		= NULL;
		void*		pBuffer		= NULL;
		
		DWORD		dwReadSize;
		BOOL		bResult;
		
		do{
			if(
				!(
					(
						/* WININET初期化 */
						hInternet = InternetOpen(
							"WININET Sample Program",
							INTERNET_OPEN_TYPE_PRECONFIG,
							NULL, NULL, 0
						)
					) && (
						/* URLのオープン */
						hFile = InternetOpenUrlW( hInternet, szFileName, NULL, 0, 0, 0 )
					)
				)
			){
				#ifdef DEBUG
					DWORD	dwError = GetLastError();
					WCHAR	szBuffer[ 1024 ];
					DWORD	dwBufferLength = 1024;
					InternetGetLastResponseInfoW(
						&dwError,
						szBuffer,
						&dwBufferLength
					);
				#endif
				result		= ERR_WININET;
				m_iStatus	= IMG_STATUS_LOAD_FAILED;
				break;
			}
			
			// グローバルメモリ確保
			#define IMG_BUF_SIZE ( 512 * 1024 )
			
			if(
				!(
					(
						hBuffer = GlobalAlloc( GMEM_MOVEABLE, IMG_BUF_SIZE )
					) && (
						pBuffer = GlobalLock( hBuffer )
					)
				)
			){
				result		= ERR_GLOBAL_MEM;
				m_iStatus	= IMG_STATUS_LOAD_FAILED;
				break;
			}
			
			/* オープンしたURLからデータを(BUF_SIZEバイトずつ)読み込む */
			UINT	uTotalSize = 0;
			
			for(;;){
				bResult = InternetReadFile( hFile, ( char *)pBuffer + uTotalSize, IMG_BUF_SIZE - uTotalSize, &dwReadSize );
				
				/* 全て読み込んだらループを抜ける */
				if( bResult && ( dwReadSize == 0 )) break;
				uTotalSize += dwReadSize;
			}
			
	        IStream* pIStream;
			if( CreateStreamOnHGlobal( hBuffer, FALSE, &pIStream ) == S_OK )
			pBitmap = Gdiplus::Bitmap::FromStream( pIStream );
		}while( 0 );
		
		/* 後処理 */
		if( hFile )		InternetCloseHandle( hFile );
		if( hInternet )	InternetCloseHandle( hInternet );
		if( pBuffer )	GlobalUnlock( hBuffer );
		if( hBuffer )	GlobalFree( hBuffer );
		
	}else{
		//--- 画像ファイルを開く
		//  【対応画像形式】  BMP, JPEG, PNG, GIF, TIFF, WMF, EMF
		pBitmap = Gdiplus::Bitmap::FromFile( szFileName );
	}
	
	if( pBitmap && pBitmap->GetLastStatus() == Gdiplus::Ok ){
		//---- 画像サイズ分の領域確保
		m_iWidth  = pBitmap->GetWidth();
		m_iHeight = pBitmap->GetHeight();
		
		// ■ロードする画像のメモリの解像度を変更/設定（この位置に任意に記述して下さい）
		m_pBuf = new PIXEL_RABY[ m_iWidth * m_iHeight ];
		
		if( !m_pBuf ){
			result		= ERR_NOT_ENOUGH_MEMORY;
			m_iStatus	= IMG_STATUS_LOAD_FAILED;
		}else{
			//---- 画像イメージの読み込み
			for( int y = 0; y < m_iHeight; ++y ){
				for( int x = 0; x < m_iWidth; ++x ){
					Gdiplus::Color srcColor;
					pBitmap->GetPixel( x, y, &srcColor );
					
					m_pBuf[ x + y * m_iWidth ].raby = raby = PIXEL_RABY::Argb2Raby(
						255 - srcColor.GetA(),
						srcColor.GetR(),
						srcColor.GetG(),
						srcColor.GetB()
					);
					
					if( raby != RABY_TRANSPARENT ){
						if     ( iMinX > x ) iMinX = x;
						else if( iMaxX < x ) iMaxX = x;
						if     ( iMinY > y ) iMinY = y;
						else if( iMaxY < y ) iMaxY = y;
					}
				}
			}
		}
	}else{
		result		= ERR_FILE_NOT_FOUND;
		m_iStatus	= IMG_STATUS_LOAD_FAILED;
	}
	
	delete pBitmap;
	
	Clip( iMinX, iMinY, iMaxX, iMaxY );
	
	if( result == ERR_OK ) m_iStatus	= IMG_STATUS_LOAD_COMPLETE;
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
		UINT u = GetPixel( i, y );
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
		UINT u = GetPixel(( int )x0, y );
		a += (( u >> 24 )        ) * alfa;
		r += (( u >> 16 ) & 0xFF ) * alfa;
		g += (( u >>  8 ) & 0xFF ) * alfa;
		b += (( u >>  0 ) & 0xFF ) * alfa;
		
		sum += alfa;
	}
	
	// 最後の半端部分
	alfa = x1 - floor( x1 );
	if( alfa > 0 ){
		UINT u = GetPixel(( int )x1, y );
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
		UINT u = GetPixel( x, i );
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
		UINT u = GetPixel( x, ( int )y0 );
		r += (( u >> 24 )        ) * alfa;
		a += (( u >> 16 ) & 0xFF ) * alfa;
		b += (( u >>  8 ) & 0xFF ) * alfa;
		y += (( u >>  0 ) & 0xFF ) * alfa;
		
		sum += alfa;
	}
	
	// 最後の半端部分
	alfa = y1 - floor( y1 );
	if( alfa > 0 ){
		UINT u = GetPixel( x, ( int )y1 );
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
	
	int	iMinX = MAXINT;
	int iMinY = MAXINT;
	int iMaxX = MININT;
	int iMaxY = MININT;
	
	UINT	raby;
	
	// x, y 両方拡大の場合のみ，一気に拡大
	if( iWidth > m_iWidth && iHeight > m_iHeight ){
		
		pNewBuf = new PIXEL_RABY[ iWidth * iHeight ];
		
		if( !pNewBuf ) return ERR_NOT_ENOUGH_MEMORY;
		
		#ifdef _OPENMP_AVS
			#pragma omp parallel for
		#endif
		for( int y = 0; y < iHeight; ++y ) for( int x = 0; x < iWidth; ++x ){
			pNewBuf[ x + iWidth * y ].raby = raby = Resampling(
				m_iWidth  * x / ( double )iWidth,
				m_iHeight * y / ( double )iHeight
			);
			
			if( raby != RABY_TRANSPARENT ){
				if     ( iMinX > x ) iMinX = x;
				else if( iMaxX < x ) iMaxX = x;
				if     ( iMinY > y ) iMinY = y;
				else if( iMaxY < y ) iMaxY = y;
			}
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
				#ifdef _OPENMP_AVS
					#pragma omp parallel for
				#endif
				for( int y = 0; y < m_iHeight; ++y ) for( int x = 0; x < iWidth; ++x ){
					pNewBuf[ x + iWidth * y ].raby = raby = Resampling(
						m_iWidth  * x / ( double )iWidth, y
					);
					if( raby != RABY_TRANSPARENT ){
						if     ( iMinX > x ) iMinX = x;
						else if( iMaxX < x ) iMaxX = x;
						if     ( iMinY > y ) iMinY = y;
						else if( iMaxY < y ) iMaxY = y;
					}
				}
			}else{
				// x 縮小
				#ifdef _OPENMP_AVS
					#pragma omp parallel for
				#endif
				for( int y = 0; y < m_iHeight; ++y ) for( int x = 0; x < iWidth; ++x ){
					pNewBuf[ x + iWidth * y ].raby = raby = Resampling(
						m_iWidth * x         / ( double )iWidth,
						m_iWidth * ( x + 1 ) / ( double )iWidth,
						y
					);
					if( raby != RABY_TRANSPARENT ){
						if     ( iMinX > x ) iMinX = x;
						else if( iMaxX < x ) iMaxX = x;
						if     ( iMinY > y ) iMinY = y;
						else if( iMaxY < y ) iMaxY = y;
					}
				}
			}
			
			delete [] m_pBuf;
			m_pBuf = pNewBuf;
			m_iWidth = iWidth;
			
			m_iRawWidth  = m_iWidth;
			m_iRawHeight = m_iHeight;
			m_iOffsX = m_iOffsY = 0;
		}
		
		/*** y 方向の処理 ***/
		
		if( iHeight != m_iHeight ){
			iMinX = MAXINT;
			iMinY = MAXINT;
			iMaxX = MININT;
			iMaxY = MININT;
			
			pNewBuf = new PIXEL_RABY[ iWidth * iHeight ];
			if( !pNewBuf ) return ERR_NOT_ENOUGH_MEMORY;
			
			if( iHeight > m_iHeight ){
				// y 拡大
				#ifdef _OPENMP_AVS
					#pragma omp parallel for
				#endif
				for( int y = 0; y < iHeight; ++y ) for( int x = 0; x < iWidth; ++x ){
					pNewBuf[ x + iWidth * y ].raby = raby = Resampling(
						x, m_iHeight * y / ( double )iHeight
					);
					if( raby != RABY_TRANSPARENT ){
						if     ( iMinX > x ) iMinX = x;
						else if( iMaxX < x ) iMaxX = x;
						if     ( iMinY > y ) iMinY = y;
						else if( iMaxY < y ) iMaxY = y;
					}
				}
			}else{
				// y 縮小
				#ifdef _OPENMP_AVS
					#pragma omp parallel for
				#endif
				for( int y = 0; y < iHeight; ++y ) for( int x = 0; x < iWidth; ++x ){
					pNewBuf[ x + iWidth * y ].raby = raby = Resampling(
						x,
						m_iHeight * y         / ( double )iHeight,
						m_iHeight * ( y + 1 ) / ( double )iHeight
					);
					if( raby != RABY_TRANSPARENT ){
						if     ( iMinX > x ) iMinX = x;
						else if( iMaxX < x ) iMaxX = x;
						if     ( iMinY > y ) iMinY = y;
						else if( iMaxY < y ) iMaxY = y;
					}
				}
			}
			
			delete [] m_pBuf;
			m_pBuf = pNewBuf;
			m_iHeight = iHeight;
			
			m_iRawWidth  = m_iWidth;
			m_iRawHeight = m_iHeight;
			m_iOffsX = m_iOffsY = 0;
		}
	}
	
	if( iMinX != MAXINT ){
		Clip( iMinX, iMinY, iMaxX, iMaxY );
	}
	return ERR_OK;
}

/*** 回転 *******************************************************************/

UINT CVsdImage::Rotate( int cx, int cy, double dAngle ){
	
	PIXEL_RABY *pNewBuf = new PIXEL_RABY[ m_iWidth * m_iHeight ];
	if( !pNewBuf ) return ERR_NOT_ENOUGH_MEMORY;
	
	double dSin = sin( dAngle * ToRAD );
	double dCos = cos( dAngle * ToRAD );
	
	int	iMinX = MAXINT;
	int iMinY = MAXINT;
	int iMaxX = MININT;
	int iMaxY = MININT;
	
	UINT	raby;
	
	#ifdef _OPENMP_AVS
		#pragma omp parallel for
	#endif
	for( int y = 0; y < m_iHeight; ++y ) for( int x = 0; x < m_iWidth; ++x ){
		pNewBuf[ x + m_iWidth * y ].raby = raby = Resampling(
			cx + ( x - cx ) * dCos + ( y - cy ) * dSin,
			cy - ( x - cx ) * dSin + ( y - cy ) * dCos
		);
		
		if( raby != RABY_TRANSPARENT ){
			if     ( iMinX > x ) iMinX = x;
			else if( iMaxX < x ) iMaxX = x;
			if     ( iMinY > y ) iMinY = y;
			else if( iMaxY < y ) iMaxY = y;
		}
	}
	
	delete [] m_pBuf;
	m_pBuf = pNewBuf;
	
	Clip( iMinX, iMinY, iMaxX, iMaxY );
	
	return ERR_OK;
}

/*** 有効部分だけクリップ ***************************************************/

UINT CVsdImage::Clip( int x1, int y1, int x2, int y2 ){
	
	int iNewWidth	= x2 - x1 + 1;
	int iNewHeight	= y2 - y1 + 1;
	
	m_iRawWidth  = m_iWidth;
	m_iRawHeight = m_iHeight;
	m_iOffsX = m_iOffsY = 0;
	
	PIXEL_RABY *pNewBuf = new PIXEL_RABY[ iNewWidth * iNewHeight ];
	if( !pNewBuf ) return ERR_NOT_ENOUGH_MEMORY;
	
	#ifdef _OPENMP_AVS
		#pragma omp parallel for
	#endif
	for( int y = 0; y < iNewHeight; ++y ) for( int x = 0; x < iNewWidth; ++x ){
		pNewBuf[ x + y * iNewWidth ].raby = GetPixel( x + x1, y + y1 );
	}
	
	delete [] m_pBuf;
	m_pBuf = pNewBuf;
	
	m_iRawWidth		= iNewWidth;
	m_iRawHeight	= iNewHeight;
	m_iOffsX		= x1;
	m_iOffsY		= y1;
	
	return ERR_OK;
}
