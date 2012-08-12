/*****************************************************************************
	
	VSD -- vehicle data logger system  Copyright(C) by DDS
	
	CVsdImage.cpp - Image file operation
	
*****************************************************************************/

#define RABY_TRANSPARENT	0x80FF8000

typedef UINT	tRABY;

class PIXEL_RABY {
  public:
	union {
		UINT	raby;
		struct {
			UCHAR	y;
			UCHAR	b;
			UCHAR	a;
			UCHAR	r;
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
		
		return (
			((( int )(( 0.500 * r - 0.419 * g - 0.081 * b ) * dAlfa )        ) << 24 ) |
			( a << 16 ) |
			((( int )((-0.169 * r - 0.331 * g + 0.500 * b ) * dAlfa ) & 0xFF ) <<  8 ) |
			((( int )(( 0.299 * r + 0.587 * g + 0.114 * b ) * dAlfa ) & 0xFF )       )
		) ^ 0x80008000;
	}
};

class CVsdImage {
  public:
	CVsdImage();
	CVsdImage( CVsdImage &Org );
	~CVsdImage();
	
	UINT Load( LPCWSTR szFileName );
	
	inline UINT GetPixel0( int x, int y ){
		return m_pBuf[ x + y * m_iWidth ].raby;
	}
	
	inline UINT GetPixel( int x, int y ){
		if( 0 <= x && x < m_iWidth && 0 <= y && y < m_iHeight ){
			return GetPixel0( x, y );
		}
		
		/*
		if( x < 0 )	x = 0;
		else if( x >= m_iWidth ) x = m_iWidth - 1;
		
		if( y < 0 ) y = 0;
		else if( y >= m_iHeight ) y = m_iHeight - 1;
		
		return GetPixel0( x, y ) | RABY_TRANSPARENT;
		*/
		return RABY_TRANSPARENT;
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

/****************************************************************************/

											//   AARRGGBB
#define	color_black		PIXEL_RABY::Argb2Raby( 0x00000000 )
#define	color_white		PIXEL_RABY::Argb2Raby( 0x00FFFFFF )
#define	color_blue		PIXEL_RABY::Argb2Raby( 0x000000FF )
#define	color_red		PIXEL_RABY::Argb2Raby( 0x00FF0000 )
#define	color_cyan		PIXEL_RABY::Argb2Raby( 0x0000FFFF )
#define	color_orange	PIXEL_RABY::Argb2Raby( 0x00FF4000 )
#define	color_gray_a	PIXEL_RABY::Argb2Raby( 0x80404040 )
#define	color_black_a	PIXEL_RABY::Argb2Raby( 0x40000000 )
