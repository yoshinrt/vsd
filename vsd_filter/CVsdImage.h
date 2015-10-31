/*****************************************************************************
	
	VSD -- vehicle data logger system  Copyright(C) by DDS
	
	CVsdImage.cpp - Image file operation
	
*****************************************************************************/

#pragma once
#include "CPixel.h"
#include "CV8If.h"
#include "CSemaphore.h"

#define RABY_TRANSPARENT	0x80FF8000
#define IMG_INET_ASYNC		0x1

class CVsdImage : public CV8If {
  public:
	CVsdImage();
	CVsdImage( CVsdImage &Org );
	~CVsdImage();
	
	void DeleteAsync( void );
	
	UINT Load( LPCWSTR szFileName, UINT uFlag = 0 );
	
	inline UINT GetPixel0( int x, int y ){
		return m_pBuf[ ( x - m_iOffsX ) + ( y - m_iOffsY ) * m_iRawWidth ].ycbcr;
	}
	
	inline CPixelImg GetPixelRaw( int x, int y ){
		return m_pBuf[ x + y * m_iRawWidth ];
	}
	
	inline UINT GetPixel( int x, int y ){
		int x0 = x - m_iOffsX;
		int y0 = y - m_iOffsY;
		
		if( 0 <= x0 && x0 < m_iRawWidth && 0 <= y0 && y0 < m_iRawHeight ){
			return GetPixel0( x, y );
		}
		return RABY_TRANSPARENT;
	}
	
	template<typename Tx, typename Ty>
	UINT Resampling( Tx x, Ty y );
	UINT Resampling( double x0, double x1, int y );
	UINT Resampling( int x, double y0, double y1 );
	UINT Resize( int iWidth, int iHeight );			// !js_func
	UINT Rotate( int cx, int cy, double dAngle );	// !js_func
	UINT Clip( int x1, int y1, int x2, int y2 );
	
	UINT WaitAsyncLoadComplete( int iMsec );	// !js_func
	
	int m_iWidth;	// !js_var:Width
	int m_iHeight;	// !js_var:Height
	
	int	m_iStatus;	// !js_var:Status
	enum {
		IMG_STATUS_LOAD_COMPLETE,
		IMG_STATUS_LOAD_INCOMPLETE,
		IMG_STATUS_LOAD_FAILED,
		IMG_STATUS_OBJ_DESTROYED,
	};
	
	int m_iRawWidth;
	int m_iRawHeight;
	int m_iOffsX;
	int m_iOffsY;
	
	LPWSTR		m_pFileName;
	CSemaphore	* volatile m_pSemaphore;
  private:
	CPixelImg		*m_pBuf;
};
