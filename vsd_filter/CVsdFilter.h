/*****************************************************************************
	
	VSD -- vehicle data logger system  Copyright(C) by DDS
	
	CVsdFilter.h - CVsdFilter class header
	$Id$
	
*****************************************************************************/

#ifndef _CVsdLog_h_
#define _CVsdLog_h_

#define BUF_SIZE	1024

#define MAX_VSD_LOG		(( int )( LOG_FREQ * 3600 * 2 ))
#define MAX_LAP			200

#ifdef AVS_PLUGIN
	#define RGB2YC( r, g, b ) { \
		(( int )( 0.299 * r + 0.587 * g + 0.114 * b ) >> 4 ), \
		(( int )(-0.169 * r - 0.331 * g + 0.500 * b ) >> 4 ) + 0x80, \
		(( int )( 0.299 * r + 0.587 * g + 0.114 * b ) >> 4 ), \
		(( int )( 0.500 * r - 0.419 * g - 0.081 * b ) >> 4 ) + 0x80 \
	}
#else
	#define RGB2YC( r, g, b ) { \
		( int )( 0.299 * r + 0.587 * g + 0.114 * b ), \
		( int )(-0.169 * r - 0.331 * g + 0.500 * b ), \
		( int )( 0.500 * r - 0.419 * g - 0.081 * b ) \
	}
#endif

#define G_CX_CNT		30
#define G_HIST			(( int )( LOG_FREQ * 3 ))
#define MAX_G_SCALE		1.5

#define LINE_WIDTH		( GetWidth() / HIREZO_TH + 1 )

#define GPS_LOG_OFFS	15

#define MAP_LINE1		yc_green
#define MAP_LINE2		yc_yellow
#define MAP_LINE3		yc_red
#define MAP_G_MAX		1.2

#define HIREZO_TH		600			// ハイレゾモード時の横幅スレッショルド
#define POS_DEFAULT		0x80000000

#ifdef CIRCUIT_TOMO
	#define CONFIG_EXT	"cfg"
#else
	#define CONFIG_EXT	"avs"
#endif

/*** track / check ID *******************************************************/

enum {
	#define DEF_TRACKBAR( id, init, min, max, name, conf_name )	id,
	#include "def_trackbar.h"
	TRACK_N
};

enum {
	#define DEF_CHECKBOX( id, init, name, conf_name )	id,
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

#ifdef AVS_PLUGIN
typedef struct {
	union {
		// yuv 個別指定
		struct {
			UCHAR	y;
			UCHAR	cb;
			UCHAR	y1;
			UCHAR	cr;
		};
		
		// ycb / ycr アクセス
		struct {
			USHORT	ycb;
			USHORT	ycr;
		};
	};
} PIXEL_YC;
#endif

class CVsdFilter {
	
  public:
	CVsdFilter();
	~CVsdFilter();
	
	/*** 画像オペレーション *************************************************/
	
	virtual void PutPixel( int x, int y, const PIXEL_YC &yc, UINT uFlag ) = 0;
	
	void DrawLine( int x1, int y1, int x2, int y2, const PIXEL_YC &yc, UINT uFlag );
	void DrawLine( int x1, int y1, int x2, int y2, int width, const PIXEL_YC &yc, UINT uFlag );
	void FillLine( int x1, int y1, int x2,         const PIXEL_YC &yc, UINT uFlag );
	void DrawRect( int x1, int y1, int x2, int y2, const PIXEL_YC &yc, UINT uFlag );
	void DrawCircle( int x, int y, int r, const PIXEL_YC &yc, UINT uFlag );
	
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
		IMG_FILL	= ( 1 << 1 ),
		IMG_POLYGON	= ( 1 << 2 ),
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
	BOOL IsConfigParam( const char *szParamName, char *szBuf, int &iVal );
	BOOL ConfigLoad( const char *szFileName );
#ifndef AVS_PLUGIN
	BOOL ConfigSave( const char *szFileName );
#endif
	BOOL ReadLog( const char *szFileName );
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
	
	virtual char *GetVideoFileName( char *szFileName ){ return ""; }
  private:
};
#endif
