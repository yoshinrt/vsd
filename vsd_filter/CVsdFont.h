/*****************************************************************************
	
	VSD -- vehicle data logger system  Copyright(C) by DDS
	
	CVsdFont.h - CVsdFont class header
	
*****************************************************************************/

#ifndef _CVsdFont_h_
#define _CVsdFont_h_

/*** new type ***************************************************************/

typedef struct {
	BYTE	*pBuf;
	int		iW, iH;
	int		iOrgY;
} tFontGlyph;

class CVsdFont {
  public:
	CVsdFont( LOGFONT &logfont );
	CVsdFont( const char *szFontName, int iSize, int iAttr );
	~CVsdFont();
	
	void CreateFont( LOGFONT &logfont );
	
	int GetW( void ){ return m_iFontW; }
	int GetH( void ){ return m_iFontH; }
	
	int	m_iFontW, m_iFontH;
	
	tFontGlyph *m_FontGlyph;
	
	// フォント
	DWORD GetPix( UCHAR c, int x, int y ){
		int	i = c - ' ';
		return 0;
	}
	
	enum {
		ATTR_BOLD		= 1 << 0,
		ATTR_ITALIC		= 1 << 1,
		ATTR_OUTLINE	= 1 << 2
	};
	
	/*** JavaScript interface ***********************************************/
	
  private:
	// クラスコンストラクタ
	static v8::Handle<v8::Value> New( const v8::Arguments& args ){
		// 引数チェック
		if ( args.Length() < 2 ) return v8::Undefined();
		
		v8::String::AsciiValue FontName( args[ 0 ] );
		CVsdFont *backend = new CVsdFont(
			*FontName,
			args[ 1 ]->Int32Value(),
			args.Length() <= 2 ? 0 : args[ 2 ]->Int32Value()
		);
		
		// internal field にバックエンドオブジェクトを設定
		v8::Local<v8::Object> thisObject = args.This();
		thisObject->SetInternalField( 0, v8::External::New( backend ));
		
		// JS オブジェクトが GC されるときにデストラクタが呼ばれるおまじない
		v8::Persistent<v8::Object> objectHolder = v8::Persistent<v8::Object>::New( thisObject );
		objectHolder.MakeWeak( backend, CVsdFont::Dispose );
		
		// コンストラクタは this を返すこと。
		return thisObject;
	}
	
	// クラスデストラクタ
	static void Dispose( v8::Persistent<v8::Value> handle, void* pVoid ){
		delete static_cast<CVsdFont*>( pVoid );
	}
	
	///// プロパティアクセサ /////
	
	#define DEF_SCR_VAR( name, var ) \
		static v8::Handle<v8::Value> Get_ ## name( \
			v8::Local<v8::String> propertyName, \
			const v8::AccessorInfo& info \
		){ \
			 const CVsdFont* backend = GetThis( info.Holder()); \
			 return v8::Integer::New( backend->var ); \
		}
	#include "def_font_var.h"
	
	///// メソッドコールバック /////
	
	/*
	static v8::Local<v8::Value> Add( const v8::Arguments& args ){
		CVsdFont* backend = GetThis( args.This());
		if ( args.Length() > 0 ){
			backend->Add( args[0]->Int32Value());
		}else{
			backend->Add();
		}
		return v8::Undefined();
	}
	*/
	
  public:
	// this へのアクセスヘルパ
	static CVsdFont* GetThis( v8::Local<v8::Object> handle ){
		 void* pThis = v8::Local<v8::External>::Cast( handle->GetInternalField( 0 ))->Value();
		 return static_cast<CVsdFont*>( pThis );
	}
	
	// クラステンプレートの初期化
	static void InitializeClass( v8::Handle<v8::ObjectTemplate> global ){
		// コンストラクタを作成
		v8::Local<v8::FunctionTemplate> tmpl = v8::FunctionTemplate::New( CVsdFont::New );
		tmpl->SetClassName( v8::String::New( "Font" ));
		
		// フィールドなどはこちらに
		v8::Handle<v8::ObjectTemplate> inst = tmpl->InstanceTemplate();
		inst->SetInternalFieldCount( 1 );
		#define DEF_SCR_VAR( name, var ) \
			inst->SetAccessor( v8::String::New( #name ), CVsdFont::Get_ ## name );
		#include "def_font_var.h"
		
		// メソッドはこちらに
		//v8::Local<v8::ObjectTemplate> proto = tmpl->PrototypeTemplate();
		//proto->Set( v8::String::New( "add" ), FunctionTemplate::New( CVsdFont::Add ));
		
		// グローバルオブジェクトにクラスを定義
		global->Set( v8::String::New( "Font" ), tmpl );
	}
};
#endif
