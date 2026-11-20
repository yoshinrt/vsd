/*****************************************************************************
	
	VSD -- vehicle data logger system  Copyright(C) by DDS
	
	CScript_qjs.cpp - JavaScript
	
*****************************************************************************/

#include "StdAfx.h"

#include "CScript_qjs.h"
#include "ScriptIF.h"

/*** static メンバ（；´д⊂）***********************************************/

LPCSTR CScript_qjs::m_szErrorMsgID[] = {
	#define DEF_ERROR(id, msg)	msg,
	#include "def_error.h"
};

/*** コンストラクタ *********************************************************/

CScript_qjs::CScript_qjs(){
	DebugMsgD(":CScript_qjs::CScript_qjs():%X\n", GetCurrentThreadId());
	m_uError		= ERR_OK;
}

/*** デストラクタ ***********************************************************/

CScript_qjs::~CScript_qjs(){
	DebugMsgD(":CScript_qjs::~CScript_qjs():%X\n", GetCurrentThreadId());
}

/*** Exception メッセージ取得 ***********************************************/

std::string CScript_qjs::ReportException(const qjsb::JSException& e){
	std::string msg = e.what();
	msg += e.stack();
	return msg;
}

/*** Print ******************************************************************/

void CScript_qjs::Print(LPCSTR szMsg){
	CVsdFilter::Print(Utf8ToWide(szMsg).c_str());
}

int CScript_qjs::MessageBox(
	LPCSTR szMsg,
	LPCSTR szCaption,
	UINT	uType
){
	return MessageBoxW(nullptr, Utf8ToWide(szMsg).c_str(),
		szCaption ? Utf8ToWide(szCaption).c_str() : L"VSD filter JavaScript message",
		uType
	);
}

void CScript_qjs::DebugPrint(LPCSTR szMsg){
	OutputDebugStringW(Utf8ToWide(szMsg).c_str());
	OutputDebugStringW(L"\n");
}

/*** JavaScript interface のセットアップ ************************************/

UINT CScript_qjs::Initialize(){
	// グローバルオブジェクトの生成
	
	js_std_init_handlers(g_JsRuntime.get());
	js_init_module_std(m_JsContext.get(), "std");
	js_init_module_os(m_JsContext.get(), "os");

	return RunScript(R"(
		import * as std from "std";
		import * as os from "os";
		globalThis.std = std;
		globalThis.os = os;
	)");
}

/*** スクリプトファイルの実行 ***********************************************/

UINT CScript_qjs::RunFile(LPCSTR szFileName){
	m_ErrorMsg.clear();

	if ((m_uError = Initialize()) != ERR_OK) {
		return m_uError;
	}

	std::ifstream ifs(szFileName, std::ios::binary);
	if (!ifs) {
		m_ErrorMsg = std::format("Failed to open script file: {}", szFileName);
		return m_uError = ERR_CANT_OPEN_FILE;
	}

	std::vector<char> source((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());
	if (source.empty()) {
		m_ErrorMsg = std::format("Script file is empty: {}", szFileName);
		return m_uError = ERR_SCRIPT;
	}

	source.push_back('\0');

	try {
		m_JsContext.eval(source.data(), szFileName);
	}
	catch (const qjsb::JSException& e) {
		m_ErrorMsg = ReportException(e);
		return m_uError = ERR_SCRIPT;
	}

	return m_uError = ERR_OK;
}

//////////////////////////////////////////////////////////////////////////////
// Script ダイレクト実行

UINT CScript_qjs::RunScript(LPCSTR szScript, bool bModule){
	try {
		bModule ? m_JsContext.evalModule(szScript) : m_JsContext.eval(szScript);
	}
	catch (const qjsb::JSException& e) {
		m_ErrorMsg = ReportException(e);
		return m_uError = ERR_SCRIPT;
	}

	return m_uError = ERR_OK;
}

/*** function 名指定実行 *********************************************************/

qjsb::Value CScript_qjs::CallRaw(
	const char* name,
	int argc,
	JSValueConst* argv
) {
	m_ErrorMsg.clear();

	qjsb::Value func = m_JsContext.getGlobal(name);

	if (!func.isFunction()) {
		throw std::runtime_error(std::string(name) + " is not a function");
	}

	JSValue v = JS_Call(m_JsContext.get(), func.get(), JS_UNDEFINED, argc, argv);

	if (JS_IsException(v)) {
		JS_FreeValue(m_JsContext.get(), v);
		qjsb::throwJSException(m_JsContext.get());
	}

	return qjsb::Value(m_JsContext.get(), v);
}

/*** ログリード by JavaScript **********************************************/

BOOL LogReaderCallback_qjs(const wchar_t *szPath, const wchar_t *szFile, void *pParam){
	const wchar_t* ext = wcsrchr(szFile, L'.');
	if (!(ext != nullptr && _wcsicmp(ext, L".js") == 0)) return TRUE;
	
	CScript_qjs &Script = *(CScript_qjs *)pParam;
	
	std::wstring path(szPath);
	Script.RunFile(WideToUtf8(path + szFile).c_str());
	
	return Script.m_uError == ERR_OK;
}

UINT CScript_qjs::InitLogReader(void){
	do {
		if (RunFile("_system/InitLogReader.js") != ERR_OK) break;

		// スクリプトロード
		std::wstring path(g_pVsd->m_szPluginDirW);
		path += WIDEN(LOG_READER_DIR) L"\\";
		if (!ListTree(path.c_str(), L"*", LogReaderCallback_qjs, this)) {
			m_uError = ERR_CANT_OPEN_FILE;
			break;
		}

		RunScript(R"(
			if (typeof globalThis["SortLogReaderInfo"] === 'function') {
				SortLogReaderInfo();
			}
		)");
	} while (0);

	if(m_uError != ERR_OK) {
		g_pVsd->DispErrorMessage(GetErrorMessage());
	}
	return m_uError = ERR_OK;
}
