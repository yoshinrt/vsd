/*****************************************************************************
	
	VSD -- vehicle data logger system  Copyright(C) by DDS
	
	CVsdLogJs.cpp - Vsd + GPS log manager for Javascript Log object
	
*****************************************************************************/

#include "StdAfx.h"
#include "CVsdFilter.h"
#include "CVsdLogJs.h"

CVsdLogJs::CVsdLogJs(){
	m_pVsd = g_pVsd;
	
	// VSD / GPS log を VSD 優先で登録
	if(m_pVsd->m_VsdLog) for (const auto& [key, log] : m_pVsd->m_VsdLog->m_Logs) {
		m_Logs[key] = log.get();
	}

	if(m_pVsd->m_GPSLog) for (const auto& [key, log] : m_pVsd->m_GPSLog->m_Logs) {
		if(!m_Logs.contains(key)) m_Logs[key] = log.get();
	}

	/*** JS 用 Log オブジェクト構築 *****************************************/

	auto ctx = m_pVsd->m_Script ? m_pVsd->m_Script->GetContextRaw() : nullptr;
	if (!ctx) return;

	qjsb::Value global(ctx, JS_GetGlobalObject(ctx));
	if (JS_IsException(global.get()) || !JS_IsObject(global.get())) {
		return;
	}

	// 既存の Log が存在する場合は削除
	JSAtom atomLog = JS_NewAtom(ctx, "Log");
	if (atomLog != JS_ATOM_NULL) {
		int hasLog = JS_HasProperty(ctx, global.get(), atomLog);
		if (hasLog > 0) {
			JS_DeleteProperty(ctx, global.get(), atomLog, 0);
		}
		JS_FreeAtom(ctx, atomLog);
	}

	// Log / Log.Max / Log.Min オブジェクト生成
	JSValue objLog = JS_NewObject(ctx);
	JSValue objMax = JS_NewObject(ctx);
	JSValue objMin = JS_NewObject(ctx);

	JSValue valSelf = JS_NewInt64(ctx, static_cast<int64_t>(reinterpret_cast<uintptr_t>(this)));

	// m_Logs の全キーに対して getter のみを持つ列挙可能プロパティを定義
	for (auto it = m_Logs.begin(); it != m_Logs.end(); ++it) {
		const char* szKey = it->first.c_str();
		JSAtom atomKey = JS_NewAtom(ctx, szKey);
		if (atomKey == JS_ATOM_NULL) continue;

		JSValue funcData[2] = {
			valSelf,
			JS_NewString(ctx, szKey)
		};

		JSValue getterVal = JS_NewCFunctionData(ctx, logPropGetter, 0, PROP_VAL, 2, funcData);
		JSValue getterMax = JS_NewCFunctionData(ctx, logPropGetter, 0, PROP_MAX, 2, funcData);
		JSValue getterMin = JS_NewCFunctionData(ctx, logPropGetter, 0, PROP_MIN, 2, funcData);

		JS_FreeValue(ctx, funcData[1]);

		// setter は不要 (JS_UNDEFINED), Object.keys() 対象にするため JS_PROP_ENUMERABLE を付与
		JS_DefinePropertyGetSet(ctx, objLog, atomKey, getterVal, JS_UNDEFINED, JS_PROP_ENUMERABLE | JS_PROP_CONFIGURABLE);
		JS_DefinePropertyGetSet(ctx, objMax, atomKey, getterMax, JS_UNDEFINED, JS_PROP_ENUMERABLE | JS_PROP_CONFIGURABLE);
		JS_DefinePropertyGetSet(ctx, objMin, atomKey, getterMin, JS_UNDEFINED, JS_PROP_ENUMERABLE | JS_PROP_CONFIGURABLE);

		JS_FreeAtom(ctx, atomKey);
	}

	// Log.Max / Log.Min / Log.ValueOfIndex を設定
	JS_DefinePropertyValueStr(ctx, objLog, "Max", objMax, JS_PROP_CONFIGURABLE);
	JS_DefinePropertyValueStr(ctx, objLog, "Min", objMin, JS_PROP_CONFIGURABLE);
	JS_DefinePropertyValueStr(ctx, objLog, "ValueOfIndex", JS_NewCFunctionData(ctx, fnValueOfIndex, 2, 0, 1, &valSelf), JS_PROP_CONFIGURABLE);

	JS_FreeValue(ctx, valSelf);

	// グローバルオブジェクトへ Log を設定 (JS_SetPropertyStr は val の所有権を消費する)
	JS_SetPropertyStr(ctx, global.get(), "Log", objLog);
}

CVsdLogJs* CVsdLogJs::GetSelf(JSContext* ctx, JSValueConst valSelf) {
	int64_t iSelf = 0;
	if (JS_ToInt64(ctx, &iSelf, valSelf) < 0 || !iSelf) {
		return nullptr;
	}
	return reinterpret_cast<CVsdLogJs*>(static_cast<uintptr_t>(iSelf));
}

// getter 共通ロジック:
//   magic == PROP_VAL: GetElement(prop)->Get(m_dLogNum)
//   magic == PROP_MAX: GetMax(prop)
//   magic == PROP_MIN: GetMin(prop)
JSValue CVsdLogJs::logPropGetter(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv, int magic, JSValue* func_data) {
	(void)this_val;
	(void)argc;
	(void)argv;

	CVsdLogJs* pSelf = GetSelf(ctx, func_data[0]);
	if (!pSelf) {
		return JS_UNDEFINED;
	}

	const char* szProp = JS_ToCString(ctx, func_data[1]);
	if (!szProp) {
		return JS_UNDEFINED;
	}

	CLog* pElem = pSelf->GetLog(szProp);
	if(!pElem) return JS_UNDEFINED;

	double dVal = NaN;

	if (magic == PROP_VAL) {
		dVal = pElem->Get();
	}
	else if (magic == PROP_MAX) {
		dVal = pElem->GetMax();
	}
	else if (magic == PROP_MIN) {
		dVal = pElem->GetMin();
	}

	JS_FreeCString(ctx, szProp);
	return isnan(dVal) ? JS_UNDEFINED : JS_NewFloat64(ctx, dVal);
}

// Log.ValueOfIndex(prop, pos)
JSValue CVsdLogJs::fnValueOfIndex(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv, int magic, JSValue* func_data) {
	(void)this_val;
	(void)magic;

	CVsdLogJs* pSelf = GetSelf(ctx, func_data[0]);
	if (!pSelf || argc < 2) {
		return JS_UNDEFINED;
	}

	auto val = pSelf->AccessLog(
		qjsb::from_js<std::string>(ctx, argv[0]),
		qjsb::from_js<double>(ctx, argv[1])
	);
	if (isnan(val)) return JS_UNDEFINED;
	return qjsb::to_js(ctx, val);
}

/*** 位置指定ログアクセス ***************************************************/

// パラメータを変換
#define GetLogIndex(frame, to, prev) \
	m_##to##Log->GetIndex(frame, &VideoSt, &to##St, prev)

double CVsdLogJs::AccessLog(const std::string& strKey, double dFrameCnt) {
	double dRet;

	if (
		m_pVsd->m_VsdLog &&
		!_isnan(dRet = m_pVsd->m_VsdLog->Get(strKey.c_str(),
			m_pVsd->m_VsdLog->GetIndex(dFrameCnt,
				&m_pVsd->m_piParamT[PARAM_LSt],
				&m_pVsd->m_piParamT[PARAM_VSt],
				m_pVsd->m_VsdLog->m_iLogNum)))
	) {
		return dRet;
	}

	if (
		m_pVsd->m_GPSLog &&
		!_isnan(dRet = m_pVsd->m_GPSLog->Get(strKey.c_str(),
			m_pVsd->m_GPSLog->GetIndex(dFrameCnt,
				&m_pVsd->m_piParamT[PARAM_GSt],
				&m_pVsd->m_piParamT[PARAM_VSt],
				m_pVsd->m_GPSLog->m_iLogNum)))
		) {
		return dRet;
	}

	return NaN;
}
