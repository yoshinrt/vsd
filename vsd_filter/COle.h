/*****************************************************************************
	
	VSD -- vehicle data logger system  Copyright(C) by DDS
	
	COle.cpp - OLE class implementation
	
*****************************************************************************/

#pragma once
#include "CV8If.h"
#include "error_code.h"

class ICallbackJSFunc : public IDispatch {
  public:
	ICallbackJSFunc( Local<Function> Func ) :
		m_pIsolate( Isolate::GetCurrent())
	{
		m_Context = *(new Persistent<Context>( m_pIsolate,
			m_pIsolate->GetCurrentContext()
		));
		m_CallbackFunc = *( new Persistent<Function>( Isolate::GetCurrent(), Func ));
		m_uRefCnt = 1;
	}
	
	~ICallbackJSFunc(){
		if( !m_CallbackFunc.IsEmpty()) m_CallbackFunc.Reset();
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
	Isolate					*m_pIsolate;
	Persistent<Context,CopyablePersistentTraits<Context>> m_Context;
	Persistent<Function,CopyablePersistentTraits<Function>>	m_CallbackFunc;
	UINT	m_uRefCnt;
};

class COle : public CV8If {
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
		IDispatch *pDispatch
	);
	
	// Ruby win32ole 移植物
	static void V8Array2SafeArray(
		Local<Array> val,
		SAFEARRAY *psa,
		long *pUB, long *pID,
		int iMaxDim, int iDim
	);
	
	static void Val2Variant(
		Local<Value> val,
		VARIANT *var
	);
	
	static Local<Value> SafeArray2V8Array(
		VARIANT& variant,
		SAFEARRAY *psa,
		long *pLB, long *pUB, long *pID,
		int iMaxDim, int iDim
	);
	
	static Local<Value> Variant2Val( VARIANT *pvar );
	void Invoke(
		DISPID DispID,
		ReturnValue<Value>& Ret,
		const 	FunctionCallbackInfo<Value>& args,
		Local<Value>& value,
		UINT wFlags
	);
	
	static void OleFuncCaller(
		const FunctionCallbackInfo<Value>& args
	);
	
	static void CallAsFunctionHandler(
		const FunctionCallbackInfo<Value>& args
	);
	
	static void OleValueGetter(
		Local<String> propertyName,
		const PropertyCallbackInfo<Value>& info
	);
	
	static void OleValueSetter(
		Local<String> propertyName,
		Local<Value> value,
		const PropertyCallbackInfo<void>& info
	);
	
	void AddOLEFunction( Local<Object> ThisObj );
	HRESULT AddOLEFunction( Local<Object> ThisObj, ITypeInfo *pTypeInfo );
	
	static void ThrowHResultError( HRESULT hr );
	
	// 初期化・終了
	static void Initialize( void ){		CoInitialize( NULL ); }
	static void Uninitialize( void ){	CoUninitialize(); }
	
  private:
	IDispatch	*m_pApp;
};
