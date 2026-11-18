class CJsMap : public Handle<Value> {
  public:
	
	// [] オペレータ
	CJsMap operator []( LPCWSTR wszPropName ){
		return Cast( ToArray()->Get( String::NewFromTwoByte( Isolate::GetCurrent(), ( uint16_t *)wszPropName )));
	}
	
	CJsMap operator []( LPCSTR szPropName ){
		return Cast( ToArray()->Get( String::NewFromOneByte( Isolate::GetCurrent(), ( uint8_t *)szPropName )));
	}
	
	CJsMap operator []( int iIndex ){
		return Cast( ToArray()->Get( iIndex ));
	}
	
	// 変換
	static CJsMap Cast( Handle<Value> handle ){
		return *( static_cast<CJsMap *>( &handle ));
	}
	
	Handle<Array> ToArray( void ){
		return Handle<Array>::Cast( ToValue());
	}
	
	Handle<Value> ToValue( void ){
		return static_cast<Handle<Value>>( *this );
	}
	
	// 値取得
	int GetInt32( void ){ return ToValue()->Int32Value(); }
	int GetInt32( int iDefault ){
		return ToValue()->IsUndefined() ? iDefault : ToValue()->Int32Value();
	}
	
	double GetDouble( void ){ return ToValue()->NumberValue(); }
	double GetDouble( double dDefault ){
		return ToValue()->IsUndefined() ? dDefault : ToValue()->NumberValue();
	}
	
	char *GetStr( void ){
		String::Utf8Value str( ToValue());
		return *str;
	}
	char *GetStr( char *szDefault ){
		if( ToValue()->IsUndefined()) return szDefault;
		return GetStr();
	}
	
	template <class T>
	T *GetObj( void ){
		return ToValue()->IsExternal() ? static_cast<T *>(
			Handle<External>::Cast( ToValue())->Value()
		) : NULL;
	}
};
