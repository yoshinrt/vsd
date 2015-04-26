/*****************************************************************************
	
	VSD -- vehicle data logger system  Copyright(C) by DDS
	
	CScript.h - JavaScript
	
*****************************************************************************/

#pragma once

#include "CSemaphore.h"
#include "error_code.h"

typedef v8::Local<v8::Array> v8Array;

class CVsdFilter;

class CScript {
  public:
	CScript( CVsdFilter *pVsd );
	~CScript( void );
	
	void Initialize( void );
	UINT RunFile( LPCWSTR szFileName );
	UINT RunFileCore( LPCWSTR szFileName );
	UINT Run( LPCWSTR szFunc, BOOL bNoFunc = FALSE );
	UINT Run_s( LPCWSTR szFunc, LPCWSTR str0, BOOL bNoFunc = FALSE );
	UINT Run_ss( LPCWSTR szFunc, LPCWSTR str0, LPCWSTR str1, BOOL bNoFunc = FALSE );
	UINT RunArg( LPCWSTR szFunc, int iArgNum, v8::Handle<v8::Value> Args[], BOOL bNoFunc = FALSE );
	
	static LPWSTR ReportException( LPWSTR pMsg, v8::TryCatch& try_catch );
	
	CVsdFilter	*m_pVsd;	// エ…
	
	v8::Persistent<v8::Context> m_Context;
	v8::Isolate	*m_pIsolate;
	
	LPWSTR m_szErrorMsg;
	UINT m_uError;
	
	LPCWSTR GetErrorMessage( void ){
		return m_szErrorMsg ? m_szErrorMsg : m_szErrorMsgID[ m_uError ];
	}
	
	UINT InitLogReader( void );
	
	// Global オブジェクト
	static void DebugPrint( LPCWSTR strMsg ){	// !js_func
		OutputDebugStringW( strMsg );
		OutputDebugStringW( L"\n" );
	}
	
	static void MessageBox( LPCWSTR strMsg ){	// !js_func
		MessageBoxW(
			NULL, strMsg,
			L"VSD filter JavaScript message",
			MB_OK
		);
	}
	
	static void Print( LPCWSTR strMsg );	// !js_func
	
	// this へのアクセスヘルパ
	template<typename T>
	static T* GetThis( v8::Local<v8::Object> handle ){
		if( handle->GetInternalField( 0 )->IsUndefined()){
			v8::ThrowException( v8::Exception::TypeError( v8::String::New( "Object is undefined" )));
			return NULL;
		}
		
		void* pThis = v8::Local<v8::External>::Cast( handle->GetInternalField( 0 ))->Value();
		return static_cast<T*>( pThis );
	}
	
	// 引数の数チェック
	static BOOL CheckArgs( BOOL cond ){
		if( !( cond )){
			v8::ThrowException( v8::Exception::Error( v8::String::New(
				"invalid number of args"
			)));
			return TRUE;
		}
		return FALSE;
	}
	
	static BOOL CheckClass( v8::Local<v8::Object> obj, char *name, char *msg ){
		if( strcmp( *( v8::String::AsciiValue )( obj->GetConstructorName()), name )){
			v8::ThrowException( v8::Exception::TypeError( v8::String::New( msg )));
			return TRUE;
		}
		return FALSE;
	}
	
  private:
	//CSemaphore *m_pSemaphore;
	static LPCWSTR m_szErrorMsgID[];
	
	void Dispose( void );
};
