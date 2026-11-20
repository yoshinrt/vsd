/*****************************************************************************
	
	VSD -- vehicle data logger system  Copyright(C) by DDS
	
	CScript_qjs.h - JavaScript
	
*****************************************************************************/

#pragma once

#include "CVsdLog.h"
#include "CJsMap.h"
#include "CJsClass.h"
#include "error_code.h"

/*** エラー *****************************************************************/

#define JsAnyError_qjs(type, msg)	throw std::runtime_error(CScript_qjs::ErrMsgOrID(msg))
#define JsRangeError_qjs(msg)		JsAnyError_qjs(RangeError, msg)
#define JsReferenceError_qjs(msg)	JsAnyError_qjs(ReferenceError, msg)
#define JsSyntaxError_qjs(msg)		JsAnyError_qjs(SyntaxError, msg)
#define JsTypeError_qjs(msg)		JsAnyError_qjs(TypeError, msg)
#define JsError_qjs(msg)			JsAnyError_qjs(Error, msg)
#define JsErrorNumOfArg_qjs()		JsSyntaxError_qjs(ERR_NUM_OF_ARG)

/****************************************************************************/

inline qjsb::Runtime g_JsRuntime;

class CScript_qjs : public CJsClass {
  public:
	CScript_qjs();
	~CScript_qjs(void);
	
	void RegisterClass();
	UINT Initialize();
	UINT RunFile(LPCSTR szFileName);
	UINT RunScript(LPCSTR szScript, bool bModule = false);

	template<typename Ret, typename... Args>
	Ret Call(const char* name, Args&&... args){
		std::array<qjsb::Value, sizeof...(Args)> jsArgs = {
			qjsb::Value(
				m_JsContext.get(),
				qjsb::to_js(
					m_JsContext.get(),
					std::forward<Args>(args)
				)
			)...
		};

		std::array<JSValueConst, sizeof...(Args)> argv{};

		for (size_t i = 0; i < jsArgs.size(); ++i) {
			argv[i] = jsArgs[i].get();
		}


		qjsb::Value result = CallRaw(
			name,
			static_cast<int>(argv.size()),
			argv.empty() ? nullptr : argv.data()
		);

		if constexpr (!std::is_void_v<Ret>) {
			return qjsb::from_js<Ret>(
				m_JsContext.get(),
				result.get()
			);
		}
	}

	static std::string ReportException(const qjsb::JSException& e);
	
	LPCSTR GetErrorMessage(void){
		return m_ErrorMsg.empty() ? m_szErrorMsgID[m_uError] : m_ErrorMsg.c_str();
	}
	
	UINT InitLogReader(void);
	
	// Global オブジェクト
	static void DebugPrint(std::string msg);	// !js_func
	static int MessageBox(	// !js_func
		std::string msg,
		std::string caption,	// !default:""
		UINT	uType		// !default:MB_OK
	);
	
	static void Print(std::string msg);	// !js_func
	
	// 距離取得
	static double GetDistanceByLngLat(	// !js_func
		double dLong0, double dLati0,
		double dLong1, double dLati1
	){
		return CVsdLog::GPSLogGetLength(dLong0, dLati0, dLong1, dLati1);
	}
	
	static CScript_qjs *GetCScript(Local<Object> handle){
		HandleScope handle_scope(Isolate::GetCurrent());
		
		return CJsMap::Cast(handle)["__CScript"].GetObj<CScript_qjs>();
	}
	
	// msg であればスルーパス，ERR_ID であればメッセージを返す
	static const char *ErrMsgOrID(const char *szMsg){return szMsg;}
	static const char *ErrMsgOrID(UINT uID){return m_szErrorMsgID[uID];}
	
	qjsb::Context& GetContext(void) { return m_JsContext; }
	JSContext* GetContextRaw(void) { return m_JsContext.get(); }

	UINT m_uError;
	
  private:
	static LPCSTR m_szErrorMsgID[];
	std::string m_ErrorMsg;
	bool m_bInitialized = false;

	qjsb::Context m_JsContext = qjsb::Context(g_JsRuntime);

	qjsb::Value CallRaw(
		const char* name,
		int argc,
		JSValueConst* argv
	);
};
