/*****************************************************************************
	
	VSD -- vehicle data logger system  Copyright(C) by DDS
	
	vsd_filter.cpp - VSD filter for AviUtl
	
*****************************************************************************/

#include "StdAfx.h"

#include "CVsdFilter.h"
#include "rev_num.h"

/*** macros *****************************************************************/

#define CONFIG_EXT		"avs"

#define	FILE_CFG_EXT		"Config File (*." CONFIG_EXT ")\0*." CONFIG_EXT "\0AllFile (*.*)\0*.*\0"
#define	FILE_SKIN_EXT		"Skin File (*.js)\0*.js\0AllFile (*.*)\0*.*\0"
#define FILE_LAPCHART_EXT	"Lap Chart (*.txt)\0*.txt\0AllFile (*.*)\0*.*\0"

/*** new type ***************************************************************/

/*** const ******************************************************************/

enum {
	ID_BUTT_SET_VSt = 0xFF00,
	ID_BUTT_SET_VEd,
	ID_BUTT_SET_LSt,
	ID_BUTT_SET_LEd,
	ID_BUTT_SET_GSt,
	ID_BUTT_SET_GEd,
	ID_EDIT_LOAD_LOG,
	ID_BUTT_LOAD_LOG,
	ID_EDIT_LOAD_GPS,
	ID_BUTT_LOAD_GPS,
	ID_BUTT_LAP_START,
	ID_BUTT_LAP_END,
	ID_EDIT_LOAD_LAPCHART,
	ID_BUTT_LOAD_LAPCHART,
	ID_COMBO_SEL_SKIN,
	ID_BUTT_SEL_SKIN,	// NOT USED
	ID_BUTT_LOAD_CFG,
	ID_BUTT_SAVE_CFG,
};

#define POS_TH_LABEL			30
#define POS_TH_SLIDER			220
#define POS_TH_EDIT				294
#define POS_ADD_LABEL			50
#define POS_ADD_SLIDER			300
#define POS_ADD_EDIT			16
#define POS_CHK_LABEL_L			36
#define POS_CHK_LABEL_R			186

#define POS_SET_BUTT_SIZE		30

#define POS_ADD_DIALOG_HEIGHT	(10 + POS_FILE_HEIGHT + POS_FILE_HEIGHT_MARGIN)
#define POS_FILE_NUM			5

#define POS_FILE_CAPTION_SIZE	70
#define POS_FILE_NAME_SIZE		(rectClient.right - (POS_FILE_CAPTION_POS + POS_FILE_CAPTION_SIZE + POS_FILE_BUTT_SIZE))
#define POS_FILE_BUTT_SIZE		40
#define POS_FILE_CAPTION_POS	150
#define POS_FILE_HEIGHT			21
#define POS_FILE_HEIGHT_MARGIN	2

#define OFFSET_ADJUST_WIDTH	(60 * 10 * 1000)	// 10分


/*** track / check ID *******************************************************/

enum {
	#define DEF_TRACKBAR(name, init, min, max, caption)	ID_TRK_##name,
	#include "def_trackbar.h"
	TRACK_N
};

enum {
	#define DEF_CHECKBOX(name, init, caption)	ID_CHK_##name,
	#include "def_checkbox.h"
	CHECK_N
};

/****************************************************************************/
//---------------------------------------------------------------------
//		フィルタ構造体定義
//---------------------------------------------------------------------

// トラックバーの名前
const TCHAR	*track_name[] = {
	#define DEF_TRACKBAR(name, init, min, max, caption)	caption,
	#include "def_trackbar.h"
};
// トラックバーの初期値
int		track_default[] = {
	#define DEF_TRACKBAR(name, init, min, max, caption)	init,
	#include "def_trackbar.h"
};
// トラックバーの下限値
int		track_s[] = {
	#define DEF_TRACKBAR(name, init, min, max, caption)	min,
	#include "def_trackbar.h"
};
// トラックバーの上限値
int		track_e[] = {
	#define DEF_TRACKBAR(name, init, min, max, caption)	max,
	#include "def_trackbar.h"
};

// チェックボックスの名前
const TCHAR	*check_name[] = {
	#define DEF_CHECKBOX(name, init, caption)	caption,
	#include "def_checkbox.h"
};

// チェックボックスの初期値 (値は0か1)
int		check_default[] = {
	#define DEF_CHECKBOX(name, init, caption)	init,
	#include "def_checkbox.h"
};

char g_szDescription[] = PROG_NAME_J " " PROG_REVISION_STR;

FILTER_DLL filter = {
	FILTER_FLAG_EX_INFORMATION | FILTER_FLAG_MAIN_MESSAGE | FILTER_FLAG_EX_INFORMATION,
	0,0,						//	設定ウインドウのサイズ (FILTER_FLAG_WINDOW_SIZEが立っている時に有効)
	(TCHAR*)PROG_NAME_J,		//	フィルタの名前
	TRACK_N,					//	トラックバーの数 (0なら名前初期値等もnullptrでよい)
	(TCHAR**)track_name,		//	トラックバーの名前郡へのポインタ
	track_default,				//	トラックバーの初期値郡へのポインタ
	track_s, track_e,			//	トラックバーの数値の下限上限 (nullptrなら全て0～256)
	CHECK_N,					//	チェックボックスの数 (0なら名前初期値等もnullptrでよい)
	(TCHAR**)check_name,		//	チェックボックスの名前郡へのポインタ
	check_default,				//	チェックボックスの初期値郡へのポインタ
	func_proc,					//	フィルタ処理関数へのポインタ (nullptrなら呼ばれません)
	nullptr, //func_init,		//	開始時に呼ばれる関数へのポインタ (nullptrなら呼ばれません)
	nullptr, //func_exit,		//	終了時に呼ばれる関数へのポインタ (nullptrなら呼ばれません)
	func_update,				//	設定が変更されたときに呼ばれる関数へのポインタ (nullptrなら呼ばれません)
	func_WndProc,				//	設定ウィンドウにウィンドウメッセージが来た時に呼ばれる関数へのポインタ (nullptrなら呼ばれません)
	nullptr, nullptr,			//	システムで使いますので使用しないでください
	nullptr,					//  拡張データ領域へのポインタ (FILTER_FLAG_EX_DATAが立っている時に有効)
	0,							//  拡張データサイズ (FILTER_FLAG_EX_DATAが立っている時に有効)
	g_szDescription,			//  フィルタ情報へのポインタ (FILTER_FLAG_EX_INFORMATIONが立っている時に有効)
	nullptr,					//	セーブが開始される直前に呼ばれる関数へのポインタ (nullptrなら呼ばれません)
	nullptr,					//	セーブが終了した直前に呼ばれる関数へのポインタ (nullptrなら呼ばれません)
};


//---------------------------------------------------------------------
//		フィルタ構造体のポインタを渡す関数
//---------------------------------------------------------------------

EXTERN_C FILTER_DLL __declspec(dllexport) * __stdcall GetFilterTable(void){
	return &filter;
}

//---------------------------------------------------------------------
//		初期化
//---------------------------------------------------------------------

/*
BOOL func_init(FILTER *fp){
	return TRUE;
}
*/

//---------------------------------------------------------------------
//		終了
//---------------------------------------------------------------------

/*
BOOL func_exit(FILTER *fp){
	return TRUE;
}
*/

//---------------------------------------------------------------------
//		フィルタ処理関数
//---------------------------------------------------------------------

/*** CVsdFilter クラス ***************************************************/

/*** コンストラクタ／デストラクタ *******************************************/

CVsdFilter* g_pVsd;

CVsdFilter::CVsdFilter(FILTER *filter, void *editp) :
	filter(filter), editp(editp)
{
	Constructor();	// 基本クラスのコンストラクタ
	
	fileinfo = std::make_unique<FILE_INFO>();
	filter->exfunc->get_file_info(editp, fileinfo.get());
	
	SetSkinFileDir(m_Param.m_strSkinFile);
	
	m_iVideoStartTime = -1;
	m_iAdjustPointNum = 0;
	m_iAdjustPointVid[0] =
	m_iAdjustPointVid[1] = INVALID_INT;
	
	m_szLogFilter	= nullptr;
	fpip			= nullptr;
}

CVsdFilter::~CVsdFilter(){
	Destructor();	// 基本クラスのデストラクタ
	
}

/*** PutPixel ***************************************************************/

/* 変換式
Y  =  0.299R+0.587G+0.114B
Cr =  0.500R-0.419G-0.081B
Cb = -0.169R-0.332G+0.500B

R = Y+1.402Cr 
G = Y-0.714Cr-0.344Cb 
B = Y+1.772Cb 
*/

void CVsdFilter::PutPixelC(int x, int y, const CPixelArg yc){
	
	PIXEL_YC	*ycp = fpip->ycp_edit;
	
	int	iIndex = GetIndex(x, y);
	if(yc.alfa){
		int iAlfa = (int)yc.alfa;
		
		ycp[iIndex].y  = (PIXEL_t)(yc.y  + ((ycp[iIndex].y  * iAlfa) >> 8));
		ycp[iIndex].cr = (PIXEL_t)(yc.cr + ((ycp[iIndex].cr * iAlfa) >> 8));
		ycp[iIndex].cb = (PIXEL_t)(yc.cb + ((ycp[iIndex].cb * iAlfa) >> 8));
	}else{
		ycp[iIndex] = (PIXEL_YC &)yc;
	}
}

void CVsdFilter::FillLineC(int x1, int y1, int x2, const CPixelArg yc){
	
	PIXEL_YC	*ycp = fpip->ycp_edit;
	
	int idx1 = GetIndex(x1, y1);
	int idx2 = GetIndex(x2, y1);
	
	int iIndex;
	if(yc.alfa){
		int iAlfa = (int)yc.alfa;
		
		for(iIndex = idx1; iIndex <= idx2; ++iIndex){
			ycp[iIndex].y  = (PIXEL_t)(yc.y  + ((ycp[iIndex].y  * iAlfa) >> 8));
			ycp[iIndex].cr = (PIXEL_t)(yc.cr + ((ycp[iIndex].cr * iAlfa) >> 8));
			ycp[iIndex].cb = (PIXEL_t)(yc.cb + ((ycp[iIndex].cb * iAlfa) >> 8));
		}
	}else{
		for(iIndex = idx1; iIndex <= idx2; ++iIndex){
			ycp[iIndex] = (PIXEL_YC &)yc;
		}
	}
}

/*** PutImage ***************************************************************/

void CVsdFilter::PutImage0C(
	int x, int y, CVsdImage &img,
	int ix_st, int iy_st, int ix_ed, int iy_ed
){
	PIXEL_YC	*ycp = fpip->ycp_edit;
	
	for(int iy = iy_st; iy < iy_ed; ++iy, ++y){
		
		int	iIndex = GetIndex(x, y);
		
		for(int ix = ix_st; ix < ix_ed; ++ix, ++iIndex){
			
			CPixel yc(img.GetPixelRaw(ix, iy));
			
			if(yc.alfa != 256){
				if(yc.alfa){
					int iAlfa = (int)yc.alfa;
					
					ycp[iIndex].y  = (PIXEL_t)(yc.y  + ((ycp[iIndex].y  * iAlfa) >> 8));
					ycp[iIndex].cr = (PIXEL_t)(yc.cr + ((ycp[iIndex].cr * iAlfa) >> 8));
					ycp[iIndex].cb = (PIXEL_t)(yc.cb + ((ycp[iIndex].cb * iAlfa) >> 8));
				}else{
					ycp[iIndex] = (PIXEL_YC &)yc;
				}
			}
		}
	}
}

/*** 同期情報描画 ***********************************************************/

WCHAR *CVsdFilter::DrawSyncInfoFormatTime(
	WCHAR	*pBuf, UINT uBufSize, INT64 &iBaseTime, int iTime
){
	struct tm	tmLocal;
	time_t	time = (iBaseTime + iTime) / 1000;
	localtime_s(&tmLocal, &time);
	
	wcsftime(pBuf, uBufSize, L"%H:%M:%S", &tmLocal);
	
	LPWSTR	p = wcschr(pBuf, '\0');
	swprintf(
		p, uBufSize - (p - pBuf), L".%02d",
		(UINT)((iBaseTime + iTime) % 1000) / 10
	);
	
	return pBuf;
}

void CVsdFilter::DrawSyncInfoSub(
	int x, int y, CVsdFont &Font,
	LPCWSTR	szCaption,
	INT64	&iBaseTime,
	int		iCurTime,
	int		iStartTime,
	int		iEndTime
){
	WCHAR	szBuf[32];
	
	// キャプション
	DrawText(x, y, szCaption, Font, color_white);
	
	// 日付
	struct tm	tmLocal;
	time_t time	= (time_t)(iBaseTime + iCurTime) / 1000;
	
	localtime_s(&tmLocal, &time);
	wcsftime(szBuf, sizeof(szBuf) / sizeof(WCHAR), L"%Y/%m/%d", &tmLocal);
	DrawText(POS_DEFAULT, POS_DEFAULT, szBuf, Font, color_white);
	
	// 時間 x 3
	DrawText(
		POS_DEFAULT, POS_DEFAULT,
		DrawSyncInfoFormatTime(szBuf, sizeof(szBuf), iBaseTime, iCurTime),
		Font, color_white
	);
	DrawText(
		POS_DEFAULT, POS_DEFAULT,
		DrawSyncInfoFormatTime(szBuf, sizeof(szBuf), iBaseTime, iStartTime),
		Font, color_white
	);
	DrawText(
		POS_DEFAULT, POS_DEFAULT,
		DrawSyncInfoFormatTime(szBuf, sizeof(szBuf), iBaseTime, iEndTime),
		Font, color_white
	);

	// ローカルタイムの 0:00:00 の time_t を得る
	tmLocal.tm_sec =
	tmLocal.tm_min =
	tmLocal.tm_hour = 0;

	INT64 iTmp = (INT64)mktime(&tmLocal) * 1000;

	DrawText(
		POS_DEFAULT, POS_DEFAULT,
		DrawSyncInfoFormatTime(szBuf, sizeof(szBuf), iTmp, iEndTime - iStartTime),
		Font, color_white
	);
}

void CVsdFilter::DrawSyncInfo(int x, int y, CVsdFont &Font, UINT uAlign){
	
	// フレーム表示
	
	#define SYNC_INFO_LINES	6
	
	if(uAlign & ALIGN_HCENTER){
		x -= Font.GetWidth() * 40 / 2;
	}else if(uAlign & ALIGN_RIGHT){
		x -= Font.GetWidth() * 40;
	}
	
	if(uAlign & ALIGN_VCENTER){
		y -= Font.GetHeight() * SYNC_INFO_LINES / 2;
	}else if(uAlign & ALIGN_BOTTOM){
		y -= Font.GetHeight() * SYNC_INFO_LINES;
	}
	
	m_iTextPosX = x;
	m_iTextPosY = y;
	
	m_iTextPosY += Font.GetHeight();
	DrawText(POS_DEFAULT, POS_DEFAULT, L"時刻", Font, color_white);
	m_iTextPosY += Font.GetHeight();
	DrawText(POS_DEFAULT, POS_DEFAULT, L"開始", Font, color_white);
	DrawText(POS_DEFAULT, POS_DEFAULT, L"終了", Font, color_white);
	DrawText(POS_DEFAULT, POS_DEFAULT, L"範囲", Font, color_white);

	x += Font.GetWidth() * 5;
	
	if(m_iVideoStartTime != -1){
		DrawSyncInfoSub(
			x, y, Font, L"ビデオ",
			m_iVideoStartTime,
			(int)(GetFrameCnt() * 1000 / GetFPS()),
			(int)(m_Param.m_iVideoStart * 1000 / GetFPS()),
			(int)(m_Param.m_iVideoEnd   * 1000 / GetFPS())
		);
		x += Font.GetWidth() * 12;
	}
	
	if(m_VsdLog){
		DrawSyncInfoSub(
			x, y, Font, L"車両ログ",
			m_VsdLog->m_iLogStartTime,
			m_VsdLog->GetTime(),
			(int)(m_Param.m_iVsdStart / SLIDER_TIME),
			(int)(m_Param.m_iVsdEnd   / SLIDER_TIME)
		);
		x += Font.GetWidth() * 12;
	}
	
	if(m_GpsLog){
		DrawSyncInfoSub(
			x, y, Font, L"GPSログ",
			m_GpsLog->m_iLogStartTime,
			m_GpsLog->GetTime(),
			(int)(m_Param.m_iGpsStart / SLIDER_TIME),
			(int)(m_Param.m_iGpsEnd   / SLIDER_TIME)
		);
		x += Font.GetWidth() * 12;
	}
}

/*** フレームをマーク *******************************************************/

void CVsdFilter::SetFrameMark(int iFrame){
	FRAME_STATUS	fsp;
	
	filter->exfunc->get_frame_status(editp, iFrame, &fsp);
	fsp.edit_flag |= EDIT_FRAME_EDIT_FLAG_MARKFRAME;
	filter->exfunc->set_frame_status(editp, iFrame, &fsp);
}

int CVsdFilter::GetFrameMark(int iFrame){
	
	FRAME_STATUS	fsp;
	
	for(; iFrame < GetFrameMax(); ++iFrame){
		filter->exfunc->get_frame_status(editp, iFrame, &fsp);
		
		if(fsp.edit_flag & EDIT_FRAME_EDIT_FLAG_MARKFRAME){
			return iFrame;
		}
	}
	return -1;
}

/*** ファイル作成日時取得 ***************************************************/

void CVsdFilter::GetFileCreationTime(void){
	FILETIME	ft;
	
	HANDLE hFile = CreateFile(
		fileinfo->name,			// ファイル名
		GENERIC_READ,			// アクセスモード
		FILE_SHARE_READ,		// 共有モード
		nullptr,					// セキュリティ記述子
		OPEN_EXISTING,			// 作成方法
		FILE_ATTRIBUTE_NORMAL,	// ファイル属性
		nullptr					// テンプレートファイルのハンドル
	);
	
	if(hFile == nullptr) return;
	
	//if(GetFileTime(hFile, nullptr, nullptr, &ft)){		// デバッグ用 ライトタイム
	if(GetFileTime(hFile, &ft, nullptr, nullptr)){
		m_iVideoStartTime =
			(
				(((LONGLONG)ft.dwHighDateTime << 32) + ft.dwLowDateTime) -
				116444736000000000
			) / 10000;
	}
	
	CloseHandle(hFile);
}

/*** ログリード *************************************************************/

BOOL CVsdFilter::ReadVsdLog(HWND hwnd){
	
	//char szMsg[BUF_SIZE];
	int iCnt;
	
	if(!(iCnt = CVsdFilter::ReadLog(m_VsdLog, m_Param.m_strLogFile.c_str(), m_Param.m_strLogFileReader.c_str()))){
		return FALSE;
	}
	
	SetWindowText(GetDlgItem(hwnd, ID_EDIT_LOAD_LOG), m_Param.m_strLogFile.c_str());
	
	// trackbar 設定
	track_e[ID_TRK_log_start] =
	track_e[ID_TRK_log_end] =
		(int)(m_VsdLog->GetTime(m_VsdLog->GetCnt() - 2) * SLIDER_TIME);

	return TRUE;
}

/*** GPS ログリード ********************************************************/

BOOL CVsdFilter::ReadGPSLog(HWND hwnd){
	
	//char szMsg[BUF_SIZE];
	
	int iCnt;
	if(!(iCnt = CVsdFilter::ReadLog(m_GpsLog, m_Param.m_strGPSLogFile.c_str(), m_Param.m_strGPSLogFileReader.c_str()))){
		return FALSE;
	}
	
	SetWindowText(GetDlgItem(hwnd, ID_EDIT_LOAD_GPS), m_Param.m_strGPSLogFile.c_str());
	
	// trackbar 設定
	track_e[ID_TRK_gps_start] =
	track_e[ID_TRK_gps_end] =
		(int)(m_GpsLog->GetTime(m_GpsLog->GetCnt() - 2) * SLIDER_TIME);

	return TRUE;
}

/*** func_proc **************************************************************/

BOOL func_proc(FILTER *fp, FILTER_PROC_INFO *fpip){
	if(!g_pVsd) return 0;
	// クラスに変換
	g_pVsd->fpip	= fpip;
	
	BOOL bRet = g_pVsd->DrawVSD();
	#ifdef DEBUG
	g_pVsd->fpip	= nullptr;
	#endif
	
	return bRet;
}

/*** *.js ファイルリスト取得 ************************************************/

BOOL ListTreeCallback(const std::string& strPath, const std::string& strFile, void *pParam){
	if(!IsExt(strFile, ".js")) return TRUE;
	
	auto path = strPath + strFile;
	const char *p = path.c_str() + strlen(g_pVsd->m_szPluginDirA.c_str());
	
	SendMessage((HWND)pParam, CB_ADDSTRING, 0, (LPARAM)p);
	
	return TRUE;
}

void SetSkinFileList(HWND hwnd){
	if(SendMessage(hwnd, CB_GETCOUNT, 0, 0) > 0) return;
	
	ListTree(g_pVsd->m_szPluginDirA, "*", ListTreeCallback, hwnd);
	SendMessage(hwnd, CB_ADDSTRING, 0, (LPARAM)"ファイル名を指定して開く...");
	SetWindowText(hwnd, g_pVsd->m_Param.m_strSkinFile.c_str());
}

/*** ダイアログサイズ拡張とパーツ追加 ***************************************/

#define CreateButton(Caption, x, y, id)(\
	hwndChild = CreateWindow(\
		"BUTTON", (Caption), WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, \
		(x), (y), POS_FILE_BUTT_SIZE, POS_FILE_HEIGHT, \
		hwnd, (id), hInst, nullptr \
	), \
	SendMessage(hwndChild, WM_SETFONT, (WPARAM)hfont, 0) \
)

void CreateControlFileName(
	HWND hwnd, HINSTANCE hInst, int &iID, HFONT hfont, int iX, int &iY, RECT rectClient,
	const char *szCap, const char *szEdit, const char *szButt
) {
	
	HWND hwndChild;
	
	hwndChild = CreateWindow(
		"STATIC", szCap, WS_CHILD | WS_VISIBLE,
		iX, iY,
		POS_FILE_CAPTION_SIZE, POS_FILE_HEIGHT,
		hwnd, 0, hInst, nullptr
	);
	SendMessage(hwndChild, WM_SETFONT, (WPARAM)hfont, 0);
	
	hwndChild = CreateWindowEx(
		WS_EX_CLIENTEDGE,
		"EDIT", szEdit, WS_CHILD | WS_VISIBLE | ES_READONLY,
		iX + POS_FILE_CAPTION_SIZE, iY,
		POS_FILE_NAME_SIZE, POS_FILE_HEIGHT,
		hwnd, (HMENU)(INT_PTR)iID, hInst, nullptr
	);
	SendMessage(hwndChild, WM_SETFONT, (WPARAM)hfont, 0);
	
	hwndChild = CreateWindow(
		"BUTTON", szButt, WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
		iX + POS_FILE_CAPTION_SIZE + POS_FILE_NAME_SIZE, iY,
		POS_FILE_BUTT_SIZE, POS_FILE_HEIGHT,
		hwnd, (HMENU)(INT_PTR)(iID + 1), hInst, nullptr
	);
	SendMessage(hwndChild, WM_SETFONT, (WPARAM)hfont, 0);
	
	iID += 2;
	iY += POS_FILE_HEIGHT + POS_FILE_HEIGHT_MARGIN;
}

void CreateControlSkinName(
	HWND hwnd, HINSTANCE hInst, int &iID, HFONT hfont, int iX, int &iY, RECT rectClient,
	const char *szCap, const char *szEdit
) {
	
	HWND hwndChild;
	
	hwndChild = CreateWindow(
		"STATIC", szCap, WS_CHILD | WS_VISIBLE,
		iX, iY,
		POS_FILE_CAPTION_SIZE, POS_FILE_HEIGHT,
		hwnd, 0, hInst, nullptr
	);
	SendMessage(hwndChild, WM_SETFONT, (WPARAM)hfont, 0);
	
	hwndChild = CreateWindowEx(
		WS_EX_CLIENTEDGE,
		"COMBOBOX", nullptr, WS_CHILD | WS_VISIBLE | WS_VSCROLL | CBS_DROPDOWN,
		iX + POS_FILE_CAPTION_SIZE, iY,
		POS_FILE_NAME_SIZE + POS_FILE_BUTT_SIZE, POS_FILE_HEIGHT * 10,
		hwnd, (HMENU)(INT_PTR)iID, hInst, nullptr
	);
	SendMessage(hwndChild, WM_SETFONT, (WPARAM)hfont, 0);
	SendMessage(hwndChild, CB_SETCURSEL, 0, 0);
	
	iID += 2;
	iY += POS_FILE_HEIGHT + POS_FILE_HEIGHT_MARGIN;
}

void ExtendDialog(HWND hwnd, HINSTANCE hInst){
	
	union {
		struct {
			POINT	topleft, bottomright;
		} points;
		RECT	rect;
	} rect;
	RECT	rectClient;
	
	HWND	hwndChild	= nullptr;
	HFONT	hfont		= nullptr;
	
	// クライアント領域のサイズ get 後にリサイズ
	GetWindowRect(hwnd, &rectClient);
	MoveWindow(hwnd,
		rectClient.left, rectClient.top,
		rectClient.right  - rectClient.left + POS_ADD_LABEL + POS_ADD_SLIDER + POS_ADD_EDIT + POS_SET_BUTT_SIZE,
		rectClient.bottom - rectClient.top + POS_ADD_DIALOG_HEIGHT,
		TRUE
	);
	
	GetClientRect(hwnd, &rectClient);
	
	while(1){
		/*** 子パーツのサイズ変更 ***/
		hwndChild = FindWindowEx(hwnd, hwndChild, nullptr, nullptr);
		if(!hwndChild) break;
		
		// screen -> client 座標に変換
		GetWindowRect(hwndChild, &rect.rect);
		ScreenToClient(hwnd, &rect.points.topleft);
		ScreenToClient(hwnd, &rect.points.bottomright);
		
		/*
		if(rect.rect.right  >= POS_TH_EDIT){
			// エディットボックスのスタイル変更
			//SendMessage(hwnd, EM_SETREADONLY, FALSE, 0);
			SetWindowLong(
				hwndChild, GWL_EXSTYLE,
				GetWindowLong(hwndChild, GWL_EXSTYLE) | WS_EX_RIGHT | WS_EX_CLIENTEDGE
			);
		}
		*/
		
		// ダイアログ左側を延ばす，EDIT ボックスのサイズを伸ばす
		#define ResizeControl(name) {\
			if(rect.rect.right >= POS_TH_ ## name) rect.rect.right += POS_ADD_ ## name; \
			if(rect.rect.left  >= POS_TH_ ## name) rect.rect.left  += POS_ADD_ ## name; \
		}
		
		// 右から順番に書くこと
		ResizeControl(EDIT);
		ResizeControl(SLIDER);
		// チェックボックスのラベル判定
		if(rect.rect.left < POS_CHK_LABEL_L && POS_CHK_LABEL_R < rect.rect.right){
			rect.rect.right = POS_FILE_CAPTION_POS;
		}else{
			ResizeControl(LABEL);
		}
		
		// 実際にリサイズ
		MoveWindow(hwndChild,
			rect.rect.left,
			rect.rect.top,
			rect.rect.right  - rect.rect.left,
			rect.rect.bottom - rect.rect.top,
			TRUE
		);
		
		if(!hfont) hfont = (HFONT)SendMessage(hwndChild, WM_GETFONT, 0, 0);
	}
	
	// 位置取得ボタン
	int i;
	for(i = 0; i <= (ID_BUTT_SET_GEd - ID_BUTT_SET_VSt); ++i){
		hwndChild = CreateWindow(
			"BUTTON", "set", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
			rectClient.right - POS_SET_BUTT_SIZE, 14 + (i + 2) * 24,
			POS_SET_BUTT_SIZE, 16,
			hwnd, (HMENU)(INT_PTR)(ID_BUTT_SET_VSt + i), hInst, nullptr
		);
		SendMessage(hwndChild, WM_SETFONT, (WPARAM)hfont, 0);
	}

	// ログ名・フォント名
	i = ID_BUTT_SET_GEd + 1;
	int y = rectClient.bottom - (POS_FILE_HEIGHT + POS_FILE_HEIGHT_MARGIN) * POS_FILE_NUM + POS_FILE_HEIGHT_MARGIN;
	
	CreateControlFileName(hwnd, hInst, i, hfont, POS_FILE_CAPTION_POS, y, rectClient, "車両ログ",	"",		"開く");
	CreateControlFileName(hwnd, hInst, i, hfont, POS_FILE_CAPTION_POS, y, rectClient, "GPSログ",	"",		"開く");
	
	CreateButton("始", rectClient.right - (POS_FILE_BUTT_SIZE * 2), y, (HMENU)(INT_PTR)(i++));
	CreateButton("終", rectClient.right - POS_FILE_BUTT_SIZE,         y, (HMENU)(INT_PTR)(i++));
	
	rectClient.right -= POS_FILE_BUTT_SIZE * 2;
	CreateControlFileName(hwnd, hInst, i, hfont, POS_FILE_CAPTION_POS, y, rectClient, "LAP表",	"",		"開く");
	rectClient.right += POS_FILE_BUTT_SIZE * 2;
	
	CreateControlSkinName(hwnd, hInst, i, hfont, POS_FILE_CAPTION_POS, y, rectClient, "スキン",	"");
	
	// rev
	hwndChild = CreateWindow(
		"STATIC", "VSD for GPS " PROG_REVISION_STR, WS_CHILD | WS_VISIBLE,
		POS_FILE_CAPTION_POS + POS_FILE_CAPTION_SIZE, y,
		POS_FILE_NAME_SIZE / 2, POS_FILE_HEIGHT,
		hwnd, 0, hInst, nullptr
	);
	SendMessage(hwndChild, WM_SETFONT, (WPARAM)hfont, 0);
	
	// cfg load/save ボタン
	hwndChild = CreateWindow(
		"STATIC", "cfgファイル", WS_CHILD | WS_VISIBLE,
		rectClient.right - (POS_FILE_BUTT_SIZE * 2 + POS_FILE_CAPTION_SIZE), y,
		POS_FILE_CAPTION_SIZE, POS_FILE_HEIGHT,
		hwnd, 0, hInst, nullptr
	);
	SendMessage(hwndChild, WM_SETFONT, (WPARAM)hfont, 0);
	
	CreateButton("開く", rectClient.right - (POS_FILE_BUTT_SIZE * 2), y, (HMENU)(INT_PTR)(i++));
	CreateButton("保存", rectClient.right - POS_FILE_BUTT_SIZE,         y, (HMENU)(INT_PTR)(i++));
}

/*** OpenDialog 用フィルタ作成 **********************************************/

BOOL CVsdFilter::CreateFilter(void) {
	char	szBuf[BUF_SIZE];
	char* pBuf = szBuf;

	{
		// JavaScript オブジェクト初期化
		CScript Script(this);
		if (Script.InitLogReader() != ERR_OK) {
			return FALSE;
		}

		JSContext* ctx = Script.GetContextRaw();
		if (!ctx) {
			return FALSE;
		}

		/*** JS の "LogReaderInfo" にアクセス *******************************/

		qjsb::Value global(ctx, JS_GetGlobalObject(ctx));
		if (JS_IsException(global.get()) || !JS_IsObject(global.get())) {
			return FALSE;
		}

		qjsb::Value hFilter(ctx, JS_GetPropertyStr(ctx, global.get(), "LogReaderInfo"));
		if (JS_IsException(hFilter.get()) || !JS_IsArray(ctx, hFilter.get())) {
			return FALSE;
		}

		qjsb::Value lenVal(ctx, JS_GetPropertyStr(ctx, hFilter.get(), "length"));
		uint32_t uLen = 0;
		if (JS_IsException(lenVal.get()) || JS_ToUint32(ctx, &uLen, lenVal.get()) < 0) {
			return FALSE;
		}

		m_vecReaderFunc.clear();

		// 表示名と filter 文字列を \0 で cat
		strcpy_s(pBuf, BUF_SIZE - (pBuf - szBuf), "自動判別 (*.*)");
		pBuf += strlen(pBuf) + 1;
		strcpy_s(pBuf, BUF_SIZE - (pBuf - szBuf), "*");
		pBuf += strlen(pBuf) + 1;

		for (uint32_t u = 0; u < uLen; ++u) {
			qjsb::Value item(ctx, JS_GetPropertyUint32(ctx, hFilter.get(), u));
			if (JS_IsException(item.get()) || !JS_IsObject(item.get())) {
				continue;
			}

			qjsb::Value capVal(ctx, JS_GetPropertyStr(ctx, item.get(), "Caption"));
			qjsb::Value extVal(ctx, JS_GetPropertyStr(ctx, item.get(), "Filter"));
			qjsb::Value funcVal(ctx, JS_GetPropertyStr(ctx, item.get(), "ReaderFunc"));

			const char* cName = JS_ToCString(ctx, capVal.get());
			const char* cExt = JS_ToCString(ctx, extVal.get());
			const char* cFunc = JS_ToCString(ctx, funcVal.get());

			if (cName && cExt && cFunc) {
				std::string ansiName = Utf8ToAnsi(cName);
				int iRem = (int)(BUF_SIZE - (pBuf - szBuf));
				if (iRem > 1) {
					strncpy_s(pBuf, iRem, ansiName.c_str(), _TRUNCATE);
					pBuf += strlen(pBuf) + 1;
				}

				iRem = (int)(BUF_SIZE - (pBuf - szBuf));
				if (iRem > 1) {
					strncpy_s(pBuf, iRem, cExt, _TRUNCATE);
					pBuf += strlen(pBuf) + 1;
				}

				m_vecReaderFunc.emplace_back(cFunc);
			}

			if (cName) JS_FreeCString(ctx, cName);
			if (cExt)  JS_FreeCString(ctx, cExt);
			if (cFunc) JS_FreeCString(ctx, cFunc);
		}

		if ((size_t)(pBuf - szBuf) < BUF_SIZE) {
			*(pBuf++) = '\0';
		}
	}

	int iSize = (int)(pBuf - szBuf);
	m_szLogFilter = std::make_unique<char[]>(iSize);
	memcpy_s(m_szLogFilter.get(), iSize, szBuf, iSize);

	return TRUE;
}

/*** ログリード複数ファイル対応版 *******************************************/

#define FILTERBUF_SIZE	128

BOOL CVsdFilter::FileOpenDialog(std::string &szOut, std::string &szReaderFunc){
	
	char szBuf[BUF_SIZE];
	
	// ログリーダ用フィルタ作成
	if(m_szLogFilter == nullptr && CreateFilter() != TRUE){
		return FALSE;
	}
	
	OPENFILENAME	ofn;
	memset(&ofn, 0, sizeof(ofn));
	
	ofn.lStructSize	= sizeof(ofn);
	ofn.lpstrFilter	= m_szLogFilter.get();
	ofn.lpstrFile	= szBuf;
	ofn.nMaxFile	= BUF_SIZE;
	ofn.Flags		= OFN_EXPLORER | OFN_ALLOWMULTISELECT | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;
	*szBuf			= '\0';
	
	if(!GetOpenFileName(&ofn)) return FALSE;
	
	// ファイル名を '/' で連結
	char *p = szBuf;
	
	while(1){
		p = strchr(p, '\0');
		if(*(p + 1) == '\0') break;
		*p = '/';
		++p;
	}
	
	szOut = szBuf;
	szReaderFunc = ofn.nFilterIndex >= 2 ? m_vecReaderFunc[ofn.nFilterIndex - 2] : "";
	return TRUE;
}

/*** 設定ロード *************************************************************/

bool CVsdFilter::ConfigLoad(const char *szFileName){
	
	bool ret;
	
	if(!(ret = m_Param.Load(szFileName))) return ret;
	
	// AvuUti 設定にコピー
	#define DEF_PARAM_TRK(name, type, var, ...)	filter->track[ID_TRK_##name] = g_pVsd->m_Param.var;
	#define DEF_PARAM_CHK(name, type, var, ...)	filter->check[ID_CHK_##name] = g_pVsd->m_Param.var;
	#include "def_param.h"

	// mark 復元
	for(auto i : m_Param.m_Mark) SetFrameMark(i);
	m_bCalcLapTimeReq = true;
	
	if(m_Param.m_strSkinFile.empty()) m_Param.m_strSkinFile = DEFAULT_SKIN;
	SetSkinFileDir(m_Param.m_strSkinFile);
	
	return true;
}

void CVsdFilter::SetSkinName(const char *szSkinFile, HWND hwnd){
	DeleteScript();
	SetSkinFileDir(szSkinFile);
	
	// skin 名をダイアログに設定
	SetWindowText(GetDlgItem(hwnd, ID_COMBO_SEL_SKIN), m_Param.m_strSkinFile.c_str());
}

/*** config セーブ **********************************************************/

bool CVsdFilter::ConfigSave(const char *szFileName){
	
	// 手動ラップ計測マーク出力
	m_Param.m_Mark.clear();
	if(m_LapLog && m_LapLog->m_iLapMode < LAPMODE_MAGNET && m_LapLog->m_iLapNum){
		FRAME_STATUS	fsp;
		
		// マークされているフレーム# を求める
		for(int i = 0; i < GetFrameMax(); ++i){
			filter->exfunc->get_frame_status(editp, i, &fsp);
			if(fsp.edit_flag & EDIT_FRAME_EDIT_FLAG_MARKFRAME){
				m_Param.m_Mark.push_back(i);
			}
		}
	}
	
	return m_Param.Save(
		szFileName,
		fileinfo->name,
		fileinfo->video_rate, fileinfo->video_scale
	);
}

/*** WndProc ****************************************************************/

BOOL func_WndProc(HWND hwnd,UINT message,WPARAM wparam,LPARAM lparam,void *editp,FILTER *filter){
	
	TCHAR	szBuf[MAX_PATH + 1];
	int		iFrame;
	
	//	TRUEを返すと全体が再描画される
	
	if(message == WM_FILTER_INIT) ExtendDialog(hwnd, 0);
	
	//	編集中でなければ何もしない
	if(filter->exfunc->is_editing(editp) != TRUE) return FALSE;
	
	switch(message) {
	  case WM_FILTER_FILE_OPEN:
		
		g_pVsd = new CVsdFilter(filter, editp);
		
		// trackbar 設定
		track_e[ID_TRK_video_start] =
		track_e[ID_TRK_video_end] = filter->exfunc->get_frame_n(editp);
		
		g_pVsd->m_Param.m_iLapChartStart = 0;
		g_pVsd->m_Param.m_iLapChartEnd = filter->exfunc->get_frame_n(editp);
		
		// リストボックスアイテム追加
		SetSkinFileList(GetDlgItem(hwnd, ID_COMBO_SEL_SKIN));
		
		// ファイルタイム取得
		g_pVsd->GetFileCreationTime();
		
		// 設定再描画
		filter->exfunc->filter_window_update(filter);
		
	  Case WM_FILTER_FILE_CLOSE:
		delete g_pVsd;
		g_pVsd = nullptr;
		
	  Case WM_FILTER_MAIN_KEY_DOWN:
		switch(wparam){
			
		  case 'M':
			// マーク
			FRAME_STATUS	fsp;
			iFrame = filter->exfunc->get_frame(editp);
			
			filter->exfunc->get_frame_status(editp, iFrame, &fsp);
			
			// 自動ラップ計測モードのとき，他のマークをすべて解除する
			if(
				g_pVsd->filter->check[ID_CHK_lap_time] &&
				g_pVsd->filter->track[ID_TRK_start_width] > 0 &&
				!(fsp.edit_flag & EDIT_FRAME_EDIT_FLAG_MARKFRAME)
			){
				for(int i = 0; i < g_pVsd->GetFrameMax(); ++i){
					filter->exfunc->get_frame_status(editp, i, &fsp);
					if(fsp.edit_flag & EDIT_FRAME_EDIT_FLAG_MARKFRAME){
						fsp.edit_flag &= ~EDIT_FRAME_EDIT_FLAG_MARKFRAME;
						filter->exfunc->set_frame_status(editp, i, &fsp);
					}
				}
				filter->exfunc->get_frame_status(editp, iFrame, &fsp);
			}
			
			fsp.edit_flag ^= EDIT_FRAME_EDIT_FLAG_MARKFRAME;
			filter->exfunc->set_frame_status(editp, iFrame, &fsp);
			
			g_pVsd->m_bCalcLapTimeReq = TRUE;
			return TRUE;
		}
		
	  Case WM_FILTER_UPDATE:
		iFrame = filter->exfunc->get_frame(editp);
		if(
			g_pVsd->m_iAdjustPointNum == 0 ||
			g_pVsd->m_iAdjustPointNum == 1 && g_pVsd->m_iAdjustPointVid[0] != iFrame ||
			g_pVsd->m_iAdjustPointNum == 2 && g_pVsd->m_iAdjustPointVid[0] != iFrame && g_pVsd->m_iAdjustPointVid[1] != iFrame
		){
			g_pVsd->filter->track[ID_TRK_vsd_log_offset] = 0;
			g_pVsd->filter->track[ID_TRK_gps_log_offset] = 0;
			filter->exfunc->filter_window_update(filter);
		}
		
	  Case WM_COMMAND:
		if(ID_BUTT_SET_VSt <= wparam && wparam <= ID_BUTT_SET_GEd){
			// フレーム数セット
			switch(wparam){
				case ID_BUTT_SET_VSt:	filter->track[ID_TRK_video_start] = filter->exfunc->get_frame(editp);
				Case ID_BUTT_SET_VEd:	filter->track[ID_TRK_video_end  ] = filter->exfunc->get_frame(editp);
				Case ID_BUTT_SET_LSt:	if(g_pVsd->m_VsdLog) filter->track[ID_TRK_log_start] = g_pVsd->m_VsdLog->m_iLogNum;
				Case ID_BUTT_SET_LEd:	if(g_pVsd->m_VsdLog) filter->track[ID_TRK_log_end  ] = g_pVsd->m_VsdLog->m_iLogNum;
				Case ID_BUTT_SET_GSt:	if(g_pVsd->m_GpsLog) filter->track[ID_TRK_gps_start] = g_pVsd->m_GpsLog->m_iLogNum;
				Case ID_BUTT_SET_GEd:	if(g_pVsd->m_GpsLog) filter->track[ID_TRK_gps_end  ] = g_pVsd->m_GpsLog->m_iLogNum;
			}
			// 設定再描画
			filter->exfunc->filter_window_update(filter);
		}else

		switch(wparam){
		  case ID_BUTT_LOAD_CFG:	// .avs ロード
			if(
				filter->exfunc->dlg_get_load_name(szBuf, (LPSTR)FILE_CFG_EXT, nullptr) &&
				g_pVsd->ConfigLoad(szBuf)
			){
				CPushDir push_dir(StrTokFile(szBuf, STF_PATH2).c_str());
				
				if(
					!g_pVsd->m_Param.m_strLapChart.empty() &&
					g_pVsd->LapChartRead(g_pVsd->m_Param.m_strLapChart.c_str())
				){
					filter->exfunc->filter_window_update(filter);
					SetWindowText(GetDlgItem(hwnd, ID_EDIT_LOAD_LAPCHART), g_pVsd->m_Param.m_strLapChart.c_str());
					g_pVsd->DeleteScript();
				}
				
				// ログリード
				if(!g_pVsd->m_Param.m_strLogFile.empty()){
					DebugCmd(
						UINT	uTimer = GetTickCount();
						DebugMsgD("VSD Log read start\n");
					);
					
					g_pVsd->ReadVsdLog(hwnd);
					
					DebugCmd(DebugMsgD("VSD Log read time = %d\n", GetTickCount() - uTimer);)
				}
				
				if(!g_pVsd->m_Param.m_strGPSLogFile.empty()){
					DebugCmd(
						UINT	uTimer = GetTickCount();
						DebugMsgD("GPS Log read start\n");
					);
					// 単発ファイル
					g_pVsd->ReadGPSLog(hwnd);
					
					DebugCmd(DebugMsgD("GPS Log read time = %d\n", GetTickCount() - uTimer);)
				}
				
				if(!g_pVsd->m_Param.m_strSkinFile.empty()){
					g_pVsd->SetSkinName(g_pVsd->m_Param.m_strSkinFile.c_str(), hwnd);
				}
				
				// 設定再描画
				filter->exfunc->filter_window_update(filter);

				// log pos 自動認識の更新
				func_update(filter, FILTER_UPDATE_STATUS_CHECK + ID_CHK_auto_sync);
			}

		  Case ID_BUTT_SAVE_CFG:
			if(filter->exfunc->dlg_get_save_name(szBuf, (LPSTR)FILE_CFG_EXT, nullptr))
				return g_pVsd->ConfigSave(szBuf);
			
		  Case ID_BUTT_LOAD_LOG:	// .log ロード
			if(g_pVsd->FileOpenDialog(g_pVsd->m_Param.m_strLogFile, g_pVsd->m_Param.m_strLogFileReader)){
				if(g_pVsd->ReadVsdLog(hwnd)){
					// 設定再描画
					filter->exfunc->filter_window_update(filter);

					// log pos 自動認識の更新
					func_update(filter, FILTER_UPDATE_STATUS_CHECK + ID_CHK_auto_sync);
				}
				g_pVsd->DeleteScript();
			}
			
		  Case ID_BUTT_LOAD_GPS:	// GPS ログロード
			if(g_pVsd->FileOpenDialog(g_pVsd->m_Param.m_strGPSLogFile, g_pVsd->m_Param.m_strGPSLogFileReader)){
				if(g_pVsd->ReadGPSLog(hwnd)){
					// 設定再描画
					filter->exfunc->filter_window_update(filter);
				}
				g_pVsd->DeleteScript();
			}
			
		  Case ID_BUTT_LOAD_LAPCHART:	// ラップチャート ロード
			if(
				filter->exfunc->dlg_get_load_name(szBuf, (LPSTR)FILE_LAPCHART_EXT, nullptr) &&
				g_pVsd->LapChartRead(szBuf)
			){
				g_pVsd->m_Param.m_strLapChart = szBuf;
				filter->exfunc->filter_window_update(filter);
				SetWindowText(GetDlgItem(hwnd, ID_EDIT_LOAD_LAPCHART), szBuf);
				g_pVsd->m_bCalcLapTimeReq = TRUE;
				g_pVsd->DeleteScript();
			}
			
		  Case ID_BUTT_LAP_START:
			g_pVsd->m_Param.m_iLapChartStart = filter->exfunc->get_frame(editp);
			g_pVsd->m_bCalcLapTimeReq = TRUE;
			return TRUE;
			
		  Case ID_BUTT_LAP_END:
			g_pVsd->m_Param.m_iLapChartEnd = filter->exfunc->get_frame(editp);
			g_pVsd->m_bCalcLapTimeReq = TRUE;
			return TRUE;
			
		  Case (CBN_SELCHANGE << 16) | ID_COMBO_SEL_SKIN:	// スキン選択
			{
				HWND hWndCombo = GetDlgItem(hwnd, ID_COMBO_SEL_SKIN);
				int i = (int)SendMessage(hWndCombo, CB_GETCURSEL, 0, 0);
				
				// ファイルを開くが選択された?
				if(SendMessage(hWndCombo, CB_GETCOUNT, 0, 0) - 1 == i){
					
					// ダイアログがキャンセルされた
					if(!filter->exfunc->dlg_get_load_name(szBuf, (LPSTR)FILE_SKIN_EXT, nullptr)){
						// とりあえず，直前の skin 名をそのままコピーしとく
						//strcpy(szBuf, g_pVsd->m_Param.m_strSkinFile);
						//SetWindowText(hWndCombo, g_pVsd->m_Param.m_strSkinFile);
						// 何しても無駄だった...
						return TRUE;
					}
					
					SendMessage(hWndCombo, CB_INSERTSTRING, 0, (LPARAM)szBuf);
					SendMessage(hWndCombo, CB_SETCURSEL, 0, 0);
				}else{
					// スキンがドロップダウンリストから選択された
					SendMessage(hWndCombo, CB_GETLBTEXT, i, (LPARAM)szBuf);
				}
				
				g_pVsd->SetSkinName(szBuf, hwnd);
				filter->exfunc->filter_window_update(filter);	// 設定再描画
			}
		  Default:
			return FALSE;
		}
		return TRUE;
	}
	
	return FALSE;
}

/*** 1スライダバー調整 ******************************************************/

void SetupLogOffset(FILTER *filter){
	
	int	iFrame = filter->exfunc->get_frame(g_pVsd->editp);
	int	iPoint;
	
	// 調整ポイントを判断する
	switch(g_pVsd->m_iAdjustPointNum){
	  case 0:
		iPoint = 0;
		g_pVsd->m_iAdjustPointNum = 1;
		
	  Case 1:
		if(abs(g_pVsd->m_iAdjustPointVid[0] - iFrame) <= (int)(g_pVsd->GetFPS() * 60)){
			// フレーム位置が前回と 1分以内なら，前回と同じ場所を使用する
			iPoint = 0;
		}else{
			iPoint = 1;
			g_pVsd->m_iAdjustPointNum = 2;
		}
		
	  Default: //Case 2:
		// 2ポイントのうち，近いほうを採用する
		iPoint = (
			abs(g_pVsd->m_iAdjustPointVid[0] - iFrame) >
			abs(g_pVsd->m_iAdjustPointVid[1] - iFrame)
		) ? 1 : 0;
	}
	
	// フレーム位置が前回と違っていれば，GPS ログ位置再取得
	// m_iAdjustPointNum = 0 時にはここを通ることはない
	if(g_pVsd->m_iAdjustPointVid[iPoint] != iFrame){
		g_pVsd->m_iAdjustPointVsd[iPoint] =
			g_pVsd->m_Param.m_iVideoEnd == g_pVsd->m_Param.m_iVideoStart ? 0
			: (int)(
				(double)(g_pVsd->m_Param.m_iVsdEnd - g_pVsd->m_Param.m_iVsdStart)
				* (iFrame - g_pVsd->m_Param.m_iVideoStart) 
				/ (g_pVsd->m_Param.m_iVideoEnd - g_pVsd->m_Param.m_iVideoStart)
				+ g_pVsd->m_Param.m_iVsdStart
			);
		filter->track[ID_TRK_vsd_log_offset] = 0;
		
		g_pVsd->m_iAdjustPointGPS[iPoint] =
			g_pVsd->m_Param.m_iVideoEnd == g_pVsd->m_Param.m_iVideoStart ? 0
			: (int)(
				(double)(g_pVsd->m_Param.m_iGpsEnd - g_pVsd->m_Param.m_iGpsStart)
				* (iFrame - g_pVsd->m_Param.m_iVideoStart) 
				/ (g_pVsd->m_Param.m_iVideoEnd - g_pVsd->m_Param.m_iVideoStart)
				+ g_pVsd->m_Param.m_iGpsStart
			);
		filter->track[ID_TRK_gps_log_offset] = 0;
		
		g_pVsd->m_iAdjustPointVid[iPoint] =
		(iPoint ? g_pVsd->m_Param.m_iVideoEnd : g_pVsd->m_Param.m_iVideoStart) = iFrame;
	}
	
	// 対象調整ポイントの設定
	(iPoint ? g_pVsd->m_Param.m_iVsdEnd : g_pVsd->m_Param.m_iVsdStart) =
		g_pVsd->m_iAdjustPointVsd[iPoint] - filter->track[ID_TRK_vsd_log_offset];
	(iPoint ? g_pVsd->m_Param.m_iGpsEnd : g_pVsd->m_Param.m_iGpsStart) =
		g_pVsd->m_iAdjustPointGPS[iPoint] - filter->track[ID_TRK_gps_log_offset];
	
	// iPoint == 1 時は，後ろの調整点を FPS に応じて自動調整
	if(g_pVsd->m_iAdjustPointNum == 1){
		g_pVsd->m_Param.m_iVideoEnd = g_pVsd->m_Param.m_iVideoStart + (int)(g_pVsd->GetFPS() * (OFFSET_ADJUST_WIDTH / 1000.0));
		g_pVsd->m_Param.m_iVsdEnd   = g_pVsd->m_Param.m_iVsdStart + (int)(SLIDER_TIME * OFFSET_ADJUST_WIDTH);
		g_pVsd->m_Param.m_iGpsEnd   = g_pVsd->m_Param.m_iGpsStart + (int)(SLIDER_TIME * OFFSET_ADJUST_WIDTH);
		
		// スライダ設定値がスライダバーの設定範囲を超えている場合，
		// スライダバー設定範囲を再設定する
		if(g_pVsd->m_Param.m_iVideoEnd > filter->track_e[ID_TRK_video_start]){
			filter->track_e[ID_TRK_video_start] =
			filter->track_e[ID_TRK_video_end] = g_pVsd->m_Param.m_iVideoEnd;
		}
		if(g_pVsd->m_Param.m_iVsdEnd > filter->track_e[ID_TRK_gps_start]){
			filter->track_e[ID_TRK_gps_start] =
			filter->track_e[ID_TRK_gps_end] = g_pVsd->m_Param.m_iVsdEnd;
		}
		if(g_pVsd->m_Param.m_iGpsEnd > filter->track_e[ID_TRK_gps_start]){
			filter->track_e[ID_TRK_gps_start] =
			filter->track_e[ID_TRK_gps_end] = g_pVsd->m_Param.m_iGpsEnd;
		}
	}
	
	filter->exfunc->filter_window_update(filter);
}

/*** スライダバー連動 *******************************************************/

BOOL func_update(FILTER *filter, int status){
	static	BOOL	bReEnter = FALSE;
	
	if(!g_pVsd || bReEnter) return TRUE;
	
	bReEnter = TRUE;
	
	// AviUtl のパラメータを m_Param にコピー
	#define DEF_PARAM_TRK(name, type, var, ...)	g_pVsd->m_Param.var = filter->track[ID_TRK_##name];
	#define DEF_PARAM_CHK(name, type, var, ...)	g_pVsd->m_Param.var = filter->check[ID_CHK_##name];
	#include "def_param.h"
	
	if(
		status >= FILTER_UPDATE_STATUS_TRACK + ID_TRK_vsd_log_offset &&
		status <= FILTER_UPDATE_STATUS_TRACK + ID_TRK_gps_end ||
		status == FILTER_UPDATE_STATUS_TRACK + ID_TRK_start_width
	) g_pVsd->m_bCalcLapTimeReq = TRUE;
	
	// ログ位置自動認識モードの設定変更
	if(status == (FILTER_UPDATE_STATUS_CHECK + ID_CHK_auto_sync)){

		if(filter->check[ID_CHK_auto_sync] && g_pVsd->m_VsdLog){
			if(g_pVsd->m_VsdLog->m_iCalibStart >= 0){
				filter->track[ID_TRK_log_start] = (int)(g_pVsd->m_VsdLog->m_iCalibStart * SLIDER_TIME);
			}

			if(g_pVsd->m_VsdLog->m_iCalibStop >= 0){
				filter->track[ID_TRK_log_end] = (int)(g_pVsd->m_VsdLog->m_iCalibStop  * SLIDER_TIME);
			}

			// 設定再描画
			filter->exfunc->filter_window_update(filter);
		}
	}
	
	// マップ回転
	else if(status == (FILTER_UPDATE_STATUS_TRACK + ID_TRK_map_angle)){
		if(g_pVsd->m_VsdLog)
			g_pVsd->m_VsdLog->RotateMap(filter->track[ID_TRK_map_angle] * (-ToRAD / 10));
		if(g_pVsd->m_GpsLog)
			g_pVsd->m_GpsLog->RotateMap(filter->track[ID_TRK_map_angle] * (-ToRAD / 10));
	}
	
	else if(
		status == FILTER_UPDATE_STATUS_TRACK + ID_TRK_vsd_log_offset ||
		status == FILTER_UPDATE_STATUS_TRACK + ID_TRK_gps_log_offset
	){
		SetupLogOffset(filter);
	}
	
	bReEnter = FALSE;
	
	return TRUE;
}
