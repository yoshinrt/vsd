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
			strcmp( *( v8::String::AsciiValue )( obj->GetConstructorName()), name ) \
		) return v8::ThrowException( v8::Exception::SyntaxError( v8::String::New( msg )))
	
	///// プロパティアクセサ /////
	static v8::Handle<v8::Value> Get_Width( v8::Local<v8::String> propertyName, const v8::AccessorInfo& info ){
		 return v8::Integer::New( GetThis<CVsdFilter>( info.Holder())->GetWidth() );
	}
	static v8::Handle<v8::Value> Get_Height( v8::Local<v8::String> propertyName, const v8::AccessorInfo& info ){
		 return v8::Integer::New( GetThis<CVsdFilter>( info.Holder())->GetHeight() );
	}
	static v8::Handle<v8::Value> Get_MaxFrame( v8::Local<v8::String> propertyName, const v8::AccessorInfo& info ){
		 return v8::Integer::New( GetThis<CVsdFilter>( info.Holder())->GetFrameMax() );
	}
	static v8::Handle<v8::Value> Get_FrameCnt( v8::Local<v8::String> propertyName, const v8::AccessorInfo& info ){
		 return v8::Integer::New( GetThis<CVsdFilter>( info.Holder())->GetFrameCnt() );
	}
	static v8::Handle<v8::Value> Get_Speed( v8::Local<v8::String> propertyName, const v8::AccessorInfo& info ){
		 return v8::Number::New( GetThis<CVsdFilter>( info.Holder())->m_dSpeed );
	}
	static v8::Handle<v8::Value> Get_Tacho( v8::Local<v8::String> propertyName, const v8::AccessorInfo& info ){
		 return v8::Number::New( GetThis<CVsdFilter>( info.Holder())->m_dTacho );
	}
	static v8::Handle<v8::Value> Get_Gx( v8::Local<v8::String> propertyName, const v8::AccessorInfo& info ){
		 return v8::Number::New( GetThis<CVsdFilter>( info.Holder())->m_dGx );
	}
	static v8::Handle<v8::Value> Get_Gy( v8::Local<v8::String> propertyName, const v8::AccessorInfo& info ){
		 return v8::Number::New( GetThis<CVsdFilter>( info.Holder())->m_dGy );
	}
	static v8::Handle<v8::Value> Get_MaxSpeed( v8::Local<v8::String> propertyName, const v8::AccessorInfo& info ){
		 return v8::Integer::New( GetThis<CVsdFilter>( info.Holder())->m_iMaxSpeed );
	}

	///// メソッドコールバック /////
	/*** DrawArc ****************************************************************/
	
	static v8::Handle<v8::Value> Func_DrawArc( const v8::Arguments& args ){
		int iLen = args.Length();
		CheckArgs( "DrawArc", 7 <= iLen && iLen <= 10 );
		
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
				args[ 8 ]->Int32Value(),
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
				args[ 6 ]->Int32Value(),
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
		CheckArgs( PutPixel, 3 <= iLen && iLen <= 4 );
		
		GetThis<CVsdFilter>( args.This())->PutPixel(
			args[ 0 ]->Int32Value(),
			args[ 1 ]->Int32Value(),
			args[ 2 ]->Int32Value(),
			iLen <= 3 ? 0 : args[ 3 ]->Int32Value()
		);
		
		return v8::Undefined();
	}
	static v8::Handle<v8::Value> Func_DrawLine( const v8::Arguments& args ){
		int iLen = args.Length();
		CheckArgs( DrawLine, 5 <= iLen && iLen <= 7 );
		
		GetThis<CVsdFilter>( args.This())->DrawLine(
			args[ 0 ]->Int32Value(),
			args[ 1 ]->Int32Value(),
			args[ 2 ]->Int32Value(),
			args[ 3 ]->Int32Value(),
			iLen <= 5 ? 1 : args[ 5 ]->Int32Value(),
			args[ 4 ]->Int32Value(),
			iLen <= 6 ? 0 : args[ 6 ]->Int32Value()
		);
		
		return v8::Undefined();
	}
	static v8::Handle<v8::Value> Func_DrawRect( const v8::Arguments& args ){
		int iLen = args.Length();
		CheckArgs( DrawRect, iLen == 6 );
		
		GetThis<CVsdFilter>( args.This())->DrawRect(
			args[ 0 ]->Int32Value(),
			args[ 1 ]->Int32Value(),
			args[ 2 ]->Int32Value(),
			args[ 3 ]->Int32Value(),
			args[ 4 ]->Int32Value(),
			args[ 5 ]->Int32Value()
		);
		
		return v8::Undefined();
	}
	static v8::Handle<v8::Value> Func_DrawCircle( const v8::Arguments& args ){
		int iLen = args.Length();
		CheckArgs( DrawCircle, 4 <= iLen && iLen <= 5 );
		
		GetThis<CVsdFilter>( args.This())->DrawCircle(
			args[ 0 ]->Int32Value(),
			args[ 1 ]->Int32Value(),
			args[ 2 ]->Int32Value(),
			args[ 3 ]->Int32Value(),
			iLen <= 4 ? 0 : args[ 4 ]->Int32Value()
		);
		
		return v8::Undefined();
	}
	static v8::Handle<v8::Value> Func_DrawText( const v8::Arguments& args ){
		int iLen = args.Length();
		CheckArgs( DrawText, 5 <= iLen && iLen <= 6 );
		v8::String::AsciiValue str2( args[ 2 ] );
		v8::Local<v8::Object> Font3 = args[ 3 ]->ToObject();
		CheckClass( Font3, "Font", "arg[ 3 ] must be Font" );
		GetThis<CVsdFilter>( args.This())->DrawText(
			args[ 0 ]->Int32Value(),
			args[ 1 ]->Int32Value(),
			*str2,
			*GetThis<CVsdFont>( Font3 ),
			args[ 4 ]->Int32Value(),
			iLen <= 5 ? 0 : args[ 5 ]->Int32Value()
		);
		
		return v8::Undefined();
	}
	static v8::Handle<v8::Value> Func_DrawGSnake( const v8::Arguments& args ){
		int iLen = args.Length();
		CheckArgs( DrawGSnake, iLen == 7 );
		
		GetThis<CVsdFilter>( args.This())->DrawGSnake(
			args[ 0 ]->Int32Value(),
			args[ 1 ]->Int32Value(),
			args[ 2 ]->Int32Value(),
			args[ 3 ]->Int32Value(),
			args[ 4 ]->Int32Value(),
			args[ 5 ]->Int32Value(),
			args[ 6 ]->Int32Value()
		);
		
		return v8::Undefined();
	}
	static v8::Handle<v8::Value> Func_DrawMeterPanel0( const v8::Arguments& args ){
		int iLen = args.Length();
		CheckArgs( DrawMeterPanel0, iLen == 5 );
		v8::Local<v8::Object> Font4 = args[ 4 ]->ToObject();
		CheckClass( Font4, "Font", "arg[ 4 ] must be Font" );
		GetThis<CVsdFilter>( args.This())->DrawMeterPanel0(
			args[ 0 ]->Int32Value(),
			args[ 1 ]->Int32Value(),
			args[ 2 ]->Int32Value(),
			args[ 3 ]->Int32Value(),
			*GetThis<CVsdFont>( Font4 )
		);
		
		return v8::Undefined();
	}
	static v8::Handle<v8::Value> Func_DrawMeterPanel1( const v8::Arguments& args ){
		int iLen = args.Length();
		CheckArgs( DrawMeterPanel1, iLen == 5 );
		v8::Local<v8::Object> Font4 = args[ 4 ]->ToObject();
		CheckClass( Font4, "Font", "arg[ 4 ] must be Font" );
		GetThis<CVsdFilter>( args.This())->DrawMeterPanel1(
			args[ 0 ]->Int32Value(),
			args[ 1 ]->Int32Value(),
			args[ 2 ]->Int32Value(),
			args[ 3 ]->Int32Value(),
			*GetThis<CVsdFont>( Font4 )
		);
		
		return v8::Undefined();
	}
	static v8::Handle<v8::Value> Func_DrawMap( const v8::Arguments& args ){
		int iLen = args.Length();
		CheckArgs( DrawMap, iLen == 9 );
		
		GetThis<CVsdFilter>( args.This())->DrawMap(
			args[ 0 ]->Int32Value(),
			args[ 1 ]->Int32Value(),
			args[ 2 ]->Int32Value(),
			args[ 3 ]->Int32Value(),
			args[ 4 ]->Int32Value(),
			args[ 5 ]->Int32Value(),
			args[ 6 ]->Int32Value(),
			args[ 7 ]->Int32Value(),
			args[ 8 ]->Int32Value()
		);
		
		return v8::Undefined();
	}
	static v8::Handle<v8::Value> Func_DrawLapTime( const v8::Arguments& args ){
		int iLen = args.Length();
		CheckArgs( DrawLapTime, iLen == 7 );
		v8::Local<v8::Object> Font2 = args[ 2 ]->ToObject();
		CheckClass( Font2, "Font", "arg[ 2 ] must be Font" );
		GetThis<CVsdFilter>( args.This())->DrawLapTime(
			args[ 0 ]->Int32Value(),
			args[ 1 ]->Int32Value(),
			*GetThis<CVsdFont>( Font2 ),
			args[ 3 ]->Int32Value(),
			args[ 4 ]->Int32Value(),
			args[ 5 ]->Int32Value(),
			args[ 6 ]->Int32Value()
		);
		
		return v8::Undefined();
	}
	static v8::Handle<v8::Value> Func_DrawNeedle( const v8::Arguments& args ){
		int iLen = args.Length();
		CheckArgs( DrawNeedle, 7 <= iLen && iLen <= 8 );
		
		GetThis<CVsdFilter>( args.This())->DrawNeedle(
			args[ 0 ]->Int32Value(),
			args[ 1 ]->Int32Value(),
			args[ 2 ]->Int32Value(),
			args[ 3 ]->Int32Value(),
			args[ 4 ]->Int32Value(),
			args[ 5 ]->NumberValue(),
			args[ 6 ]->Int32Value(),
			iLen <= 7 ? 1 : args[ 7 ]->Int32Value()
		);
		
		return v8::Undefined();
	}
	static v8::Handle<v8::Value> Func_PutImage( const v8::Arguments& args ){
		int iLen = args.Length();
		CheckArgs( PutImage, iLen == 3 );
		v8::Local<v8::Object> Image2 = args[ 2 ]->ToObject();
		CheckClass( Image2, "Image", "arg[ 2 ] must be Image" );
		int ret = GetThis<CVsdFilter>( args.This())->PutImage(
			args[ 0 ]->Int32Value(),
			args[ 1 ]->Int32Value(),
			*GetThis<CVsdImage>( Image2 )
		);
		
		return v8::Integer::New( ret );
	}

  public:
	// this へのアクセスヘルパ
	template<typename T>
	static T* GetThis( v8::Local<v8::Object> handle ){
		 void* pThis = v8::Local<v8::External>::Cast( handle->GetInternalField( 0 ))->Value();
		 return static_cast<T*>( pThis );
	}
	
	// クラステンプレートの初期化
	static void InitializeClass( v8::Handle<v8::ObjectTemplate> global ){
		// コンストラクタを作成
		v8::Local<v8::FunctionTemplate> tmpl = v8::FunctionTemplate::New( New );
		tmpl->SetClassName( v8::String::New( "Vsd" ));
		
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
		proto->Set( v8::String::New( "DrawLine" ), v8::FunctionTemplate::New( Func_DrawLine ));
		proto->Set( v8::String::New( "DrawRect" ), v8::FunctionTemplate::New( Func_DrawRect ));
		proto->Set( v8::String::New( "DrawCircle" ), v8::FunctionTemplate::New( Func_DrawCircle ));
		proto->Set( v8::String::New( "DrawText" ), v8::FunctionTemplate::New( Func_DrawText ));
		proto->Set( v8::String::New( "DrawGSnake" ), v8::FunctionTemplate::New( Func_DrawGSnake ));
		proto->Set( v8::String::New( "DrawMeterPanel0" ), v8::FunctionTemplate::New( Func_DrawMeterPanel0 ));
		proto->Set( v8::String::New( "DrawMeterPanel1" ), v8::FunctionTemplate::New( Func_DrawMeterPanel1 ));
		proto->Set( v8::String::New( "DrawMap" ), v8::FunctionTemplate::New( Func_DrawMap ));
		proto->Set( v8::String::New( "DrawLapTime" ), v8::FunctionTemplate::New( Func_DrawLapTime ));
		proto->Set( v8::String::New( "DrawNeedle" ), v8::FunctionTemplate::New( Func_DrawNeedle ));
		proto->Set( v8::String::New( "PutImage" ), v8::FunctionTemplate::New( Func_PutImage ));

		// グローバルオブジェクトにクラスを定義
		global->Set( v8::String::New( "Vsd" ), tmpl );
	}
};
/****************************************************************************/

class CVsdImageIF {
  private:
	// クラスコンストラクタ
	static v8::Handle<v8::Value> New( const v8::Arguments& args ){
		
		// 引数チェック
		if ( args.Length() <= 0 ) return v8::Undefined();
		
		CVsdImage* obj = new CVsdImage();
		v8::String::AsciiValue FileName( args[ 0 ] );
		
		if( obj->Load( *FileName ) != ERROR_OK ){
			delete obj;
			return v8::Undefined();
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
			strcmp( *( v8::String::AsciiValue )( obj->GetConstructorName()), name ) \
		) return v8::ThrowException( v8::Exception::SyntaxError( v8::String::New( msg )))
	
	///// プロパティアクセサ /////
	static v8::Handle<v8::Value> Get_Width( v8::Local<v8::String> propertyName, const v8::AccessorInfo& info ){
		 return v8::Integer::New( GetThis<CVsdImage>( info.Holder())->m_iWidth );
	}
	static v8::Handle<v8::Value> Get_Height( v8::Local<v8::String> propertyName, const v8::AccessorInfo& info ){
		 return v8::Integer::New( GetThis<CVsdImage>( info.Holder())->m_iHeight );
	}

	///// メソッドコールバック /////
	static v8::Handle<v8::Value> Func_Resize( const v8::Arguments& args ){
		int iLen = args.Length();
		CheckArgs( Resize, iLen == 2 );
		
		int ret = GetThis<CVsdImage>( args.This())->Resize(
			args[ 0 ]->Int32Value(),
			args[ 1 ]->Int32Value()
		);
		
		return v8::Integer::New( ret );
	}
	static v8::Handle<v8::Value> Func_Rotate( const v8::Arguments& args ){
		int iLen = args.Length();
		CheckArgs( Rotate, iLen == 3 );
		
		int ret = GetThis<CVsdImage>( args.This())->Rotate(
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
		 void* pThis = v8::Local<v8::External>::Cast( handle->GetInternalField( 0 ))->Value();
		 return static_cast<T*>( pThis );
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
			strcmp( *( v8::String::AsciiValue )( obj->GetConstructorName()), name ) \
		) return v8::ThrowException( v8::Exception::SyntaxError( v8::String::New( msg )))
	
	///// プロパティアクセサ /////
	static v8::Handle<v8::Value> Get_Height( v8::Local<v8::String> propertyName, const v8::AccessorInfo& info ){
		 return v8::Integer::New( GetThis<CVsdFont>( info.Holder())->GetH() );
	}

	///// メソッドコールバック /////
	static v8::Handle<v8::Value> Func_GetTextWidth( const v8::Arguments& args ){
		int iLen = args.Length();
		CheckArgs( GetTextWidth, iLen == 1 );
		v8::String::AsciiValue str0( args[ 0 ] );
		int ret = GetThis<CVsdFont>( args.This())->GetTextWidth(
			*str0
		);
		
		return v8::Integer::New( ret );
	}

  public:
	// this へのアクセスヘルパ
	template<typename T>
	static T* GetThis( v8::Local<v8::Object> handle ){
		 void* pThis = v8::Local<v8::External>::Cast( handle->GetInternalField( 0 ))->Value();
		 return static_cast<T*>( pThis );
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
