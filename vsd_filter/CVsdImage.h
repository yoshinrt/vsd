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
	
	BOOL Load( const char *szFileName );
	BOOL ConvRGBA2YCA( void );
	
	int m_iWidth;	// !js_var:Width
	int m_iHeight;	// !js_var:Height
	
	PIXEL_RGBA	*m_pRGBA_Buf;
	PIXEL_YCA	*m_pPixelBuf;
};
