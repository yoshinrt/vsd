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
	
	#define RGB2YCA( r, g, b, a ) { \
		( int )(( 0.299 * ( r ) + 0.587 * ( g ) + 0.114 * ( b )        ) / 16 * ( 256 - ( a )) / 256 ), \
		( int )((-0.169 * ( r ) - 0.331 * ( g ) + 0.500 * ( b ) + 2048 ) / 16 * ( 256 - ( a )) / 256 ), \
		( int )(( 0.299 * ( r ) + 0.587 * ( g ) + 0.114 * ( b )        ) / 16 * ( 256 - ( a )) / 256 ), \
		( int )(( 0.500 * ( r ) - 0.419 * ( g ) - 0.081 * ( b ) + 2048 ) / 16 * ( 256 - ( a )) / 256 ), \
		( a ) \
	}
#else
	typedef short	PIXEL_t;
	
	#define RGB2YCA( r, g, b, a ) { \
		( int )(( 0.299 * ( r ) + 0.587 * ( g ) + 0.114 * ( b )) * ( 256 - ( a )) / 256 ), \
		( int )((-0.169 * ( r ) - 0.331 * ( g ) + 0.500 * ( b )) * ( 256 - ( a )) / 256 ), \
		( int )(( 0.500 * ( r ) - 0.419 * ( g ) - 0.081 * ( b )) * ( 256 - ( a )) / 256 ), \
		( a ) \
	}
#endif
#define RGB2YC( r, g, b )	RGB2YCA(( r ), ( g ), ( b ), 0 )

#define G_CX_CNT		30

#define LINE_WIDTH		( GetWidth() / HIREZO_TH + 1 )

#define GPS_LOG_OFFS	15

#define MAP_LINE1		yc_green
#define MAP_LINE2		yc_yellow
#define MAP_LINE3		yc_red

#define HIREZO_TH		600			// ハイレゾモード時の横幅スレッショルド
#define POS_DEFAULT		0x80000000

#define BESTLAP_NONE	599999

#ifdef GPS_ONLY
	#define DEFAULT_FONT	"Arial"
#else
	#define DEFAULT_FONT	"Impact"
#endif

#ifdef GPS_ONLY
	#define VideoSt			m_piParamS[ PARAM_VSt ]
	#define VideoEd			m_piParamS[ PARAM_VEd ]
	#define LogSt			0
	#define LogEd			0
	#define GPSSt			m_piParamS[ PARAM_GSt ]
	#define GPSEd			m_piParamS[ PARAM_GEd ]
#else
	#define VideoSt			m_piParamT[ PARAM_VSt ]
	#define VideoEd			m_piParamT[ PARAM_VEd ]
	#define LogSt			m_piParamT[ PARAM_LSt ]
	#define LogEd			m_piParamT[ PARAM_LEd ]
	#define GPSSt			m_piParamT[ PARAM_GSt ]
	#define GPSEd			m_piParamT[ PARAM_GEd ]
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
		
		// yuv 全部指定
		UINT	ycbcr;
	};
	USHORT	alfa;
} PIXEL_YCA;
#else
typedef	struct {
	short	y;					//	画素(輝度    )データ (     0 ～ 4096 )
	short	cb;					//	画素(色差(青))データ ( -2048 ～ 2048 )
	short	cr;					//	画素(色差(赤))データ ( -2048 ～ 2048 )
								//	画素データは範囲外に出ていることがあります
								//	また範囲内に収めなくてもかまいません
	USHORT	alfa;
} PIXEL_YCA;
#endif

class CVsdFilter {
	
  public:
	CVsdFilter();
	~CVsdFilter();
	
	/*** 画像オペレーション *************************************************/
	
	void PutPixel( int x, int y, const PIXEL_YCA &yc, UINT uFlag );
	void FillLine( int x1, int y1, int x2, const PIXEL_YCA &yc, UINT uFlag );
	virtual void PutPixelLow( int x, int y, const PIXEL_YCA &yc, UINT uFlag ) = 0;
	virtual void FillLineLow( int x1, int y1, int x2, const PIXEL_YCA &yc, UINT uFlag ) = 0;
	
	void DrawLine( int x1, int y1, int x2, int y2, const PIXEL_YCA &yc, UINT uFlag );
	void DrawLine( int x1, int y1, int x2, int y2, int width, const PIXEL_YCA &yc, UINT uFlag );
	
	void DrawRect( int x1, int y1, int x2, int y2, const PIXEL_YCA &yc, UINT uFlag );
	void DrawCircle( int x, int y, int r, const PIXEL_YCA &yc, UINT uFlag );
	void DrawCircle( int x, int y, int a, int b, const PIXEL_YCA &yc, UINT uFlag );
	void DrawArc(
		int x, int y,
		int a, int b,
		int iStart, int iEnd,
		const PIXEL_YCA &yc, UINT uFlag
	);
	void DrawArc(
		int x, int y,
		int a, int b,
		int c, int d,
		int iStart, int iEnd,
		const PIXEL_YCA &yc, UINT uFlag
	);
	
	void DrawFont( int x, int y, UCHAR c, CVsdFont *pFont, const PIXEL_YCA &yc, UINT uFlag );
	void DrawFont( int x, int y, UCHAR c, CVsdFont *pFont, const PIXEL_YCA &yc, const PIXEL_YCA &ycEdge, UINT uFlag );
	void DrawString( char *szMsg, CVsdFont *pFont, const PIXEL_YCA &yc, UINT uFlag, int x = POS_DEFAULT, int y = POS_DEFAULT );
	void DrawString( char *szMsg, CVsdFont *pFont, const PIXEL_YCA &yc, const PIXEL_YCA &ycEdge, UINT uFlag, int x = POS_DEFAULT, int y = POS_DEFAULT );
	
	// ポリゴン描写
	void PolygonClear( void );
	void PolygonDraw( const PIXEL_YCA &yc, UINT uFlag );
	
	PIXEL_YCA *BlendColor(
		PIXEL_YCA	&ycDst,
		const PIXEL_YCA	&ycColor0,
		const PIXEL_YCA	&ycColor1,
		double	dAlfa
	);
	
	BOOL DrawVSD( void );
	void DrawGSnake( int iCx, int iCy, int iR );
	void DrawMeterPanel0( void );
	void DrawMeterPanel1( void );
	
	enum {
		IMG_FILL	= ( 1 << 0 ),
		IMG_POLYGON	= ( 1 << 1 ),
	};
	
	int m_iPosX, m_iPosY;
	
	int	m_iLapIdx;
	int m_iBestLogNumRunning;
	
	PolygonData_t	*m_Polygon;
	
	// フォント
	CVsdFont	*m_pFontS;
	CVsdFont	*m_pFontM;
	CVsdFont	*m_pFontL;
	LOGFONT		m_logfont;
	
	// 仮想関数
	virtual int	GetWidth( void )	= 0;
	virtual int	GetHeight( void )	= 0;
	virtual int	GetFrameMax( void )	= 0;
	virtual int	GetFrameCnt( void )	= 0;
	virtual double	GetFPS( void )	= 0;
	
	/*** ログオペレーション *************************************************/
	
  public:
	char *IsConfigParam( const char *szParamName, char *szBuf, int &iVal );
	char *IsConfigParamStr( const char *szParamName, char *szBuf, char *szDst );
	BOOL ConfigLoad( const char *szFileName );
	BOOL ParseMarkStr( const char *szMark );
	BOOL GPSLogLoad( const char *szFileName );
	double GPSLogGetLength(
		double dLong0, double dLati0,
		double dLong1, double dLati1
	);
	
	BOOL ReadLog( const char *szFileName );
	
	double LapNum2LogNum( CVsdLog *Log, int iLapNum );
	
	void DrawSpeedGraph(
		CVsdLog *Log,
		int iX, int iY, int iW, int iH,
		const PIXEL_YCA &yc,
		int	iDirection
	);
	void DrawTachoGraph(
		CVsdLog *Log,
		int iX, int iY, int iW, int iH,
		const PIXEL_YCA &yc,
		int	iDirection
	);
	
	CVsdLog		*m_VsdLog;
	CVsdLog		*m_GPSLog;
	
	LAP_t		*m_Lap;
	int			m_iLapMode;
	int			m_iLapNum;
	int			m_iBestTime;
	int			m_iBestLap;
	
	int			m_iLogStart;
	int			m_iLogStop;
	
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
	
  private:
};
#endif
