/*****************************************************************************
	
	VSD -- vehicle data logger system  Copyright(C) by DDS
	
	CVsdFilter.h - CVsdFilter class header
	
*****************************************************************************/

#ifndef _CVsdFilter_h_
#define _CVsdFilter_h_

#define PROG_NAME		"VSDFilter"
#define PROG_NAME_J		"VSDメーター合成"
#define PROG_NAME_LONG	"`VSDFilter' vehicle data logger overlay plugin"
#define PROG_VERSION	"v1.10beta1"

#define BUF_SIZE	1024
#define MAX_LAP		200
#define GPS_FREQ	10

#define G_CX_CNT		30
#define GPS_LOG_OFFS	15
#define POS_DEFAULT		0x80000000
#define BESTLAP_NONE	599999

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

// パラメータを変換
#define ConvParam( p, from, to ) ( \
	from##Ed == from##St ? 0 : \
	( double )( to##Ed - to##St ) * (( p ) - from##St ) \
	/ ( from##Ed - from##St ) \
	+ to##St \
)

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
	
	void PutPixel(	// !js_func
		int x, int y, tRABY uColor,
		UINT uFlag	// !default:0
	);
	void         PutPixel( int x, int y, const PIXEL_YCA_ARG yc, UINT uFlag );
	virtual void PutPixel( int x, int y, const PIXEL_YCA_ARG yc ) = 0;
	void         FillLine( int x1, int y1, int x2, const PIXEL_YCA_ARG yc, UINT uFlag );
	virtual void FillLine( int x1, int y1, int x2, const PIXEL_YCA_ARG yc ) = 0;
	virtual UINT PutImage( int x, int y, CVsdImage &img ) = 0;	// !js_func
	
	void DrawLine( int x1, int y1, int x2, int y2, const PIXEL_YCA_ARG yc, UINT uFlag );
	void DrawLine( int x1, int y1, int x2, int y2, tRABY uColor, UINT uFlag );
	void DrawLine(		// !js_func
		int x1, int y1, int x2, int y2,
		int width,		// !arg:5 !default:1
		tRABY uColor,	// !arg:4
		UINT uFlag		// !default:0
	);
	
	void DrawRect(	// !js_func
		int x1, int y1, int x2, int y2,
		tRABY uColor, UINT uFlag
	);
	void DrawCircle(	// !js_func
		int x, int y, int r,
		tRABY uColor,
		UINT uFlag		// !default:0
	);
	void DrawCircle( int x, int y, int a, int b, tRABY uColor, UINT uFlag );
	void DrawArc(
		int x, int y,
		int a, int b,
		double dStart, double dEnd,
		tRABY uColor,
		UINT uFlag
	);
	void DrawArc(
		int x, int y,
		int a, int b,
		int c, int d,
		double dStart, double dEnd,
		tRABY uColor,
		UINT uFlag
	);
	
	int DrawFont0( int x, int y, UCHAR c, CVsdFont &Font, tRABY uColor );
	int DrawFont( int x, int y, UCHAR c, CVsdFont &Font, tRABY uColor, tRABY uColorOutline = color_black );
	void DrawText( // !js_func
		int x, int y, char *szMsg, CVsdFont &Font, tRABY uColor,
		tRABY uColorOutline = color_black	// !default:color_black
	);
	void DrawTextAlign( // !js_func
		int x, int y, UINT uAlign, char *szMsg, CVsdFont &Font, tRABY uColor,
		tRABY uColorOutline = color_black	// !default:color_black
	);
	
	void DrawGraph(
		int x1, int y1, int x2, int y2,
		char *szFormat,
		CVsdFont &Font,
		tRABY uColor,
		CVsdLog& Log,
		double ( *GetDataFunc )( CVsdLog&, int ),
		double dMaxVal
	);
	void DrawGraph(	// !js_func
		int x1, int y1, int x2, int y2,
		CVsdFont &Font
	);
	
	// ポリゴン描写
	void InitPolygon( void );			// !js_func
	void DrawPolygon( tRABY uColor );	// !js_func
	void DrawPolygon( const PIXEL_YCA_ARG yc );
	
	UINT BlendColor(
		tRABY uColor0,
		tRABY uColor1,
		double	dAlfa
	);
	
	BOOL DrawVSD( void );
	void DrawGSnake( // !js_func
		int iCx, int iCy, int iR, int iIndicatorR, int iWidth,
		tRABY uColorBall, tRABY uColorLine
	);
	
	void DrawMeterScale(	// !js_func
		int iCx, int iCy, int iR,
		int iLineLen1, int iLineWidth1, tRABY uColorLine1,
		int iLineLen2, int iLineWidth2, tRABY uColorLine2,
		int iLine2Cnt,
		int iMinDeg, int iMaxDeg,
		int iRNum,
		int iMaxVal, int iMaxNumCnt, tRABY uColorNum,
		CVsdFont &Font
	);
	void DrawMap( // !js_func
		int iX, int iY, int iSize, int iWidth,
		int iIndicatorR,
		tRABY uColorIndicator,
		tRABY uColorG0,
		tRABY uColorGPlus,
		tRABY uColorGMinus
	);
	void DrawLapTime( // !js_func
		int x, int y, CVsdFont &Font,
		tRABY uColor, tRABY uColorOutline, tRABY uColorBest, tRABY uColorPlus
	);
	void DrawNeedle( // !js_func
		int x, int y, int r1, int r2,
		int iStart, int iEnd, double dVal,
		tRABY uColor,
		int iWidth // !default:1
	);
	
	void DispErrorMessage( char *szMsg );
	
	enum {
		IMG_FILL	= ( 1 << 0 ),
	};
	
	enum {
		ALIGN_LEFT		= 0,
		ALIGN_TOP		= 0,
		ALIGN_HCENTER	= 1,
		ALIGN_RIGHT		= 2,
		ALIGN_VCENTER	= 4,
		ALIGN_BOTTOM	= 8,
	};
	
	// 仮想関数
	virtual int	GetWidth( void )	= 0;	// !js_var:Width
	virtual int	GetHeight( void )	= 0;	// !js_var:Height
	virtual int	GetFrameMax( void )	= 0;	// !js_var:MaxFrameCnt
	virtual int	GetFrameCnt( void )	= 0;	// !js_var:FrameCnt
	virtual double GetFPS( void )	= 0;
	
	char	*m_szLogFile;
	char	*m_szGPSLogFile;
	char	*m_szSkinFile;
	char	*m_szSkinDir;	// !js_var:SkinDir
	
	char *SetSkinDir( void ){
		return StrTokFile( m_szSkinDir, m_szSkinFile, STF_FULL | STF_PATH2 );
	}
	
	int		*m_piParamT;
	int		*m_piParamC;
	int		*m_piParamS;
	
	// フォント
	CVsdFont	*m_pFont;
	
	CVsdLog	*m_VsdLog;
	CVsdLog	*m_GPSLog;
	
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
	
	// JavaScript 用パラメータ
	double	m_dSpeed;		// !js_var:Speed
	double	m_dTacho;		// !js_var:Tacho
	double	m_dGx;			// !js_var:Gx
	double	m_dGy;			// !js_var:Gy
	int		m_iMaxSpeed;	// !js_var:MaxSpeed
	
  protected:
	
	CScript	*m_Script;
	
	LAP_t		*m_Lap;
	int			m_iLapMode;
	int			m_iLapNum;
	int			m_iBestTime;
	int			m_iBestLap;
	
  private:
	
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
	
	// 解像度変更検出用
	int	m_iWidth;
	int m_iHeight;
};
#endif
