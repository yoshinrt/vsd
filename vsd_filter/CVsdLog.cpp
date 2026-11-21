/*****************************************************************************
	
	VSD -- vehicle data logger system  Copyright(C) by DDS
	
	CVsdLog.cpp - CVsdLog class implementation
	
*****************************************************************************/

#include "StdAfx.h"
#include "CVsdLog.h"
#include "CVsdFilter.h"

/*** macros *****************************************************************/

#define GRAVITY				9.80665
#define CALIB_MARK_SPEED	600
#define MAX_TURN_R			200

/*** コンストラクタ *********************************************************/

CVsdLog::CVsdLog(){
	m_dFreq	= LOG_FREQ;
	
	m_iLogStartTime	= -1;
	
	m_dLong2Meter = 0;
	m_dLati2Meter = 0;
	m_iCnt		= 0;
	
	m_iCalibStart	=
	m_iCalibStop	= -1;
	
	#define DEF_LOG(name)	m_pLog##name = nullptr;
	#include "def_log.h"
}

CVsdLog::~CVsdLog(){
	m_Logs.clear();
}

/*** frame# に対応するログ index を得る *************************************/

double CVsdLog::GetIndex(double dFromVal, int *piFrom, int *piLog, int iPrevIdx){
	if(piFrom[0] == piFrom[1]) return 2;	// 番犬を除いた Time=0 のログ
	return GetIndex(
		(int)((
			piLog[0] +
			(piLog[1] - piLog[0]) * (dFromVal - piFrom[0]) /
			(double)(piFrom[1] - piFrom[0])
		) / SLIDER_TIME),
		iPrevIdx
	);
}

double CVsdLog::GetIndex(int iTime, int iPrevIdx){
	int nCnt = GetCnt();
	if(nCnt <= 0) return 0.0;
	if(nCnt == 1) return 0.0;
	if(iTime <= GetTime(0)) return 0.0;
	if(iTime >= GetTime(nCnt - 1)) return (double)(nCnt - 1);
	
	int idx = 0;
	
	// Time(idx) <= iTime < Time(idx + 1)
	// となる idx を見つける
	if(
		iPrevIdx < 0 || iPrevIdx >= nCnt - 1 ||
		GetTime(iPrevIdx) > iTime
	){
		// iPrevIdx がおかしいので，binary serch する
		int iSt = 0;
		int iEd = nCnt - 2;
		while(iSt <= iEd){
			idx = iSt + (iEd - iSt) / 2;
			if(GetTime(idx) > iTime){
				iEd = idx - 1;
			}else if(GetTime(idx + 1) <= iTime){
				iSt = idx + 1;
			}else{
				// ヒット
				break;
			}
		}
		if(idx < 0) idx = 0;
		if(idx > nCnt - 2) idx = nCnt - 2;
	}else{
		// iPrevIdx は正常なので，これを起点に単純サーチする
		idx = iPrevIdx;
		while(idx < nCnt - 2 && GetTime(idx + 1) <= iTime) ++idx;
	}
	
	// index の端数を求める
	int iDt = GetTime(idx + 1) - GetTime(idx);
	if(iDt == 0) return (double)idx;
	return idx + (double)(iTime - GetTime(idx)) / iDt;
}

/*** GPS ログのダンプ *******************************************************/

#ifdef DEBUG
void CVsdLog::Dump(const char *szFileName){
	FILE *fp = fopen(szFileName, "w");
	
	BOOL bFirst = TRUE;
	
	// ヘッダ出力
	for(auto it = m_Logs.begin(); it != m_Logs.end(); ++it){
		if(!bFirst) fputs("\t", fp); bFirst = FALSE;
		fputs(it->first.c_str(), fp);
	}
	fputs("\n", fp);
	
	for(int i = 0; i < GetCnt(); ++i){
		BOOL bFirst = TRUE;
		for(auto it = m_Logs.begin(); it != m_Logs.end(); ++it){
			if(!bFirst) fputs("\t", fp); bFirst = FALSE;
			fprintf(fp, "%f", it->second->GetRaw(i));
		}
		fputs("\n", fp);
	}
	fclose(fp);
}
#endif

/*** key の存在確認 *********************************************************/

CLog *CVsdLog::GetElement(const char *szKey, BOOL bCreate){
	std::string strKey(szKey);
	auto itr = m_Logs.find(strKey);
	
	if(itr != m_Logs.end()){
		return itr->second.get();
	}
	if(bCreate){
		std::unique_ptr<CLog> upLog;
		
		// Speed とか基本データの参照用ポインタに代入
		if(0);
		#define DEF_LOG(name)	DEF_LOG_T(name, CLogFloat)
		#define DEF_LOG_T(name, type)	\
			else if(strKey == #name) { auto *pTyped = new type(); m_pLog##name = pTyped; upLog.reset(pTyped); }
		#include "def_log.h"
		else{
			upLog.reset(new CLogFloat());
		}
		
		CLog *pRaw = upLog.get();
		m_Logs[strKey] = std::move(upLog);
		return pRaw;
	}
	return nullptr;
}

/*** ログの Set *************************************************************/

void CVsdLog::Set(const char *szKey, int iIndex, double dVal){
	GetElement(szKey, TRUE)->Set(iIndex, dVal);
	if(m_iCnt <= iIndex) m_iCnt = iIndex + 1;
}

/*** 1レコードコピー ********************************************************/

void CVsdLog::CopyRecord(int iTo, int iFrom){
	for(auto it = m_Logs.begin(); it != m_Logs.end(); ++it){
		CLog *pLog = it->second.get();
		pLog->Set(iTo, pLog->GetRaw(iFrom));
	}
	if(m_iCnt <= iTo) m_iCnt = iTo + 1;
}

/*** 番犬追加 ***************************************************************/

void CVsdLog::AddWatchDog(void){
	int iCnt = GetCnt() - 1;
	CopyRecord(0, 2);           AddStopRecord(0, -WATCHDOG_TIME);
	CopyRecord(1, 2);           AddStopRecord(1, GetTime(2) - 500);
	CopyRecord(iCnt + 1, iCnt); AddStopRecord(iCnt + 1, GetTime(iCnt) + 500);
	CopyRecord(iCnt + 2, iCnt); AddStopRecord(iCnt + 2, WATCHDOG_TIME);
}

/*** GPS ログの up-convert **************************************************/

UINT CVsdLog::GPSLogRescan(void){
	
	/*** CLog の方を走査して，いろいろ補正 *****************************/
	
	BOOL	bCreateSpeed	= FALSE;
	BOOL	bCreateG		= FALSE;

	if(m_pLogLongitude != nullptr && m_pLogLatitude != nullptr){
		if(!m_pLogSpeed){
			bCreateSpeed = TRUE;
			GetElement("Speed", TRUE);
			m_pLogSpeed->Resize(GetCnt(), 0);
		}

		if(!m_pLogGx){
			bCreateG = TRUE;
			GetElement("Gx", TRUE);
			m_pLogGx->Resize(GetCnt(), 0);
			GetElement("Gy", TRUE);
			m_pLogGy->Resize(GetCnt(), 0);
		}

	}
	
	#pragma omp parallel
	{
		double dMaxSpeed;
		double dMaxGx, dMinGx;
		double dMaxGy, dMinGy;
		
		dMaxSpeed = -FLT_MAX;
		
		// スピード生成
		if(bCreateSpeed){
			#pragma omp for
			for(int i = 1; i < GetCnt(); ++i){
				int iDiffTime = GetTime(i) - GetTime(i - 1);
				if(
					iDiffTime > (TIME_STOP - TIME_STOP_MARGIN * 2) &&
					m_pLogDistance &&
					(Distance(i) - Distance(i - 1)) * 3600 / iDiffTime < KPH_STOP
				){
					// 時間が開いていてなおかつ移動していない
					SetRawSpeed(i, 0);
					SetRawSpeed(i - 1, 0);
					if(dMaxSpeed < 0) dMaxSpeed = 0;
				}else{
					double d;
					SetRawSpeed(i,
						d = (Distance(i) - Distance(i - 1))
						* (3600.0 / 1000 * 1000) /
						(GetTime(i) - GetTime(i - 1))
					);
					// 100km/h 以上は，+20% 以上の値は無視する
					if(dMaxSpeed < d && (d < 100 || dMaxSpeed * 1.2 > d)) dMaxSpeed = d;
				}
			}
			// 番犬はすでに 0 → SetRawSpeed(GetCnt() - 1, 0);
			
			#pragma omp critical
			{
				//DebugMsgD("val=%f max=%f\n", dMaxSpeed, m_pLogSpeed->GetMax());
				SetMaxMinSpeed(dMaxSpeed, 0);
			}
		}
		
		// G 計算
		if(bCreateG){
			#pragma omp for
			for(int i = 2; i < GetCnt() - 1; ++i){
				// 横 G 計算
				// Gx / Gy を作る
				SetRawGy(i,
					(Speed(i) - Speed(i - 1))
					* (1000 / 3.600 / GRAVITY)
					/ (GetTime(i) - GetTime(i - 1))
				);
				
				// 横G = vω
				double dBearingDelta = (Direction(i) - Direction(i - 1)) * ToRAD;
				if     (dBearingDelta >  M_PI) dBearingDelta -= M_PI * 2;
				else if(dBearingDelta < -M_PI) dBearingDelta += M_PI * 2;
				
				SetRawGx(i,
					dBearingDelta * (1000 / GRAVITY)
					/ (GetTime(i) - GetTime(i - 1))
					* (Speed(i) / 3.600)
				);
				
				// ±5G 以上は，削除
				if(Gx(i) < -3 || Gx(i) > 3){
					SetRawGx(i, Gx(i - 1));
				}
			}
			
			// スムージング
			constexpr int G_SMOOTH_NUM = 3;	// 平均化する要素数
			const UINT G_SMOOTH_CNT = (m_dFreq > 5 ? 2 : 1);	// スムージング回数
			constexpr double G_SMOOTH_RATIO = 0.3;	// min/max 計算用係数
			(void)G_SMOOTH_RATIO;
			
			#pragma omp single
			{
				// 番犬と，最初 / 最後の G 初期化
				SetRawGx(0, 0); SetRawGy(0, 0);
				SetRawGx(1, 0); SetRawGy(1, 0);
				SetRawGx(2, 0); SetRawGy(2, 0);
				SetRawGx(GetCnt() - 3, 0); SetRawGy(GetCnt() - 3, 0);
				SetRawGx(GetCnt() - 2, 0); SetRawGy(GetCnt() - 2, 0);
				SetRawGx(GetCnt() - 1, 0); SetRawGy(GetCnt() - 1, 0);
			}
			
			int iStart = (G_SMOOTH_NUM - 1) / 2;
			if(iStart < 2) iStart = 2;
			int iEnd = GetCnt() - G_SMOOTH_NUM / 2;
			if(iEnd >= GetCnt() - 2) iEnd = GetCnt() - 2;
			
			double dSumGx = NaN;
			double dSumGy;
			float fBufGx[G_SMOOTH_NUM];
			float fBufGy[G_SMOOTH_NUM];
			
			for(UINT v = G_SMOOTH_CNT; v; --v){
				dMaxGx = dMaxGy = 0;
				dMinGx = dMinGy = 0;
				
				#pragma omp for
				for(int i = iStart; i < iEnd; ++i){
					
					if(_isnan(dSumGx)){
						dSumGx = 0;
						dSumGy = 0;
						
						// バッファ初期化
						for(int j = i - (G_SMOOTH_NUM - 1) / 2; j < i + G_SMOOTH_NUM / 2; ++j){
							fBufGx[j % G_SMOOTH_NUM] = (float)Gx(j); dSumGx += Gx(j);
							fBufGy[j % G_SMOOTH_NUM] = (float)Gy(j); dSumGy += Gy(j);
						}
					}
					
					// スムージング対象データの最後尾をロード
					int k = i + G_SMOOTH_NUM / 2;
					fBufGx[k % G_SMOOTH_NUM] = (float)Gx(k); dSumGx += Gx(k);
					fBufGy[k % G_SMOOTH_NUM] = (float)Gy(k); dSumGy += Gy(k);
					
					if(Speed(i) == 0){
						SetRawGx(i, 0);
						SetRawGy(i, 0);
					}else{
						double dx, dy;
						SetRawGx(i, dx = dSumGx / G_SMOOTH_NUM);
						SetRawGy(i, dy = dSumGy / G_SMOOTH_NUM);
						
						if(dMaxGx < dx) dMaxGx = dx;
						else if(dMinGx > dx) dMinGx = dx;
						if(dMaxGy < dy) dMaxGy = dy;
						else if(dMinGy > dy) dMinGy = dy;
					}
					
					// スムージング対象データの先頭をアンロード
					k = i - (G_SMOOTH_NUM - 1) / 2;
					dSumGx -= fBufGx[k % G_SMOOTH_NUM];
					dSumGy -= fBufGy[k % G_SMOOTH_NUM];
				}
			}
			
			#pragma omp critical
			{
				SetMaxMinGx(dMaxGx, dMinGx);
				SetMaxMinGy(dMaxGy, dMinGy);
			}

		}
		
		// Direction 補正
		if(m_pLogDirection){
			#pragma omp for
			for(int i = 1; i < GetCnt(); ++i){
				if(Speed(i) < KPH_STOP) SetDirection(i, Direction(i - 1));
			}
		}
  	}
	
	return GetCnt();
}

/*** 距離算出 **************************************************************/

double CVsdLog::GPSLogGetLength(
	double dLong0, double dLati0,
	double dLong1, double dLati1
){
	// ヒュベニの公式 http://yamadarake.jp/trdi/report000001.html
	const double a	= 6378137.000;
	const double b	= 6356752.314245;
	const double e2	= (a * a - b * b) / (a * a);
	
	double dx	= (dLong1 - dLong0) * ToRAD;
	double dy	= (dLati1 - dLati0) * ToRAD;
	double uy	= (dLati0 + dLati1) / 2 * ToRAD;
	double W	= sqrt(1 - e2 * sin(uy) * sin(uy));
	double M	= a * (1 - e2) / pow(W, 3);
	double N	= a / W;
	
	return	sqrt(dy * dy * M * M + pow(dx * N * cos(uy), 2));
}

//////////////////////////////////////////////////////////////////////////////
// Log の全キー取得

struct JsAtomGuard {
	JSContext* ctx;
	JSAtom atom;
	JsAtomGuard(JSContext* c, JSAtom a) : ctx(c), atom(a) {}
	~JsAtomGuard() {
		if (atom != JS_ATOM_NULL) {
			JS_FreeAtom(ctx, atom);
		}
	}
	JsAtomGuard(const JsAtomGuard&) = delete;
	JsAtomGuard& operator=(const JsAtomGuard&) = delete;
};

std::vector<std::string> CVsdLog::GetLogKeys(JSContext* ctx) {
	std::vector<std::string> keys;

	qjsb::Value global(ctx, JS_GetGlobalObject(ctx));
	if (JS_IsException(global.get()) || !JS_IsObject(global.get())) {
		return keys;
	}

	qjsb::Value logObj(ctx, JS_GetPropertyStr(ctx, global.get(), "Log"));
	if (JS_IsException(logObj.get()) || !JS_IsObject(logObj.get())) {
		return keys;
	}

	JSPropertyEnum* props = nullptr;
	uint32_t propCount = 0;

	// 列挙可能な文字列キー（およびシンボルキー）を取得
	int flags = JS_GPN_STRING_MASK | JS_GPN_ENUM_ONLY;
	if (JS_GetOwnPropertyNames(ctx, &props, &propCount, logObj.get(), flags) < 0) {
		return keys;
	}

	// props 配列自体の解放を保証
	struct PropArrayGuard {
		JSContext* ctx;
		JSPropertyEnum* ptr;
		uint32_t count;
		~PropArrayGuard() {
			if (ptr) {
				js_free(ctx, ptr);
			}
		}
	} propGuard{ ctx, props, propCount };

	keys.reserve(propCount);
	for (uint32_t i = 0; i < propCount; ++i) {
		JsAtomGuard atomGuard(ctx, props[i].atom);

		// JSAtom -> C文字列（UTF-8）へ変換
		const char* cstr = JS_AtomToCString(ctx, atomGuard.atom);
		if (cstr) {
			keys.emplace_back(cstr);
			JS_FreeCString(ctx, cstr);
		}
	}

	return keys;
}

static uint32_t JsArrayLength(JSContext* ctx, const qjsb::Value& arr) {
	if (!JS_IsArray(ctx, arr.get())) return 0;
	qjsb::Value lenVal(ctx, JS_GetPropertyStr(ctx, arr.get(), "length"));
	uint32_t len = 0;
	if (JS_ToUint32(ctx, &len, lenVal.get()) < 0) return 0;
	return len;
}

static double JsArrayGetNumber(JSContext* ctx, const qjsb::Value& arr, uint32_t index) {
	qjsb::Value elem(ctx, JS_GetPropertyUint32(ctx, arr.get(), index));
	if (JS_IsException(elem.get())) return NaN;
	double v = NaN;
	if (JS_ToFloat64(ctx, &v, elem.get()) < 0) {
		return NaN;
	}
	return v;
}

/*** ログリード *************************************************************/

int CVsdLog::ReadLog(const char *szFileName, const char *szReaderFunc, CLapLog *&pLapLog){
	{
		// JavaScript オブジェクト初期化
		CScript_qjs Script;
		if(Script.InitLogReader() != ERR_OK){
			return 0;
		}
		
		// スクリプト実行
		Script.Call<void>("ReadLog", AnsiToUtf8(szFileName), std::string(szReaderFunc ? szReaderFunc : ""));
		
		if(Script.m_uError != ERR_OK){
			g_pVsd->DispErrorMessage(Script.GetErrorMessage());
			return 0;
		}
		
		JSContext* ctx = Script.GetContextRaw();

		/*** JS の Log にアクセス *******************************************/
		
		{
			// "Log" 取得
			qjsb::Value global(ctx, JS_GetGlobalObject(ctx));
			if (JS_IsException(global.get()) || !JS_IsObject(global.get())) {
				return 0;
			}

			qjsb::Value logObj(ctx, JS_GetPropertyStr(ctx, global.get(), "Log"));
			if (JS_IsException(logObj.get()) || !JS_IsObject(logObj.get())) {
				return 0;
			}
			
			// Log の key 取得
			auto Keys = GetLogKeys(ctx);
			
			// JavaScript Array の vector
			std::vector<qjsb::Value>	JSArrays;
			
			// CLog * の vector
			std::vector<CLog *>	CArrays;
			
			UINT	uIdxTime		= ~0;
			UINT	uIdxLapTime		= ~0;
			UINT	uIdxDistance	= ~0;
			UINT	uIdxDirection	= ~0;
			UINT	uIdxSpeed		= ~0;
			UINT	uIdxTacho		= ~0;
			UINT	uIdxLong		= ~0;
			UINT	uIdxLati		= ~0;
			
			UINT	uIdx = 0;
			for(UINT u = 0; u < Keys.size(); ++u){
				const char *strKey = Keys[u].c_str();
				
				qjsb::Value JsArr(ctx, JS_GetPropertyStr(ctx, logObj.get(), strKey));
				if (JS_IsException(JsArr.get()) || !JS_IsArray(ctx, JsArr.get())) {
					continue;
				}

				if     (!strcmp(strKey, "Time"		)) uIdxTime			= uIdx;
				else if(!strcmp(strKey, "Distance"	)) uIdxDistance		= uIdx;
				else if(!strcmp(strKey, "Direction"	)) uIdxDirection	= uIdx;
				else if(!strcmp(strKey, "Speed"		)) uIdxSpeed		= uIdx;
				else if(!strcmp(strKey, "Tacho"		)) uIdxTacho		= uIdx;
				else if(!strcmp(strKey, "Longitude"	)) uIdxLong			= uIdx;
				else if(!strcmp(strKey, "Latitude"	)) uIdxLati			= uIdx;
				else if(!strcmp(strKey, "LapTime")){
					uIdxLapTime	= uIdx;
					CArrays.push_back(nullptr);
					goto Skip;	// LapTime は CVsdLog には積まない
				}
					
				// strKey で vector 作成
				CArrays.push_back(GetElement(strKey, TRUE));
					
				Skip:
				// JS の property 名解決後の Array の vector を作る
				JSArrays.push_back(std::move(JsArr));
					
				++uIdx;
			}
			
			// Time 存在確認
			if(uIdxTime == ~0) return 0;
			m_iLogStartTime = (time_t)JsArrayGetNumber(ctx, JSArrays[uIdxTime], 0);
			
			BOOL bCreateDistance  = FALSE;
			BOOL bCreateDirection = FALSE;
			
			if(uIdxLong != ~0 && uIdxLati != ~0){
				// Distance が無いときは，作る
				if(uIdxDistance == ~0){
					uIdxDistance = uIdx++;
					CArrays.push_back(GetElement("Distance", TRUE));
					bCreateDistance = TRUE;
				}
				
				// 緯度経度→メートル 変換定数
				double	dLong0;
				double	dLati0;
				
				dLong0 = JsArrayGetNumber(ctx, JSArrays[uIdxLong], 0);
				m_pLogLongitude->SetBaseVal(dLong0);
				dLati0 = JsArrayGetNumber(ctx, JSArrays[uIdxLati], 0);
				m_pLogLatitude->SetBaseVal(dLati0);
				
				m_dLong2Meter =  GPSLogGetLength(dLong0, dLati0, dLong0 + 1.0 / 3600, dLati0) * 3600;
				m_dLati2Meter = -GPSLogGetLength(dLong0, dLati0, dLong0, dLati0 + 1.0 / 3600) * 3600;
				
				// Direction が無いときは，作る
				if(uIdxDirection == ~0){
					uIdxDirection = uIdx++;
					CArrays.push_back(GetElement("Direction", TRUE));
					bCreateDirection = TRUE;
				}
			}
			
			// vector に積む
			double	dDistance = 0;
			int		iSameCnt = 0;
			
			int		iLogHzTime	= 0;
			int		iLogHzCnt	= 0;
			
			UINT	uLapCnt	= 1;
			
			UINT	uCalibrating = 0;
			
			for(UINT uIdx = 0; uIdx < JsArrayLength(ctx, JSArrays[uIdxTime]); ++uIdx){
				int iCnt = GetCnt();
				int	iLapTime = -1;
				
				for(UINT uKey = 0; uKey < JSArrays.size(); ++uKey){
					
					double dVal = JsArrayGetNumber(ctx, JSArrays[uKey], uIdx);
					
					if(uKey == uIdxTime){
						time_t t = (time_t)dVal;
						if(t < m_iLogStartTime) t += 24 * 3600 * 1000;
						SetTime(iCnt, (double)(t - m_iLogStartTime));
					}else if(uKey == uIdxSpeed && uIdxTacho != ~0){
						// キャリブレーション中は，一旦 0km/h にする
						if(dVal >= CALIB_MARK_SPEED){
							++uCalibrating;
							dVal = 0;
						}else{
							uCalibrating = 0;
						}
						CArrays[uKey]->Set(iCnt, dVal);
					}else if(uKey == uIdxLapTime){
						if(!_isnan(dVal)) iLapTime = (int)dVal;
					}else{
						CArrays[uKey]->Set(iCnt, dVal);
					}
				}
				
				if(iCnt == 0){
					// 番犬作成
					if(uIdxDistance  != ~0) SetDistance(0, 0);
					if(uIdxDirection != ~0) SetDirection(0, 0);
					CopyRecord(1, 0);
					CopyRecord(2, 0);
					AddStopRecord(0, -WATCHDOG_TIME);
					AddStopRecord(1, -TIME_STOP_MARGIN);
				}else{
					// 同じ時間が連続する場合の時刻調整
					if(GetTime(iCnt - 1) == GetTime(iCnt)){
						// 時刻が同じログが続くときそのカウントをする
						++iSameCnt;
					}else if(iSameCnt){
						// 時刻が同じログが途切れたので，時間を補正する
						++iSameCnt;
						
						// -4 -3 -2 -1 +0
						//  A  A  A  A  B
						//u =  1  2   3
						
						for(int j = 1; j < iSameCnt; ++j){
							SetTime(
								iCnt - iSameCnt + j,
								GetTime(iCnt - iSameCnt) +
								(GetTime(iCnt) - GetTime(iCnt - iSameCnt)) * j / iSameCnt
							);
						}
						iSameCnt = 0;
					}
					
					// 走行距離を作る
					if(bCreateDistance){
						double x = X0(iCnt - 1) - X0(iCnt); x *= x;
						double y = Y0(iCnt - 1) - Y0(iCnt); y *= y;
						
						dDistance += sqrt(x + y);
						SetDistance(iCnt, dDistance);
					}
					
					// 方位を作る
					if(bCreateDirection){
						// 停止なら，直前の方位を引き継ぐ
						if(Distance(iCnt) == Distance(iCnt - 1)){
							SetDirection(iCnt, Direction(iCnt - 1));
						}else{
							double dDir = atan2(Y0(iCnt) - Y0(iCnt - 1), X0(iCnt) - X0(iCnt - 1)) / ToRAD + 90;
							if(dDir < 0) dDir += 360;
							SetDirection(iCnt, dDir);
						}
					}
					
					// 前のログから TIME_STOP 離れていてかつ 指定km/h 以下なら，停止とみなす
					int iDiffTime = GetTime(iCnt) - GetTime(iCnt - 1);
					if(
						uIdxDistance != ~0 &&
						iDiffTime > TIME_STOP &&
						(Distance(iCnt) - Distance(iCnt - 1)) * 3600 / iDiffTime < KPH_STOP
					){
						// -1 -0
						// A  B
						//     ↓
						// -1 -0 +1 +2
						// A  A' B' B
						CopyRecord(iCnt + 1, iCnt); // B'
						CopyRecord(iCnt + 2, iCnt); // B
						CopyRecord(iCnt,     iCnt - 1); // A'
						AddStopRecord(iCnt,     GetTime(iCnt - 1) + TIME_STOP_MARGIN); // A'
						AddStopRecord(iCnt + 1, GetTime(iCnt + 2) - TIME_STOP_MARGIN); // B'
						
						if(uIdxDirection != ~0){
							SetDirection(iCnt + 1, Direction(iCnt - 1));
							SetDirection(iCnt + 2, Direction(iCnt - 1));
						}
					}else{
						// 停止期間でなければ，ログ Hz を計算する
						iLogHzTime += iDiffTime;
						++iLogHzCnt;
					}
				}
				
				// ラップタイムを積む
				if(iLapTime >= 0){
					if(pLapLog == nullptr){
						pLapLog = new CLapLog();
						pLapLog->m_iLapMode = LAPMODE_MAGNET;
					}
					LAP_t	LapTime;
					
					LapTime.uLap	= uLapCnt;
					LapTime.fLogNum	= (float)(GetCnt() - 1);
					LapTime.iTime	= iLapTime;
					pLapLog->PushLap(LapTime);
					
					if(iLapTime) ++uLapCnt;
				}
				
				// キャリブレーション時の時間を記録
				if(uCalibrating){
					if(uCalibrating == 1){
						m_iCalibStart = m_iCalibStop;
						m_iCalibStop  = GetTime(GetCnt() - 1);
						
						if(m_iCalibStart < 0) m_iCalibStart = m_iCalibStop;
					}
					// 300km/h に戻す
					SetRawSpeed(GetCnt() - 1, CALIB_MARK_SPEED);
				}
			}
			// ログ Hz 最終集計
			m_dFreq = 1000.0 * iLogHzCnt / iLogHzTime;
			
			// 番犬の Direction 修正
			if(uIdxDirection != ~0){
				for(int i = 0; i < GetCnt(); ++i){
					if(Direction(i) != 0){
						for(int j = 0; j < i; ++j){
							SetDirection(j, Direction(i));
						}
						break;
					}
				}
			}
		}
	}
	
	/************************************************************************/
	
	if(GetCnt()){
		int iCnt = GetCnt();
		
		// 終端側の番犬
		CopyRecord(iCnt,     iCnt - 1);
		CopyRecord(iCnt + 1, iCnt - 1);
		AddStopRecord(iCnt,     GetTime(iCnt - 1) + TIME_STOP_MARGIN);
		AddStopRecord(iCnt + 1, WATCHDOG_TIME);
		
		#define DUMP_LOG
		#if defined DEBUG && defined DUMP_LOG
			Dump("C:\\Users\\yoshi\\Documents\\develop\\vsd\\vsd_filter\\z_gpslog_raw.txt");
		#endif
		
		// ログ再スキャン
		GPSLogRescan();
		
		#if defined DEBUG && defined DUMP_LOG
			Dump("C:\\Users\\yoshi\\Documents\\develop\\vsd\\vsd_filter\\z_gpslog_upcon.txt");
		#endif
		
		// Time の Max Min 設定
		m_pLogTime->InitMinMax();
		m_pLogTime->SetMaxMin(GetTime(GetCnt() - 2), 0);
	}
	
	// Lap log の番犬
	if(pLapLog){
		LAP_t	LapTime;
		LapTime.fLogNum	= FLT_MAX;	// 番犬
		LapTime.iTime	= 0;		// 番犬
		pLapLog->m_Lap.push_back(LapTime);
	}
	
	return GetCnt();
}

/*** MAP 回転処理 ***********************************************************/

void CVsdLog::RotateMap(double dAngle){
	
	if(!m_pLogLongitude) return;
	
	GetElement("X", TRUE);
	GetElement("Y", TRUE);
	
	m_pLogX->InitMinMax();
	m_pLogY->InitMinMax();
	
	for(int i = 0; i < GetCnt(); ++i){
		SetX(i,  cos(dAngle) * X0(i) + sin(dAngle) * Y0(i));
		SetY(i, -sin(dAngle) * X0(i) + cos(dAngle) * Y0(i));
	}
}

/*** JS 用 Log オブジェクト構築 *********************************************/

void CVsdLog::BuildLogObject(JSContext* ctx){
	if(!ctx) return;
	
	qjsb::Value global(ctx, JS_GetGlobalObject(ctx));
	if(JS_IsException(global.get()) || !JS_IsObject(global.get())){
		return;
	}
	
	// 既存の Log が存在する場合は削除
	JSAtom atomLog = JS_NewAtom(ctx, "Log");
	if(atomLog != JS_ATOM_NULL){
		int hasLog = JS_HasProperty(ctx, global.get(), atomLog);
		if(hasLog > 0){
			JS_DeleteProperty(ctx, global.get(), atomLog, 0);
		}
		JS_FreeAtom(ctx, atomLog);
	}
	
	// getter 共通ロジック:
	//   magic == 0: GetElement(prop)->Get(m_dLogNum)
	//   magic == 1: GetMax(prop)
	//   magic == 2: GetMin(prop)
	auto logPropGetter = [](JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv, int magic, JSValue* func_data) -> JSValue {
		(void)this_val;
		(void)argc;
		(void)argv;
		
		int64_t iSelf = 0;
		if(JS_ToInt64(ctx, &iSelf, func_data[0]) < 0 || !iSelf){
			return JS_NewFloat64(ctx, NaN);
		}
		CVsdLog* pSelf = reinterpret_cast<CVsdLog*>(static_cast<uintptr_t>(iSelf));
		
		const char* szProp = JS_ToCString(ctx, func_data[1]);
		if(!szProp){
			return JS_NewFloat64(ctx, NaN);
		}
		
		double dVal = NaN;
		if(magic == 0){
			CLog* pElem = pSelf->GetElement(szProp);
			dVal = pElem ? pElem->Get(pSelf->m_dLogNum) : NaN;
		}else if(magic == 1){
			dVal = pSelf->GetMax(szProp);
		}else if(magic == 2){
			dVal = pSelf->GetMin(szProp);
		}
		
		JS_FreeCString(ctx, szProp);
		return JS_NewFloat64(ctx, dVal);
	};
	
	// Log.ValueOfIndex(prop, pos)
	auto fnValueOfIndex = [](JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv) -> JSValue {
		(void)this_val;
		(void)argc;
		(void)argv;
		return JS_NewInt32(ctx, 0);
	};
	
	// Log / Log.Max / Log.Min オブジェクト生成
	JSValue objLog = JS_NewObject(ctx);
	JSValue objMax = JS_NewObject(ctx);
	JSValue objMin = JS_NewObject(ctx);
	
	JSValue valSelf = JS_NewInt64(ctx, static_cast<int64_t>(reinterpret_cast<uintptr_t>(this)));
	
	// m_Logs の全キーに対して getter のみを持つ列挙可能プロパティを定義
	for(auto it = m_Logs.begin(); it != m_Logs.end(); ++it){
		const char* szKey = it->first.c_str();
		JSAtom atomKey = JS_NewAtom(ctx, szKey);
		if(atomKey == JS_ATOM_NULL) continue;
		
		JSValue funcData[2] = {
			valSelf,
			JS_NewString(ctx, szKey)
		};
		
		JSValue getterVal = JS_NewCFunctionData(ctx, logPropGetter, 0, 0, 2, funcData);
		JSValue getterMax = JS_NewCFunctionData(ctx, logPropGetter, 0, 1, 2, funcData);
		JSValue getterMin = JS_NewCFunctionData(ctx, logPropGetter, 0, 2, 2, funcData);
		
		JS_FreeValue(ctx, funcData[1]);
		
		// setter は不要 (JS_UNDEFINED), Object.keys() 対象にするため JS_PROP_ENUMERABLE を付与
		JS_DefinePropertyGetSet(ctx, objLog, atomKey, getterVal, JS_UNDEFINED, JS_PROP_ENUMERABLE | JS_PROP_CONFIGURABLE);
		JS_DefinePropertyGetSet(ctx, objMax, atomKey, getterMax, JS_UNDEFINED, JS_PROP_ENUMERABLE | JS_PROP_CONFIGURABLE);
		JS_DefinePropertyGetSet(ctx, objMin, atomKey, getterMin, JS_UNDEFINED, JS_PROP_ENUMERABLE | JS_PROP_CONFIGURABLE);
		
		JS_FreeAtom(ctx, atomKey);
	}
	
	JS_FreeValue(ctx, valSelf);
	
	// Log.Max / Log.Min / Log.ValueOfIndex を設定
	JS_DefinePropertyValueStr(ctx, objLog, "Max", objMax, JS_PROP_CONFIGURABLE);
	JS_DefinePropertyValueStr(ctx, objLog, "Min", objMin, JS_PROP_CONFIGURABLE);
	JS_DefinePropertyValueStr(ctx, objLog, "ValueOfIndex", JS_NewCFunction(ctx, fnValueOfIndex, "ValueOfIndex", 2), JS_PROP_CONFIGURABLE);
	
	// グローバルオブジェクトへ Log を設定 (JS_SetPropertyStr は val の所有権を消費する)
	JS_SetPropertyStr(ctx, global.get(), "Log", objLog);
}

/*** チャートリード *********************************************************/

int CLapLogAll::LapChartRead(const char *szFileName){
	return 0;
#if 0 // ★_qjs
	m_iLapMode = LAPMODE_CHART;
	m_iLapSrc  = LAPSRC_VIDEO;
	
	{
		// JavaScript オブジェクト初期化
		CScript Script;
		
		if(Script.Initialize(L"_system/InitLapChartReader.js") == ERR_OK){
			// スクリプト実行
			LPWSTR pStr = nullptr;
			LPWSTR pReader = nullptr;
			LPWSTR p = nullptr;
			Script.Run_s(L"ReadLapChart", StringNewA2W(pStr, szFileName));
			delete [] pStr;
			delete [] pReader;
		}
		
		if(Script.m_uError != ERR_OK){
			g_pVsd->DispErrorMessage(Script.GetErrorMessage());
			return 0;
		}
		
		/*** JS の Log にアクセス *******************************************/
		
		{
			Locker locker(Script.m_pIsolate);
			Isolate::Scope IsolateScope(Script.m_pIsolate);
			HandleScope handle_scope(Script.m_pIsolate);
			Context::Scope context_scope(Script.GetContext());
			
			// "LapTime" 取得
			Local<Array> hLapTime = Local<Array>::Cast(
				Script.GetContext()->Global()->Get(String::NewFromTwoByte(Script.m_pIsolate , (uint16_t *)L"LapTime"))
			);
			if(!hLapTime->IsArray()) return 0;
			
			// カメラカー No 取得
			m_iCamCarIdx = Script.GetContext()->Global()->Get(String::NewFromTwoByte(Script.m_pIsolate , (uint16_t *)L"CameraCarID"))->Int32Value();
			
			// LapTime の key 取得
			Local<Array> Keys = hLapTime->GetPropertyNames();
			
			for(UINT u = 0; u < Keys->Length(); ++u){
				String::Value strKey(Keys->Get(u));
				m_strName.push_back(reinterpret_cast<LPWSTR>(*strKey));
				
				std::vector<int>	int_vec;	// ダミー
				m_LapTable.push_back(int_vec);
				
				Local<Array> ArrayTmp = Local<Array>::Cast(hLapTime->Get(Keys->Get(u)));
				for(UINT v = 0; v < ArrayTmp->Length(); ++ v){
					m_LapTable[u].push_back(ArrayTmp->Get(v)->Int32Value());
				}
			}
		}
	}
	
	if(m_LapTable.size() == 0 || CamCarLap().size() <= 1){
		return 0;
	}
	
	// ラップタイム表を時刻表に変換
	for(int j = 0; j < (int)m_LapTable.size(); ++j){
		for(int i = m_LapTable[j].size() - 2; i >= 0; --i){
			m_LapTable[j][i] = m_LapTable[j][i + 1] - m_LapTable[j][i];
		}
	}
	
	// vector 配列を作る
	m_iAllLapIdx.resize(m_strName.size(), -1);
	m_iAllGapInfo.resize(m_strName.size(), 0);
	m_iPositionInfo.resize(m_strName.size(), -1);
	
	return CamCarLap().size();
#endif
}

/*** 自車ラップデータ構築 ***************************************************/

void CLapLogAll::MakeCamLapData(int iStartFrame, int iEndFrame){
	
	LAP_t	Lap = {
		0,	// uLap
		0,	// fLogNum
		0	// iTime
	};
	
	m_Lap.clear();
	m_iLapNum = 0;
	
	m_iStartFrame = iStartFrame;
	m_iEndFrame = iEndFrame;
	
	int iTimeSum = CamCarLap()[CamCarLap().size() - 1] - CamCarLap()[0];
	
	// ラップデータを構築
	Lap.fLogNum = (float)m_iStartFrame;
	PushLap(Lap);	// 計測スタート (iTime = 0)
	int iTime = 0;
	
	for(int i = 1; i < (int)CamCarLap().size(); ++i){
		iTime += Lap.iTime = GetLapTime(i);
		++Lap.uLap;
		
		Lap.fLogNum = (float)(
			m_iStartFrame +
			(m_iEndFrame - m_iStartFrame) * (double)iTime / iTimeSum
		);
		PushLap(Lap);
	}
	
	Lap.fLogNum	= FLT_MAX;	// 番犬
	Lap.iTime	= 0;		// 番犬
	m_Lap.push_back(Lap);
	
	// CalcLapInfo() をリセット
	m_iPrevFrame	= INT_MAX;
}

/*** 各自のラップ情報再計算 *************************************************/

void CLapLogAll::CalcLapInfo(int iFrameCnt, double dFPS){
	
	int iFrameCntRaw = iFrameCnt;
	int i;
	
	// スタート前・ゴール後の iFrameCnt を補正
	if(m_iLapIdx < 0){
		iFrameCnt = GetLapFrame(0);
	}else if(m_iLapIdx >= (int)CamCarLap().size() - 1){
		iFrameCnt = GetLapFrame(CamCarLap().size() - 1);
	}else{
		//iFrameCnt = GetFrameCnt();
	}
	
	// 巻き戻しされたときは，先頭からサーチし直す
	if(iFrameCntRaw < m_iPrevFrame){
		for(i = 0; i < (int)m_iPositionInfo.size(); ++i){
			m_iAllLapIdx[i] = m_iPositionInfo[i] = -1;
			m_iPrevFrame = -1;
		}
	}
	
	// 前回の演算位置から Frame# まで進めていって，順位を計算する
	for(i = m_iPrevFrame + 1; i <= iFrameCntRaw; ++i){
		for(UINT u = 0; u < m_iPositionInfo.size(); ++u){
			
			// 現在ラップ数を求める
			int iLapIdx = m_iAllLapIdx[u];
			for(;
				iLapIdx <= (int)m_LapTable[u].size() - 2 &&
				i >= GetLapFrame(u, iLapIdx + 1);
				++iLapIdx
			);
			
			// ラップ数が更新された
			if(iLapIdx != m_iAllLapIdx[u]){
				
				m_iAllLapIdx[u] = iLapIdx;
				
				// 一旦自車の要素を消す
				m_iPositionInfo.erase(
					std::remove(
						m_iPositionInfo.begin(),
						m_iPositionInfo.end(),
						u
					),
					m_iPositionInfo.end()
				);
				
				if(m_iPositionInfo.size() == m_iAllLapIdx.size()){
					m_iPositionInfo.pop_back();
				}
				
				// 挿入ポイントを探して挿入
				std::vector<int>::iterator v;
				for(v = m_iPositionInfo.begin(); v != m_iPositionInfo.end(); ++v){
					if(
						*v == -1 ||
						m_iAllLapIdx[*v] < iLapIdx
					) break;
				}
				
				// ラップリーダーを格納
				if(v == m_iPositionInfo.begin()){
					if((int)m_iLapLeader.size() <= iLapIdx){
						m_iLapLeader.push_back(u);
					}else{
						m_iLapLeader[iLapIdx] = u;
					}
				}
				
				m_iPositionInfo.insert(v, u);
			}
		}
	}
	m_iPrevFrame = iFrameCntRaw;
	
	// 各車の周回 index を求める
	#ifdef _OPENMP_AVS
		#pragma omp parallel for
	#endif
	for(UINT u = 0; u < m_strName.size(); ++u){
		int iLapIdx = m_iAllLapIdx[u];
		
		// ラップ数と何 % を進んだかを求める
		double dProceeding;
		if(iLapIdx < 0){
			iLapIdx = 0;
		}else if(iLapIdx == m_LapTable[u].size() - 1){
			--iLapIdx;
		}
		dProceeding =
			(double)(iFrameCnt           - GetLapFrame(u, iLapIdx)) /
			(GetLapFrame(u, iLapIdx + 1) - GetLapFrame(u, iLapIdx));
		
		// 上で求めた位置の，カメラ車におけるフレーム番号を求め
		// そこからカメラ車との時間差を求めて push
		// 下位 8bit が id, 上位残りがタイム差
		m_iAllGapInfo[u] = (
			(int)(
				(
					GetLapFrame(iLapIdx) - iFrameCnt +
					(GetLapFrame(iLapIdx + 1) - GetLapFrame(iLapIdx)) * dProceeding
				) / dFPS * -1000
			) << 8
		) | u;
	}
	
	std::sort(m_iAllGapInfo.begin(), m_iAllGapInfo.end());	// ソート
}
