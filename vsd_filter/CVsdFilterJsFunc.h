static v8::Handle<v8::Value> Func_PutPixel( const v8::Arguments& args ){
	int iLen = args.Length();
	CheckArgs( "PutPixel", 3 <= iLen && iLen <= 4 );
	
	CScript::m_Vsd->PutPixel(
		args[ 0 ]->Int32Value(),
		args[ 1 ]->Int32Value(),
		args[ 2 ]->Int32Value(),
		iLen <= 3 ? 0 : args[ 3 ]->Int32Value()
	);
	
	return v8::Undefined();
}
static v8::Handle<v8::Value> Func_DrawLine( const v8::Arguments& args ){
	int iLen = args.Length();
	CheckArgs( "DrawLine", 5 <= iLen && iLen <= 7 );
	
	CScript::m_Vsd->DrawLine(
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
	CheckArgs( "DrawRect", iLen == 6 );
	
	CScript::m_Vsd->DrawRect(
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
	CheckArgs( "DrawCircle", 4 <= iLen && iLen <= 5 );
	
	CScript::m_Vsd->DrawCircle(
		args[ 0 ]->Int32Value(),
		args[ 1 ]->Int32Value(),
		args[ 2 ]->Int32Value(),
		args[ 3 ]->Int32Value(),
		iLen <= 4 ? 0 : args[ 4 ]->Int32Value()
	);
	
	return v8::Undefined();
}
static v8::Handle<v8::Value> Func_DrawGSnake( const v8::Arguments& args ){
	int iLen = args.Length();
	CheckArgs( "DrawGSnake", iLen == 7 );
	
	CScript::m_Vsd->DrawGSnake(
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
	CheckArgs( "DrawMeterPanel0", iLen == 5 );
	v8::Local<v8::Object> font4 = args[ 4 ]->ToObject();
	CheckClass( font4, "Font", "arg[ 4 ] must be Font" );
	CScript::m_Vsd->DrawMeterPanel0(
		args[ 0 ]->Int32Value(),
		args[ 1 ]->Int32Value(),
		args[ 2 ]->Int32Value(),
		args[ 3 ]->Int32Value(),
		*CVsdFont::GetThis( font4 )
	);
	
	return v8::Undefined();
}
static v8::Handle<v8::Value> Func_DrawMeterPanel1( const v8::Arguments& args ){
	int iLen = args.Length();
	CheckArgs( "DrawMeterPanel1", iLen == 5 );
	v8::Local<v8::Object> font4 = args[ 4 ]->ToObject();
	CheckClass( font4, "Font", "arg[ 4 ] must be Font" );
	CScript::m_Vsd->DrawMeterPanel1(
		args[ 0 ]->Int32Value(),
		args[ 1 ]->Int32Value(),
		args[ 2 ]->Int32Value(),
		args[ 3 ]->Int32Value(),
		*CVsdFont::GetThis( font4 )
	);
	
	return v8::Undefined();
}
static v8::Handle<v8::Value> Func_DrawMap( const v8::Arguments& args ){
	int iLen = args.Length();
	CheckArgs( "DrawMap", iLen == 9 );
	
	CScript::m_Vsd->DrawMap(
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
	CheckArgs( "DrawLapTime", iLen == 7 );
	v8::Local<v8::Object> font2 = args[ 2 ]->ToObject();
	CheckClass( font2, "Font", "arg[ 2 ] must be Font" );
	CScript::m_Vsd->DrawLapTime(
		args[ 0 ]->Int32Value(),
		args[ 1 ]->Int32Value(),
		*CVsdFont::GetThis( font2 ),
		args[ 3 ]->Int32Value(),
		args[ 4 ]->Int32Value(),
		args[ 5 ]->Int32Value(),
		args[ 6 ]->Int32Value()
	);
	
	return v8::Undefined();
}
static v8::Handle<v8::Value> Func_DrawNeedle( const v8::Arguments& args ){
	int iLen = args.Length();
	CheckArgs( "DrawNeedle", 7 <= iLen && iLen <= 8 );
	
	CScript::m_Vsd->DrawNeedle(
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
	CheckArgs( "PutImage", iLen == 3 );
	v8::Local<v8::Object> img2 = args[ 2 ]->ToObject();
	CheckClass( img2, "Image", "arg[ 2 ] must be Image" );
	CScript::m_Vsd->PutImage(
		args[ 0 ]->Int32Value(),
		args[ 1 ]->Int32Value(),
		*CVsdImage::GetThis( img2 )
	);
	
	return v8::Undefined();
}
