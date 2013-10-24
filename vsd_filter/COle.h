/*****************************************************************************
	
	VSD -- vehicle data logger system  Copyright(C) by DDS
	
	COle.cpp - OLE class implementation
	
*****************************************************************************/

#pragma once
#include "error_code.h"

class COle {
  public:
	COle( void ){
		m_pApp			= NULL;
	}
	
	~COle(){
		if( m_pApp ) m_pApp->Release();
	}
	
	UINT CreateInstance( LPCWSTR strServer );
	
	static v8::Handle<v8::Value> CreateActiveXObject(
		IDispatch *pDispatch,
		v8::Handle<v8::Context> Context
	);
	
	// Ruby win32ole à⁄êAï®
	void Val2Variant(
		v8::Local<v8::Value> val,
		VARIANT *var
	);
	v8::Handle<v8::Value> Variant2Val( VARIANT *pvar, v8::Handle<v8::Context> Context );
	v8::Handle<v8::Value> Invoke(
		LPCWSTR wszPropName,
		const 	v8::Arguments& args,
		v8::Local<v8::Value> value,
		v8::Handle<v8::Context>	Context,
		UINT wFlags
	);
	v8::Handle<v8::Value> Invoke(
		DISPID DispID,
		const 	v8::Arguments& args,
		v8::Local<v8::Value> value,
		v8::Handle<v8::Context>	Context,
		UINT wFlags
	);
	
	static v8::Handle<v8::Value> OleFuncCaller(
		const v8::Arguments& args
	);
	
	static v8::Handle<v8::Value> OleValueGetter(
		v8::Local<v8::String> propertyName,
		const v8::AccessorInfo& info
	);
	
	static void OleValueSetter(
		v8::Local<v8::String> propertyName,
		v8::Local<v8::Value> value,
		const v8::AccessorInfo& info
	);
	
	void AddOLEFunction( v8::Local<v8::Object> ThisObj );
	
	// èâä˙âªÅEèIóπ
	static void Initialize( void ){		CoInitialize( NULL ); }
	static void Uninitialize( void ){	CoUninitialize(); }
	
  private:
	IDispatch	*m_pApp;
};
