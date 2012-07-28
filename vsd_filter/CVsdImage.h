/*****************************************************************************
	
	VSD -- vehicle data logger system  Copyright(C) by DDS
	
	CVsdImage.cpp - Image file operation
	
*****************************************************************************/

typedef union {
	struct {
		UCHAR	b;
		UCHAR	g;
		UCHAR	r;
		UCHAR	a;
	};
	
	UINT	argb;
} PIXEL_RGBA;

class CVsdImage {
  public:
	CVsdImage();
	~CVsdImage();
	
	BOOL Load( const char *szFileName );
	BOOL ConvRGBA2YCA( void );
	
	int m_iWidth;
	int m_iHeight;
	
	PIXEL_RGBA	*m_pRGBA_Buf;
	PIXEL_YCA	*m_pPixelBuf;
	
	/*** JavaScript interface ***********************************************/
	
  private:
	// クラスコンストラクタ
	static v8::Handle<v8::Value> New( const v8::Arguments& args ){
		// 引数チェック
		if ( args.Length() <= 0 ) return v8::Undefined();
		
		CVsdImage* backend = new CVsdImage();
		v8::String::AsciiValue FileName( args[ 0 ] );
		
		if( !backend->Load( *FileName )) return v8::Undefined();
		
		// internal field にバックエンドオブジェクトを設定
		v8::Local<v8::Object> thisObject = args.This();
		thisObject->SetInternalField( 0, v8::External::New( backend ));
		
		// JS オブジェクトが GC されるときにデストラクタが呼ばれるおまじない
		v8::Persistent<v8::Object> objectHolder = v8::Persistent<v8::Object>::New( thisObject );
		objectHolder.MakeWeak( backend, CVsdImage::Dispose );
		
		// コンストラクタは this を返すこと。
		return thisObject;
	}
	
	// クラスデストラクタ
	static void Dispose( v8::Persistent<v8::Value> handle, void* pVoid ){
		delete static_cast<CVsdImage*>( pVoid );
	}
	
	///// プロパティアクセサ /////
	
	#define DEF_SCR_VAR( name, var ) \
		static v8::Handle<v8::Value> Get_ ## name( \
			v8::Local<v8::String> propertyName, \
			const v8::AccessorInfo& info \
		){ \
			 const CVsdImage* backend = GetThis( info.Holder()); \
			 return v8::Integer::New( backend->var ); \
		}
	#include "def_image_var.h"
	
	///// メソッドコールバック /////
	
	/*
	static v8::Local<v8::Value> Add( const v8::Arguments& args ){
		CVsdImage* backend = GetThis( args.This());
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
	static CVsdImage* GetThis( v8::Local<v8::Object> handle ){
		 void* pThis = v8::Local<v8::External>::Cast( handle->GetInternalField( 0 ))->Value();
		 return static_cast<CVsdImage*>( pThis );
	}
	
	// クラステンプレートの初期化
	static void InitializeClass( v8::Handle<v8::ObjectTemplate> global ){
		// コンストラクタを作成
		v8::Local<v8::FunctionTemplate> tmpl = v8::FunctionTemplate::New( CVsdImage::New );
		tmpl->SetClassName( v8::String::New( "Image" ));
		
		// フィールドなどはこちらに
		v8::Handle<v8::ObjectTemplate> inst = tmpl->InstanceTemplate();
		inst->SetInternalFieldCount( 1 );
		#define DEF_SCR_VAR( name, var ) \
			inst->SetAccessor( v8::String::New( #name ), CVsdImage::Get_ ## name );
		#include "def_image_var.h"
		
		// メソッドはこちらに
		//v8::Local<v8::ObjectTemplate> proto = tmpl->PrototypeTemplate();
		//proto->Set( v8::String::New( "add" ), FunctionTemplate::New( CVsdImage::Add ));
		
		// グローバルオブジェクトにクラスを定義
		global->Set( v8::String::New( "Image" ), tmpl );
	}
};
