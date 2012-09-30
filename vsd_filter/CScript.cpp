/*****************************************************************************
	
	VSD -- vehicle data logger system  Copyright(C) by DDS
	
	CScript.cpp - JavaScript
	
*****************************************************************************/

#include "StdAfx.h"

#include "CScript.h"
#include "CVsdFilter.h"
#include "CVsdFile.h"
#include "ScriptIF.h"

using namespace v8;

/*** static メンバ（；´д⊂）***********************************************/

CVsdFilter *CScript::m_pVsd;

LPCWSTR CScript::m_szErrorMsgID[] = {
	#define DEF_ERROR( id, msg )	L##msg,
	#include "def_error.h"
};

/*** Exception メッセージ表示 ***********************************************/

#define MSGBUF_SIZE	( 4 * 1024 )

void CScript::ReportException( TryCatch* try_catch ){
	HandleScope handle_scope;
	String::Value exception( try_catch->Exception());
	Handle<Message> message = try_catch->Message();
	
	if( !m_szErrorMsg ) m_szErrorMsg = new WCHAR[ MSGBUF_SIZE ];
	LPWSTR p = m_szErrorMsg;
	
	if ( message.IsEmpty()){
		// V8 didn't provide any extra information about this error; just
		// print the exception.
		swprintf( p, MSGBUF_SIZE, L"%s\n", *exception );
		p = wcschr( p, '\0' );
	}else{
		// Print ( filename ):( line number ): ( message ).
		String::Value filename( message->GetScriptResourceName());
		int linenum = message->GetLineNumber();
		swprintf( p, MSGBUF_SIZE - ( p - m_szErrorMsg ), L"%s:%i: %s\n", *filename, linenum, *exception );
		p = wcschr( p, '\0' );
		// Print line of source code.
		String::Value sourceline( message->GetSourceLine());
		swprintf( p, MSGBUF_SIZE - ( p - m_szErrorMsg ), L"%s\n", *sourceline );
		p = wcschr( p, '\0' );
		// Print wavy underline ( GetUnderline is deprecated ).
		int start = message->GetStartColumn();
		for ( int i = 0; i < start; i++ ){
			*p++ = L' ';
		}
		int end = message->GetEndColumn();
		for ( int i = start; i < end; i++ ){
			*p++ = L'^';
		}
		*p++ = L'\n';
		*p = L'\0';
		
		/*
		String::Utf8Value stack_trace( try_catch->StackTrace());
		if ( stack_trace.length() > 0 ){
			String::Value stack_trace_string( stack_trace );
			swprintf( p, MSGBUF_SIZE - ( p - m_szErrorMsg ), L"%s\n", *stack_trace_string );
			p = wcschr( p, '\0' );
		}
		*/
	}
}

/*** コンストラクタ *********************************************************/

CScript::CScript( CVsdFilter *pVsd ){
	DebugMsgD( ":CScript::CScript():%X\n", GetCurrentThreadId());
	
	m_pIsolate = v8::Isolate::New();
	v8::Isolate::Scope IsolateScope( m_pIsolate );
	
	DebugMsgD( ":CScript::CScript():m_pIsolate = %X\n", m_pIsolate );
	
	DebugMsgD( ":CScript::CScript():m_Context\n" );
	m_Context.Clear();
	
	if( pVsd ) m_pVsd = pVsd;
	
	m_szErrorMsg	= NULL;
	m_uError		= ERR_OK;
}

/*** デストラクタ ***********************************************************/

CScript::~CScript(){
	DebugMsgD( ":CScript::~CScript():%X\n", GetCurrentThreadId());
	DebugMsgD( ":CScript::~CScript():m_pIsolate = %X\n", m_pIsolate );
	
	{
		v8::Isolate::Scope IsolateScope( m_pIsolate );
		m_Context.Dispose();
		while( !v8::V8::IdleNotification());
	}
	m_pIsolate->Dispose();
	
	delete [] m_szErrorMsg;
}

/*** JavaScript オブジェクトディスポーザ ************************************/

void CScript::Dispose( void ){
	if( Run( L"DisposeAll", TRUE )){
		m_pVsd->DispErrorMessage( GetErrorMessage());
	}
}

/*** デバッグ用 *************************************************************/

static v8::Handle<v8::Value> Func_DebugPrintD( const v8::Arguments& args ){
	v8::String::AsciiValue str( args[ 0 ] );
	DebugMsgD( "%s\n", *str );
	return v8::Undefined();
}

static v8::Handle<v8::Value> Func_DebugPrintW( const v8::Arguments& args ){
	v8::String::Value str( args[ 0 ] );
	
	MessageBoxW(
		NULL, ( LPCWSTR )*str,
		L"VSD filter JavaScript message",
		MB_OK
	);
	return v8::Undefined();
}

/*** JavaScript interface のセットアップ ************************************/

void CScript::Initialize( void ){
	v8::Isolate::Scope IsolateScope( m_pIsolate );
	
	// 準備
	HandleScope handle_scope;
	
	// グローバルオブジェクトの生成
	Handle<ObjectTemplate> global = ObjectTemplate::New();
	
	// Image クラス登録
	CVsdImageIF::InitializeClass( global );
	CVsdFontIF::InitializeClass( global );
	CVsdFilterIF::InitializeClass( global );
	CVsdFileIF::InitializeClass( global );
	
	// デバッグ用の DebugPrint* 登録
	global->Set( v8::String::New( "DebugPrintD" ),  v8::FunctionTemplate::New( Func_DebugPrintD ));
	global->Set( v8::String::New( "MessageBox" ),  v8::FunctionTemplate::New( Func_DebugPrintW ));
	
	// グローバルオブジェクトから環境を生成
	m_Context = Context::New( NULL, global );
}

/*** スクリプトファイルの実行 ***********************************************/

UINT CScript::RunFile( LPCWSTR szFileName ){
	v8::Isolate::Scope IsolateScope( m_pIsolate );
	
	HandleScope handle_scope;
	
	// 環境からスコープを生成
	Context::Scope context_scope( m_Context );
	
	TryCatch try_catch;
	
	FILE *fp;
	{
		// スクリプト ロード
		CPushDir push_dir( m_pVsd->m_szPluginDirA );
		fp = _wfopen( szFileName, L"r" );
	}
	
	if( fp == NULL ) return m_uError = ERR_FILE_NOT_FOUND;
	
	// ファイルサイズ取得
	fseek( fp, 0, SEEK_END );
	fpos_t Size;
	fgetpos( fp, &Size );
	fseek( fp, 0, SEEK_SET );
	
	char *szBuf = new char[ ( UINT )Size + 1 ];
	
	int iReadSize = fread( szBuf, 1, ( size_t )Size, fp );
	fclose( fp );
	szBuf[ iReadSize ] = '\0';
	
	Handle<Script> script = Script::Compile(
		String::New( szBuf ), String::New(( uint16_t *)szFileName )
	);
	
	delete [] szBuf;
	
	if( script.IsEmpty()){
		// Print errors that happened during compilation.
		ReportException( &try_catch );
		return m_uError = ERR_SCRIPT;
	}
	
	// とりあえず初期化処理
	Handle<Value> result = script->Run();
	
	if( try_catch.HasCaught()){
		ReportException( &try_catch );
		return m_uError = ERR_SCRIPT;
	}
	
	// ガーベッジコレクション?
	//while( !v8::V8::IdleNotification());
	
	return m_uError = ERR_OK;
}

/*** function 名指定実行，引数なし ******************************************/

UINT CScript::Run( LPCWSTR szFunc, BOOL bNoFunc ){
	v8::Isolate::Scope IsolateScope( m_pIsolate );
	HandleScope handle_scope;
	Context::Scope context_scope( m_Context );
	
	return RunArg( szFunc, 0, NULL, bNoFunc );
}

UINT CScript::Run_ss( LPCWSTR szFunc, LPCWSTR str0, LPCWSTR str1, BOOL bNoFunc ){
	v8::Isolate::Scope IsolateScope( m_pIsolate );
	HandleScope handle_scope;
	Context::Scope context_scope( m_Context );
	
	Handle<Value> Args[] = {
		str0 ? String::New(( uint16_t *)str0 ) : v8::Undefined(),
		str1 ? String::New(( uint16_t *)str1 ) : v8::Undefined()
	};
	return RunArg( szFunc, 2, Args, bNoFunc );
}

UINT CScript::RunArg( LPCWSTR szFunc, int iArgNum, Handle<Value> Args[], BOOL bNoFunc ){
	TryCatch try_catch;
	
	Local<Function> hFunction = Local<Function>::Cast( m_Context->Global()->Get( String::New(( uint16_t *)szFunc )));
	if( hFunction->IsUndefined()){
		if( bNoFunc ) return ERR_OK;
		if( !m_szErrorMsg ) m_szErrorMsg = new WCHAR[ MSGBUF_SIZE ];
		
		swprintf( m_szErrorMsg, MSGBUF_SIZE, L"Undefined function \"%s()\"", szFunc );
		return m_uError = ERR_SCRIPT;
	}
	Handle<Value> result = hFunction->Call( hFunction, iArgNum, Args );
	
	if( try_catch.HasCaught()){
		ReportException( &try_catch );
		return m_uError = ERR_SCRIPT;
	}
	
	/*
	if( !result->IsUndefined()) {
		// If all went well and the result wasn't undefined then print
		// the returned value.
		return m_uError = result->Int32Value();
	}
	*/
	// ガーベッジコレクション?
	//while( !v8::V8::IdleNotification());
	//v8::V8::IdleNotification();
	
	return m_uError = ERR_OK;
}

/*** ログリード by JavaScript **********************************************/

BOOL LogReaderCallback( const char *szPath, const char *szFile, void *pParam ){
	if( !IsExt( szFile, "js" )) return TRUE;
	
	CScript &Script = *( CScript *)pParam;
	
	char szBuf[ MAX_PATH + 1 ];
	strcat( strcpy( szBuf, szPath ), szFile );
	
	LPWSTR pFile = NULL;
	Script.RunFile( StringNew( pFile, szBuf ));
	delete pFile;
	
	if( Script.m_uError ){
		// エラー
		Script.m_pVsd->DispErrorMessage( Script.GetErrorMessage());
		return FALSE;
	}
	
	return TRUE;
}

UINT CScript::InitLogReader( void ){
	Initialize();
	
	{
		v8::Isolate::Scope IsolateScope( m_pIsolate );
		v8::HandleScope handle_scope;
		v8::Context::Scope context_scope( m_Context );
		
		// log 用の global array 登録
		m_Context->Global()->Set( v8::String::New( "Log" ), v8::Array::New( 0 ));
		m_Context->Global()->Set( v8::String::New( "LogReaderInfo" ), v8::Array::New( 0 ));
	}
	
	// スクリプトロード
	char szBuf[ MAX_PATH + 1 ];
	strcpy( szBuf, m_pVsd->m_szSkinDirA );
	strcat( szBuf, LOG_READER_DIR "\\" );
	if( !ListTree( szBuf, "*", LogReaderCallback, this )){
		return ERR_FILE_NOT_FOUND;
	}
	
	return ERR_OK;
}
