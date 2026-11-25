/*****************************************************************************
	
	VSD -- vehicle data logger system  Copyright(C) by DDS
	
	CVsdLogJs.h - Vsd + GPS log manager for Javascript Log object
	
*****************************************************************************/

#pragma once
#include "CVsdLog.h"

class CVsdFilter;
class CVsdLogJs {
public:
	CVsdLogJs(CVsdFilter *pVsd);

private:
	enum {
		PROP_VAL,
		PROP_MAX,
		PROP_MIN,
	};

	CVsdFilter *m_pVsd;
	std::map<std::string, CLog*> m_Logs;

	double AccessLog(const std::string& strKey, double dFrameCnt);

	// CLog 取得
	CLog* GetLog(const std::string& strKey) {
		auto itr = m_Logs.find(strKey);
		if (itr == m_Logs.end()) {
			return nullptr;
		}
		return itr->second;
	}

	static CVsdLogJs* GetSelf(JSContext* ctx, JSValueConst valSelf);
	static JSValue logPropGetter(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv, int magic, JSValue* func_data);
	static JSValue fnValueOfIndex(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv, int magic, JSValue* func_data);
};
