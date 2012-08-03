/*****************************************************************************
	
	VSD -- vehicle data logger system  Copyright(C) by DDS
	
	CVsdImage.cpp - Image file operation
	
*****************************************************************************/

#define RABY_TRANSPARENT	0x00FF0000

typedef UINT	tRABY;

class PIXEL_RABY {
  public:
	union {
		UINT	raby;
		struct {
			UCHAR	y;
			char	b;
			UCHAR	a;
			char	r;
		};
	};
	
	PIXEL_RABY(){}
	~PIXEL_RABY(){}
	
	static inline UINT Argb2Raby( UINT uColor ){
		int a = ( uColor >> 24 );
		int r = ( uColor >> 16 ) & 0xFF;
		int g = ( uColor >>  8 ) & 0xFF;
		int b = ( uColor       ) & 0xFF;
		
		return Argb2Raby( a, r, g, b );
	}
	
	static inline UINT Argb2Raby(
		int a, int r, int g, int b
	){
		double dAlfa = ( 255 - a ) / 255.0;
		
		return
			((( int )(( 0.500 * r - 0.419 * g - 0.081 * b ) * dAlfa )        ) << 24 ) |
			( a << 16 ) |
			((( int )((-0.169 * r - 0.331 * g + 0.500 * b ) * dAlfa ) & 0xFF ) <<  8 ) |
			((( int )(( 0.299 * r + 0.587 * g + 0.114 * b ) * dAlfa ) & 0xFF )       );
	}
};

class CVsdImage {
  public:
	CVsdImage();
	~CVsdImage();
	
	UINT Load( const char *szFileName );
	
	inline UINT GetPixel0( int x, int y ){
		return m_pBuf[ x + y * m_iWidth ].raby;
	}
	
	inline UINT GetPixel( int x, int y ){
		if( 0 <= x && x < m_iWidth && 0 <= y && y < m_iHeight ){
			return GetPixel0( x, y );
		}
		
		if( x < 0 )	x = 0;
		else if( x >= m_iWidth ) x = m_iWidth - 1;
		
		if( y < 0 ) y = 0;
		else if( y >= m_iHeight ) y = m_iHeight - 1;
		
		return GetPixel0( x, y ) | RABY_TRANSPARENT;
	}
	
	template<typename Tx, typename Ty>
	UINT Resampling( Tx x, Ty y );
	UINT Resampling( double x0, double x1, int y );
	UINT Resampling( int x, double y0, double y1 );
	UINT Resize( int iWidth, int iHeight );			// !js_func
	UINT Rotate( int cx, int cy, double dAngle );	// !js_func
	
	int m_iWidth;	// !js_var:Width
	int m_iHeight;	// !js_var:Height
	
  private:
	PIXEL_RABY	*m_pBuf;
};
