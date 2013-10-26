/*****************************************************************************
	
	VSD -- vehicle data logger system  Copyright(C) by DDS
	
	COle.cpp - OLE class implementation
	
*****************************************************************************/

#pragma once
#include "error_code.h"

typedef std::map<DISPID, std::wstring>	PROPNAME_TBL;

class COle {
  public:
	COle( void ){
		m_pApp			= NULL;
	}
	
	~COle(){
		if( m_pApp ) m_pApp->Release();
	}
	
	UINT CreateInstance( LPCWSTR strServer );
	static void InitJS( v8::Local<v8::FunctionTemplate> tmpl );
	
	static v8::Handle<v8::Value> CreateActiveXObject(
		IDispatch *pDispatch,
		v8::Handle<v8::Context> Context
	);
	
	// プロパティ名マネジメント
	void RegisterPropName( DISPID DispID, LPCWSTR wszName ){
		PROPNAME_TBL::iterator itr;
		
		if(( itr = m_PropNameTbl.find( DispID )) == m_PropNameTbl.end()){
			std::wstring strName( wszName );
			m_PropNameTbl[ DispID ] = strName;
		}
	}
	
	LPCWSTR GetPropName( DISPID DispID ){
		PROPNAME_TBL::iterator itr;
		
		if(( itr = m_PropNameTbl.find( DispID )) == m_PropNameTbl.end()){
			return L"(unknown)";
		}
		return itr->second.c_str();
	}
	
	// Ruby win32ole 移植物
	void Val2Variant(
		v8::Local<v8::Value> val,
		VARIANT *var
	);
	v8::Handle<v8::Value> Variant2Val( VARIANT *pvar, v8::Handle<v8::Context> Context );
	v8::Handle<v8::Value> Invoke(
		v8::Handle<v8::Context>	Context,
		DISPID DispID,
		const 	v8::Arguments& args,
		v8::Local<v8::Value> value,
		UINT wFlags
	);
	
	static v8::Handle<v8::Value> OleFuncCaller(
		const v8::Arguments& args
	);
	
	static v8::Handle<v8::Value> CallAsFunctionHandler(
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
	
	// 初期化・終了
	static void Initialize( void ){		CoInitialize( NULL ); }
	static void Uninitialize( void ){	CoUninitialize(); }
	
  private:
	IDispatch		*m_pApp;
	PROPNAME_TBL	m_PropNameTbl;
};
