/*****************************************************************************
	
	VSD -- vehicle data logger system  Copyright(C) by DDS
	
	CVsdFilter.h - CVsdFilter class header
	$Id$
	
*****************************************************************************/

#ifndef _CVsdFilter_h_
#define _CVsdFilter_h_

#define BUF_SIZE	1024
#define MAX_LAP		200
#define GPS_FREQ	10

#ifdef AVS_PLUGIN
	typedef UCHAR	PIXEL_t;
	
	#define RGB2YC( r, g, b ) { \
		(( int )( 0.299 * r + 0.587 * g + 0.114 * b ) >> 4 ), \
		(( int )(-0.169 * r - 0.331 * g + 0.500 * b ) >> 4 ) + 0x80, \
		(( int )( 0.299 * r + 0.587 * g + 0.114 * b ) >> 4 ), \
		(( int )( 0.500 * r - 0.419 * g - 0.081 * b ) >> 4 ) + 0x80 \
	}
#else
	typedef short	PIXEL_t;
	
	#define RGB2YC( r, g, b ) { \
		( int )( 0.299 * r + 0.587 * g + 0.114 * b ), \
		( int )(-0.169 * r - 0.331 * g + 0.500 * b ), \
		( int )( 0.500 * r - 0.419 * g - 0.081 * b ) \
	}
#endif

#define G_CX_CNT		30

#define LINE_WIDTH		( GetWidth() / HIREZO_TH + 1 )

#define GPS_LOG_OFFS	15

#define MAP_LINE1		yc_green
#define MAP_LINE2		yc_yellow
#define MAP_LINE3		yc_red
#define MAP_G_MAX		1.2

#define HIREZO_TH		600			// ハイレゾモード時の横幅スレッショルド
#define POS_DEFAULT		0x80000000

#define BESTLAP_NONE	599999

#ifdef GPS_ONLY
	#define DEFAULT_FONT	"ＭＳ ゴシック"
#else
	#define DEFAULT_FONT	"Impact"
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

enum {
	#define DEF_SHADOW( id, init, conf_name )	id,
	#include "def_shadow.h"
	SHADOW_N
};

enum {
	LAPMODE_HAND_VIDEO,		// 手動計測モード・Video フレーム
	LAPMODE_HAND_GPS,		// 手動計測モード・GPS ログ時計
	LAPMODE_HAND_MAGNET,	// 手動計測モード・磁気センサー時計
	LAPMODE_GPS,			// GPS 自動計測モード
	LAPMODE_MAGNET,			// 磁気センサー自動計測モード
};

/*** new type ***************************************************************/

typedef struct {
	USHORT	uLap;
	float	fLogNum;
	int		iTime;
} LAP_t;

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
	void DrawCircle( int x, int y, int r, int a, int b, const PIXEL_YC &yc, UINT uFlag );
	
	void DrawFont( int x, int y, UCHAR c, const PIXEL_YC &yc, UINT uFlag );
	void DrawFont( int x, int y, UCHAR c, const PIXEL_YC &yc, const PIXEL_YC &ycEdge, UINT uFlag );
	void DrawString( char *szMsg, const PIXEL_YC &yc, UINT uFlag, int x = POS_DEFAULT, int y = POS_DEFAULT );
	void DrawString( char *szMsg, const PIXEL_YC &yc, const PIXEL_YC &ycEdge, UINT uFlag, int x = POS_DEFAULT, int y = POS_DEFAULT );
	
	// ポリゴン描写
	void PolygonClear( void );
	void PolygonDraw( const PIXEL_YC &yc, UINT uFlag );
	
	BOOL DrawVSD( void );
	
	enum {
		IMG_ALFA	= ( 1 << 0 ),
		IMG_FILL	= ( 1 << 1 ),
		IMG_POLYGON	= ( 1 << 2 ),
	};
	
	int m_iPosX, m_iPosY;
	
	int	m_iLapIdx;
	int m_iBestLogNumRunning;
	
	PolygonData_t	*m_Polygon;
	
	// フォント
	CVsdFont	*m_pFont;
	LOGFONT		m_logfont;
	char		*m_szFontName;
	
	// 仮想関数
	virtual int	GetWidth( void )	= 0;
	virtual int	GetHeight( void )	= 0;
	virtual int	GetFrameMax( void )	= 0;
	virtual int	GetFrameCnt( void )	= 0;
	
	/*** ログオペレーション *************************************************/
	
  public:
	char *IsConfigParam( const char *szParamName, char *szBuf, int &iVal );
	char *IsConfigParamStr( const char *szParamName, char *szBuf, char *szDst );
	BOOL ConfigLoad( const char *szFileName );
	BOOL ParseMarkStr( const char *szMark );
	BOOL GPSLogLoad( const char *szFileName );
	
	BOOL ReadLog( const char *szFileName );
	
	double LapNum2LogNum( CVsdLog *Log, int iLapNum );
	
	void DrawSpeedGraph( CVsdLog *Log, const PIXEL_YC &yc );
	
	CVsdLog		*m_VsdLog;
	CVsdLog		*m_GPSLog;
	
	LAP_t		*m_Lap;
	int			m_iLapMode;
	int			m_iLapNum;
	int			m_iBestTime;
	int			m_iBestLap;
	
	int			m_iLogStart;
	int			m_iLogStop;
	
	double		m_dVideoFPS;
	
	int			*m_piParamT;
	int			*m_piParamC;
	int			*m_piParamS;
	
	BOOL		m_bCalcLapTimeReq;
	
	// スタートライン@GPS 計測モード
	double	m_dStartLineX1;
	double	m_dStartLineY1;
	double	m_dStartLineX2;
	double	m_dStartLineY2;
	
	static const char *m_szTrackbarName[];
	static const char *m_szCheckboxName[];
	static const char *m_szShadowParamName[];
	
	char	*m_szLogFile;
	char	*m_szGPSLogFile;
	
	virtual void SetFrameMark( int iFrame ) = 0;
	virtual int  GetFrameMark( int iFrame ) = 0;
	void CalcLapTime( void );
	void CalcLapTimeAuto( void );
	
	virtual char *GetVideoFileName( char *szFileName ){ return ""; }
  private:
};
#endif
