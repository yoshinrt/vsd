/*****************************************************************************
	
	VSD -- vehicle data logger system  Copyright(C) by DDS
	
	COle.cpp - OLE class implementation
	
*****************************************************************************/

#pragma once
#include "error_code.h"

class ICallbackJSFunc : public IDispatch {
  public:
	ICallbackJSFunc(
		v8::Persistent<v8::Object> Global,
		v8::Persistent<v8::Function> Func
	) :
		m_Global( Global ),
		m_CallbackFunc( Func )
	{
		m_uRefCnt = 1;
	}
	
	~ICallbackJSFunc(){
		if( !m_CallbackFunc.IsEmpty()) m_CallbackFunc.Dispose();
		if( !m_Global.IsEmpty()) m_Global.Dispose();
	}
	
	HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void **ppvObject){
		*ppvObject = static_cast<ICallbackJSFunc *>( this );
		return S_OK;
	}
	
	ULONG STDMETHODCALLTYPE AddRef( void ){
		return m_uRefCnt++;
	}
	
	ULONG STDMETHODCALLTYPE Release(void){
		if( --m_uRefCnt == 0 ){
			delete this;
			return 0;
		}
		return m_uRefCnt;
	}
	
	HRESULT STDMETHODCALLTYPE GetTypeInfoCount( unsigned int FAR* pctinfo ){ return 1; }
	HRESULT STDMETHODCALLTYPE GetTypeInfo( UINT iTInfo, LCID lcid, ITypeInfo **ppTInfo ){
		return S_OK;
	}
	
	HRESULT STDMETHODCALLTYPE GetIDsOfNames(
		REFIID riid,
		LPOLESTR *rgszNames,
		UINT cNames,
		LCID lcid,
		DISPID *rgDispId
	){
		return S_OK;
	}
	
	HRESULT STDMETHODCALLTYPE Invoke(
		DISPID dispIdMember,
		REFIID riid,
		LCID lcid,
		WORD wFlags,
		DISPPARAMS *pDispParams,
		VARIANT *pVarResult,
		EXCEPINFO *pExcepInfo,
		UINT *puArgErr
	);
	
  private:
	v8::Persistent<v8::Object>		m_Global;
	v8::Persistent<v8::Function>	m_CallbackFunc;
	UINT	m_uRefCnt;
};

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
	
	static v8::Local<v8::Value> CreateActiveXObject(
		IDispatch *pDispatch,
		v8::Local<v8::Context> Context
	);
	
	// Ruby win32ole à⁄êAï®
	static void V8Array2SafeArray(
		v8::Local<v8::Context> Context,
		v8::Local<v8::Array> val,
		SAFEARRAY *psa,
		long *pUB, long *pID,
		int iMaxDim, int iDim
	);
	
	static void Val2Variant(
		v8::Local<v8::Value> val,
		VARIANT *var,
		v8::Local<v8::Context> Context
	);
	
	static v8::Local<v8::Value> SafeArray2V8Array(
		v8::Local<v8::Context> Context,
		VARIANT& variant,
		SAFEARRAY *psa,
		long *pLB, long *pUB, long *pID,
		int iMaxDim, int iDim
	);
	
	static v8::Local<v8::Value> Variant2Val( VARIANT *pvar, v8::Local<v8::Context> Context );
	v8::Local<v8::Value> Invoke(
		v8::Local<v8::Context>	Context,
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
	HRESULT AddOLEFunction( v8::Local<v8::Object> ThisObj, ITypeInfo *pTypeInfo );
	
	static void ThrowHResultError( HRESULT hr );
	
	// èâä˙âªÅEèIóπ
	static void Initialize( void ){		CoInitialize( NULL ); }
	static void Uninitialize( void ){	CoUninitialize(); }
	
  private:
	IDispatch	*m_pApp;
};
