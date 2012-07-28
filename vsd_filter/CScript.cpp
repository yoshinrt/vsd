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

using namespace v8;

// 関数オブジェクト print の実体 
Handle<Value> Func_print(const Arguments& args) {
  String::AsciiValue str(args[0]);
  DebugMsgD("%s\n", *str);
  return Undefined();
}

// Extracts a C string from a V8 Utf8Value.
const char* ToCString(const String::Utf8Value& value) {
  return *value ? *value : "<string conversion failed>";
}

void ReportException(TryCatch* try_catch) {
  HandleScope handle_scope;
  String::Utf8Value exception(try_catch->Exception());
  const char* exception_string = ToCString(exception);
  Handle<Message> message = try_catch->Message();
  if (message.IsEmpty()) {
    // V8 didn't provide any extra information about this error; just
    // print the exception.
    DebugMsgD("%s\n", exception_string);
  } else {
    // Print (filename):(line number): (message).
    String::Utf8Value filename(message->GetScriptResourceName());
    const char* filename_string = ToCString(filename);
    int linenum = message->GetLineNumber();
    DebugMsgD("%s:%i: %s\n", filename_string, linenum, exception_string);
    // Print line of source code.
    String::Utf8Value sourceline(message->GetSourceLine());
    const char* sourceline_string = ToCString(sourceline);
    DebugMsgD("%s\n", sourceline_string);
    // Print wavy underline (GetUnderline is deprecated).
    int start = message->GetStartColumn();
    for (int i = 0; i < start; i++) {
      DebugMsgD(" ");
    }
    int end = message->GetEndColumn();
    for (int i = start; i < end; i++) {
      DebugMsgD("^");
    }
    DebugMsgD("\n");
    String::Utf8Value stack_trace(try_catch->StackTrace());
    if (stack_trace.length() > 0) {
      const char* stack_trace_string = ToCString(stack_trace);
      DebugMsgD("%s\n", stack_trace_string);
    }
  }
}

/*** static メンバ（；´д⊂）***********************************************/

CVsdFilter *CScript::m_Vsd;

/*** マクロ *****************************************************************/

#define CheckArgs( func, cond ) \
	if( !( cond )){ \
		return v8::ThrowException( v8::Exception::SyntaxError( v8::String::New( \
			#func ":invalid number of args" \
		))); \
	}

#define CheckClass( obj, name, msg ) \
	if( \
		obj.IsEmpty() || \
		strcmp( *( String::AsciiValue )( obj->GetConstructorName()), name ) \
	) return v8::ThrowException( v8::Exception::SyntaxError( v8::String::New( msg )))

/*** ライン描画 *************************************************************/

Handle<Value> Func_DrawLine( const Arguments& args ){
	
	int iLen = args.Length();
	CheckArgs( "DrawLine", iLen == 5 || iLen == 6 );
	
	PIXEL_YCA yc; Color2YCA( yc, args[ 4 ]->Int32Value());
	
	CScript::m_Vsd->DrawLine(
		args[ 0 ]->Int32Value(), // x1
		args[ 1 ]->Int32Value(), // y1
		args[ 2 ]->Int32Value(), // x2
		args[ 3 ]->Int32Value(), // y2
		iLen <= 5 ? 1 : args[ 5 ]->Int32Value(), // width
		yc, 0
	);
	
	return Undefined();
}

/*** 文字列描画 *************************************************************/

Handle<Value> Func_DrawString( const Arguments& args ){
	// arg: x, y, msg, font, color
	// arg: x, y, msg, font, color, color
	
	int iLen = args.Length();
	CheckArgs( "DrawString", iLen == 5 || iLen == 6 );
	
	// arg2 が Font かチェック
	v8::Local<v8::Object> font = args[ 3 ]->ToObject();
	CheckClass( font, "Font", "PutImage: arg[ 4 ] must be Font" );
	
	PIXEL_YCA yc;
	Color2YCA( yc, args[ 4 ]->Int32Value());
	
	String::AsciiValue msg( args[ 2 ] );
	
	if( iLen >= 6 ){
		PIXEL_YCA yc_edge;
		Color2YCA( yc, args[ 5 ]->Int32Value());
		CScript::m_Vsd->DrawString(
			*msg,
			CVsdFont::GetThis( font ),
			yc, yc_edge, 0,
			args[ 0 ]->Int32Value(), // x
			args[ 1 ]->Int32Value()  // y
		);
	}else{
		CScript::m_Vsd->DrawString(
			*msg,
			CVsdFont::GetThis( font ),
			yc, 0,
			args[ 0 ]->Int32Value(), // x
			args[ 1 ]->Int32Value()  // y
		);
	}
	
	return Undefined();
}

/*** メーター針描画 *********************************************************/

Handle<Value> Func_DrawNeedle( const Arguments& args ){
	
	int iLen = args.Length();
	CheckArgs( "DrawNeedle", iLen == 7 || iLen == 8 );
	
	PIXEL_YCA yc; Color2YCA( yc, args[ 6 ]->Int32Value());
	
	CScript::m_Vsd->DrawNeedle(
		args[ 0 ]->Int32Value(), // x
		args[ 1 ]->Int32Value(), // y
		args[ 2 ]->Int32Value(), // r
		args[ 3 ]->Int32Value(), // start
		args[ 4 ]->Int32Value(), // end
		args[ 5 ]->NumberValue(), // val
		yc,
		iLen <= 7 ? 1 : args[ 7 ]->Int32Value() // width
	);
	
	return Undefined();
}

/*** イメージ描画 ***********************************************************/

Handle<Value> Func_PutImage( const Arguments& args ){
	
	int iLen = args.Length();
	CheckArgs( "PutImage", iLen == 3 );
	
	// arg2 が Image かチェック
	v8::Local<v8::Object> img = args[ 2 ]->ToObject();
	CheckClass( img, "Image", "PutImage: arg[ 3 ] must be Image" );
	
	CScript::m_Vsd->PutImage(
		args[ 0 ]->Int32Value(),	// x1
		args[ 1 ]->Int32Value(),	// y1
		*CVsdImage::GetThis( img )	// CImage
	);
	
	return Undefined();
}

/*** コンストラクタ *********************************************************/

CScript::CScript( CVsdFilter *pVsd ){
	m_context.Clear();
	m_script.Clear();
	
	m_Vsd = pVsd;
}

/*** デストラクタ ***********************************************************/

CScript::~CScript(){
	m_context.Dispose();
}

/*** ロード・コンパイル *****************************************************/

#define SCRIPT_SIZE	( 64 * 1024 )

BOOL CScript::Load( char *szFileName ){
	// 準備
	HandleScope handle_scope;
	
	// グローバルオブジェクトの生成
	Handle<ObjectTemplate> global = ObjectTemplate::New();
	
	// 関数オブジェクトの定義
	#define DEF_SCR_FUNC( name ) \
		global->Set( \
			String::New( #name ), \
			FunctionTemplate::New( Func_ ## name ) \
		);
	#include "def_scr_func.h"
	
	// Image クラス登録
	CVsdImage::InitializeClass( global );
	CVsdFont::InitializeClass( global );
	
	// グローバルオブジェクトから環境を生成
	m_context = Context::New( NULL, global );
	
	TryCatch try_catch;
	
	// ダミーのスコープを生成  Script::New するときは
	// 何らかのコンテキストに Enter しておかなければならないらしい
	Context::Scope context_scope( m_context );
	
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
	
	Handle<Script> script = Script::New(
		ScriptBody, String::New( szFileName )
	);
	if( script.IsEmpty()){
		// Print errors that happened during compilation.
		ReportException( &try_catch );
		return false;
	}
	
	m_script = handle_scope.Close( script );
	return TRUE;
}

/*** Run ********************************************************************/

BOOL CScript::Run( void ){
	HandleScope handle_scope;
	TryCatch try_catch;
	
	// 環境からスコープを生成
	Context::Scope context_scope( m_context );
	
	Handle<Value> result = m_script->Run();
	
	if( result.IsEmpty()){
		assert( try_catch.HasCaught());
		// Print errors that happened during execution.
		ReportException( &try_catch );
		return FALSE;
	}else{
		assert( !try_catch.HasCaught());
		if( !result->IsUndefined()) {
			// If all went well and the result wasn't undefined then print
			// the returned value.
			String::Utf8Value str( result );
			const char* cstr = ToCString( str );
			DebugMsgD( "%s\n", cstr );
		}
		return TRUE;
	}
	
	return TRUE;
}

/*** function 名指定実行，引数なし ******************************************/

BOOL CScript::RunFunction( const char *szFunc ){
	HandleScope handle_scope;
	Context::Scope context_scope( m_context );
	
	Local<Function> hFunction = Local<Function>::Cast( m_context->Global()->Get( String::New( szFunc )));
	Handle<Value> result = hFunction->Call( hFunction, 0, 0 );
	return TRUE;
}
