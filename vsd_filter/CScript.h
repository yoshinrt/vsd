/*****************************************************************************
	
	VSD -- vehicle data logger system  Copyright(C) by DDS
	
	CScript.h - JavaScript
	
*****************************************************************************/

#pragma once

#include "CSemaphore.h"
#include "CVsdLog.h"
#include "CV8Map.h"
#include "CV8If.h"
#include "error_code.h"

/*** エラー *****************************************************************/

#define V8AnyError( type, msg ) Isolate::GetCurrent()->ThrowException( Exception::type( CScript::ErrMsgOrID( msg )))
#define V8RangeError( msg )		V8AnyError( RangeError, msg )
#define V8ReferenceError( msg ) V8AnyError( ReferenceError, msg )
#define V8SyntaxError( msg )	V8AnyError( SyntaxError, msg )
#define V8TypeError( msg )		V8AnyError( TypeError, msg )
#define V8Error( msg )			V8AnyError( Error, msg )

#define V8ErrorNumOfArg()		V8SyntaxError( ERR_NUM_OF_ARG )

/****************************************************************************/

typedef Local<Array> v8Array;

class CVsdFilter;

class ArrayBufferAllocator : public ArrayBuffer::Allocator {
public:
	virtual void* Allocate(size_t length) {
		void* data = AllocateUninitialized(length);
		return data == NULL ? data : memset(data, 0, length);
	}
	virtual void* AllocateUninitialized(size_t length) { return malloc(length); }
	virtual void Free(void* data, size_t) { free(data); }
};

class CScriptRoot {
  public:
	CScriptRoot(){
		#ifdef DEBUG
			char *opt[] = { "hoge", "--expose_gc" };
			int i = 2;
			V8::SetFlagsFromCommandLine( &i, opt, 0 );
		#endif
		
		// Initialize V8.
		V8::InitializeICU();
		V8::InitializeExternalStartupData( "." );
		m_Platform = platform::CreateDefaultPlatform();
		V8::InitializePlatform( m_Platform );
		V8::Initialize();
		m_CreateParams.array_buffer_allocator = &m_Allocator;
		m_pIsolate = Isolate::New( m_CreateParams );
	}
	
	~CScriptRoot(){
		m_pIsolate->Dispose();
		V8::Dispose();
		V8::ShutdownPlatform();
		delete m_Platform;
	}
	
	Isolate *m_pIsolate;

  private:
	Platform* m_Platform;
	ArrayBufferAllocator m_Allocator;
	Isolate::CreateParams m_CreateParams;
};

class CScript : public CV8If {
  public:
	CScript( CVsdFilter *pVsd );
	~CScript( void );
	
	Local<Context> GetContext( void ){
		return Local<Context>::New( m_pIsolate, m_Context );
	}
	
	UINT Initialize( LPCWSTR wszFileName = NULL );
	UINT RunFile( LPCWSTR szFileName );
	UINT RunFileCore( LPCWSTR szFileName );
	UINT Run( LPCWSTR szFunc, BOOL bNoFunc = FALSE );
	UINT Run_s( LPCWSTR szFunc, LPCWSTR str0, BOOL bNoFunc = FALSE );
	UINT Run_ss( LPCWSTR szFunc, LPCWSTR str0, LPCWSTR str1, BOOL bNoFunc = FALSE );
	UINT RunArg( LPCWSTR szFunc, int iArgNum, Handle<Value> Args[], BOOL bNoFunc = FALSE );
	
	static LPWSTR ReportException( LPWSTR pMsg, TryCatch& try_catch );
	
	LPCWSTR GetErrorMessage( void ){
		return m_szErrorMsg ? m_szErrorMsg : m_szErrorMsgID[ m_uError ];
	}
	
	UINT InitLogReader( void );
	
	// Global オブジェクト
	void Include( LPCWSTR wszFileName );	// !js_func
	static void DebugPrint( const FunctionCallbackInfo<Value>& args );	// !js_func
	static int MessageBox(	// !js_func
		LPCWSTR szMsg,
		LPCWSTR szCaption,	// !default:NULL
		UINT	uType		// !default:MB_OK
	);
	
	static void Print( LPCWSTR szMsg );	// !js_func
	static void Printf( const FunctionCallbackInfo<Value>& args );	// !js_func
	static void Sprintf( const FunctionCallbackInfo<Value>& args );	// !js_func
	static LPWSTR SprintfSub( const FunctionCallbackInfo<Value>& args );
	
	// 距離取得
	static double GetDistanceByLngLat(	// !js_func
		double dLong0, double dLati0,
		double dLong1, double dLati1
	){
		return CVsdLog::GPSLogGetLength( dLong0, dLati0, dLong1, dLati1 );
	}
	
	static CScript *GetCScript( Local<Object> handle ){
		HandleScope handle_scope( Isolate::GetCurrent());
		
		return CV8Map::Cast( handle )[ "__CScript" ].GetObj<CScript>();
	}
	
	// msg であればスルーパス，ERR_ID であればメッセージを返す
	static Handle<String> ErrMsgOrID( char *szMsg ){
		return String::NewFromOneByte( Isolate::GetCurrent(), ( uint8_t *)szMsg );
	}
	
	static Handle<String> ErrMsgOrID( UINT uID ){
		return String::NewFromTwoByte( Isolate::GetCurrent(), ( uint16_t *)m_szErrorMsgID[ uID ]);
	}
	
	CVsdFilter	*m_pVsd;	// エ…
	Persistent<Context,CopyablePersistentTraits<Context>> m_Context;
	Isolate *m_pIsolate;
	LPWSTR m_szErrorMsg;
	UINT m_uError;
	
  private:
	static LPCWSTR m_szErrorMsgID[];
	
	void Dispose( void );
};
