/*****************************************************************************
	
	VSD -- vehicle data logger system  Copyright(C) by DDS
	
	CVsdFilter.h - CVsdFilter class header
	
*****************************************************************************/

#ifndef _CVsdFilter_h_
#define _CVsdFilter_h_

#define BUF_SIZE	1024
#define MAX_LAP		200
#define GPS_FREQ	10

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
	void DrawGSnake(
		int iCx, int iCy, int iR,
		const PIXEL_YCA &ycBall, const PIXEL_YCA &ycLine
	);
	void DrawMeterPanel0( void );
	void DrawMeterPanel1( void );
	void DrawMap(
		int iX, int iY, int iSize,
		const PIXEL_YCA &ycIndicator,
		const PIXEL_YCA &ycG0,
		const PIXEL_YCA &ycGPlus,
		const PIXEL_YCA &ycGMinus
	);
	void DrawLapTime( void );
	void DrawNeedle(
		int x, int y, int r,
		int iStart, int iEnd, double dVal,
		const PIXEL_YCA yc, int iWidth
	);
	virtual void PutImage( int x, int y, CVsdImage &img ) = 0;
	
	enum {
		IMG_FILL	= ( 1 << 0 ),
		IMG_POLYGON	= ( 1 << 1 ),
	};
	
	// 仮想関数
	virtual int	GetWidth( void )	= 0;
	virtual int	GetHeight( void )	= 0;
	virtual int	GetFrameMax( void )	= 0;
	virtual int	GetFrameCnt( void )	= 0;
	virtual double	GetFPS( void )	= 0;
	
	static const char *m_szTrackbarName[];
	static const char *m_szCheckboxName[];
	static const char *m_szShadowParamName[];
	
	char	*m_szLogFile;
	char	*m_szGPSLogFile;
	
	int			*m_piParamT;
	int			*m_piParamC;
	int			*m_piParamS;
	
	// フォント
	CVsdFont	*m_pFontS;
	CVsdFont	*m_pFontM;
	CVsdFont	*m_pFontL;
	LOGFONT		m_logfont;
	
	CVsdLog		*m_VsdLog;
	CVsdLog		*m_GPSLog;
	
	BOOL ConfigLoad( const char *szFileName );
	BOOL ParseMarkStr( const char *szMark );
	BOOL GPSLogLoad( const char *szFileName );
	BOOL ReadLog( const char *szFileName );
	double GPSLogGetLength(
		double dLong0, double dLati0,
		double dLong1, double dLati1
	);
	
	BOOL		m_bCalcLapTimeReq;
	int			m_iLogStart;
	int			m_iLogStop;
	
  protected:
	
	LAP_t		*m_Lap;
	int			m_iLapMode;
	int			m_iLapNum;
	int			m_iBestTime;
	int			m_iBestLap;
	
  private:
	
	char *IsConfigParam( const char *szParamName, char *szBuf, int &iVal );
	char *IsConfigParamStr( const char *szParamName, char *szBuf, char *szDst );
	
	double LapNum2LogNum( CVsdLog *Log, int iLapNum );
	
	// スタートライン@GPS 計測モード
	double	m_dStartLineX1;
	double	m_dStartLineY1;
	double	m_dStartLineX2;
	double	m_dStartLineY2;
	
	virtual void SetFrameMark( int iFrame ) = 0;
	virtual int  GetFrameMark( int iFrame ) = 0;
	void CalcLapTime( void );
	void CalcLapTimeAuto( void );
	
	int m_iPosX, m_iPosY;
	
	int	m_iLapIdx;
	int m_iBestLogNumRunning;
	
	PolygonData_t	*m_Polygon;
	
	CScript	*m_Script;
};
#endif
