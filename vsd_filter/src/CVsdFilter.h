/*****************************************************************************
	
	VSD -- vehicle data logger system  Copyright(C) by DDS
	
	CVsdFilter.h - CVsdFilter class header
	
*****************************************************************************/

#pragma once

#include "CVsdLog.h"
#include "CVsdFont.h"
#include "CScript.h"
#include "CVsdImage.h"
#include "CPixel.h"
#include "CVsdLogJs.h"

#ifdef AVS_PLUGIN
	#include "avisynth.h"
#else
	#include "external/filter.h"
#endif

/****************************************************************************/

#define PROG_NAME		"VSDFilter"
#define PROG_NAME_J		"VSDメーター合成"
#define PROG_NAME_J_W	WIDEN(PROG_NAME_J)
#define PROG_NAME_LONG	"`VSDFilter' vehicle data logger overlay plugin"

#define G_CX_CNT		30
#define POS_DEFAULT		0x80000000

#define VideoSt			m_piParamT[PARAM_VSt]
#define VideoEd			m_piParamT[PARAM_VEd]
#define VsdSt			m_piParamT[PARAM_LSt]
#define VsdEd			m_piParamT[PARAM_LEd]
#define GPSSt			m_piParamT[PARAM_GSt]
#define GPSEd			m_piParamT[PARAM_GEd]
#define DEFAULT_SKIN	"standard_r.js"

#define PLUGIN_DIR		"vsd_plugins"
#define LOG_READER_DIR	"_log_reader"

// パラメータを変換
#define GetLogIndex(frame, to, prev) \
	m_##to##Log->GetIndex(frame, &VideoSt, &to##St, prev)

// VSD log を優先，ただしチェックボックスでオーバーライドできる
#define SelectLogVsd (m_CurLog = (GPSPriority && m_GPSLog || !m_VsdLog) ? m_GPSLog.get() : m_VsdLog.get())

// GPS log を優先
#define SelectLogGPS (m_CurLog = m_GPSLog ? m_GPSLog.get() : m_VsdLog.get())

// Laptime 計算用
#define SelectLogForLapTime	(m_CurLog = \
	m_LapLog && m_LapLog->m_iLapSrc == LAPSRC_VSD \
	? m_VsdLog.get() : m_GPSLog.get())

/*** track / check ID *******************************************************/

enum {
	#define DEF_TRACKBAR(id, init, min, max, name, conf_name)	id,
	#include "def_trackbar.h"
	TRACK_N
};

enum {
	#define DEF_CHECKBOX(id, init, name, conf_name)	id,
	#include "def_checkbox.h"
	CHECK_N
};

enum {
	#define DEF_SHADOW(id, init, conf_name)	id,
	#include "def_shadow.h"
	SHADOW_N
};

/*** new type ***************************************************************/

typedef struct {
	int	iLeft, iRight;
} PolygonData_t;

/* 辺の定義 */
struct Edge {
	double	dx, dy;	// 座標
	int		x, y;	// 始点座標
	USHORT	Flag;
};

class CVsdFilter
	#ifdef AVS_PLUGIN
		: public GenericVideoFilter
	#endif
{
  public:
	~CVsdFilter();
	void Constructor(void);
	void Destructor(void);
	
	/*** 画像オペレーション *************************************************/
	// *C なメソッドは，クリッピングが完了している想定
	
	void PutPixel(	// !js_func
		int x, int y, CPixelArg yc,
		UINT uFlag	// !default:0
	);
	void PutPixelC(int x, int y, CPixelArg yc);
	
	void PutPixel(int x, int y, CPixelArg yc){
		if(!IsClipped(x, y)) PutPixelC(x, y, yc);
	}
	
	void FillLineC(int x1, int y1, int x2, CPixelArg yc);
	
	void PutImage(	// !js_func
		int x, int y, CVsdImage& img,
		UINT uAlign = 0,		// !default:0
		int iImgX = 0,			// !default:0
		int iImgY = 0,			// !default:0
		int iImgW = INT_MIN,	// !default:INT_MIN
		int iImgH = INT_MIN		// !default:INT_MIN
	);
	// 座標，width 等補正後
	void PutImage0C(
		int x, int y, CVsdImage& img,
	int ix_st, int iy_st, int ix_ed, int iy_ed
	);
	
	void DrawLine1(int x1, int y1, int x2, int y2, CPixelArg yc, UINT uPattern = 0xFFFFFFFF);
	void DrawLine(		// !js_func
		int x1, int y1, int x2, int y2,
		CPixelArg yc,
		int width,		// !default:1
		UINT uPattern = 0xFFFFFFFF	// !default:0xFFFFFFFF
	);
	
	void DrawRect(	// !js_func
		int x1, int y1, int x2, int y2,
		CPixelArg yc,
		UINT uFlag	// !default:0
	);
	void DrawCircle(	// !js_func
		int x, int y, int r,
		CPixelArg yc,
		UINT uFlag		// !default:0
	);
	void DrawCircle(int x, int y, int a, int b, CPixelArg yc, UINT uFlag);
	void DrawArc(
		int x, int y,
		int a, int b,
		double dStart, double dEnd,
		CPixelArg yc,
		UINT uFlag
	);
	void DrawArc(
		int x, int y,
		int a, int b,
		int c, int d,
		double dStart, double dEnd,
		CPixelArg yc
	);
	
	/* !js_bind
		.method("DrawArc", [](JSContext* ctx, __VSD_System__* self, int argc, JSValueConst* argv) -> JSValue {
			if(argc == 9){
				g_pVsd->DrawArc(
					qjsb::from_js<int>(ctx, argv[0]),
					qjsb::from_js<int>(ctx, argv[1]),
					qjsb::from_js<int>(ctx, argv[2]),
					qjsb::from_js<int>(ctx, argv[3]),
					qjsb::from_js<int>(ctx, argv[4]),
					qjsb::from_js<int>(ctx, argv[5]),
					qjsb::from_js<double>(ctx, argv[6]),
					qjsb::from_js<double>(ctx, argv[7]),
					CPixel(qjsb::from_js<int>(ctx, argv[8]))
				);
			}else if(7 <= argc && argc <= 8){
				g_pVsd->DrawArc(
					qjsb::from_js<int>(ctx, argv[0]),
					qjsb::from_js<int>(ctx, argv[1]),
					qjsb::from_js<int>(ctx, argv[2]),
					qjsb::from_js<int>(ctx, argv[3]),
					qjsb::from_js<double>(ctx, argv[4]),
					qjsb::from_js<double>(ctx, argv[5]),
					qjsb::from_js<int>(ctx, argv[6]),
					argc <= 7 ? 0 : qjsb::from_js<int>(ctx, argv[7])
				);
			}else{
				JsErrorNumOfArg();
			}
			
			return JS_UNDEFINED;
		})
	*/
	
	int DrawFont0(int x, int y, WCHAR c, CVsdFont& Font, CPixelArg yc);
	int DrawFont(int x, int y, WCHAR c, CVsdFont& Font, CPixelArg yc, CPixelArg ycOutline = color_black);
	void DrawText(// !js_func
		int x, int y, LPCWSTR szMsg, CVsdFont& Font,
		CPixelArg yc,						// !default:color_white
		CPixelArg ycOutline = color_black	// !default:color_black
	);
	void DrawTextAlign(// !js_func
		int x, int y, UINT uAlign, LPCWSTR szMsg, CVsdFont& Font,
		CPixelArg yc,						// !default:color_white
		CPixelArg ycOutline = color_black	// !default:color_black
	);
	
	void DrawGraphSingle(	// !js_func
		int x1, int y1, int x2, int y2,
		const std::string& strKey,
		LPCWSTR szFormat,
		CVsdFont& Font,
		CPixelArg yc
	);
	
	void DrawGraphSub(
		int x1, int y1, int x2, int y2,
		LPCWSTR szFormat,
		CVsdFont& Font,
		CPixelArg yc,
		CVsdLog& Log,
		CLog	&Data
	);
	
	enum {
		GRAPH_HTILE	= 1 << 30,
		GRAPH_VTILE	= 1 << 31,
	};
	
	void DrawSyncGraph(	// !js_func
		int x1, int y1, int x2, int y2,
		CVsdFont& Font
	);
	
	void DrawSyncInfo(// !js_func
		int x, int y, CVsdFont& Font,
		UINT uAlign = 0	// !default:0
	);
	
	// ポリゴン描写
	void InitPolygon(void);
	void FillPolygon(CPixelArg yc);
	void DrawPolygon(
		const std::vector<double>& pixs,
		CPixelArg yc,
		UINT uFlag
	);
	/* !js_bind
		.method("DrawPolygon", [](JSContext* ctx, __VSD_System__* self, int argc, JSValueConst* argv) -> JSValue {
			if(!(2 <= argc && argc <= 3)) JsErrorNumOfArg();
			
			g_pVsd->DrawPolygon(
				qjsb::from_js<std::vector<double> >(ctx, argv[0]),
				CPixel(qjsb::from_js<int>(ctx, argv[1])),
				argc <= 2 ? 0 : qjsb::from_js<UINT>(ctx, argv[2])
			);
			
			return JS_UNDEFINED;
		})
	*/
	
	void DrawPolygon(UINT uEdgeCnt, Edge *EdgeList, CPixelArg yc);
	void DrawPolygon(UINT uEdgeCnt, Edge *EdgeList, int iMinY, int iMaxY, CPixelArg yc);
	
	UINT BlendColor(
		CPixelArg yc0,
		CPixelArg yc1,
		double	dAlfa
	);
	
	BOOL DrawVSD(void);
	void DrawGSnake(// !js_func
		int iCx, int iCy, int iR, int iIndicatorR, int iWidth,
		CPixelArg ycBall, CPixelArg ycLine,
		double dLength	// !default:3
	);
	
	void DrawRoundMeterScaleSub(	// !js_func
		int iCx, int iCy, int iR,
		int iLineLen1, int iLineWidth1, CPixelArg ycLine1, int iLine1Cnt,
		int iLineLen2, int iLineWidth2, CPixelArg ycLine2, int iLine2Cnt,
		int iMinDeg, int iMaxDeg,
		int iMinVal, int iMaxVal,
		int iRNum, CPixelArg ycNum, CVsdFont& Font
	);
	
	void DrawLinearMeterScaleSub(	// !js_func
		UINT uFlag,
		int iX, int iY, int iWidth,
		int iLineLen1, int iLineWidth1, CPixelArg ycLine1, int iLine1Cnt,
		int iLineLen2, int iLineWidth2, CPixelArg ycLine2, int iLine2Cnt,
		int iMinVal, int iMaxVal,
		int iNumPos, CPixelArg ycNum, CVsdFont& Font
	);
	
	void DrawMap(
		int x1, int y1, int x2, int y2,
		UINT uFlag,
		int iWidth,
		int iIndicatorR,
		CPixelArg ycIndicator,
		CPixelArg ycG0,
		CPixelArg ycGPlus,
		CPixelArg ycGMinus,
		int	iLength = INVALID_INT
	);
	void DrawMap(
		int x1, int y1, int x2, int y2,
		int iLineWidth,
		int iZoomLv,
		CPixelArg yc
	);
	
	/* !js_bind
		// 引数 8個の方は argv[4] は無視してそれ以外を渡す
		.method("DrawMap", [](JSContext* ctx, __VSD_System__* self, int argc, JSValueConst* argv) -> JSValue {
			if(argc == 8){
				g_pVsd->DrawMap(
					qjsb::from_js<int>(ctx, argv[0]),
					qjsb::from_js<int>(ctx, argv[1]),
					qjsb::from_js<int>(ctx, argv[2]),
					qjsb::from_js<int>(ctx, argv[3]),
					qjsb::from_js<int>(ctx, argv[5]),
					qjsb::from_js<int>(ctx, argv[6]),
					CPixel(qjsb::from_js<int>(ctx, argv[7]))
				);
			}else if(11 <= argc && argc <= 12){
				g_pVsd->DrawMap(
					qjsb::from_js<int>(ctx, argv[0]),
					qjsb::from_js<int>(ctx, argv[1]),
					qjsb::from_js<int>(ctx, argv[2]),
					qjsb::from_js<int>(ctx, argv[3]),
					qjsb::from_js<int>(ctx, argv[4]),
					qjsb::from_js<int>(ctx, argv[5]),
					qjsb::from_js<int>(ctx, argv[6]),
					CPixel(qjsb::from_js<int>(ctx, argv[7])),
					CPixel(qjsb::from_js<int>(ctx, argv[8])),
					CPixel(qjsb::from_js<int>(ctx, argv[9])),
					CPixel(qjsb::from_js<int>(ctx, argv[10])),
					argc <= 11 ? INVALID_INT : qjsb::from_js<int>(ctx, argv[11])
				);
			}else{
				JsErrorNumOfArg();
			}
			
			return JS_UNDEFINED;
		})
	*/
	
	void DrawMapPosition(	// !js_func
		int x1, int y1, int x2, int y2,	UINT uFlag,
		int iLineWidth, CPixelArg yc,
		CVsdFont& Font,
		CPixelArg ycFont,					// !default:color_white
		CPixelArg ycOutline = color_black	// !default:color_black
	);
	void DrawLapTime(	// !js_func
		int x, int y, UINT uAlign, CVsdFont& Font,
		CPixelArg yc,			// !default:color_white
		CPixelArg ycBest,		// !default:color_cyan
		CPixelArg ycPlus,		// !default:color_red
		CPixelArg ycOutline		// !default:color_black
	);
	void DrawLapTimeLog(	// !js_func
		int x, int y, UINT uAlign, int iNum, CVsdFont& Font,
		CPixelArg yc,			// !default:color_white
		CPixelArg ycBest,		// !default:color_cyan
		CPixelArg ycOutline		// !default:color_black
	);
	void DrawRaceLapTime(	// !js_func
		int x, int y, UINT uAlign, int iNum, CVsdFont& Font,
		CPixelArg yc,			// !default:color_white
		CPixelArg ycOutline		// !default:color_black
	);
	void DrawNeedle(// !js_func
		int x, int y, int r1, int r2,
		int iStart, int iEnd, double dVal,
		CPixelArg yc,
		int iWidth // !default:1
	);
	
	// ラップタイム情報
	std::string FormatTime(int iTime); // !js_func
	std::wstring FormatTimeW(int iTime);

	int CurTime(){	// !js_var:ElapsedTime
		if(!m_LapLog) return TIME_NONE;
		return m_LapLog->m_iCurTime;
	}
	int BestLapTime(){	// !js_var:BestLapTime
		if(!m_LapLog) return TIME_NONE;
		return m_LapLog->m_iBestTime;
	}
	int DiffTime(){	// !js_var:DiffTime
		if(!m_LapLog) return TIME_NONE;
		return m_LapLog->m_iDiffTime;
	}
	int LapTime(){	// !js_var:LapTime
		if(!m_LapLog || m_LapLog->m_iLapIdx < 0){
			return TIME_NONE;
		}else if(m_LapLog->m_Lap[m_LapLog->m_iLapIdx + 1].iTime){
			return m_LapLog->m_Lap[m_LapLog->m_iLapIdx + 1].iTime;
		}
		return m_LapLog->m_Lap[m_LapLog->m_iLapIdx].iTime;
	}
	
	int LapCnt(void){// !js_var:LapCnt
		if(!m_LapLog) return 0;
		return m_LapLog->m_Lap[m_LapLog->m_iLapIdx + 1].uLap;
	}
	int MaxLapCnt(void){// !js_var:MaxLapCnt
		return m_LapLog ? m_LapLog->m_Lap[m_LapLog->m_iLapNum - 1].uLap : 0;
	}
	
	void DispErrorMessage(LPCWSTR szMsg);
	void DispErrorMessage(LPCSTR szMsg){DispErrorMessage(Utf8ToWide(szMsg).c_str());}

	enum {
		IMG_FILL		= (1 << 0),
		IMG_NOCLOSE		= (1 << 1),
	};
	
	enum {
		ALIGN_LEFT		= 0,
		ALIGN_TOP		= 0,
		ALIGN_HCENTER	= 1 << 0,
		ALIGN_RIGHT		= 1 << 1,
		ALIGN_VCENTER	= 1 << 2,
		ALIGN_BOTTOM	= 1 << 3,
		DRAW_MAP_START	= 1 << 4,	// DrawMap 専用フラグ，スタートライン描画
		IMG_LOADMAP		= 1 << 5,	// 地図用軌跡描画
		LMS_VERTICAL	= 1 << 4,	// DrawLinearMeterScale 専用フラグ
	};
	
	// clip
	int m_iClipX1, m_iClipY1, m_iClipX2, m_iClipY2;
	void SetClip(
		int x1 = INVALID_INT,
		int y1 = INVALID_INT,
		int x2 = INVALID_INT,
		int y2 = INVALID_INT
	){
		if(x1 > x2){int x = x1; x1 = x2; x2 = x;}
		if(y1 > y2){int y = y1; y1 = y2; y2 = y;}
		
		if(x1 < 0 || x1 == INVALID_INT) x1 = 0;
		if(x2 >= GetWidth() || x2 == INVALID_INT) x2 = GetWidth() - 1;
		if(y1 < 0 || y1 == INVALID_INT) y1 = 0;
		if(y2 >= GetHeight() || y2 == INVALID_INT) y2 = GetHeight() - 1;
		
		m_iClipX1 = x1;
		m_iClipX2 = x2;
		m_iClipY1 = y1;
		m_iClipY2 = y2;
	}
	
	template <class T>
	static void Sort2(T &a, T &b){
		if(a > b){
			T tmp = a;
			a = b;
			b = tmp;
		}
	}
	
	BOOL IsClippedX(int x1, int x2){
		return x1 < m_iClipX1 && x2 < m_iClipX1 || x1 > m_iClipX2 && x2 > m_iClipX2;
	}
	
	BOOL IsClippedY(int y1, int y2){
		return y1 < m_iClipY1 && y2 < m_iClipY1 || y1 > m_iClipY2 && y2 > m_iClipY2;
	}
	
	BOOL IsClipped(int x1, int y1, int x2, int y2){
		return IsClippedX(x1, x2) || IsClippedY(y1, y2);
	}
	
	BOOL IsClipped(int x1, int y1, int x2){
		return (
			IsClippedX(x1, x2) ||
			(y1 < m_iClipY1 || y1 > m_iClipY2)
		);
	}
	
	BOOL IsClipped(int x1, int y1){
		return (
			(x1 < m_iClipX1 || x1 > m_iClipX2) ||
			(y1 < m_iClipY1 || y1 > m_iClipY2)
		);
	}
	
	// ダイアログ設定リード
	
	int LineTrace(void){return m_piParamT[TRACK_LineTrace];}	// !js_var:Config_map_length
	int DispLap(void){return m_piParamC[CHECK_LAP];}			// !js_var:Config_lap_time
	int DispGraph(void){return m_piParamC[CHECK_GRAPH];}		// !js_var:Config_graph
	
	std::string	m_szLogFile;
	std::string	m_szLogFileReader;
	std::string	m_szGPSLogFile;
	std::string	m_szGPSLogFileReader;
	std::string	m_szLapChart;
	std::string	m_szSkinFile;
	std::string	m_szSkinDirA;
	std::string	m_SkinDirU;	// !js_const:SkinDir
	std::string	m_szPluginDirA;
	std::string m_PluginDirU;	// !js_const:VsdRootDir
	
	// リビジョン番号
	static const int	m_iRevision;	// !js_const:Revision
	static const int	m_iPrivateBuild;	// !js_const:PrivateBuild
	
	// ログリードヘルパ
	int ReadLog(std::unique_ptr<CVsdLog> &pLog, const char *szFileName, const char *szReaderFunc);
	double LapNum2LogNum(CVsdLog *Log, int iLapNum);
	std::unique_ptr<CLapLog> CreateLapTimeHand(int iLapSrc);
	std::unique_ptr<CLapLog> CreateLapTimeAuto(void);
	void CalcLapTime(void);
	
	// ラップチャートリーダ
	int LapChartRead(const char *szFileName);
	
	// スキン dir 取得
	std::string &SetSkinFile(const std::string& szSkinFile){
		if(szSkinFile.empty()){
			m_SkinDirU = "";
			m_szSkinDirA.clear();
			return m_szSkinDirA;
		}
		
		m_szSkinFile = szSkinFile;
		
		// スキンファイル名を CWD=m_szPluginDirA とみなしフルパスに変換
		auto skinPath = StrTokFile(szSkinFile, STF_FULL | STF_PATH2, m_szPluginDirA);
		
		// ↑のディレクトリ名を得る
		m_szSkinDirA = skinPath;
		m_SkinDirU = AnsiToUtf8(m_szSkinDirA);
		return m_szSkinDirA;
	}
	
	// プラグイン dll dir 取得
	std::string &SetPluginDir(void){
		auto s = StrTokFile(GetModuleFileNameDirectString(m_hInst), STF_PATH2);
		s += PLUGIN_DIR "\\";
		
		m_szPluginDirA = s;
		m_PluginDirU = AnsiToUtf8(m_szPluginDirA);
		
		return m_szPluginDirA;
	}
	
	int		*m_piParamT;
	int		*m_piParamC;
	int		*m_piParamS;
	
	// フォント
	std::unique_ptr<CVsdFont> m_pFont;
	
	std::unique_ptr<CVsdLog> m_VsdLog;
	std::unique_ptr<CVsdLog> m_GPSLog;
	CVsdLog *m_CurLog;
	std::unique_ptr<CLapLog> m_LapLog;
	
	BOOL ParseMarkStr(const char *szMark);
	
	BOOL		m_bCalcLapTimeReq;
	
	// JavaScript 用パラメータ
	
	#define DEF_LOG(name) double Get##name(void){\
		if(m_VsdLog && m_VsdLog->m_pLog##name) return m_VsdLog->name(); \
		return m_GPSLog->name(); \
	}
	#include "def_log.h"
	
	double DateTime(){	// !js_var:DateTime
		if     (m_VsdLog) return m_VsdLog->DateTime();
		else if(m_GPSLog) return m_GPSLog->DateTime();
		return NaN;
	}
	
	static HINSTANCE	m_hInst;	// dll handle
	
	void DeleteScript(void){
		m_Script.reset();
		m_LogJs.reset();
	}
	
	static void OpenConsole(void);
	static void Print(LPCWSTR szMsg);
	static void Print(const char *szFormat, ...);
	
	BOOL	m_bConsoleOn;
	
	std::unique_ptr<CScript> m_Script;
	
  protected:
	int m_iTextPosX, m_iTextPosY;
	
  private:
	// GDI+オブジェクト（画像展開に必要）
	Gdiplus::GdiplusStartupInput	m_gdiplusStartupInput;
	ULONG_PTR						m_gdiplusToken;
	
	std::unique_ptr<CVsdLogJs> m_LogJs;

	// スタートライン@GPS 計測モード
	double	m_dStartLineX1;
	double	m_dStartLineY1;
	double	m_dStartLineX2;
	double	m_dStartLineY2;
	
	void SetFrameMark(int iFrame);
	int  GetFrameMark(int iFrame);
	
	std::unique_ptr<PolygonData_t[]> m_Polygon;
	int m_iPolygonMinY;
	int m_iPolygonMaxY;
	
	// 解像度変更検出用
	int		m_iWidth;
	int		m_iHeight;
	BOOL	m_bSaving;
	
#ifdef AVS_PLUGIN
	/*** AviSynth ***********************************************************/
  public:
	CVsdFilter(
		PClip _child,
		AVSValue args,
		IScriptEnvironment* env
	);
	
	PVideoFrame __stdcall GetFrame(int n, IScriptEnvironment* env);
	int __stdcall SetCacheHints(int cachehints, int frame_range);

	/////////////
	
	int GetIndex(int x, int y){return m_iBytesPerLine * y + x * 2;}
	
	// 仮想関数
	void PutPixelC(int iIndex, CPixelArg yc, int iAlfa);
	
	int	GetWidth(void)	{return m_SrcFrame->GetRowSize()>>1;}	// !js_var:Width
	int	GetHeight(void)	{return m_SrcFrame->GetHeight();}	// !js_var:Height
	int	GetFrameMax(void)	{return vi.num_frames;}	// !js_var:MaxFrameCnt
	int	GetFrameCnt(void)	{return m_iFrameCnt;}	// !js_var:FrameCnt
	double GetFPS(void)	{return (double)vi.fps_numerator / vi.fps_denominator;} // !js_var:FramePerSecond
	int IsSaving(void)	{return m_piParamS[SHADOW_SAVING];}	// !js_var:IsSaving
	
	// パラメータ
	int	m_iFrameCnt, m_iFrameMax;
	int m_iBytesPerLine;
	std::vector<int> m_iMark;
	int m_iMarkCnt;
	
	BYTE	*m_pPlane;
	const char *m_szMark;
	IScriptEnvironment* m_env;
	PVideoFrame	m_SrcFrame;
#else
	/*** AviUtl *************************************************************/
	
  public:
	void				*editp;
	FILTER				*filter;
	std::unique_ptr<FILE_INFO> fileinfo;
	FILTER_PROC_INFO	*fpip;
	
	CVsdFilter(FILTER *filter, void *editp);
	
	int GetIndex(int x, int y){return fpip->max_w * y + x;}
	BOOL ConfigSave(const char *szFileName);
	BOOL ReadVsdLog(HWND hwnd);
	BOOL ReadGPSLog(HWND hwnd);
	char *IsConfigParamStr(const char *szParamName, char *szBuf, std::string &szDst);
	char *IsConfigParam(const char *szParamName, char *szBuf, int &iVal);
	BOOL ConfigLoad(const char *szFileName);
	void SetSkinName(const char *szSkinFile, HWND hwnd);

	// ログリーダ用
	BOOL CreateFilter(void);
	BOOL FileOpenDialog(std::string &szOut, std::string &szReaderFunc);
	std::unique_ptr<char[]> m_szLogFilter;
	std::vector<std::string> m_vecReaderFunc;
	
	// 仮想関数
	int	GetWidth(void)	{return fpip->w;}
	int	GetHeight(void)	{return fpip->h;}
	int	GetFrameMax(void)	{return filter->exfunc->get_frame_n(editp);}
	int	GetFrameCnt(void)	{return fpip->frame;}
	double	GetFPS(void)	{return (double)fileinfo->video_rate / fileinfo->video_scale;}
	int IsSaving(void)	{return filter->exfunc->is_saving(editp);}
	
	// 同期情報表示
	WCHAR *DrawSyncInfoFormatTime(
		WCHAR	*pBuf, UINT uBufSize, INT64 &iBaseTime, int iTime
	);
	void DrawSyncInfoSub(
		int x, int y, CVsdFont& Font,
		LPCWSTR	szCaption,
		INT64	&iBaseTime,
		int		iCurTime,
		int		iStartTime,
		int		iEndTime
	);
	
	// 1スライダ調整用パラメータ
	int	m_iAdjustPointNum;
	int	m_iAdjustPointVid[2];
	int	m_iAdjustPointVsd[2];
	int	m_iAdjustPointGPS[2];
	
	static const char *m_szTrackbarName[];
	static const char *m_szCheckboxName[];
	static const char *m_szShadowParamName[];
	
	void GetFileCreationTime(void);

  private:
	INT64	m_iVideoStartTime;
#endif
};

inline CVsdFilter* g_pVsd;
