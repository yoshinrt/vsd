#pragma once

#define _CRT_NO_VA_START_VALIDATION

#include <windows.h>
#include <crtdbg.h>
#include <assert.h>
#include <ctype.h>
#include <float.h>
#include <gdiplus.h>
#include <map>
#include <algorithm>
#define _USE_MATH_DEFINES
#include <math.h>
#include <objbase.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <direct.h>
#include <tchar.h>
#include <time.h>
#include "libplatform/libplatform.h"
#include <v8.h>
#include <ole2.h>
#include <Wininet.h>
#include <limits.h>
#include <wincon.h>

#include <string>
#include <vector>
#include <map>
#include <functional>
#include <limits>
#include <thread>

#include <zconf.h>
#include <zlib.h>
#include <contrib/minizip/unzip.h>

#include "dds_lib/dds.h"
#include <assert.h>
#include "dds_lib/dds_lib.h"

#include "quickjs.h"
#include "quickjs-libc.h"

inline bool JS_IsArray(JSContext*, JSValueConst val) {
	return JS_IsArray(val);
}

#define QJSBRIDGE_QUICKJS_NG
#include "qjsbridge.hpp"

using namespace v8;

#ifdef _OPENMP
	#include <omp.h>
	#ifdef AVS_PLUGIN
//		#define _OPENMP_AVS
	#endif
#endif

#define LOG_FREQ	16.0
#define BUF_SIZE	1024
#define ToRAD		(M_PI / 180)
#define SLIDER_TIME	(LOG_FREQ / 1000.0)
#define NaN			std::numeric_limits<double>::quiet_NaN()
#define INVALID_INT		INT_MIN

#define WIDEN_(x) L##x
#define WIDEN(x)  WIDEN_(x)

static char *StringNew(char *&szDst, const char *szSrc){
	if(szDst == szSrc) return(szDst);
	if(szDst) delete [] szDst;
	
	if(szSrc == nullptr || *szSrc == '\0'){
		return szDst = nullptr;
	}
	szDst = new char[strlen(szSrc) + 1];
	return strcpy(szDst, szSrc);
}

static LPWSTR StringNew(LPWSTR& szDst, LPCWSTR szSrc){
	if(szDst == szSrc) return(szDst);
	if(szDst) delete [] szDst;
	
	if(szSrc == nullptr || *szSrc == '\0'){
		return szDst = nullptr;
	}
	szDst = new WCHAR[wcslen(szSrc) + 1];
	return wcscpy(szDst, szSrc);
}

static LPWSTR StringNew(LPWSTR& szDst, const char *szSrc){
	if(szDst) delete [] szDst;
	
	if(szSrc == nullptr || *szSrc == '\0'){
		return szDst = nullptr;
	}
	// SJIS->WCHAR 変換
	int iLen = MultiByteToWideChar(
		CP_ACP,				// コードページ
		0,					// 文字の種類を指定するフラグ
		szSrc,				// マップ元文字列のアドレス
		-1,					// マップ元文字列のバイト数
		nullptr,				// マップ先ワイド文字列を入れるバッファのアドレス
		0					// バッファのサイズ
	);
	szDst = new WCHAR[iLen];
	
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

static char *StringNew(char *&szDst, LPCWSTR &szSrc){
	if(szDst) delete [] szDst;
	
	if(szSrc == nullptr || *szSrc == '\0'){
		return szDst = nullptr;
	}
	// SJIS->WCHAR 変換
	int iLen = WideCharToMultiByte(
		CP_ACP,				// コードページ
		0,					// 文字の種類を指定するフラグ
		szSrc,				// マップ元文字列のアドレス
		-1,					// マップ元文字列のバイト数
		nullptr,				// マップ先ワイド文字列を入れるバッファのアドレス
		0,					// バッファのサイズ
		nullptr, nullptr
	);
	szDst = new char[iLen];
	
	WideCharToMultiByte(
		CP_ACP,				// コードページ
		0,					// 文字の種類を指定するフラグ
		szSrc,				// マップ元文字列のアドレス
		-1,					// マップ元文字列のバイト数
		szDst,				// マップ先ワイド文字列を入れるバッファのアドレス
		iLen,				// バッファのサイズ
		nullptr, nullptr
	);
	
	return szDst;
}

inline std::wstring MultiByteToWide(const char* text, UINT codePage) {
	if (text == nullptr || *text == '\0') {
		return {};
	}

	const int size = MultiByteToWideChar(
		codePage, 0,
		text, -1,
		nullptr, 0
	);

	if (size <= 0) {
		return {};
	}

	std::wstring result(static_cast<size_t>(size), L'\0');

	MultiByteToWideChar(
		codePage, 0,
		text, -1,
		result.data(), size
	);

	result.resize(static_cast<size_t>(size - 1));
	return result;
}

inline std::string WideToMultiByte(const wchar_t* text, UINT codePage) {
	if (text == nullptr || *text == L'\0') {
		return {};
	}

	const int size = WideCharToMultiByte(
		codePage, 0,
		text, -1,
		nullptr, 0,
		nullptr, nullptr
	);

	if (size <= 0) {
		return {};
	}

	std::string result(static_cast<size_t>(size), '\0');

	WideCharToMultiByte(
		codePage, 0,
		text, -1,
		result.data(), size,
		nullptr, nullptr
	);

	result.resize(static_cast<size_t>(size - 1));
	return result;
}

// ANSI -> Wide
inline std::wstring AnsiToWide(const char* text) { return MultiByteToWide(text, CP_ACP); }
inline std::wstring AnsiToWide(const std::string& text) { return AnsiToWide(text.c_str()); }

// Wide -> ANSI
inline std::string WideToAnsi(const wchar_t* text) { return WideToMultiByte(text, CP_ACP); }
inline std::string WideToAnsi(const std::wstring& text) { return WideToAnsi(text.c_str()); }

// UTF-8 -> Wide
inline std::wstring Utf8ToWide(const char* text) { return MultiByteToWide(text, CP_UTF8); }
inline std::wstring Utf8ToWide(const std::string& text) { return Utf8ToWide(text.c_str()); }

// Wide -> UTF-8
inline std::string WideToUtf8(const wchar_t* text) { return WideToMultiByte(text, CP_UTF8); }
inline std::string WideToUtf8(const std::wstring& text) { return WideToUtf8(text.c_str()); }

// ANSI -> UTF-8
inline std::string AnsiToUtf8(const char* text) { return WideToUtf8(AnsiToWide(text)); }
inline std::string AnsiToUtf8(const std::string& text) { return AnsiToUtf8(text.c_str()); }

// UTF-8 -> ANSI
inline std::string Utf8ToAnsi(const char* text) { return WideToAnsi(Utf8ToWide(text)); }
inline std::string Utf8ToAnsi(const std::string& text) { return Utf8ToAnsi(text.c_str()); }
