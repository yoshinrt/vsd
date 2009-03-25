/*****************************************************************************
	
	VSD -- vehicle data logger system  Copyright(C) by DDS
	
	CVsdFilter.h - CVsdFilter class header
	$Id$
	
*****************************************************************************/

#ifndef _CVsdLog_h_
#define _CVsdLog_h_

#define BUF_SIZE	1024

#define PI			3.14159265358979323
#define ToRAD		( PI / 180 )

#define MAX_VSD_LOG		(( int )( LOG_FREQ * 3600 * 2 ))
#define MAX_LAP			200

#define RGB2YC( r, g, b ) { \
	( int )( 0.299 * r + 0.587 * g + 0.114 * b + .5 ), \
	( int )(-0.169 * r - 0.332 * g + 0.500 * b + .5 ), \
	( int )( 0.500 * r - 0.419 * g - 0.081 * b + .5 ) \
}

#define G_CX_CNT		30
#define G_HIST			(( int )( LOG_FREQ * 3 ))
#define MAX_G_SCALE		1.5

#define INVALID_POS_I	0x7FFFFFFF
#define INVALID_POS_D	NaN

#define LINE_WIDTH		( GetWidth() / HIREZO_TH + 1 )

#define GPS_LOG_OFFS	15

#define MAP_LINE1		yc_green
#define MAP_LINE2		yc_yellow
#define MAP_LINE3		yc_red
#define MAP_G_MAX		1.2

#define PTD_LOG_FREQ	11025.0

#define POS_DEFAULT		0x80000000
#define HIREZO_TH		600			// ハイレゾモード時の横幅スレッショルド

#define MAX_POLY_HEIGHT		2000		// polygon 用ライン数

/*** track / check ID *******************************************************/

enum {
	#define DEF_TRACKBAR( id, init, min, max, name )	id,
	#include "def_trackbar.h"
	TRACK_N
};

enum {
	#define DEF_CHECKBOX( id, init, name )	id,
	#include "def_checkbox.h"
	CHECK_N
};

/*** new type ***************************************************************/

typedef struct {
	float	fSpeed;
	float	fTacho;
#ifndef CIRCUIT_TOMO
	float	fMileage;
	float	fGx, fGy;
	float	fX, fX0;
	float	fY, fY0;
#endif
} VSD_LOG_t;

typedef struct {
	USHORT	uLap;
	int		iLogNum;
	float	fTime;
} LAP_t;

typedef struct {
	float	fX;
	float	fY;
	float	fVX;
	float	fVY;
	int		iLogNum;
} GPS_LOG_t;

typedef struct {
	short	iLeft, iRight;
} PolygonData_t;

class CVsdFilter {
	
  public:
	CVsdFilter();
	~CVsdFilter();
	
	/*** 画像オペレーション *************************************************/
	
	virtual void PutPixel( int x, int y, const PIXEL_YC &yc, UINT uFlag ) = 0;
	virtual PIXEL_YC &GetPixel( int x, int y, UINT uFlag ) = 0;
	
	void DrawLine( int x1, int y1, int x2, int y2, const PIXEL_YC &yc, UINT uFlag );
	void DrawLine( int x1, int y1, int x2, int y2, int width, const PIXEL_YC &yc, UINT uFlag );
	void FillLine( int x1, int y1, int x2,         const PIXEL_YC &yc, UINT uFlag );
	void DrawRect( int x1, int y1, int x2, int y2, const PIXEL_YC &yc, UINT uFlag );
	void DrawCircle( int x, int y, int r, const PIXEL_YC &yc, UINT uFlag );
	
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
	
	BOOL DrawVSD( void );
	
	enum {
		IMG_ALFA	= ( 1 << 0 ),
		IMG_TMP		= ( 1 << 1 ),
		IMG_FILL	= ( 1 << 2 ),
		IMG_TMP_DST	= ( 1 << 3 ),
		IMG_POLYGON	= ( 1 << 4 ),
	};
	
	int m_iPosX, m_iPosY;
	int	m_iFontW, m_iFontH, m_iBMP_H, m_iBMP_BytesPerLine;
	
	int	m_iPreW;
	
	int	m_iLapIdx;
	int m_iBestLogNum;
	
	PolygonData_t	*m_Polygon;
	
	// 仮想関数
	virtual int	GetWidth( void )	= 0;
	virtual int	GetHeight( void )	= 0;
	virtual int	GetFrameMax( void )	= 0;
	virtual int	GetFrameCnt( void )	= 0;
	
  private:
	static const UCHAR m_Font9p[];
	static const UCHAR m_Font18p[];
	
	const UCHAR *m_pFontData;
	
	/*** ログオペレーション *************************************************/
	
  public:
	BOOL ConfigLoad( char *szFileName );
#ifndef AVS_PLUGIN
	BOOL ConfigSave( char *szFileName );
#endif
	BOOL ReadLog( char *szFileName );
#ifdef CIRCUIT_TOMO
	UINT ReadPTD( FILE *fp, UINT uOffs );
#else
	void RotateMap( void );
#endif
	
	// 手動ラップタイム計測モードかどうか
	BOOL IsHandLaptime( void ){
		#ifdef CIRCUIT_TOMO
			return TRUE;
		#else
			return m_iVsdLogNum == 0;
		#endif
	}
	
	VSD_LOG_t	*m_VsdLog;
	int			m_iVsdLogNum;
	LAP_t		*m_Lap;
	int			m_iLapNum;
	float		m_fBestTime;
	int			m_iBestLapLogNum;
	
	double		m_dMapSize;
	double		m_dMapOffsX;
	double		m_dMapOffsY;
	
	int			m_iLogStart;
	int			m_iLogStop;
	
	double		m_dVideoFPS;
	
	int			*m_piParamT;
	int			*m_piParamC;
	
	BOOL		m_bCalcLapTimeReq;
	
	static const char *m_szTrackbarName[];
	static const char *m_szCheckboxName[];
	
	// 仮想関数
	virtual void SetFrameMark( int iFrame ) = 0;
	virtual void CalcLapTime( void ) = 0;
	
  private:
};
#endif
