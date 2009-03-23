/*****************************************************************************
	
	VSD ** vehicle data logger system  Copyright(C) by DDS
	
	CVsdImg.h - CVsdImg class header
	$Id$
	
*****************************************************************************/

#ifndef _CVsdImg_h_
#define _CVsdImg_h_

#define POS_DEFAULT		0x80000000
#define HIREZO_TH		600			// ハイレゾモード時の横幅スレッショルド

class CVsdImg : public FILTER_PROC_INFO {
	
  public:
	//void PutPixel( int x, int y, short iY, short iCr, short iCb );
	void PutPixel( int x, int y, const PIXEL_YC &yc, UINT uFlag );
	PIXEL_YC &GetPixel( int x, int y, UINT uFlag );
	void DrawLine( int x1, int y1, int x2, int y2, const PIXEL_YC &yc, UINT uFlag );
	void DrawLine( int x1, int y1, int x2, int y2, int width, const PIXEL_YC &yc, UINT uFlag );
	void FillLine( int x1, int y1, int x2,         const PIXEL_YC &yc, UINT uFlag );
	void DrawRect( int x1, int y1, int x2, int y2, const PIXEL_YC &yc, UINT uFlag );
	void DrawCircle( int x, int y, int r, const PIXEL_YC &yc, UINT uFlag );
	int GetIndex( int x, int y ){ return max_w * y + x; }
	
	PIXEL_YC &GetYC( PIXEL_YC &yc, int r, int g, int b ){
		yc.y  = ( int )( 0.299 * r + 0.587 * g + 0.114 * b + .5 );
		yc.cr = ( int )( 0.500 * r - 0.419 * g - 0.081 * b + .5 );
		yc.cb = ( int )(-0.169 * r - 0.332 * g + 0.500 * b + .5 );
		
		return yc;
	}
	
	void CopyRect(
		int	Sx1, int Sy1,
		int	Sx2, int Sy2,
		int Dx,  int Dy,
		const PIXEL_YC &yc, UINT uFlag
	);
	
	void DrawFont( int x, int y, UCHAR c, const PIXEL_YC &yc, UINT uFlag );
	void DrawFont( int x, int y, UCHAR c, const PIXEL_YC &yc, const PIXEL_YC &ycEdge, UINT uFlag );
	void DrawString( char *szMsg, const PIXEL_YC &yc, UINT uFlag, int x = POS_DEFAULT, int y = POS_DEFAULT );
	void DrawString( char *szMsg, const PIXEL_YC &yc, const PIXEL_YC &ycEdge, UINT uFlag, int x = POS_DEFAULT, int y = POS_DEFAULT );
	
	// ポリゴン描写
	void PolygonClear( void );
	void PolygonDraw( const PIXEL_YC &yc, UINT uFlag );
	
	// フォント
	UCHAR GetBMPPix( int x, int y ){
		return m_pFontData[ ( m_iBMP_H - y - 1 ) * m_iBMP_BytesPerLine + ( x >> 3 ) ] & ( 1 << ( 7 - ( x & 0x7 )));
	}
	
	int GetFontW( void ){ return m_iFontW; }
	int GetFontH( void ){ return m_iFontH; }
	void InitFont( void );
	
	enum {
		IMG_ALFA	= ( 1 << 0 ),
		IMG_TMP		= ( 1 << 1 ),
		IMG_FILL	= ( 1 << 2 ),
		IMG_TMP_DST	= ( 1 << 3 ),
		IMG_POLYGON	= ( 1 << 4 ),
	};
	
	static int m_iPosX, m_iPosY;
	
  private:
	static const UCHAR m_Font9p[];
	static const UCHAR m_Font18p[];
	
	static int	m_iFontW, m_iFontH, m_iBMP_H, m_iBMP_BytesPerLine;
	static const UCHAR *m_pFontData;
};

#endif
