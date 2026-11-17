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

#include "dds_lib/dds.h"
#include <assert.h>
#include "dds_lib/dds_lib.h"

using namespace v8;

#ifdef _OPENMP
	#include <omp.h>
	#ifdef AVS_PLUGIN
//		#define _OPENMP_AVS
	#endif
#endif

#define LOG_FREQ	16.0
#define BUF_SIZE	1024
#define ToRAD		( M_PI / 180 )
#define SLIDER_TIME	( LOG_FREQ / 1000.0 )
#define NaN			std::numeric_limits<double>::quiet_NaN()
#ifdef PUBLIC_MODE
	#define USE_TURN_R
#endif
#define INVALID_INT		INT_MIN

static char *StringNew( char *&szDst, const char *szSrc ){
	if( szDst == szSrc ) return( szDst );
	if( szDst ) delete [] szDst;
	
	if( szSrc == NULL || *szSrc == '\0' ){
		return szDst = NULL;
	}
	szDst = new char[ strlen( szSrc ) + 1 ];
	return strcpy( szDst, szSrc );
}

static LPWSTR StringNew( LPWSTR& szDst, LPCWSTR szSrc ){
	if( szDst == szSrc ) return( szDst );
	if( szDst ) delete [] szDst;
	
	if( szSrc == NULL || *szSrc == '\0' ){
		return szDst = NULL;
	}
	szDst = new WCHAR[ wcslen( szSrc ) + 1 ];
	return wcscpy( szDst, szSrc );
}

static LPWSTR StringNew( LPWSTR& szDst, const char *szSrc ){
	if( szDst ) delete [] szDst;
	
	if( szSrc == NULL || *szSrc == '\0' ){
		return szDst = NULL;
	}
	// SJIS->WCHAR 変換
	int iLen = MultiByteToWideChar(
		CP_ACP,				// コードページ
		0,					// 文字の種類を指定するフラグ
		szSrc,				// マップ元文字列のアドレス
		-1,					// マップ元文字列のバイト数
		NULL,				// マップ先ワイド文字列を入れるバッファのアドレス
		0					// バッファのサイズ
	);
	szDst = new WCHAR[ iLen ];
	
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

static char *StringNew( char *&szDst, LPCWSTR &szSrc ){
	if( szDst ) delete [] szDst;
	
	if( szSrc == NULL || *szSrc == '\0' ){
		return szDst = NULL;
	}
	// SJIS->WCHAR 変換
	int iLen = WideCharToMultiByte(
		CP_ACP,				// コードページ
		0,					// 文字の種類を指定するフラグ
		szSrc,				// マップ元文字列のアドレス
		-1,					// マップ元文字列のバイト数
		NULL,				// マップ先ワイド文字列を入れるバッファのアドレス
		0,					// バッファのサイズ
		NULL, NULL
	);
	szDst = new char[ iLen ];
	
	WideCharToMultiByte(
		CP_ACP,				// コードページ
		0,					// 文字の種類を指定するフラグ
		szSrc,				// マップ元文字列のアドレス
		-1,					// マップ元文字列のバイト数
		szDst,				// マップ先ワイド文字列を入れるバッファのアドレス
		iLen,				// バッファのサイズ
		NULL, NULL
	);
	
	return szDst;
}
