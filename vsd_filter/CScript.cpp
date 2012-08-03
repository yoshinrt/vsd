/*****************************************************************************
	
	VSD -- vehicle data logger system  Copyright(C) by DDS
	
	CScript.cpp - JavaScript
	
*****************************************************************************/

#include "StdAfx.h"

#include "dds.h"
#include "CScript.h"
#include "CVsdFont.h"
#include "CVsdLog.h"
#include "pixel.h"
#include "CVsdImage.h"
#include "CVsdFilter.h"
#include "error_code.h"
#include "ScriptIF.h"

using namespace v8;

// Extracts a C string from a V8 Utf8Value.
const char* CScript::ToCString( const String::Utf8Value& value ){
	return *value ? *value : "<string conversion failed>";
}

void CScript::ReportException( TryCatch* try_catch ){
	HandleScope handle_scope;
	String::Utf8Value exception( try_catch->Exception());
	const char* exception_string = ToCString( exception );
	Handle<Message> message = try_catch->Message();
	
	if( !m_szErrorMsg ) m_szErrorMsg = new char[ 10240 ];
	char *p = m_szErrorMsg;
	
	if ( message.IsEmpty()){
		// V8 didn't provide any extra information about this error; just
		// print the exception.
		sprintf( p, "%s\n", exception_string );
		p = strchr( p, '\0' );
	}else{
		// Print ( filename ):( line number ): ( message ).
		String::Utf8Value filename( message->GetScriptResourceName());
		const char* filename_string = ToCString( filename );
		int linenum = message->GetLineNumber();
		sprintf( p, "%s:%i: %s\n", filename_string, linenum, exception_string );
		p = strchr( p, '\0' );
		// Print line of source code.
		String::Utf8Value sourceline( message->GetSourceLine());
		const char* sourceline_string = ToCString( sourceline );
		sprintf( p, "%s\n", sourceline_string );
		p = strchr( p, '\0' );
		// Print wavy underline ( GetUnderline is deprecated ).
		int start = message->GetStartColumn();
		for ( int i = 0; i < start; i++ ){
			sprintf( p, " " );
			p = strchr( p, '\0' );
		}
		int end = message->GetEndColumn();
		for ( int i = start; i < end; i++ ){
			sprintf( p, "^" );
			p = strchr( p, '\0' );
		}
		sprintf( p, "\n" );
		p = strchr( p, '\0' );
		
		/*
		String::Utf8Value stack_trace( try_catch->StackTrace());
		if ( stack_trace.length() > 0 ){
			const char* stack_trace_string = ToCString( stack_trace );
			sprintf( p, "%s\n", stack_trace_string );
			p = strchr( p, '\0' );
		}
		*/
	}
}

/*** static メンバ（；´д⊂）***********************************************/

CVsdFilter *CScript::m_Vsd;

/*** コンストラクタ *********************************************************/

CScript::CScript( CVsdFilter *pVsd ){
	m_context.Clear();
	m_Vsd			= pVsd;
	m_szErrorMsg	= NULL;
	m_bError		= FALSE;
}

/*** デストラクタ ***********************************************************/

CScript::~CScript(){
	m_context.Dispose();
	delete [] m_szErrorMsg;
}

/*** ロード・コンパイル *****************************************************/

#define SCRIPT_SIZE	( 64 * 1024 )

UINT CScript::Initialize( char *szFileName ){
	// 準備
	HandleScope handle_scope;
	
	// グローバルオブジェクトの生成
	Handle<ObjectTemplate> global = ObjectTemplate::New();
	
	// Image クラス登録
	CVsdImageIF::InitializeClass( global );
	CVsdFontIF::InitializeClass( global );
	CVsdFilterIF::InitializeClass( global );
	
	// グローバルオブジェクトから環境を生成
	m_context = Context::New( NULL, global );
	
	TryCatch try_catch;
	
	char *szBuf = new char[ SCRIPT_SIZE ];
	
	// スクリプト ロード
	FILE *fp;
	if(( fp = fopen( szFileName, "r" )) == NULL ){
		// エラー処理
		return FALSE;
	}
	
	int iReadSize = fread( szBuf, 1, SCRIPT_SIZE, fp );
	fclose( fp );
	szBuf[ iReadSize ] = '\0';
	
	Handle<String> ScriptBody = String::New( szBuf );
	delete [] szBuf;
	
	// 環境からスコープを生成
	Context::Scope context_scope( m_context );
	
	Handle<Script> script = Script::Compile(
		ScriptBody, String::New( szFileName )
	);
	
	if( script.IsEmpty()){
		// Print errors that happened during compilation.
		ReportException( &try_catch );
		return FALSE;
	}
	
	// とりあえず初期化処理
	Handle<Value> result = script->Run();
	
	if( result.IsEmpty()){
		assert( try_catch.HasCaught());
		// Print errors that happened during execution.
		ReportException( &try_catch );
		return ERR_SCRIPT;
	}
	
	assert( !try_catch.HasCaught());
	if( !result->IsUndefined()) {
		// If all went well and the result wasn't undefined then print
		// the returned value.
		return result->Int32Value();
	}
	return ERR_OK;
}

/*** function 名指定実行，引数なし ******************************************/

UINT CScript::Run( const char *szFunc ){
	HandleScope handle_scope;
	Context::Scope context_scope( m_context );
	
	TryCatch try_catch;
	
	Local<Function> hFunction = Local<Function>::Cast( m_context->Global()->Get( String::New( szFunc )));
	if( hFunction->IsUndefined()){
		
		if( !m_szErrorMsg ) m_szErrorMsg = new char[ 10240 ];
		
		sprintf( m_szErrorMsg, "Undefined function \"%s()\"", szFunc );
		return ERR_SCRIPT;
	}
	Handle<Value> result = hFunction->Call( hFunction, 0, 0 );
	
	if( result.IsEmpty()){
		//assert( try_catch.HasCaught());
		try_catch.HasCaught();
		// Print errors that happened during execution.
		ReportException( &try_catch );
		return ERR_SCRIPT;
	}
	
	assert( !try_catch.HasCaught());
	if( !result->IsUndefined()) {
		// If all went well and the result wasn't undefined then print
		// the returned value.
		return result->Int32Value();
	}
	return ERR_OK;
}
