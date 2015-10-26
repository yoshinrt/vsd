/*****************************************************************************
	
	VSD -- vehicle data logger system  Copyright(C) by DDS
	
	CScript.h - JavaScript
	
*****************************************************************************/

#pragma once

#include "CSemaphore.h"
#include "CVsdLog.h"
#include "CV8Map.h"
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

#define V8Int( i )			Int32::New( Isolate::GetCurrent(), i )
#define LocalUndefined()	Local<Value>( *Undefined( m_pIsolate ))

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

class CScript {
  public:
	CScript( CVsdFilter *pVsd );
	~CScript( void );
	
	UINT Initialize( LPCWSTR wszFileName = NULL );
	UINT RunFile( LPCWSTR szFileName );
	UINT RunFileCore( LPCWSTR szFileName );
	UINT Run( LPCWSTR szFunc, BOOL bNoFunc = FALSE );
	UINT Run_s( LPCWSTR szFunc, LPCWSTR str0, BOOL bNoFunc = FALSE );
	UINT Run_ss( LPCWSTR szFunc, LPCWSTR str0, LPCWSTR str1, BOOL bNoFunc = FALSE );
	UINT RunArg( LPCWSTR szFunc, int iArgNum, Handle<Value> Args[], BOOL bNoFunc = FALSE );
	
	static LPWSTR ReportException( LPWSTR pMsg, TryCatch& try_catch );
	
	CVsdFilter	*m_pVsd;	// エ…
	
	Persistent<Context> m_Context;
	Isolate	*m_pIsolate;
	
	LPWSTR m_szErrorMsg;
	UINT m_uError;
	
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
	static Handle<Value> Sprintf( const FunctionCallbackInfo<Value>& args );	// !js_func
	static LPWSTR SprintfSub( const FunctionCallbackInfo<Value>& args );
	
	// 距離取得
	static double GetDistanceByLngLat(	// !js_func
		double dLong0, double dLati0,
		double dLong1, double dLati1
	){
		return CVsdLog::GPSLogGetLength( dLong0, dLati0, dLong1, dLati1 );
	}
	
	// this へのアクセスヘルパ
	template<typename T>
	static T* GetThis( Local<Object> handle ){
		if( handle->GetInternalField( 0 )->IsUndefined()){
			V8TypeError( "Object is undefined" );
			return NULL;
		}
		
		void* pThis = Local<External>::Cast( handle->GetInternalField( 0 ))->Value();
		return static_cast<T*>( pThis );
	}
	
	static CScript *GetCScript( Local<Object> handle ){
		HandleScope handle_scope( Isolate::GetCurrent());
		
		return CV8Map::Cast( handle )[ "__CScript" ].GetObj<CScript>();
	}
	
	// 引数の数チェック
	static BOOL CheckArgs( BOOL cond ){
		if( !( cond )){
			V8ErrorNumOfArg();
			return TRUE;
		}
		return FALSE;
	}
	
	static BOOL CheckClass( Local<Object> obj, char *name, char *msg ){
		if( strcmp( *( String::Utf8Value )( obj->GetConstructorName()), name )){
			V8TypeError( msg );
			return TRUE;
		}
		return FALSE;
	}
	
	// msg であればスルーパス，ERR_ID であればメッセージを返す
	static Handle<String> ErrMsgOrID( char *szMsg ){
		return String::NewFromOneByte( Isolate::GetCurrent(), ( uint8_t *)szMsg );
	}
	
	static Handle<String> ErrMsgOrID( UINT uID ){
		return String::NewFromTwoByte( Isolate::GetCurrent(), ( uint16_t *)m_szErrorMsgID[ uID ]);
	}
	
  private:
	//CSemaphore *m_pSemaphore;
	static LPCWSTR m_szErrorMsgID[];
	
	void Dispose( void );
};
