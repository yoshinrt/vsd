/*****************************************************************************
	
	VSD -- vehicle data logger system  Copyright(C) by DDS
	
	CVsdFilter.cpp - CVsdFilter class implementation
	
*****************************************************************************/

#include "StdAfx.h"

#include "CVsdFilter.h"
#include "CVsdFile.h"
#include "rev_num.h"

/*** macros *****************************************************************/

#define SLineWidth		(m_piParamT[TRACK_SLineWidth] / 10.0)

/*** static member **********************************************************/

HINSTANCE	CVsdFilter::m_hInst 	= nullptr;
const int CVsdFilter::m_iRevision	= PROG_REVISION;

const int CVsdFilter::m_iPrivateBuild = 1;

/*** コンストラクタ *********************************************************/

void CVsdFilter::Constructor(void){
	DebugMsgD("this = %08X\n", this);

	m_bCalcLapTimeReq	= TRUE;
	
	m_bConsoleOn		= FALSE;
	
	// str param に初期値設定
	#define DEF_STR_PARAM(id, var, init, conf_name) var = (init == nullptr ? "" : init);
	#include "def_str_param.h"
	
	// plugin dll path 取得
	SetPluginDir();
	
	// SkinDir セット
	SetSkinFile(m_szSkinFile);
	
	m_Script	= nullptr;
	
	m_iWidth	=
	m_iHeight	= 0;
	m_bSaving	= FALSE;
	
	//---- GDI+の初期設定
	Gdiplus::GdiplusStartup(&m_gdiplusToken, &m_gdiplusStartupInput, nullptr);
	
	// WinInet 同時接続数
	DWORD		uMaxConnect = 30;
	InternetSetOption(nullptr, INTERNET_OPTION_MAX_CONNS_PER_SERVER, &uMaxConnect, sizeof(DWORD));
	InternetSetOption(nullptr, INTERNET_OPTION_MAX_CONNS_PER_1_0_SERVER, &uMaxConnect, sizeof(DWORD));
}

/*** デストラクタ ***********************************************************/

void CVsdFilter::Destructor(void){
	//---- GDI+の解放
	Gdiplus::GdiplusShutdown(m_gdiplusToken);
}

/*** ログリード ************************************************************/

int CVsdFilter::ReadLog(std::unique_ptr<CVsdLog> &pLog, const char *szFileName, const char *szReaderFunc){
	pLog = std::make_unique<CVsdLog>(this);
	
	std::unique_ptr<CLapLog> pLapLog;
	
	int iRet = pLog->ReadLog(szFileName, szReaderFunc, pLapLog);
	if(iRet){
		if(pLog->m_pLogLongitude)
			pLog->RotateMap(m_piParamT[TRACK_MapAngle] * (-ToRAD / 10));
	}else{
		pLog.reset();
	}
	
	if(pLapLog){
		pLapLog->m_iLapSrc = pLog.get() == m_VsdLog.get() ? LAPSRC_VSD : LAPSRC_GPS;
		
		if(m_LapLog){
			// 重複しているので，優先度の高いモードを残すか，VSD の方を優先
			if(
				m_LapLog->m_iLapMode <  LAPMODE_MAGNET ||
				m_LapLog->m_iLapMode == LAPMODE_MAGNET && pLog.get() == m_VsdLog.get()
			){
				m_LapLog = std::move(pLapLog);
			}
		}else{
			m_LapLog = std::move(pLapLog);
		}
	}
	
	return iRet;
}

/***************************************************************************/

BOOL CVsdFilter::ParseMarkStr(const char *szMark){
	
	do{
		while(*szMark && !isdigit(*szMark)) ++szMark;	// 数字までスキップ
		if(!*szMark) break;
		
		SetFrameMark(atoi(szMark));
		if(szMark = strchr(szMark, ',')) ++szMark;	// 次のパラメータ
	}while(szMark);
	
	m_bCalcLapTimeReq = TRUE;
	return TRUE;
}


/*** ラップ番号 → ログ番号 変換 ********************************************/

double CVsdFilter::LapNum2LogNum(CVsdLog* Log, int iLapNum){
	
	// iLapNum がおかしいときは 0 を返しとく
	if(iLapNum < 0) return 2;	// 番犬を除いた Time=0 のログ
	
	if(m_LapLog->m_iLapSrc == LAPSRC_VSD){
		// fLogNum は VSD ログ番号
		if(Log == m_VsdLog.get()) return m_LapLog->m_Lap[iLapNum].fLogNum;
		
		// ラップ番号(VSD)→GPS ログ番号に変換
		return Log->GetIndex(
			m_VsdLog->GetTime(m_LapLog->m_Lap[iLapNum].fLogNum) * SLIDER_TIME,
			&VsdSt, &GPSSt
		);
	}else if(m_LapLog->m_iLapSrc == LAPSRC_GPS){
		// fLogNum は GPS ログ番号
		if(Log == m_GPSLog.get()) return m_LapLog->m_Lap[iLapNum].fLogNum;
		
		// ラップ番号(GPS)→VSD ログ番号に変換
		return Log->GetIndex(
			Log->GetTime(m_LapLog->m_Lap[iLapNum].fLogNum) * SLIDER_TIME,
			&GPSSt, &VsdSt
		);
	}
	
	// fLogNum はビデオフレーム番号
	return Log->GetIndex(
		m_LapLog->m_Lap[iLapNum].fLogNum,
		&VideoSt, Log == m_VsdLog.get() ? &VsdSt : &GPSSt
	);
}

/*** ラップタイム再生成 (手動) **********************************************/

std::unique_ptr<CLapLog> CVsdFilter::CreateLapTimeHand(int iLapSrc){
	
	int		iTime, iPrevTime = 0;
	int		iFrame = 0;
	double	dLogNum;
	LAP_t	LapTime;
	
	auto pLapLog = std::make_unique<CLapLog>();
	pLapLog->m_iLapMode = LAPMODE_HAND;
	pLapLog->m_iLapSrc  = iLapSrc;
	
	while((iFrame = GetFrameMark(iFrame)) >= 0){
		
		if(pLapLog->m_iLapSrc == LAPSRC_GPS){
			dLogNum	= GetLogIndex(iFrame, GPS, -1);
			iTime	= m_GPSLog->GetTime(dLogNum);
			LapTime.fLogNum	= (float)dLogNum;
		}else{
			// LAPSRC_VIDEO
			iTime	= (int)(iFrame * 1000.0 / GetFPS());
			LapTime.fLogNum	= (float)iFrame;
		}
		
		if(m_piParamT[TRACK_SLineWidth] < 0){
			// ジムカーナモード
			LapTime.uLap	= (pLapLog->m_iLapNum / 2) + 1;
			LapTime.iTime	= (pLapLog->m_iLapNum & 1) ? iTime - iPrevTime : 0;
		}else{
			LapTime.uLap	= pLapLog->m_iLapNum;
			LapTime.iTime	= pLapLog->m_iLapNum ? iTime - iPrevTime : 0;
		}
		
		iPrevTime = iTime;
		++iFrame;
		
		pLapLog->PushLap(LapTime);
	}
	
	if(pLapLog->m_Lap.size() == 0){
		return nullptr;
	}
	LapTime.fLogNum	= FLT_MAX;	// 番犬
	LapTime.iTime	= 0;		// 番犬
	pLapLog->m_Lap.push_back(LapTime);
	
	return pLapLog;
}

/*** ラップタイム再生成 (GPS auto) ******************************************/

std::unique_ptr<CLapLog> CVsdFilter::CreateLapTimeAuto(void){
	
	int iFrame;
	LAP_t	LapTime;
	
	if((iFrame = GetFrameMark(0)) < 0) return nullptr;
	
	/*** スタートラインの位置を取得 ***/
	// iFrame に対応する GPS ログ番号取得
	double dLogNum = GetLogIndex(iFrame, GPS, -1);
	
	int iLogNum = (int)dLogNum;
	
	// iLogNum ～ iLogNum + 1 の方位を算出
	
	double dAngle = atan2(
		(m_GPSLog->Y0(iLogNum + 1) - m_GPSLog->Y0(iLogNum)),
		(m_GPSLog->X0(iLogNum + 1) - m_GPSLog->X0(iLogNum))
	);
	
	#define x1 m_dStartLineX1
	#define y1 m_dStartLineY1
	#define x2 m_dStartLineX2
	#define y2 m_dStartLineY2
	#define x3 m_GPSLog->X0(i)
	#define y3 m_GPSLog->Y0(i)
	#define x4 m_GPSLog->X0(i + 1)
	#define y4 m_GPSLog->Y0(i + 1)
	
	// 仮想光電管の位置を求める
	x2 = m_GPSLog->X0(dLogNum);	// スタート地点
	y2 = m_GPSLog->Y0(dLogNum);
	
	x1 = x2 + cos(dAngle + 90 * ToRAD) * SLineWidth / 2;
	y1 = y2 + sin(dAngle + 90 * ToRAD) * SLineWidth / 2;
	x2 = x2 + cos(dAngle - 90 * ToRAD) * SLineWidth / 2;
	y2 = y2 + sin(dAngle - 90 * ToRAD) * SLineWidth / 2;
	
	/*****/
	
	auto pLapLog = std::make_unique<CLapLog>();
	pLapLog->m_iLapMode = LAPMODE_GPS;
	pLapLog->m_iLapSrc  = LAPSRC_GPS;
	
	int iTime, iPrevTime = 0;
	int	iLapNum = 0;
	
	for(int i = 1; i < m_GPSLog->GetCnt() - 1; ++i){
		
		/*** 交差判定，交点判定 ***/
		double s1, s2, s3, s4;
		
		// 交点が iLogNum ～ +1 線分上かの判定
		s1 = (x1 - x2) * (y3 - y1) + (y1 - y2) * (x1 - x3);
		s2 = (x1 - x2) * (y4 - y1) + (y1 - y2) * (x1 - x4);
		if(s2 == 0 || s1 * s2 > 0) continue;
		
		// 交点がスタートライン線分上かの判定
		s3 = (x3 - x4) * (y1 - y3) + (y3 - y4) * (x3 - x1);
		s4 = (x3 - x4) * (y2 - y3) + (y3 - y4) * (x3 - x2);
		if(s3 * s4 > 0) continue;
		
		// 進行方向の判定，dAngle ±45度
		double dAngle2 = dAngle - atan2(
			(m_GPSLog->Y0(i + 1) - m_GPSLog->Y0(i)),
			(m_GPSLog->X0(i + 1) - m_GPSLog->X0(i))
		);
		if	 (dAngle2 < -180 * ToRAD) dAngle2 += 360 * ToRAD;
		else if(dAngle2 >  180 * ToRAD) dAngle2 -= 360 * ToRAD;
		if(dAngle2 < -45 * ToRAD || dAngle2 > 45 * ToRAD) continue;
		
		#undef x1
		#undef y1
		#undef x2
		#undef y2
		#undef x3
		#undef y3
		#undef x4
		#undef y4
		
		// 半端な LogNum
		dLogNum = i + s1 / (s1 - s2);
		iTime = m_GPSLog->GetTime(dLogNum);
		
		if(m_piParamS[SHADOW_LAP_START] - 1 <= iLapNum && iLapNum <= m_piParamS[SHADOW_LAP_END]){
			LapTime.uLap	= pLapLog->m_iLapNum;
			LapTime.fLogNum	= (float)dLogNum;
			LapTime.iTime	= pLapLog->m_iLapNum ? iTime - iPrevTime : 0;
			
			iPrevTime = iTime;
			
			pLapLog->PushLap(LapTime);
		}
		++iLapNum;
	}
	
	if(pLapLog->m_Lap.size() == 0){
		return nullptr;
	}
	
	LapTime.fLogNum	= FLT_MAX;	// 番犬
	LapTime.iTime	= 0;		// 番犬
	pLapLog->m_Lap.push_back(LapTime);
	
	return pLapLog;
}

/*** チャートリード *********************************************************/

constexpr auto NAME_BUF_SIZE = 64;

int CVsdFilter::LapChartRead(const char *szFileName){
	
	// 既に優先度の高い Log があればリターン (今はない)
	if(m_LapLog && m_LapLog->m_iLapMode > LAPMODE_CHART){
		return 0;
	}
	
	auto pLapLog = std::make_unique<CLapLogAll>(this);
	int iRet = pLapLog->LapChartRead(szFileName);
	if(iRet == 0){
		return 0;
	}
	
	// ここでやっと m_LapLog を上書き
	m_LapLog = std::move(pLapLog);
	return iRet;
}

/*** コンソールウィンドウに表示 *********************************************/

BOOL WINAPI HandlerRoutine(DWORD dwCtrlType){
	if(dwCtrlType == CTRL_C_EVENT){
		FreeConsole();
		return TRUE;
	}
	
	return FALSE;
}

void CVsdFilter::OpenConsole(void){
	if(AllocConsole()){
		freopen ("CONOUT$", "w", stdout);
		freopen ("CONIN$", "r", stdin);
		
		SetConsoleCtrlHandler(HandlerRoutine, TRUE);
		// ウィンドウタイトル変更
		SetConsoleTitle("VSD for GPS コンソール");
		
		// 閉じるボタン無効化
		HMENU hmenu = GetSystemMenu(GetConsoleWindow(), FALSE);
		RemoveMenu(hmenu, SC_CLOSE, MF_BYCOMMAND);
		
		Print("このウィンドウを閉じるには Ctrl+C を押してください\n");
	}
}

void CVsdFilter::Print(LPCWSTR szMsg){
	OpenConsole();
	fputws(szMsg, stdout);
}

void CVsdFilter::Print(const char *szFormat, ...){
	OpenConsole();
	
	va_list	arg;
	va_start(arg, szFormat);
	vprintf(szFormat, arg);
	va_end(arg);
}

/****************************************************************************/

BOOL WINAPI DllMain(
	HINSTANCE	hinstDLL,	// handle to DLL module
	DWORD		fdwReason,	// reason for calling function
	LPVOID		lpvReserved	// reserved
){
	if(fdwReason == DLL_PROCESS_ATTACH){
		CVsdFilter::m_hInst = hinstDLL;
	}
	return TRUE;
}
