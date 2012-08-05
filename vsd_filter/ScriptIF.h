/****************************************************************************/

class CVsdFilterIF {
  private:
	// クラスコンストラクタ
	static v8::Handle<v8::Value> New( const v8::Arguments& args ){
		
		CVsdFilter* obj = CScript::m_Vsd;

		// internal field にバックエンドオブジェクトを設定
		v8::Local<v8::Object> thisObject = args.This();
		thisObject->SetInternalField( 0, v8::External::New( obj ));
		
		// JS オブジェクトが GC されるときにデストラクタが呼ばれるおまじない
		v8::Persistent<v8::Object> objectHolder = v8::Persistent<v8::Object>::New( thisObject );
		objectHolder.MakeWeak( obj, Dispose );
		
		// コンストラクタは this を返すこと。
		return thisObject;
	}
	
	// クラスデストラクタ
	static void Dispose( v8::Persistent<v8::Value> handle, void* pVoid ){
		delete static_cast<CVsdFilter*>( pVoid );
	}
	
	///// プロパティアクセサ /////
	static v8::Handle<v8::Value> Get_Width( v8::Local<v8::String> propertyName, const v8::AccessorInfo& info ){
		CVsdFilter *obj = GetThis<CVsdFilter>( info.Holder());
		return obj ? v8::Integer::New( obj->GetWidth() ) : v8::Undefined();
	}
	static v8::Handle<v8::Value> Get_Height( v8::Local<v8::String> propertyName, const v8::AccessorInfo& info ){
		CVsdFilter *obj = GetThis<CVsdFilter>( info.Holder());
		return obj ? v8::Integer::New( obj->GetHeight() ) : v8::Undefined();
	}
	static v8::Handle<v8::Value> Get_MaxFrame( v8::Local<v8::String> propertyName, const v8::AccessorInfo& info ){
		CVsdFilter *obj = GetThis<CVsdFilter>( info.Holder());
		return obj ? v8::Integer::New( obj->GetFrameMax() ) : v8::Undefined();
	}
	static v8::Handle<v8::Value> Get_FrameCnt( v8::Local<v8::String> propertyName, const v8::AccessorInfo& info ){
		CVsdFilter *obj = GetThis<CVsdFilter>( info.Holder());
		return obj ? v8::Integer::New( obj->GetFrameCnt() ) : v8::Undefined();
	}
	static v8::Handle<v8::Value> Get_Speed( v8::Local<v8::String> propertyName, const v8::AccessorInfo& info ){
		CVsdFilter *obj = GetThis<CVsdFilter>( info.Holder());
		return obj ? v8::Number::New( obj->m_dSpeed ) : v8::Undefined();
	}
	static v8::Handle<v8::Value> Get_Tacho( v8::Local<v8::String> propertyName, const v8::AccessorInfo& info ){
		CVsdFilter *obj = GetThis<CVsdFilter>( info.Holder());
		return obj ? v8::Number::New( obj->m_dTacho ) : v8::Undefined();
	}
	static v8::Handle<v8::Value> Get_Gx( v8::Local<v8::String> propertyName, const v8::AccessorInfo& info ){
		CVsdFilter *obj = GetThis<CVsdFilter>( info.Holder());
		return obj ? v8::Number::New( obj->m_dGx ) : v8::Undefined();
	}
	static v8::Handle<v8::Value> Get_Gy( v8::Local<v8::String> propertyName, const v8::AccessorInfo& info ){
		CVsdFilter *obj = GetThis<CVsdFilter>( info.Holder());
		return obj ? v8::Number::New( obj->m_dGy ) : v8::Undefined();
	}
	static v8::Handle<v8::Value> Get_MaxSpeed( v8::Local<v8::String> propertyName, const v8::AccessorInfo& info ){
		CVsdFilter *obj = GetThis<CVsdFilter>( info.Holder());
		return obj ? v8::Integer::New( obj->m_iMaxSpeed ) : v8::Undefined();
	}

	///// メソッドコールバック /////
	/*** DrawArc ****************************************************************/
	
	static v8::Handle<v8::Value> Func_DrawArc( const v8::Arguments& args ){
		int iLen = args.Length();
		if( CheckArgs( 7 <= iLen && iLen <= 10 )) return v8::Undefined();
		
		if( iLen >= 9 ){
			CScript::m_Vsd->DrawArc(
				args[ 0 ]->Int32Value(),
				args[ 1 ]->Int32Value(),
				args[ 2 ]->Int32Value(),
				args[ 3 ]->Int32Value(),
				args[ 4 ]->Int32Value(),
				args[ 5 ]->Int32Value(),
				args[ 6 ]->NumberValue(),
				args[ 7 ]->NumberValue(),
				PIXEL_RABY::Argb2Raby( args[ 8 ]->Int32Value()),
				iLen <= 9 ? 0 : args[ 9 ]->Int32Value()
			);
		}else{
			CScript::m_Vsd->DrawArc(
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
		if( CheckArgs( iLen == 3 )) return v8::Undefined();
		v8::Local<v8::Object> Image2 = args[ 2 ]->ToObject();
		if( CheckClass( Image2, "Image", "arg[ 3 ] must be Image" )) return v8::Undefined();
		CVsdImage *obj2 = GetThis<CVsdImage>( Image2 );
		if( !obj2 ) return v8::Undefined();
		CVsdFilter *thisObj = GetThis<CVsdFilter>( args.This());
		if( !thisObj ) return v8::Undefined();
		int ret = thisObj->PutImage(
			args[ 0 ]->Int32Value(),
			args[ 1 ]->Int32Value(),
			*obj2
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
		if( CheckArgs( iLen == 6 )) return v8::Undefined();
		
		CVsdFilter *thisObj = GetThis<CVsdFilter>( args.This());
		if( !thisObj ) return v8::Undefined();
		thisObj->DrawRect(
			args[ 0 ]->Int32Value(),
			args[ 1 ]->Int32Value(),
			args[ 2 ]->Int32Value(),
			args[ 3 ]->Int32Value(),
			PIXEL_RABY::Argb2Raby( args[ 4 ]->Int32Value()),
			args[ 5 ]->Int32Value()
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
		if( CheckArgs( 5 <= iLen && iLen <= 6 )) return v8::Undefined();
		v8::String::AsciiValue str2( args[ 2 ] );
		v8::Local<v8::Object> Font3 = args[ 3 ]->ToObject();
		if( CheckClass( Font3, "Font", "arg[ 4 ] must be Font" )) return v8::Undefined();
		CVsdFont *obj3 = GetThis<CVsdFont>( Font3 );
		if( !obj3 ) return v8::Undefined();
		CVsdFilter *thisObj = GetThis<CVsdFilter>( args.This());
		if( !thisObj ) return v8::Undefined();
		thisObj->DrawText(
			args[ 0 ]->Int32Value(),
			args[ 1 ]->Int32Value(),
			*str2,
			*obj3,
			PIXEL_RABY::Argb2Raby( args[ 4 ]->Int32Value()),
			iLen <= 5 ? color_black : PIXEL_RABY::Argb2Raby( args[ 5 ]->Int32Value())
		);
		
		return v8::Undefined();
	}
	static v8::Handle<v8::Value> Func_DrawGSnake( const v8::Arguments& args ){
		int iLen = args.Length();
		if( CheckArgs( iLen == 7 )) return v8::Undefined();
		
		CVsdFilter *thisObj = GetThis<CVsdFilter>( args.This());
		if( !thisObj ) return v8::Undefined();
		thisObj->DrawGSnake(
			args[ 0 ]->Int32Value(),
			args[ 1 ]->Int32Value(),
			args[ 2 ]->Int32Value(),
			args[ 3 ]->Int32Value(),
			args[ 4 ]->Int32Value(),
			PIXEL_RABY::Argb2Raby( args[ 5 ]->Int32Value()),
			PIXEL_RABY::Argb2Raby( args[ 6 ]->Int32Value())
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
		if( CheckArgs( iLen == 9 )) return v8::Undefined();
		
		CVsdFilter *thisObj = GetThis<CVsdFilter>( args.This());
		if( !thisObj ) return v8::Undefined();
		thisObj->DrawMap(
			args[ 0 ]->Int32Value(),
			args[ 1 ]->Int32Value(),
			args[ 2 ]->Int32Value(),
			args[ 3 ]->Int32Value(),
			args[ 4 ]->Int32Value(),
			PIXEL_RABY::Argb2Raby( args[ 5 ]->Int32Value()),
			PIXEL_RABY::Argb2Raby( args[ 6 ]->Int32Value()),
			PIXEL_RABY::Argb2Raby( args[ 7 ]->Int32Value()),
			PIXEL_RABY::Argb2Raby( args[ 8 ]->Int32Value())
		);
		
		return v8::Undefined();
	}
	static v8::Handle<v8::Value> Func_DrawLapTime( const v8::Arguments& args ){
		int iLen = args.Length();
		if( CheckArgs( iLen == 7 )) return v8::Undefined();
		v8::Local<v8::Object> Font2 = args[ 2 ]->ToObject();
		if( CheckClass( Font2, "Font", "arg[ 3 ] must be Font" )) return v8::Undefined();
		CVsdFont *obj2 = GetThis<CVsdFont>( Font2 );
		if( !obj2 ) return v8::Undefined();
		CVsdFilter *thisObj = GetThis<CVsdFilter>( args.This());
		if( !thisObj ) return v8::Undefined();
		thisObj->DrawLapTime(
			args[ 0 ]->Int32Value(),
			args[ 1 ]->Int32Value(),
			*obj2,
			PIXEL_RABY::Argb2Raby( args[ 3 ]->Int32Value()),
			PIXEL_RABY::Argb2Raby( args[ 4 ]->Int32Value()),
			PIXEL_RABY::Argb2Raby( args[ 5 ]->Int32Value()),
			PIXEL_RABY::Argb2Raby( args[ 6 ]->Int32Value())
		);
		
		return v8::Undefined();
	}
	static v8::Handle<v8::Value> Func_DrawNeedle( const v8::Arguments& args ){
		int iLen = args.Length();
		if( CheckArgs( 7 <= iLen && iLen <= 8 )) return v8::Undefined();
		
		CVsdFilter *thisObj = GetThis<CVsdFilter>( args.This());
		if( !thisObj ) return v8::Undefined();
		thisObj->DrawNeedle(
			args[ 0 ]->Int32Value(),
			args[ 1 ]->Int32Value(),
			args[ 2 ]->Int32Value(),
			args[ 3 ]->Int32Value(),
			args[ 4 ]->Int32Value(),
			args[ 5 ]->NumberValue(),
			PIXEL_RABY::Argb2Raby( args[ 6 ]->Int32Value()),
			iLen <= 7 ? 1 : args[ 7 ]->Int32Value()
		);
		
		return v8::Undefined();
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
		tmpl->SetClassName( v8::String::New( "__builtin_vsd_class__" ));
		
		// フィールドなどはこちらに
		v8::Handle<v8::ObjectTemplate> inst = tmpl->InstanceTemplate();
		inst->SetInternalFieldCount( 1 );
		inst->SetAccessor( v8::String::New( "Width" ), Get_Width );
		inst->SetAccessor( v8::String::New( "Height" ), Get_Height );
		inst->SetAccessor( v8::String::New( "MaxFrame" ), Get_MaxFrame );
		inst->SetAccessor( v8::String::New( "FrameCnt" ), Get_FrameCnt );
		inst->SetAccessor( v8::String::New( "Speed" ), Get_Speed );
		inst->SetAccessor( v8::String::New( "Tacho" ), Get_Tacho );
		inst->SetAccessor( v8::String::New( "Gx" ), Get_Gx );
		inst->SetAccessor( v8::String::New( "Gy" ), Get_Gy );
		inst->SetAccessor( v8::String::New( "MaxSpeed" ), Get_MaxSpeed );

		// メソッドはこちらに
		v8::Handle<v8::ObjectTemplate> proto = tmpl->PrototypeTemplate();
		proto->Set( v8::String::New( "DrawArc" ), v8::FunctionTemplate::New( Func_DrawArc ));
		proto->Set( v8::String::New( "print" ), v8::FunctionTemplate::New( Func_print ));
		proto->Set( v8::String::New( "PutPixel" ), v8::FunctionTemplate::New( Func_PutPixel ));
		proto->Set( v8::String::New( "PutImage" ), v8::FunctionTemplate::New( Func_PutImage ));
		proto->Set( v8::String::New( "DrawLine" ), v8::FunctionTemplate::New( Func_DrawLine ));
		proto->Set( v8::String::New( "DrawRect" ), v8::FunctionTemplate::New( Func_DrawRect ));
		proto->Set( v8::String::New( "DrawCircle" ), v8::FunctionTemplate::New( Func_DrawCircle ));
		proto->Set( v8::String::New( "DrawText" ), v8::FunctionTemplate::New( Func_DrawText ));
		proto->Set( v8::String::New( "DrawGSnake" ), v8::FunctionTemplate::New( Func_DrawGSnake ));
		proto->Set( v8::String::New( "DrawMeterScale" ), v8::FunctionTemplate::New( Func_DrawMeterScale ));
		proto->Set( v8::String::New( "DrawMap" ), v8::FunctionTemplate::New( Func_DrawMap ));
		proto->Set( v8::String::New( "DrawLapTime" ), v8::FunctionTemplate::New( Func_DrawLapTime ));
		proto->Set( v8::String::New( "DrawNeedle" ), v8::FunctionTemplate::New( Func_DrawNeedle ));

		// グローバルオブジェクトにクラスを定義
		global->Set( v8::String::New( "__builtin_vsd_class__" ), tmpl );
	}
};
/****************************************************************************/

class CVsdImageIF {
  private:
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
			v8::String::AsciiValue FileName( args[ 0 ] );
			
			if( obj->Load( *FileName ) != ERR_OK ){
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
		
		// コンストラクタは this を返すこと。
		return thisObject;
	}
	
	// クラスデストラクタ
	static void Dispose( v8::Persistent<v8::Value> handle, void* pVoid ){
		delete static_cast<CVsdImage*>( pVoid );
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

	///// メソッドコールバック /////
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

		// メソッドはこちらに
		v8::Handle<v8::ObjectTemplate> proto = tmpl->PrototypeTemplate();
		proto->Set( v8::String::New( "Resize" ), v8::FunctionTemplate::New( Func_Resize ));
		proto->Set( v8::String::New( "Rotate" ), v8::FunctionTemplate::New( Func_Rotate ));

		// グローバルオブジェクトにクラスを定義
		global->Set( v8::String::New( "Image" ), tmpl );
	}
};
/****************************************************************************/

class CVsdFontIF {
  private:
	// クラスコンストラクタ
	static v8::Handle<v8::Value> New( const v8::Arguments& args ){
		
		// 引数チェック
		if ( args.Length() < 2 ) return v8::Undefined();
		
		v8::String::AsciiValue FontName( args[ 0 ] );
		CVsdFont *obj = new CVsdFont(
			*FontName,
			args[ 1 ]->Int32Value(),
			args.Length() <= 2 ? 0 : args[ 2 ]->Int32Value()
		);

		// internal field にバックエンドオブジェクトを設定
		v8::Local<v8::Object> thisObject = args.This();
		thisObject->SetInternalField( 0, v8::External::New( obj ));
		
		// JS オブジェクトが GC されるときにデストラクタが呼ばれるおまじない
		v8::Persistent<v8::Object> objectHolder = v8::Persistent<v8::Object>::New( thisObject );
		objectHolder.MakeWeak( obj, Dispose );
		
		// コンストラクタは this を返すこと。
		return thisObject;
	}
	
	// クラスデストラクタ
	static void Dispose( v8::Persistent<v8::Value> handle, void* pVoid ){
		delete static_cast<CVsdFont*>( pVoid );
	}
	
	///// プロパティアクセサ /////
	static v8::Handle<v8::Value> Get_Height( v8::Local<v8::String> propertyName, const v8::AccessorInfo& info ){
		CVsdFont *obj = GetThis<CVsdFont>( info.Holder());
		return obj ? v8::Integer::New( obj->GetHeight() ) : v8::Undefined();
	}

	///// メソッドコールバック /////
	static v8::Handle<v8::Value> Func_GetTextWidth( const v8::Arguments& args ){
		int iLen = args.Length();
		if( CheckArgs( iLen == 1 )) return v8::Undefined();
		v8::String::AsciiValue str0( args[ 0 ] );
		CVsdFont *thisObj = GetThis<CVsdFont>( args.This());
		if( !thisObj ) return v8::Undefined();
		int ret = thisObj->GetTextWidth(
			*str0
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

		// メソッドはこちらに
		v8::Handle<v8::ObjectTemplate> proto = tmpl->PrototypeTemplate();
		proto->Set( v8::String::New( "GetTextWidth" ), v8::FunctionTemplate::New( Func_GetTextWidth ));

		// グローバルオブジェクトにクラスを定義
		global->Set( v8::String::New( "Font" ), tmpl );
	}
};
