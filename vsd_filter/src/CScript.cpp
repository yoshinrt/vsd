/*****************************************************************************
	
	VSD -- vehicle data logger system  Copyright(C) by DDS
	
	CScript.cpp - JavaScript
	
*****************************************************************************/

#include "StdAfx.h"
#include "CVsdFilter.h"
#include "CScript.h"

/*** static メンバ（；´д⊂）***********************************************/

LPCSTR CScript::m_szErrorMsgID[] = {
	#define DEF_ERROR(id, msg)	msg,
	#include "def_error.h"
};

/*** コンストラクタ *********************************************************/

CScript::CScript(CVsdFilter *pVsd, bool bNoVsdObject) :
	m_pVsd(pVsd),
	m_pRuntime(pVsd->m_JsRuntime.get()),
	m_JsContext(pVsd->m_JsRuntime.get()),
	m_bNoVsdObject(bNoVsdObject) {
	DebugMsgD(":CScript::CScript():%X\n", GetCurrentThreadId());
	m_uError		= ERR_OK;
}

/*** デストラクタ ***********************************************************/

CScript::~CScript(){
	DebugMsgD(":CScript::~CScript():%X\n", GetCurrentThreadId());
}

/*** Exception メッセージ取得 ***********************************************/

std::string CScript::ReportException(const qjsb::JSException& e){
	std::string msg = e.what();
	msg += e.stack();
	return msg;
}

/*** Print ******************************************************************/

void CScript::Print(const std::string& msg){
	CVsdFilter::Print(Utf8ToWide(msg).c_str());
}

int CScript::MessageBox(
	const std::string& msg,
	const std::string& caption,
	UINT	uType
){
	return MessageBoxW(nullptr, Utf8ToWide(msg).c_str(),
		caption.empty() ? L"VSD filter JavaScript message" : Utf8ToWide(caption).c_str(),
		uType
	);
}

void CScript::DebugPrint(const std::string& msg){
	OutputDebugStringW(Utf8ToWide(msg).c_str());
}

/*** JavaScript interface のセットアップ ************************************/

UINT CScript::Initialize(){
	if (m_bInitialized) return ERR_OK;
	m_bInitialized = true;
	
	// グローバルオブジェクトの生成
	js_std_init_handlers(m_pRuntime);
	js_init_module_std(m_JsContext.get(), "std");
	js_init_module_os(m_JsContext.get(), "os");

	RegisterJsClass();
	
	// Vsd 公開
	if (!m_bNoVsdObject) {
		m_JsContext.setGlobal("Vsd", qjsb::pushBorrowed(m_JsContext, m_pVsd));
	}
	
	return RunScript(R"(
		import * as std from "std";
		import * as os from "os";
		globalThis.std = std;
		globalThis.os = os;
	)", true);
}

/*** スクリプトファイルの実行 ***********************************************/

UINT CScript::RunFile(LPCSTR szFileName){
	CPushDir push_dir(m_pVsd->m_szPluginDirA.c_str());
	
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

UINT CScript::RunScript(LPCSTR szScript, bool bModule){
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

qjsb::Value CScript::CallRaw(
	const char* name,
	int argc,
	JSValueConst* argv
) {
	m_ErrorMsg.clear();

	qjsb::Value func = m_JsContext.getGlobal(name);

	if (!func.isFunction()) {
		m_ErrorMsg = std::string(name) + " is not a function";
		m_uError = ERR_SCRIPT;
		return qjsb::Value(m_JsContext.get(), JS_UNDEFINED);
	}

	JSValue v = JS_Call(m_JsContext.get(), func.get(), JS_UNDEFINED, argc, argv);

	if (JS_IsException(v)) {
		JS_FreeValue(m_JsContext.get(), v);
		try{
			qjsb::throwJSException(m_JsContext.get());
		}
		catch (const qjsb::JSException& e) {
			m_ErrorMsg = ReportException(e);
			m_uError = ERR_SCRIPT;
			return qjsb::Value(m_JsContext.get(), JS_UNDEFINED);
		}
	}

	return qjsb::Value(m_JsContext.get(), v);
}

/*** ログリード by JavaScript **********************************************/

BOOL LogReaderCallback(const std::string& strPath, const std::string& strFile, void *pParam){
	if(!IsExt(strFile, ".js")) return true;
	
	CScript &Script = *(CScript *)pParam;
	
	std::string path(strPath);
	path += strFile;
	Script.RunFile(path.c_str());
	
	return Script.m_uError == ERR_OK;
}

UINT CScript::InitLogReader(void){
	m_uError = ERR_OK;

	do {
		if (RunFile("_system/InitLogReader.js") != ERR_OK) break;

		// スクリプトロード
		std::string path(m_pVsd->m_szPluginDirA);
		path += LOG_READER_DIR;
		
		if (!ListTree(path, "*", LogReaderCallback, this)) {
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
		m_pVsd->DispErrorMessage(GetErrorMessage());
	}
	return m_uError;
}
