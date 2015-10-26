/*****************************************************************************
	
	VSD -- vehicle data logger system  Copyright(C) by DDS
	
	COle.cpp - OLE class implementation
	
*****************************************************************************/

#pragma once
#include "error_code.h"

class ICallbackJSFunc : public IDispatch {
  public:
	ICallbackJSFunc(
		Persistent<Object> Global,
		Persistent<Function> Func
	) :
		m_Global( Global ),
		m_CallbackFunc( Func )
	{
		m_uRefCnt = 1;
	}
	
	~ICallbackJSFunc(){
		if( !m_CallbackFunc.IsEmpty()) m_CallbackFunc.Reset();
		if( !m_Global.IsEmpty()) m_Global.Reset();
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
	Persistent<Object>		Holder;
	Persistent<Object>		m_Global;
	Persistent<Function>	m_CallbackFunc;
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
	static void InitJS( Local<FunctionTemplate> tmpl );
	
	static Local<Value> CreateActiveXObject(
		IDispatch *pDispatch,
		Local<Context> Context
	);
	
	// Ruby win32ole à⁄êAï®
	static void V8Array2SafeArray(
		Local<Context> Context,
		Local<Array> val,
		SAFEARRAY *psa,
		long *pUB, long *pID,
		int iMaxDim, int iDim
	);
	
	static void Val2Variant(
		Local<Value> val,
		VARIANT *var,
		Local<Context> Context
	);
	
	static Local<Value> SafeArray2V8Array(
		Local<Context> Context,
		VARIANT& variant,
		SAFEARRAY *psa,
		long *pLB, long *pUB, long *pID,
		int iMaxDim, int iDim
	);
	
	static Local<Value> Variant2Val( VARIANT *pvar, Local<Context> Context );
	Local<Value> Invoke(
		Local<Context>	Context,
		DISPID DispID,
		const 	FunctionCallbackInfo<Value>& args,
		Local<Value> value,
		UINT wFlags
	);
	
	static Handle<Value> OleFuncCaller(
		const FunctionCallbackInfo<Value>& args
	);
	
	static Handle<Value> CallAsFunctionHandler(
		const FunctionCallbackInfo<Value>& args
	);
	
	static Handle<Value> OleValueGetter(
		Local<String> propertyName,
		const PropertyCallbackInfo<Value>& info
	);
	
	static void OleValueSetter(
		Local<String> propertyName,
		Local<Value> value,
		const PropertyCallbackInfo<Value>& info
	);
	
	void AddOLEFunction( Local<Object> ThisObj );
	HRESULT AddOLEFunction( Local<Object> ThisObj, ITypeInfo *pTypeInfo );
	
	static void ThrowHResultError( HRESULT hr );
	
	// èâä˙âªÅEèIóπ
	static void Initialize( void ){		CoInitialize( NULL ); }
	static void Uninitialize( void ){	CoUninitialize(); }
	
  private:
	IDispatch	*m_pApp;
};
