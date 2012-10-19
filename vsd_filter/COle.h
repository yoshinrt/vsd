/*****************************************************************************
	
	VSD -- vehicle data logger system  Copyright(C) by DDS
	
	COle.cpp - OLE class implementation
	
*****************************************************************************/

#pragma once
#include "error_code.h"

class COle {
  public:
	COle( void ){
		m_pApp	= NULL;
	}
	
	~COle(){
		if( m_pApp ) m_pApp->Release();
	}
	
	HRESULT AutoWrap( int autoType, VARIANT *pvResult, LPOLESTR ptName, int cArgs... );
	UINT CreateInstance( LPCWSTR strServer );
	
	static void COle::InitJS( v8::Local<v8::FunctionTemplate> tmpl );
	
	static v8::Handle<v8::Value> PropGetter(
		v8::Local<v8::String> property,
		const v8::AccessorInfo& info
	);
	
	static v8::Handle<v8::Value> PropSetter(
		v8::Local<v8::String> property,
		v8::Local<v8::Value> value,
		const v8::AccessorInfo& info
	);
	
	// èâä˙âªÅEèIóπ
	static void Initialize( void ){		CoInitialize( NULL ); }
	static void Uninitialize( void ){	CoUninitialize(); }
	
  private:
	IDispatch	*m_pApp;
};
