/*****************************************************************************
	
	VSD -- vehicle data logger system  Copyright(C) by DDS
	
	CVsdFilter.h - CVsdFilter class header
	
*****************************************************************************/

#ifndef _CVsdFilter_h_
#define _CVsdFilter_h_

#define PROG_NAME		"VSDFilter"
#define PROG_NAME_J		"VSDメーター合成"
#define PROG_NAME_J_W	L"VSDメーター合成"
#define PROG_NAME_LONG	"`VSDFilter' vehicle data logger overlay plugin"
#define PROG_VERSION	"v1.10beta1"

#define MAX_LAP		200

#define G_CX_CNT		30
#define POS_DEFAULT		0x80000000

#ifdef GPS_ONLY
	#define VideoSt			m_piParamS[ PARAM_VSt ]
	#define VideoEd			m_piParamS[ PARAM_VEd ]
	#define VsdSt			0
	#define VsdEd			0
	#define GPSSt			m_piParamS[ PARAM_GSt ]
	#define GPSEd			m_piParamS[ PARAM_GEd ]
#else
	#define VideoSt			m_piParamT[ PARAM_VSt ]
	#define VideoEd			m_piParamT[ PARAM_VEd ]
	#define VsdSt			m_piParamT[ PARAM_LSt ]
	#define VsdEd			m_piParamT[ PARAM_LEd ]
	#define GPSSt			m_piParamT[ PARAM_GSt ]
	#define GPSEd			m_piParamT[ PARAM_GEd ]
#endif

// パラメータを変換
#define GetLogIndex( frame, to, prev ) \
	m_##to##Log->GetIndex( frame, VideoSt, VideoEd, to##St, to##Ed, prev )

// VSD log を優先，ただしチェックボックスでオーバーライドできる
#define SelectLogVsd ( m_CurLog = ( GPSPriority && m_GPSLog || !m_VsdLog ) ? m_GPSLog : m_VsdLog )

// GPS log を優先
#define SelectLogGPS ( m_CurLog = m_GPSLog ? m_GPSLog : m_VsdLog )

// Laptime 計算用
#define SelectLogForLapTime	( m_CurLog = \
	m_LapLog && m_LapLog->m_iLapMode == LAPMODE_MAGNET \
	? m_VsdLog : m_GPSLog )

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

/*** new type ***************************************************************/

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
	virtual UINT PutImage(	// !js_func
		int x, int y, CVsdImage &img,
		UINT uAlign = 0	// !default:0
	) = 0;
	
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
		tRABY uColor,
		UINT uFlag	// !default:0
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
		tRABY uColor
	);
	
	int DrawFont0( int x, int y, WCHAR c, CVsdFont &Font, tRABY uColor );
	int DrawFont( int x, int y, WCHAR c, CVsdFont &Font, tRABY uColor, tRABY uColorOutline = color_black );
	void DrawText( // !js_func
		int x, int y, LPCWSTR szMsg, CVsdFont &Font,
		tRABY uColor,						// !default:color_white
		tRABY uColorOutline = color_black	// !default:color_black
	);
	void DrawTextAlign( // !js_func
		int x, int y, UINT uAlign, LPCWSTR szMsg, CVsdFont &Font,
		tRABY uColor,						// !default:color_white
		tRABY uColorOutline = color_black	// !default:color_black
	);
	
	void DrawGraph(
		int x1, int y1, int x2, int y2,
		LPCWSTR szFormat,
		CVsdFont &Font,
		tRABY uColor,
		CVsdLog& Log,
		double ( CVsdLog::*GetDataFunc )( int ),
		double dMaxVal
	);
	
	enum {
		GRAPH_SPEED	= 1 << 0,
		GRAPH_TACHO	= 1 << 1,
		GRAPH_GX	= 1 << 2,
		GRAPH_GY	= 1 << 3,
		GRAPH_HTILE	= 1 << 30,
		GRAPH_VTILE	= 1 << 31,
	};
	
	void DrawGraph(	// !js_func
		int x1, int y1, int x2, int y2,
		CVsdFont &Font,
		UINT uFlag = GRAPH_SPEED		// !default:1
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
		tRABY uColorBall, tRABY uColorLine,
		double dLength	// !default:3
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
	void DrawMap(	// !js_func
		int x1, int y1, int x2, int y2,
		UINT uAlign,
		int iWidth,
		int iIndicatorR,
		tRABY uColorIndicator,
		tRABY uColorG0,
		tRABY uColorGPlus,
		tRABY uColorGMinus
	);
	void DrawLapTime(	// !js_func
		int x, int y, UINT uAlign, CVsdFont &Font,
		tRABY uColor,			// !default:color_white
		tRABY uColorBest,		// !default:color_cyan
		tRABY uColorPlus,		// !default:color_red
		tRABY uColorOutline		// !default:color_black
	);
	void DrawLapTimeLog(	// !js_func
		int x, int y, UINT uAlign, int iNum, CVsdFont &Font,
		tRABY uColor,			// !default:color_white
		tRABY uColorBest,		// !default:color_cyan
		tRABY uColorOutline		// !default:color_black
	);
	void DrawNeedle( // !js_func
		int x, int y, int r1, int r2,
		int iStart, int iEnd, double dVal,
		tRABY uColor,
		int iWidth // !default:1
	);
	
	// ラップタイム情報
	LPCWSTR FormatTime( int iTime ); // !js_func
	
	int CurTime( void ){ return m_LapLog ? m_LapLog->m_iCurTime : TIME_NONE; }	// !js_var:ElapsedTime
	int BestLapTime( void ){ return m_LapLog ? m_LapLog->m_iBestTime : TIME_NONE; }	// !js_var:BestLapTime
	int DiffTime( void ){ return m_LapLog ? m_LapLog->m_iDiffTime : TIME_NONE; }	// !js_var:DiffTime
	int LapTime( void ){	// !js_var:LapTime
		if( !m_LapLog || m_LapLog->m_iLapIdx < 0 ) return TIME_NONE;
		if( m_LapLog->m_Lap[ m_LapLog->m_iLapIdx + 1 ].iTime ){
			return m_LapLog->m_Lap[ m_LapLog->m_iLapIdx + 1 ].iTime;
		}
		return m_LapLog->m_Lap[ m_LapLog->m_iLapIdx ].iTime;
	}
	int LapCnt( void ){ // !js_var:LapCnt
		if( !m_LapLog ) return 0;
		return m_LapLog->m_Lap[ m_LapLog->m_iLapIdx + 1 ].uLap;
	}
	int MaxLapCnt( void ){ // !js_var:MaxLapCnt
		return m_LapLog ? m_LapLog->m_Lap[ m_LapLog->m_iLapNum - 1 ].uLap : 0;
	}
	
	virtual void DispErrorMessage( LPCWSTR szMsg ) = 0;
	
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
	char	*m_szSkinDirA;
	LPWSTR	m_szSkinDirW;	// !js_var:SkinDir
	char	*m_szPluginDirA;
	LPWSTR	m_szPluginDirW;	// !js_var:VsdRootDir
	
	// ログリードヘルパ
	int ReadGPSLog( const char *szFileName );
	int ReadLog( const char *szFileName );
	double LapNum2LogNum( CVsdLog *Log, int iLapNum );
	CLapLog *CreateLapTime( int iLapMode );
	CLapLog *CreateLapTimeAuto( void );
	void CalcLapTime( void );
	
	static char *StringNew( char *&szDst, const char *szSrc ){
		if( szDst ) delete [] szDst;
		
		if( szSrc == NULL || *szSrc == '\0' ){
			return szDst = NULL;
		}
		szDst = new char[ strlen( szSrc ) + 1 ];
		return strcpy( szDst, szSrc );
	}
	
	static LPWSTR StringNew( LPWSTR& szDst, LPCWSTR szSrc ){
		if( szDst ) delete [] szDst;
		
		if( szSrc == NULL || *szSrc == '\0' ){
			return szDst = NULL;
		}
		szDst = new WCHAR[ wcslen( szSrc ) + 1 ];
		return wcscpy( szDst, szSrc );
	}
	
	static LPWSTR StringNew( LPWSTR& szDst, const char *szSrc ){
		if( szDst ) delete [] szDst;
		
		if( szSrc == NULL || *szSrc == '\0' ){
			return szDst = NULL;
		}
		// SJIS->WCHAR 変換
		int iLen = MultiByteToWideChar(
			CP_ACP,				// コードページ
			0,					// 文字の種類を指定するフラグ
			szSrc,				// マップ元文字列のアドレス
			-1,					// マップ元文字列のバイト数
			NULL,				// マップ先ワイド文字列を入れるバッファのアドレス
			0					// バッファのサイズ
		);
		szDst = new WCHAR[ iLen ];
		
		MultiByteToWideChar(
			CP_ACP,				// コードページ
			0,					// 文字の種類を指定するフラグ
			szSrc,				// マップ元文字列のアドレス
			-1,					// マップ元文字列のバイト数
			szDst,				// マップ先ワイド文字列を入れるバッファのアドレス
			iLen				// バッファのサイズ
		);
		
		return szDst;
	}
	
	static char *StringNew( char *&szDst, LPCWSTR &szSrc ){
		if( szDst ) delete [] szDst;
		
		if( szSrc == NULL || *szSrc == '\0' ){
			return szDst = NULL;
		}
		// SJIS->WCHAR 変換
		int iLen = WideCharToMultiByte(
			CP_ACP,				// コードページ
			0,					// 文字の種類を指定するフラグ
			szSrc,				// マップ元文字列のアドレス
			-1,					// マップ元文字列のバイト数
			NULL,				// マップ先ワイド文字列を入れるバッファのアドレス
			0,					// バッファのサイズ
			NULL, NULL
		);
		szDst = new char[ iLen ];
		
		WideCharToMultiByte(
			CP_ACP,				// コードページ
			0,					// 文字の種類を指定するフラグ
			szSrc,				// マップ元文字列のアドレス
			-1,					// マップ元文字列のバイト数
			szDst,				// マップ先ワイド文字列を入れるバッファのアドレス
			iLen,				// バッファのサイズ
			NULL, NULL
		);
		
		return szDst;
	}
	
	// スキン dir 取得
	char *SetSkinFile( const char *szSkinFile ){
		char szBuf[ MAX_PATH + 1 ];
		
		if( szSkinFile == NULL ){
			StringNew( m_szSkinDirW, ( LPCWSTR )NULL );
			return StringNew( m_szSkinDirA, NULL );
		}
		
		// スキンファイル名を CWD=m_szPluginDirA とみなしフルパスに変換
		GetFullPathWithCDir( szBuf, szSkinFile, m_szPluginDirA );
		
		// ↑のディレクトリ名を得る
		StringNew( m_szSkinFile, szBuf );
		StringNew( m_szSkinDirA, StrTokFile( szBuf, m_szSkinFile, STF_FULL | STF_PATH2 ));
		StringNew( m_szSkinDirW, m_szSkinDirA );
		return m_szSkinDirA;
	}
	
	// プラグイン dll dir 取得
	char *SetPluginDir( void ){
		char szBuf[ MAX_PATH + 1 ];
		GetModuleFileName(( HMODULE )m_hInst, szBuf, MAX_PATH );
		char *p = StrTokFile( NULL, szBuf, STF_NODE );
		if( p ) strcpy( p, "vsd_skins\\" );
		StringNew( m_szPluginDirA, szBuf );
		StringNew( m_szPluginDirW, m_szPluginDirA );
		return m_szPluginDirA;
	}
	
	int		*m_piParamT;
	int		*m_piParamC;
	int		*m_piParamS;
	
	// フォント
	CVsdFont	*m_pFont;
	
	CVsdLog	*m_VsdLog;
	CVsdLog	*m_GPSLog;
	CVsdLog *m_CurLog;
	CLapLog	*m_LapLog;
	
	BOOL ParseMarkStr( const char *szMark );
	
	BOOL		m_bCalcLapTimeReq;
	
	// JavaScript 用パラメータ
	double	m_dSpeed;		// !js_var:Speed
	double	m_dTacho;		// !js_var:Tacho
	double	m_dGx;			// !js_var:Gx
	double	m_dGy;			// !js_var:Gy
	double MaxGx( void ){ return m_CurLog ? m_CurLog->m_dMaxGx : 0; } // !js_var:MaxGx
	double MaxGy( void ){ return m_CurLog ? m_CurLog->m_dMaxGy : 0; } // !js_var:MaxGy
	
	int	GetMaxSpeed( void ){ // !js_var:MaxSpeed
		return m_CurLog ? m_CurLog->m_iMaxSpeed : 180;
	}
	int	GetMaxTacho( void ){ // !js_var:MaxTacho
		return m_CurLog ? m_CurLog->m_iMaxTacho : 0;
	}
	
	static HINSTANCE	m_hInst;	// dll handle
	
	void ReloadScript( void ){
		if( m_Script ){
			delete m_Script;
			m_Script = NULL;
		}
	}
	
  protected:
	
	CScript	*m_Script;
	
  private:
	
	// スタートライン@GPS 計測モード
	double	m_dStartLineX1;
	double	m_dStartLineY1;
	double	m_dStartLineX2;
	double	m_dStartLineY2;
	
	virtual void SetFrameMark( int iFrame ) = 0;
	virtual int  GetFrameMark( int iFrame ) = 0;
	
	int m_iTextPosX, m_iTextPosY;
	
	PolygonData_t	*m_Polygon;
	
	// 解像度変更検出用
	int	m_iWidth;
	int m_iHeight;
};
#endif
