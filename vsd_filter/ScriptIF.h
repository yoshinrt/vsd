/****************************************************************************/

class CVsdFilterIF {
  public:
	// クラスコンストラクタ
	static v8::Handle<v8::Value> New( const v8::Arguments& args ){
		
		CVsdFilter* obj = CScript::m_pVsd;

		// internal field にバックエンドオブジェクトを設定
		v8::Local<v8::Object> thisObject = args.This();
		thisObject->SetInternalField( 0, v8::External::New( obj ));
		
		// JS オブジェクトが GC されるときにデストラクタが呼ばれるおまじない
		v8::Persistent<v8::Object> objectHolder = v8::Persistent<v8::Object>::New( thisObject );
		objectHolder.MakeWeak( obj, Dispose );
		
		#ifdef DEBUG
			DebugMsgD( ">>>new js obj CVsdFilter:%X\n", obj );
		#endif
		// コンストラクタは this を返すこと。
		return thisObject;
	}
	
	// クラスデストラクタ
	static void Dispose( v8::Persistent<v8::Value> handle, void* pVoid ){
		if( 0 ) {
			v8::HandleScope handle_scope;
			CVsdFilter *thisObj = GetThis<CVsdFilter>( handle->ToObject());
			if( thisObj ){
				delete static_cast<CVsdFilter*>( thisObj );
				#ifdef DEBUG
					DebugMsgD( "<<<del js obj CVsdFilter:%X\n", thisObj );
				#endif
			}
		}
		handle.Dispose();
	}
	
	// JavaScript からの明示的な破棄
	static v8::Handle<v8::Value> Func_Dispose( const v8::Arguments& args ){
		// obj の Dispose() を呼ぶ
		CVsdFilter *thisObj = GetThis<CVsdFilter>( args.This());
		if( 0 ) if( thisObj ){
			delete thisObj;
			#ifdef DEBUG
				DebugMsgD( "<<<DISPOSE js obj CVsdFilter:%X\n", thisObj );
			#endif
			
			// internalfield を null っぽくする
			args.This()->SetInternalField( 0, v8::External::New( NULL ));
		}
		return v8::Undefined();
	}
	
	///// プロパティアクセサ /////
	static v8::Handle<v8::Value> Get_ElapsedTime( v8::Local<v8::String> propertyName, const v8::AccessorInfo& info ){
		CVsdFilter *obj = GetThis<CVsdFilter>( info.Holder());
		return obj ? v8::Integer::New( obj->CurTime() ) : v8::Undefined();
	}
	static v8::Handle<v8::Value> Get_BestLapTime( v8::Local<v8::String> propertyName, const v8::AccessorInfo& info ){
		CVsdFilter *obj = GetThis<CVsdFilter>( info.Holder());
		return obj ? v8::Integer::New( obj->BestLapTime() ) : v8::Undefined();
	}
	static v8::Handle<v8::Value> Get_DiffTime( v8::Local<v8::String> propertyName, const v8::AccessorInfo& info ){
		CVsdFilter *obj = GetThis<CVsdFilter>( info.Holder());
		return obj ? v8::Integer::New( obj->DiffTime() ) : v8::Undefined();
	}
	static v8::Handle<v8::Value> Get_LapTime( v8::Local<v8::String> propertyName, const v8::AccessorInfo& info ){
		CVsdFilter *obj = GetThis<CVsdFilter>( info.Holder());
		return obj ? v8::Integer::New( obj->LapTime() ) : v8::Undefined();
	}
	static v8::Handle<v8::Value> Get_LapCnt( v8::Local<v8::String> propertyName, const v8::AccessorInfo& info ){
		CVsdFilter *obj = GetThis<CVsdFilter>( info.Holder());
		return obj ? v8::Integer::New( obj->LapCnt() ) : v8::Undefined();
	}
	static v8::Handle<v8::Value> Get_MaxLapCnt( v8::Local<v8::String> propertyName, const v8::AccessorInfo& info ){
		CVsdFilter *obj = GetThis<CVsdFilter>( info.Holder());
		return obj ? v8::Integer::New( obj->MaxLapCnt() ) : v8::Undefined();
	}
	static v8::Handle<v8::Value> Get_FrameCnt( v8::Local<v8::String> propertyName, const v8::AccessorInfo& info ){
		CVsdFilter *obj = GetThis<CVsdFilter>( info.Holder());
		return obj ? v8::Integer::New( obj->GetFrameCnt() ) : v8::Undefined();
	}

	///// メャbドコールバック /////
	/*** DrawArc ****************************************************************/
	
	static v8::Handle<v8::Value> Func_DrawArc( const v8::Arguments& args ){
		int iLen = args.Length();
		if( CheckArgs( 7 <= iLen && iLen <= 9 )) return v8::Undefined();
		
		CVsdFilter *thisObj = GetThis<CVsdFilter>( args.This());
		if( !thisObj ) return v8::Undefined();
		
		if( iLen >= 9 ){
			thisObj->DrawArc(
				args[ 0 ]->Int32Value(),
				args[ 1 ]->Int32Value(),
				args[ 2 ]->Int32Value(),
				args[ 3 ]->Int32Value(),
				args[ 4 ]->Int32Value(),
				args[ 5 ]->Int32Value(),
				args[ 6 ]->NumberValue(),
				args[ 7 ]->NumberValue(),
				PIXEL_RABY::Argb2Raby( args[ 8 ]->Int32Value())
			);
		}else{
			thisObj->DrawArc(
				args[ 0 ]->Int32Value(),
				args[ 1 ]->Int32Value(),
				args[ 2 ]->Int32Value(),
				args[ 3 ]->Int32Value(),
				args[ 4 ]->NumberValue(),
				args[ 5 ]->NumberValue(),
				PIXEL_RABY::Argb2Raby( args[ 6 ]->Int32Value()),
				iLen <= 7 ? 0 : args[ 7 ]->Int32Value()
			);
		}
		return v8::Undefined();
	}
	
	/*** ログデータ取得用 *******************************************************/
	
	#define DEF_LOG( name ) \
		static v8::Handle<v8::Value> Get_##name( v8::Local<v8::String> propertyName, const v8::AccessorInfo& info ){ \
			CVsdFilter *obj = GetThis<CVsdFilter>( info.Holder()); \
			return obj ? v8::Number::New( obj->Get##name() ) : v8::Undefined(); \
		}
	#include "def_log.h"
	
	static v8::Handle<v8::Value> Get_Value( v8::Local<v8::String> propertyName, const v8::AccessorInfo& info ){
		CVsdFilter *obj = GetThis<CVsdFilter>( info.Holder());
		v8::String::AsciiValue str( propertyName );
		return obj ? v8::Number::New( obj->GetValue( *str )) : v8::Undefined();
	}
	
	/*** デバッグ用 *************************************************************/
	
	// 関数オブジェクト print の実体
	static v8::Handle<v8::Value> Func_print(const v8::Arguments& args) {
		v8::String::AsciiValue str( args[ 0 ] );
		DebugMsgD( "%s\n", *str );
		return v8::Undefined();
	}
	
	static v8::Handle<v8::Value> Func_PutPixel( const v8::Arguments& args ){
		int iLen = args.Length();
		if( CheckArgs( 3 <= iLen && iLen <= 4 )) return v8::Undefined();
		
		CVsdFilter *thisObj = GetThis<CVsdFilter>( args.This());
		if( !thisObj ) return v8::Undefined();
		thisObj->PutPixel(
			args[ 0 ]->Int32Value(),
			args[ 1 ]->Int32Value(),
			PIXEL_RABY::Argb2Raby( args[ 2 ]->Int32Value()),
			iLen <= 3 ? 0 : args[ 3 ]->Int32Value()
		);
		
		return v8::Undefined();
	}
	static v8::Handle<v8::Value> Func_PutImage( const v8::Arguments& args ){
		int iLen = args.Length();
		if( CheckArgs( 3 <= iLen && iLen <= 4 )) return v8::Undefined();
		v8::Local<v8::Object> Image2 = args[ 2 ]->ToObject();
		if( CheckClass( Image2, "Image", "arg[ 3 ] must be Image" )) return v8::Undefined();
		CVsdImage *obj2 = GetThis<CVsdImage>( Image2 );
		if( !obj2 ) return v8::Undefined();
		CVsdFilter *thisObj = GetThis<CVsdFilter>( args.This());
		if( !thisObj ) return v8::Undefined();
		int ret = thisObj->PutImage(
			args[ 0 ]->Int32Value(),
			args[ 1 ]->Int32Value(),
			*obj2,
			iLen <= 3 ? 0 : args[ 3 ]->Int32Value()
		);
		
		return v8::Integer::New( ret );
	}
	static v8::Handle<v8::Value> Func_DrawLine( const v8::Arguments& args ){
		int iLen = args.Length();
		if( CheckArgs( 5 <= iLen && iLen <= 7 )) return v8::Undefined();
		
		CVsdFilter *thisObj = GetThis<CVsdFilter>( args.This());
		if( !thisObj ) return v8::Undefined();
		thisObj->DrawLine(
			args[ 0 ]->Int32Value(),
			args[ 1 ]->Int32Value(),
			args[ 2 ]->Int32Value(),
			args[ 3 ]->Int32Value(),
			iLen <= 5 ? 1 : args[ 5 ]->Int32Value(),
			PIXEL_RABY::Argb2Raby( args[ 4 ]->Int32Value()),
			iLen <= 6 ? 0 : args[ 6 ]->Int32Value()
		);
		
		return v8::Undefined();
	}
	static v8::Handle<v8::Value> Func_DrawRect( const v8::Arguments& args ){
		int iLen = args.Length();
		if( CheckArgs( 5 <= iLen && iLen <= 6 )) return v8::Undefined();
		
		CVsdFilter *thisObj = GetThis<CVsdFilter>( args.This());
		if( !thisObj ) return v8::Undefined();
		thisObj->DrawRect(
			args[ 0 ]->Int32Value(),
			args[ 1 ]->Int32Value(),
			args[ 2 ]->Int32Value(),
			args[ 3 ]->Int32Value(),
			PIXEL_RABY::Argb2Raby( args[ 4 ]->Int32Value()),
			iLen <= 5 ? 0 : args[ 5 ]->Int32Value()
		);
		
		return v8::Undefined();
	}
	static v8::Handle<v8::Value> Func_DrawCircle( const v8::Arguments& args ){
		int iLen = args.Length();
		if( CheckArgs( 4 <= iLen && iLen <= 5 )) return v8::Undefined();
		
		CVsdFilter *thisObj = GetThis<CVsdFilter>( args.This());
		if( !thisObj ) return v8::Undefined();
		thisObj->DrawCircle(
			args[ 0 ]->Int32Value(),
			args[ 1 ]->Int32Value(),
			args[ 2 ]->Int32Value(),
			PIXEL_RABY::Argb2Raby( args[ 3 ]->Int32Value()),
			iLen <= 4 ? 0 : args[ 4 ]->Int32Value()
		);
		
		return v8::Undefined();
	}
	static v8::Handle<v8::Value> Func_DrawText( const v8::Arguments& args ){
		int iLen = args.Length();
		if( CheckArgs( 4 <= iLen && iLen <= 6 )) return v8::Undefined();
		v8::String::Value str2( args[ 2 ] );
		v8::Local<v8::Object> Font3 = args[ 3 ]->ToObject();
		if( CheckClass( Font3, "Font", "arg[ 4 ] must be Font" )) return v8::Undefined();
		CVsdFont *obj3 = GetThis<CVsdFont>( Font3 );
		if( !obj3 ) return v8::Undefined();
		CVsdFilter *thisObj = GetThis<CVsdFilter>( args.This());
		if( !thisObj ) return v8::Undefined();
		thisObj->DrawText(
			args[ 0 ]->Int32Value(),
			args[ 1 ]->Int32Value(),
			( LPCWSTR )*str2,
			*obj3,
			iLen <= 4 ? color_white : PIXEL_RABY::Argb2Raby( args[ 4 ]->Int32Value()),
			iLen <= 5 ? color_black : PIXEL_RABY::Argb2Raby( args[ 5 ]->Int32Value())
		);
		
		return v8::Undefined();
	}
	static v8::Handle<v8::Value> Func_DrawTextAlign( const v8::Arguments& args ){
		int iLen = args.Length();
		if( CheckArgs( 5 <= iLen && iLen <= 7 )) return v8::Undefined();
		v8::String::Value str3( args[ 3 ] );
		v8::Local<v8::Object> Font4 = args[ 4 ]->ToObject();
		if( CheckClass( Font4, "Font", "arg[ 5 ] must be Font" )) return v8::Undefined();
		CVsdFont *obj4 = GetThis<CVsdFont>( Font4 );
		if( !obj4 ) return v8::Undefined();
		CVsdFilter *thisObj = GetThis<CVsdFilter>( args.This());
		if( !thisObj ) return v8::Undefined();
		thisObj->DrawTextAlign(
			args[ 0 ]->Int32Value(),
			args[ 1 ]->Int32Value(),
			args[ 2 ]->Int32Value(),
			( LPCWSTR )*str3,
			*obj4,
			iLen <= 5 ? color_white : PIXEL_RABY::Argb2Raby( args[ 5 ]->Int32Value()),
			iLen <= 6 ? color_black : PIXEL_RABY::Argb2Raby( args[ 6 ]->Int32Value())
		);
		
		return v8::Undefined();
	}
	static v8::Handle<v8::Value> Func_DrawGraph( const v8::Arguments& args ){
		int iLen = args.Length();
		if( CheckArgs( 5 <= iLen && iLen <= 6 )) return v8::Undefined();
		v8::Local<v8::Object> Font4 = args[ 4 ]->ToObject();
		if( CheckClass( Font4, "Font", "arg[ 5 ] must be Font" )) return v8::Undefined();
		CVsdFont *obj4 = GetThis<CVsdFont>( Font4 );
		if( !obj4 ) return v8::Undefined();
		CVsdFilter *thisObj = GetThis<CVsdFilter>( args.This());
		if( !thisObj ) return v8::Undefined();
		thisObj->DrawGraph(
			args[ 0 ]->Int32Value(),
			args[ 1 ]->Int32Value(),
			args[ 2 ]->Int32Value(),
			args[ 3 ]->Int32Value(),
			*obj4,
			iLen <= 5 ? 1 : args[ 5 ]->Int32Value()
		);
		
		return v8::Undefined();
	}
	static v8::Handle<v8::Value> Func_InitPolygon( const v8::Arguments& args ){
		int iLen = args.Length();
		if( CheckArgs( iLen == 0 )) return v8::Undefined();
		
		CVsdFilter *thisObj = GetThis<CVsdFilter>( args.This());
		if( !thisObj ) return v8::Undefined();
		thisObj->InitPolygon();
		
		return v8::Undefined();
	}
	static v8::Handle<v8::Value> Func_DrawPolygon( const v8::Arguments& args ){
		int iLen = args.Length();
		if( CheckArgs( iLen == 1 )) return v8::Undefined();
		
		CVsdFilter *thisObj = GetThis<CVsdFilter>( args.This());
		if( !thisObj ) return v8::Undefined();
		thisObj->DrawPolygon(
			PIXEL_RABY::Argb2Raby( args[ 0 ]->Int32Value())
		);
		
		return v8::Undefined();
	}
	static v8::Handle<v8::Value> Func_DrawGSnake( const v8::Arguments& args ){
		int iLen = args.Length();
		if( CheckArgs( 7 <= iLen && iLen <= 8 )) return v8::Undefined();
		
		CVsdFilter *thisObj = GetThis<CVsdFilter>( args.This());
		if( !thisObj ) return v8::Undefined();
		thisObj->DrawGSnake(
			args[ 0 ]->Int32Value(),
			args[ 1 ]->Int32Value(),
			args[ 2 ]->Int32Value(),
			args[ 3 ]->Int32Value(),
			args[ 4 ]->Int32Value(),
			PIXEL_RABY::Argb2Raby( args[ 5 ]->Int32Value()),
			PIXEL_RABY::Argb2Raby( args[ 6 ]->Int32Value()),
			iLen <= 7 ? 3 : args[ 7 ]->NumberValue()
		);
		
		return v8::Undefined();
	}
	static v8::Handle<v8::Value> Func_DrawMeterScale( const v8::Arguments& args ){
		int iLen = args.Length();
		if( CheckArgs( iLen == 17 )) return v8::Undefined();
		v8::Local<v8::Object> Font16 = args[ 16 ]->ToObject();
		if( CheckClass( Font16, "Font", "arg[ 17 ] must be Font" )) return v8::Undefined();
		CVsdFont *obj16 = GetThis<CVsdFont>( Font16 );
		if( !obj16 ) return v8::Undefined();
		CVsdFilter *thisObj = GetThis<CVsdFilter>( args.This());
		if( !thisObj ) return v8::Undefined();
		thisObj->DrawMeterScale(
			args[ 0 ]->Int32Value(),
			args[ 1 ]->Int32Value(),
			args[ 2 ]->Int32Value(),
			args[ 3 ]->Int32Value(),
			args[ 4 ]->Int32Value(),
			PIXEL_RABY::Argb2Raby( args[ 5 ]->Int32Value()),
			args[ 6 ]->Int32Value(),
			args[ 7 ]->Int32Value(),
			PIXEL_RABY::Argb2Raby( args[ 8 ]->Int32Value()),
			args[ 9 ]->Int32Value(),
			args[ 10 ]->Int32Value(),
			args[ 11 ]->Int32Value(),
			args[ 12 ]->Int32Value(),
			args[ 13 ]->Int32Value(),
			args[ 14 ]->Int32Value(),
			PIXEL_RABY::Argb2Raby( args[ 15 ]->Int32Value()),
			*obj16
		);
		
		return v8::Undefined();
	}
	static v8::Handle<v8::Value> Func_DrawMap( const v8::Arguments& args ){
		int iLen = args.Length();
		if( CheckArgs( iLen == 11 )) return v8::Undefined();
		
		CVsdFilter *thisObj = GetThis<CVsdFilter>( args.This());
		if( !thisObj ) return v8::Undefined();
		thisObj->DrawMap(
			args[ 0 ]->Int32Value(),
			args[ 1 ]->Int32Value(),
			args[ 2 ]->Int32Value(),
			args[ 3 ]->Int32Value(),
			args[ 4 ]->Int32Value(),
			args[ 5 ]->Int32Value(),
			args[ 6 ]->Int32Value(),
			PIXEL_RABY::Argb2Raby( args[ 7 ]->Int32Value()),
			PIXEL_RABY::Argb2Raby( args[ 8 ]->Int32Value()),
			PIXEL_RABY::Argb2Raby( args[ 9 ]->Int32Value()),
			PIXEL_RABY::Argb2Raby( args[ 10 ]->Int32Value())
		);
		
		return v8::Undefined();
	}
	static v8::Handle<v8::Value> Func_DrawLapTime( const v8::Arguments& args ){
		int iLen = args.Length();
		if( CheckArgs( 4 <= iLen && iLen <= 8 )) return v8::Undefined();
		v8::Local<v8::Object> Font3 = args[ 3 ]->ToObject();
		if( CheckClass( Font3, "Font", "arg[ 4 ] must be Font" )) return v8::Undefined();
		CVsdFont *obj3 = GetThis<CVsdFont>( Font3 );
		if( !obj3 ) return v8::Undefined();
		CVsdFilter *thisObj = GetThis<CVsdFilter>( args.This());
		if( !thisObj ) return v8::Undefined();
		thisObj->DrawLapTime(
			args[ 0 ]->Int32Value(),
			args[ 1 ]->Int32Value(),
			args[ 2 ]->Int32Value(),
			*obj3,
			iLen <= 4 ? color_white : PIXEL_RABY::Argb2Raby( args[ 4 ]->Int32Value()),
			iLen <= 5 ? color_cyan : PIXEL_RABY::Argb2Raby( args[ 5 ]->Int32Value()),
			iLen <= 6 ? color_red : PIXEL_RABY::Argb2Raby( args[ 6 ]->Int32Value()),
			iLen <= 7 ? color_black : PIXEL_RABY::Argb2Raby( args[ 7 ]->Int32Value())
		);
		
		return v8::Undefined();
	}
	static v8::Handle<v8::Value> Func_DrawLapTimeLog( const v8::Arguments& args ){
		int iLen = args.Length();
		if( CheckArgs( 5 <= iLen && iLen <= 8 )) return v8::Undefined();
		v8::Local<v8::Object> Font4 = args[ 4 ]->ToObject();
		if( CheckClass( Font4, "Font", "arg[ 5 ] must be Font" )) return v8::Undefined();
		CVsdFont *obj4 = GetThis<CVsdFont>( Font4 );
		if( !obj4 ) return v8::Undefined();
		CVsdFilter *thisObj = GetThis<CVsdFilter>( args.This());
		if( !thisObj ) return v8::Undefined();
		thisObj->DrawLapTimeLog(
			args[ 0 ]->Int32Value(),
			args[ 1 ]->Int32Value(),
			args[ 2 ]->Int32Value(),
			args[ 3 ]->Int32Value(),
			*obj4,
			iLen <= 5 ? color_white : PIXEL_RABY::Argb2Raby( args[ 5 ]->Int32Value()),
			iLen <= 6 ? color_cyan : PIXEL_RABY::Argb2Raby( args[ 6 ]->Int32Value()),
			iLen <= 7 ? color_black : PIXEL_RABY::Argb2Raby( args[ 7 ]->Int32Value())
		);
		
		return v8::Undefined();
	}
	static v8::Handle<v8::Value> Func_DrawNeedle( const v8::Arguments& args ){
		int iLen = args.Length();
		if( CheckArgs( 8 <= iLen && iLen <= 9 )) return v8::Undefined();
		
		CVsdFilter *thisObj = GetThis<CVsdFilter>( args.This());
		if( !thisObj ) return v8::Undefined();
		thisObj->DrawNeedle(
			args[ 0 ]->Int32Value(),
			args[ 1 ]->Int32Value(),
			args[ 2 ]->Int32Value(),
			args[ 3 ]->Int32Value(),
			args[ 4 ]->Int32Value(),
			args[ 5 ]->Int32Value(),
			args[ 6 ]->NumberValue(),
			PIXEL_RABY::Argb2Raby( args[ 7 ]->Int32Value()),
			iLen <= 8 ? 1 : args[ 8 ]->Int32Value()
		);
		
		return v8::Undefined();
	}
	static v8::Handle<v8::Value> Func_FormatTime( const v8::Arguments& args ){
		int iLen = args.Length();
		if( CheckArgs( iLen == 1 )) return v8::Undefined();
		
		CVsdFilter *thisObj = GetThis<CVsdFilter>( args.This());
		if( !thisObj ) return v8::Undefined();
		LPCWSTR ret = thisObj->FormatTime(
			args[ 0 ]->Int32Value()
		);
		
		return v8::String::New(( uint16_t *)ret );
	}

  public:
	// this へのアクセスヘルパ
	template<typename T>
	static T* GetThis( v8::Local<v8::Object> handle ){
		if( handle->GetInternalField( 0 )->IsUndefined()){
			v8::ThrowException( v8::Exception::TypeError( v8::String::New( "Invalid object ( maybe \"new\" failed )" )));
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
	
	// クラステンプレートの初期化
	static void InitializeClass( v8::Handle<v8::ObjectTemplate> global ){
		// コンストラクタを作成
		v8::Local<v8::FunctionTemplate> tmpl = v8::FunctionTemplate::New( New );
		tmpl->SetClassName( v8::String::New( "__VSD_System__" ));
		
		// フィールドなどはこちらに
		v8::Handle<v8::ObjectTemplate> inst = tmpl->InstanceTemplate();
		inst->SetInternalFieldCount( 1 );
		inst->SetAccessor( v8::String::New( "ElapsedTime" ), Get_ElapsedTime );
		inst->SetAccessor( v8::String::New( "BestLapTime" ), Get_BestLapTime );
		inst->SetAccessor( v8::String::New( "DiffTime" ), Get_DiffTime );
		inst->SetAccessor( v8::String::New( "LapTime" ), Get_LapTime );
		inst->SetAccessor( v8::String::New( "LapCnt" ), Get_LapCnt );
		inst->SetAccessor( v8::String::New( "MaxLapCnt" ), Get_MaxLapCnt );
		inst->SetAccessor( v8::String::New( "FrameCnt" ), Get_FrameCnt );

		// メャbドはこちらに
		v8::Handle<v8::ObjectTemplate> proto = tmpl->PrototypeTemplate();
		proto->Set( v8::String::New( "Dispose" ), v8::FunctionTemplate::New( Func_Dispose ));
		proto->Set( v8::String::New( "DrawArc" ), v8::FunctionTemplate::New( Func_DrawArc ));
		proto->Set( v8::String::New( "print" ), v8::FunctionTemplate::New( Func_print ));
		proto->Set( v8::String::New( "PutPixel" ), v8::FunctionTemplate::New( Func_PutPixel ));
		proto->Set( v8::String::New( "PutImage" ), v8::FunctionTemplate::New( Func_PutImage ));
		proto->Set( v8::String::New( "DrawLine" ), v8::FunctionTemplate::New( Func_DrawLine ));
		proto->Set( v8::String::New( "DrawRect" ), v8::FunctionTemplate::New( Func_DrawRect ));
		proto->Set( v8::String::New( "DrawCircle" ), v8::FunctionTemplate::New( Func_DrawCircle ));
		proto->Set( v8::String::New( "DrawText" ), v8::FunctionTemplate::New( Func_DrawText ));
		proto->Set( v8::String::New( "DrawTextAlign" ), v8::FunctionTemplate::New( Func_DrawTextAlign ));
		proto->Set( v8::String::New( "DrawGraph" ), v8::FunctionTemplate::New( Func_DrawGraph ));
		proto->Set( v8::String::New( "InitPolygon" ), v8::FunctionTemplate::New( Func_InitPolygon ));
		proto->Set( v8::String::New( "DrawPolygon" ), v8::FunctionTemplate::New( Func_DrawPolygon ));
		proto->Set( v8::String::New( "DrawGSnake" ), v8::FunctionTemplate::New( Func_DrawGSnake ));
		proto->Set( v8::String::New( "DrawMeterScale" ), v8::FunctionTemplate::New( Func_DrawMeterScale ));
		proto->Set( v8::String::New( "DrawMap" ), v8::FunctionTemplate::New( Func_DrawMap ));
		proto->Set( v8::String::New( "DrawLapTime" ), v8::FunctionTemplate::New( Func_DrawLapTime ));
		proto->Set( v8::String::New( "DrawLapTimeLog" ), v8::FunctionTemplate::New( Func_DrawLapTimeLog ));
		proto->Set( v8::String::New( "DrawNeedle" ), v8::FunctionTemplate::New( Func_DrawNeedle ));
		proto->Set( v8::String::New( "FormatTime" ), v8::FunctionTemplate::New( Func_FormatTime ));

		proto->Set( v8::String::New( "Width" ), v8::Integer::New( CScript::m_pVsd->GetWidth() ));
		proto->Set( v8::String::New( "Height" ), v8::Integer::New( CScript::m_pVsd->GetHeight() ));
		proto->Set( v8::String::New( "MaxFrameCnt" ), v8::Integer::New( CScript::m_pVsd->GetFrameMax() ));
		proto->Set( v8::String::New( "SkinDir" ), v8::String::New(( uint16_t *) CScript::m_pVsd->m_szSkinDirW ));
		proto->Set( v8::String::New( "VsdRootDir" ), v8::String::New(( uint16_t *) CScript::m_pVsd->m_szPluginDirW ));

		CScript::m_pVsd->InitJS( tmpl );

		// グローバルオブジェクトにクラスを定義
		global->Set( v8::String::New( "__VSD_System__" ), tmpl );
	}
};
/****************************************************************************/

class CVsdImageIF {
  public:
	// クラスコンストラクタ
	static v8::Handle<v8::Value> New( const v8::Arguments& args ){
		
		// 引数チェック
		if ( args.Length() <= 0 ) return v8::Undefined();
		
		CVsdImage* obj;
		
		// arg[ 0 ] が Image だった場合，そのコピーを作る
		if( args[ 0 ]->IsObject()){
			v8::Local<v8::Object> Image0 = args[ 0 ]->ToObject();
			if( strcmp( *( v8::String::AsciiValue )( Image0->GetConstructorName()), "Image" ) == 0 ){
				CVsdImage *obj0 = GetThis<CVsdImage>( Image0 );
				if( !obj0 ) return v8::Undefined();
				
				obj = new CVsdImage( *obj0 );
			}else{
				return v8::ThrowException( v8::Exception::Error( v8::String::New(
					"arg[ 0 ] must be Image or string"
				)));
			}
		}else{
			// ファイル名指定で画像ロード
			obj = new CVsdImage();
			v8::String::Value FileName( args[ 0 ] );
			
			if( obj->Load(( LPCWSTR )*FileName ) != ERR_OK ){
				delete obj;
				return v8::Undefined();
			}
		}

		// internal field にバックエンドオブジェクトを設定
		v8::Local<v8::Object> thisObject = args.This();
		thisObject->SetInternalField( 0, v8::External::New( obj ));
		
		// JS オブジェクトが GC されるときにデストラクタが呼ばれるおまじない
		v8::Persistent<v8::Object> objectHolder = v8::Persistent<v8::Object>::New( thisObject );
		objectHolder.MakeWeak( obj, Dispose );
		
		#ifdef DEBUG
			DebugMsgD( ">>>new js obj CVsdImage:%X\n", obj );
		#endif
		// コンストラクタは this を返すこと。
		return thisObject;
	}
	
	// クラスデストラクタ
	static void Dispose( v8::Persistent<v8::Value> handle, void* pVoid ){
		 {
			v8::HandleScope handle_scope;
			CVsdImage *thisObj = GetThis<CVsdImage>( handle->ToObject());
			if( thisObj ){
				delete static_cast<CVsdImage*>( thisObj );
				#ifdef DEBUG
					DebugMsgD( "<<<del js obj CVsdImage:%X\n", thisObj );
				#endif
			}
		}
		handle.Dispose();
	}
	
	// JavaScript からの明示的な破棄
	static v8::Handle<v8::Value> Func_Dispose( const v8::Arguments& args ){
		// obj の Dispose() を呼ぶ
		CVsdImage *thisObj = GetThis<CVsdImage>( args.This());
		 if( thisObj ){
			delete thisObj;
			#ifdef DEBUG
				DebugMsgD( "<<<DISPOSE js obj CVsdImage:%X\n", thisObj );
			#endif
			
			// internalfield を null っぽくする
			args.This()->SetInternalField( 0, v8::External::New( NULL ));
		}
		return v8::Undefined();
	}
	
	///// プロパティアクセサ /////
	static v8::Handle<v8::Value> Get_Width( v8::Local<v8::String> propertyName, const v8::AccessorInfo& info ){
		CVsdImage *obj = GetThis<CVsdImage>( info.Holder());
		return obj ? v8::Integer::New( obj->m_iWidth ) : v8::Undefined();
	}
	static v8::Handle<v8::Value> Get_Height( v8::Local<v8::String> propertyName, const v8::AccessorInfo& info ){
		CVsdImage *obj = GetThis<CVsdImage>( info.Holder());
		return obj ? v8::Integer::New( obj->m_iHeight ) : v8::Undefined();
	}

	///// メャbドコールバック /////
	static v8::Handle<v8::Value> Func_Resize( const v8::Arguments& args ){
		int iLen = args.Length();
		if( CheckArgs( iLen == 2 )) return v8::Undefined();
		
		CVsdImage *thisObj = GetThis<CVsdImage>( args.This());
		if( !thisObj ) return v8::Undefined();
		int ret = thisObj->Resize(
			args[ 0 ]->Int32Value(),
			args[ 1 ]->Int32Value()
		);
		
		return v8::Integer::New( ret );
	}
	static v8::Handle<v8::Value> Func_Rotate( const v8::Arguments& args ){
		int iLen = args.Length();
		if( CheckArgs( iLen == 3 )) return v8::Undefined();
		
		CVsdImage *thisObj = GetThis<CVsdImage>( args.This());
		if( !thisObj ) return v8::Undefined();
		int ret = thisObj->Rotate(
			args[ 0 ]->Int32Value(),
			args[ 1 ]->Int32Value(),
			args[ 2 ]->NumberValue()
		);
		
		return v8::Integer::New( ret );
	}

  public:
	// this へのアクセスヘルパ
	template<typename T>
	static T* GetThis( v8::Local<v8::Object> handle ){
		if( handle->GetInternalField( 0 )->IsUndefined()){
			v8::ThrowException( v8::Exception::TypeError( v8::String::New( "Invalid object ( maybe \"new\" failed )" )));
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
	
	// クラステンプレートの初期化
	static void InitializeClass( v8::Handle<v8::ObjectTemplate> global ){
		// コンストラクタを作成
		v8::Local<v8::FunctionTemplate> tmpl = v8::FunctionTemplate::New( New );
		tmpl->SetClassName( v8::String::New( "Image" ));
		
		// フィールドなどはこちらに
		v8::Handle<v8::ObjectTemplate> inst = tmpl->InstanceTemplate();
		inst->SetInternalFieldCount( 1 );
		inst->SetAccessor( v8::String::New( "Width" ), Get_Width );
		inst->SetAccessor( v8::String::New( "Height" ), Get_Height );

		// メャbドはこちらに
		v8::Handle<v8::ObjectTemplate> proto = tmpl->PrototypeTemplate();
		proto->Set( v8::String::New( "Dispose" ), v8::FunctionTemplate::New( Func_Dispose ));
		proto->Set( v8::String::New( "Resize" ), v8::FunctionTemplate::New( Func_Resize ));
		proto->Set( v8::String::New( "Rotate" ), v8::FunctionTemplate::New( Func_Rotate ));



		// グローバルオブジェクトにクラスを定義
		global->Set( v8::String::New( "Image" ), tmpl );
	}
};
/****************************************************************************/

class CVsdFontIF {
  public:
	// クラスコンストラクタ
	static v8::Handle<v8::Value> New( const v8::Arguments& args ){
		
		// 引数チェック
		if ( args.Length() < 2 ) return v8::Undefined();
		
		v8::String::Value FontName( args[ 0 ] );
		CVsdFont *obj = new CVsdFont(
			( LPCWSTR )*FontName,
			args[ 1 ]->Int32Value(),
			args.Length() <= 2 ? 0 : args[ 2 ]->Int32Value()
		);

		// internal field にバックエンドオブジェクトを設定
		v8::Local<v8::Object> thisObject = args.This();
		thisObject->SetInternalField( 0, v8::External::New( obj ));
		
		// JS オブジェクトが GC されるときにデストラクタが呼ばれるおまじない
		v8::Persistent<v8::Object> objectHolder = v8::Persistent<v8::Object>::New( thisObject );
		objectHolder.MakeWeak( obj, Dispose );
		
		#ifdef DEBUG
			DebugMsgD( ">>>new js obj CVsdFont:%X\n", obj );
		#endif
		// コンストラクタは this を返すこと。
		return thisObject;
	}
	
	// クラスデストラクタ
	static void Dispose( v8::Persistent<v8::Value> handle, void* pVoid ){
		 {
			v8::HandleScope handle_scope;
			CVsdFont *thisObj = GetThis<CVsdFont>( handle->ToObject());
			if( thisObj ){
				delete static_cast<CVsdFont*>( thisObj );
				#ifdef DEBUG
					DebugMsgD( "<<<del js obj CVsdFont:%X\n", thisObj );
				#endif
			}
		}
		handle.Dispose();
	}
	
	// JavaScript からの明示的な破棄
	static v8::Handle<v8::Value> Func_Dispose( const v8::Arguments& args ){
		// obj の Dispose() を呼ぶ
		CVsdFont *thisObj = GetThis<CVsdFont>( args.This());
		 if( thisObj ){
			delete thisObj;
			#ifdef DEBUG
				DebugMsgD( "<<<DISPOSE js obj CVsdFont:%X\n", thisObj );
			#endif
			
			// internalfield を null っぽくする
			args.This()->SetInternalField( 0, v8::External::New( NULL ));
		}
		return v8::Undefined();
	}
	
	///// プロパティアクセサ /////
	static v8::Handle<v8::Value> Get_Height( v8::Local<v8::String> propertyName, const v8::AccessorInfo& info ){
		CVsdFont *obj = GetThis<CVsdFont>( info.Holder());
		return obj ? v8::Integer::New( obj->GetHeight() ) : v8::Undefined();
	}

	///// メャbドコールバック /////
	static v8::Handle<v8::Value> Func_GetTextWidth( const v8::Arguments& args ){
		int iLen = args.Length();
		if( CheckArgs( iLen == 1 )) return v8::Undefined();
		v8::String::Value str0( args[ 0 ] );
		CVsdFont *thisObj = GetThis<CVsdFont>( args.This());
		if( !thisObj ) return v8::Undefined();
		int ret = thisObj->GetTextWidth(
			( LPCWSTR )*str0
		);
		
		return v8::Integer::New( ret );
	}

  public:
	// this へのアクセスヘルパ
	template<typename T>
	static T* GetThis( v8::Local<v8::Object> handle ){
		if( handle->GetInternalField( 0 )->IsUndefined()){
			v8::ThrowException( v8::Exception::TypeError( v8::String::New( "Invalid object ( maybe \"new\" failed )" )));
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
	
	// クラステンプレートの初期化
	static void InitializeClass( v8::Handle<v8::ObjectTemplate> global ){
		// コンストラクタを作成
		v8::Local<v8::FunctionTemplate> tmpl = v8::FunctionTemplate::New( New );
		tmpl->SetClassName( v8::String::New( "Font" ));
		
		// フィールドなどはこちらに
		v8::Handle<v8::ObjectTemplate> inst = tmpl->InstanceTemplate();
		inst->SetInternalFieldCount( 1 );
		inst->SetAccessor( v8::String::New( "Height" ), Get_Height );

		// メャbドはこちらに
		v8::Handle<v8::ObjectTemplate> proto = tmpl->PrototypeTemplate();
		proto->Set( v8::String::New( "Dispose" ), v8::FunctionTemplate::New( Func_Dispose ));
		proto->Set( v8::String::New( "GetTextWidth" ), v8::FunctionTemplate::New( Func_GetTextWidth ));



		// グローバルオブジェクトにクラスを定義
		global->Set( v8::String::New( "Font" ), tmpl );
	}
};
/****************************************************************************/

class CVsdFileIF {
  public:
	// クラスコンストラクタ
	static v8::Handle<v8::Value> New( const v8::Arguments& args ){
		
		CVsdFile *obj = new CVsdFile();

		// internal field にバックエンドオブジェクトを設定
		v8::Local<v8::Object> thisObject = args.This();
		thisObject->SetInternalField( 0, v8::External::New( obj ));
		
		// JS オブジェクトが GC されるときにデストラクタが呼ばれるおまじない
		v8::Persistent<v8::Object> objectHolder = v8::Persistent<v8::Object>::New( thisObject );
		objectHolder.MakeWeak( obj, Dispose );
		
		#ifdef DEBUG
			DebugMsgD( ">>>new js obj CVsdFile:%X\n", obj );
		#endif
		// コンストラクタは this を返すこと。
		return thisObject;
	}
	
	// クラスデストラクタ
	static void Dispose( v8::Persistent<v8::Value> handle, void* pVoid ){
		 {
			v8::HandleScope handle_scope;
			CVsdFile *thisObj = GetThis<CVsdFile>( handle->ToObject());
			if( thisObj ){
				delete static_cast<CVsdFile*>( thisObj );
				#ifdef DEBUG
					DebugMsgD( "<<<del js obj CVsdFile:%X\n", thisObj );
				#endif
			}
		}
		handle.Dispose();
	}
	
	// JavaScript からの明示的な破棄
	static v8::Handle<v8::Value> Func_Dispose( const v8::Arguments& args ){
		// obj の Dispose() を呼ぶ
		CVsdFile *thisObj = GetThis<CVsdFile>( args.This());
		 if( thisObj ){
			delete thisObj;
			#ifdef DEBUG
				DebugMsgD( "<<<DISPOSE js obj CVsdFile:%X\n", thisObj );
			#endif
			
			// internalfield を null っぽくする
			args.This()->SetInternalField( 0, v8::External::New( NULL ));
		}
		return v8::Undefined();
	}
	
	///// プロパティアクセサ /////

	///// メャbドコールバック /////
	static v8::Handle<v8::Value> Func_Open( const v8::Arguments& args ){
		int iLen = args.Length();
		if( CheckArgs( iLen == 2 )) return v8::Undefined();
		v8::String::Value str0( args[ 0 ] );
		v8::String::Value str1( args[ 1 ] );
		CVsdFile *thisObj = GetThis<CVsdFile>( args.This());
		if( !thisObj ) return v8::Undefined();
		int ret = thisObj->Open(
			( LPCWSTR )*str0,
			( LPCWSTR )*str1
		);
		
		return v8::Integer::New( ret );
	}
	static v8::Handle<v8::Value> Func_Close( const v8::Arguments& args ){
		int iLen = args.Length();
		if( CheckArgs( iLen == 0 )) return v8::Undefined();
		
		CVsdFile *thisObj = GetThis<CVsdFile>( args.This());
		if( !thisObj ) return v8::Undefined();
		thisObj->Close();
		
		return v8::Undefined();
	}
	static v8::Handle<v8::Value> Func_ReadLine( const v8::Arguments& args ){
		int iLen = args.Length();
		if( CheckArgs( iLen == 0 )) return v8::Undefined();
		
		CVsdFile *thisObj = GetThis<CVsdFile>( args.This());
		if( !thisObj ) return v8::Undefined();
		char *ret = thisObj->ReadLine();
		
		return v8::String::New( ret );
	}
	static v8::Handle<v8::Value> Func_IsEOF( const v8::Arguments& args ){
		int iLen = args.Length();
		if( CheckArgs( iLen == 0 )) return v8::Undefined();
		
		CVsdFile *thisObj = GetThis<CVsdFile>( args.This());
		if( !thisObj ) return v8::Undefined();
		int ret = thisObj->IsEOF();
		
		return v8::Integer::New( ret );
	}

  public:
	// this へのアクセスヘルパ
	template<typename T>
	static T* GetThis( v8::Local<v8::Object> handle ){
		if( handle->GetInternalField( 0 )->IsUndefined()){
			v8::ThrowException( v8::Exception::TypeError( v8::String::New( "Invalid object ( maybe \"new\" failed )" )));
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
	
	// クラステンプレートの初期化
	static void InitializeClass( v8::Handle<v8::ObjectTemplate> global ){
		// コンストラクタを作成
		v8::Local<v8::FunctionTemplate> tmpl = v8::FunctionTemplate::New( New );
		tmpl->SetClassName( v8::String::New( "File" ));
		
		// フィールドなどはこちらに
		v8::Handle<v8::ObjectTemplate> inst = tmpl->InstanceTemplate();
		inst->SetInternalFieldCount( 1 );

		// メャbドはこちらに
		v8::Handle<v8::ObjectTemplate> proto = tmpl->PrototypeTemplate();
		proto->Set( v8::String::New( "Dispose" ), v8::FunctionTemplate::New( Func_Dispose ));
		proto->Set( v8::String::New( "Open" ), v8::FunctionTemplate::New( Func_Open ));
		proto->Set( v8::String::New( "Close" ), v8::FunctionTemplate::New( Func_Close ));
		proto->Set( v8::String::New( "ReadLine" ), v8::FunctionTemplate::New( Func_ReadLine ));
		proto->Set( v8::String::New( "IsEOF" ), v8::FunctionTemplate::New( Func_IsEOF ));



		// グローバルオブジェクトにクラスを定義
		global->Set( v8::String::New( "File" ), tmpl );
	}
};
