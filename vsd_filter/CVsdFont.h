/*****************************************************************************
	
	VSD -- vehicle data logger system  Copyright(C) by DDS
	
	CVsdFont.h - CVsdFont class header
	
*****************************************************************************/

#ifndef _CVsdFont_h_
#define _CVsdFont_h_

/*** new type ***************************************************************/

class CFontGlyph {
  public:
	BYTE	*pBuf;
	BYTE	*pBufOutline;
	int		iW, iH;
	int		iOrgY;
	int		iCellIncX;
	
	CFontGlyph(){
		pBuf = pBufOutline = NULL;
	}
	
	~CFontGlyph(){
		if( pBuf        ) delete [] pBuf;
		if( pBufOutline ) delete [] pBufOutline;
	}
};

class CVsdFont {
  public:
	CVsdFont( LOGFONT &logfont, UINT uAttr = 0 );
	CVsdFont( const char *szFontName, int iSize, UINT uAttr = 0 );
	~CVsdFont(){}
	
	void CreateFont( LOGFONT &logfont );
	
	static BOOL ExistFont( UCHAR c ){ return FONT_CHAR_FIRST <= c && c <= FONT_CHAR_LAST; }
	BOOL IsOutline( void ){ return m_uAttr & ATTR_OUTLINE; }
	BOOL IsFixed( void ){ return m_uAttr & ATTR_FIXED; }
	
	CFontGlyph& FontGlyph( UCHAR c ){
		return m_FontGlyph[ c - FONT_CHAR_FIRST ];
	}
	
	static const UINT ATTR_BOLD		= 1 << 0;
	static const UINT ATTR_ITALIC	= 1 << 1;
	static const UINT ATTR_OUTLINE	= 1 << 2;
	static const UINT ATTR_FIXED	= 1 << 3;
	
	int GetW( void ){ return m_iFontW; }
	int GetH( void ){ return m_iFontH; }
	int GetW_Space( void ){ return m_iFontW_Space; }
	
	int GetTextWidth( char *szMsg ){
		
		if( m_uAttr & ATTR_FIXED ){
			return strlen( szMsg ) * GetW();
		}
		
		int iWidth = 0;
		for( int i = 0; szMsg[ i ]; ++i ){
			iWidth += FontGlyph( szMsg[ i ] ).iW;
		}
		return iWidth;
	}
	
  private:
	static const int FONT_CHAR_FIRST = '!';
	static const int FONT_CHAR_LAST	 = '~';
	
	CFontGlyph m_FontGlyph[ FONT_CHAR_LAST - FONT_CHAR_FIRST + 1 ];
	
	int	m_iFontW, m_iFontH, m_iFontW_Space;
	
	UINT	m_uAttr;
	
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
