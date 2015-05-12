class CV8Map : public v8::Handle<v8::Value> {
  public:
	
	// [] オペレータ
	CV8Map operator []( LPCWSTR wszPropName ){
		return Cast( ToArray()->Get( v8::String::New(( uint16_t *)wszPropName )));
	}
	
	CV8Map operator []( LPCSTR szPropName ){
		return Cast( ToArray()->Get( v8::String::New( szPropName )));
	}
	
	CV8Map operator []( int iIndex ){
		return Cast( ToArray()->Get( iIndex ));
	}
	
	// 変換
	static CV8Map Cast( v8::Handle<v8::Value> handle ){
		return *( static_cast<CV8Map *>( &handle ));
	}
	
	v8::Handle<v8::Array> ToArray( void ){
		return v8::Handle<v8::Array>::Cast( ToValue());
	}
	
	v8::Handle<v8::Value>& ToValue( void ){
		return *static_cast<v8::Handle<v8::Value> *>( this );
	}
	
	// 値取得
	int Int32( void ){ return ToValue()->Int32Value(); }
	int Int32( int iDefault ){
		return ToValue()->IsUndefined() ? iDefault : ToValue()->Int32Value();
	}
	
	double GetDouble( void ){ return ToValue()->NumberValue(); }
	double GetDouble( double dDefault ){
		return ToValue()->IsUndefined() ? dDefault : ToValue()->NumberValue();
	}
	
	char *GetStr( void ){
		v8::String::AsciiValue str( ToValue());
		return *str;
	}
	char *GetStr( char *szDefault ){
		if( ToValue()->IsUndefined()) return szDefault;
		return GetStr();
	}
	
	template <class T>
	T *GetObj( void ){
		return ToValue()->IsExternal() ? static_cast<T *>(
			v8::Handle<v8::External>::Cast( ToValue())->Value()
		) : NULL;
	}
};
