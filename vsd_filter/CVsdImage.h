/*****************************************************************************
	
	VSD -- vehicle data logger system  Copyright(C) by DDS
	
	CVsdImage.cpp - Image file operation
	
*****************************************************************************/

typedef union {
	struct {
		UCHAR	b;
		UCHAR	g;
		UCHAR	r;
		UCHAR	a;
	};
	
	UINT	argb;
} PIXEL_RGBA;

class CVsdImage {
  public:
	CVsdImage();
	~CVsdImage();
	
	UINT Load( const char *szFileName );
	
	inline UINT GetPixel0( int x, int y );
	inline UINT GetPixel( int x, int y );
	template<typename Tx, typename Ty>
	UINT Resampling( Tx x, Ty y );
	UINT Resampling( double x0, double x1, int y );
	UINT Resampling( int x, double y0, double y1 );
	UINT Resize( int iWidth, int iHeight );			// !js_func
	UINT Rotate( int cx, int cy, double dAngle );	// !js_func
	
	UINT ConvRGBA2YCA( void );
	
	int m_iWidth;	// !js_var:Width
	int m_iHeight;	// !js_var:Height
	
	PIXEL_RGBA	*m_pRGBA_Buf;
	PIXEL_YCA	*m_pPixelBuf;
};
