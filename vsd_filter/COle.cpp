/*****************************************************************************
	
	VSD -- vehicle data logger system  Copyright( C ) by DDS
	
	COle.cpp - OLE class implementation
	
*****************************************************************************/

#include "StdAfx.h"
#include "CVsdFilter.h"
#include "ScriptIF.h"
#include "COle.h"

/*** OLE インスタンス作成 ***************************************************/

UINT COle::CreateInstance( LPCWSTR strServer ){
	// Get CLSID for our server...
	CLSID clsid;
	HRESULT hr = CLSIDFromProgID( strServer, &clsid );
	
	if( FAILED( hr )) return ERR_OLE_NO_SERVER;
	
	// Start server and get IDispatch...
	hr = ::CoCreateInstance( clsid, NULL, CLSCTX_LOCAL_SERVER, IID_IDispatch, ( void ** )&m_pApp );
	if( FAILED( hr )){
		hr = ::CoCreateInstance( clsid, NULL, CLSCTX_INPROC_SERVER, IID_IDispatch, ( void ** )&m_pApp );
		if( FAILED( hr )) return ERR_OLE_CREATE_FAILED;
	}
	
	return ERR_OK;
}

/*** JavaScript IF 初期化追加処理 *******************************************/

void COle::InitJS( Local<FunctionTemplate> tmpl ){
	Local<ObjectTemplate> inst = tmpl->InstanceTemplate();
	// Default function 登録
	inst->SetCallAsFunctionHandler( COle::CallAsFunctionHandler, Int32::New( Isolate::GetCurrent(), 0 ));
}

/*** OLE function 追加 ******************************************************/

// 共通メソッド caller
void COle::OleFuncCaller(
	const FunctionCallbackInfo<Value>& args
){
	COle *obj = CScriptIFBase::GetThis<COle>( args.This());
	if( !obj ) return;
	
	obj->Invoke(
		args.Data()->Int32Value(),
		args.GetReturnValue(), args,
		*( Local<Value> *)NULL, DISPATCH_METHOD
	);
}

// 共通メソッド caller
void COle::CallAsFunctionHandler(
	const FunctionCallbackInfo<Value>& args
){
	COle *obj = CScriptIFBase::GetThis<COle>( args.This());
	if( !obj ) return;
	
	obj->Invoke(
		args.Data()->Int32Value(),
		args.GetReturnValue(), args,
		*( Local<Value> *)NULL, DISPATCH_PROPERTYGET
	);
}

/****************************************************************************/
// 共通プロパティセッタ
void COle::OleValueSetter(
	Local<String> propertyName,
	Local<Value> value,
	const PropertyCallbackInfo<void>& info
){
	COle *obj = CScriptIFBase::GetThis<COle>( info.Holder());
	if( !obj ) return;
	
	obj->Invoke(
		info.Data()->Int32Value(),
		*( ReturnValue<Value> *)NULL,
		*( FunctionCallbackInfo<Value> *)NULL,
		value, DISPATCH_PROPERTYPUT
	);
}

// 共通プロパティゲッタ
void COle::OleValueGetter(
	Local<String> propertyName,
	const PropertyCallbackInfo<Value>& info
){
	COle *obj = CScriptIFBase::GetThis<COle>( info.Holder());
	if( !obj ) return;
	
	obj->Invoke(
		info.Data()->Int32Value(),
		info.GetReturnValue(),
		*( FunctionCallbackInfo<Value> *)NULL,
		*( Local<Value> *)NULL, DISPATCH_PROPERTYGET
	);
}

void COle::AddOLEFunction( Local<Object> ThisObj ){
	UINT	uTypeInfoCnt;
	HRESULT	hr;
	
	if( m_pApp->GetTypeInfoCount( &uTypeInfoCnt ) != S_OK || uTypeInfoCnt == 0 ){
		return;
	}
	
	ITypeInfo	*pTypeInfo;
	for( UINT u = 0; u < uTypeInfoCnt; ++u ){
		if( FAILED( hr = m_pApp->GetTypeInfo( u, LOCALE_SYSTEM_DEFAULT, &pTypeInfo ))) break;
		hr = AddOLEFunction( ThisObj, pTypeInfo );
		pTypeInfo->Release();
		if( FAILED( hr )) break;
	}
	
	if( FAILED( hr )){
		ThrowHResultError( hr );
	}
}

HRESULT COle::AddOLEFunction( Local<Object> ThisObj, ITypeInfo *pTypeInfo ){
	HRESULT	hr;
	
	TYPEATTR	*pTypeAttr;
	FUNCDESC	*pFuncDesc;
	VARDESC		*pVarDesc;
	BSTR		Name;
	
	if( FAILED( hr = pTypeInfo->GetTypeAttr( &pTypeAttr ))){
		return hr;
	}
	
	// 変数一覧を取得
	for( UINT u = 0; u < pTypeAttr->cVars; ++u ){
		if( FAILED( hr = pTypeInfo->GetVarDesc( u, &pVarDesc ))) break;
		
		UINT v;
		hr = pTypeInfo->GetNames( pVarDesc->memid, &Name, 1, &v );
		
		// var 追加
		ThisObj->SetAccessor(
			String::NewFromTwoByte( Isolate::GetCurrent(), ( uint16_t *)Name ),
			OleValueGetter,
			OleValueSetter,
			Int32::New( Isolate::GetCurrent(), pVarDesc->memid )
		);
		DebugMsgD( L"Var:%X:%s\n", pVarDesc->memid, Name );
		
		SysFreeString( Name );
		pTypeInfo->ReleaseVarDesc( pVarDesc );
	}
	
	// function 一覧を取得
	for( UINT u = 0; u < pTypeAttr->cFuncs; ++u ){
		if( FAILED( hr = pTypeInfo->GetFuncDesc( u, &pFuncDesc ))) break;
		
		UINT v;
		hr = pTypeInfo->GetNames( pFuncDesc->memid, &Name, 1, &v );
		
		if( pFuncDesc->invkind & INVOKE_FUNC ){
			// function 追加
			ThisObj->Set(
				String::NewFromTwoByte( Isolate::GetCurrent(), ( uint16_t *)Name ),
				FunctionTemplate::New( Isolate::GetCurrent(),
					OleFuncCaller,
					Int32::New( Isolate::GetCurrent(), pFuncDesc->memid )
				)->GetFunction()
			);
			DebugMsgD( L"Fuc:%08X:%s\n", pFuncDesc->memid, Name );
		}else{
			// var 追加
			ThisObj->SetAccessor(
				String::NewFromTwoByte( Isolate::GetCurrent(), ( uint16_t *)Name ),
				OleValueGetter,
				OleValueSetter,
				Int32::New( Isolate::GetCurrent(), pFuncDesc->memid )
			);
			DebugMsgD( L"Var:%08X:%s\n", pFuncDesc->memid, Name );
		}
		SysFreeString( Name );
		pTypeInfo->ReleaseFuncDesc( pFuncDesc );
	}
	
    // 継承クラスの情報を登録
    ITypeInfo	*pRefTypeInfo;
    HREFTYPE href;
	
	for( UINT u = 0; u < pTypeAttr->cImplTypes; ++u ){
		hr = pTypeInfo->GetRefTypeOfImplType( u, &href );
		if( FAILED( hr )) break;
		hr = pTypeInfo->GetRefTypeInfo( href, &pRefTypeInfo );
		if( FAILED( hr )) break;
		hr = AddOLEFunction( ThisObj, pRefTypeInfo );
		pRefTypeInfo->Release();
		if( FAILED( hr )) break;
	}
	
	pTypeInfo->ReleaseTypeAttr( pTypeAttr );
	return hr;
}

/*** IDispatch -> ActiveXObject *********************************************/

Local<Value> COle::CreateActiveXObject(
	IDispatch *pDispatch
){
	// ActiveXObject を New する
	Local<Function> hFunction = Local<Function>::Cast(
		Isolate::GetCurrent()->GetCurrentContext()->Global()->Get( String::NewFromOneByte( Isolate::GetCurrent(), ( uint8_t *)"ActiveXObject" ))
	);
	if( hFunction->IsUndefined()){
		Isolate::GetCurrent()->ThrowException( Exception::Error( String::NewFromOneByte( Isolate::GetCurrent(), ( uint8_t *)
			"Internal error : Can't call ActiveXOjbect constructor"
		)));
		return Undefined( Isolate::GetCurrent());
	}
	
	Local<Object> JSObj = hFunction->NewInstance( 0, NULL );
	COle *obj = static_cast<COle *>(
		Local<External>::Cast( JSObj->GetInternalField( 0 ))->Value()
	);
	obj->m_pApp = pDispatch;
	obj->m_pApp->AddRef();
	obj->AddOLEFunction( JSObj );
	
	return JSObj;
}

/*** invoke *****************************************************************/

#define ALLOCA_N( t, n )	new t[ n ]

struct oleparam {
	DISPPARAMS dp;
	OLECHAR** pNamedArgs;
};

void COle::V8Array2SafeArray(
	Local<Array> val,
	SAFEARRAY *psa,
	long *pUB, long *pID,
	int iMaxDim, int iDim
){
	VARIANT variant;
	
	for( pID[ iDim ] = 0; pID[ iDim ] <= pUB[ iDim ]; ++pID[ iDim ]){
		if( iDim == iMaxDim - 1 ){
			// 要素を書く
			HRESULT hr = SafeArrayPtrOfIndex( psa, pID, &V_BYREF( &variant ));
			if( FAILED( hr )) break;
			Val2Variant( val->Get( pID[ iDim ]), &variant );
		}else{
			// 子 array を walk
			V8Array2SafeArray( Local<Array>::Cast( val->Get( pID[ iDim ])), psa, pUB, pID, iMaxDim, iDim + 1 );
		}
	}
}

void COle::Val2Variant(
	Local<Value> val,
	VARIANT *var
){
#if 0
	struct oledata *pole;
	if( rb_obj_is_kind_of( val, cWIN32OLE )){
		Data_Get_Struct( val, struct oledata, pole );
		OLE_ADDREF( pole->pDispatch );
		V_VT( var ) = VT_DISPATCH;
		V_DISPATCH( var ) = pole->pDispatch;
		return;
	}
	if( rb_obj_is_kind_of( val, rb_cTime )){
		V_VT( var ) = VT_DATE;
		V_DATE( var ) = time_object2date( val );
		return;
	}
#endif
	if( val->IsArray()){
		Local<Value> val1;
		long dim = 0;
		int  i = 0;

		HRESULT hr;
		SAFEARRAYBOUND *psab;
		SAFEARRAY *psa;
		long	  *pub, *pid;

		val1 = val;
		while( val1->IsArray()){
			val1 = Local<Array>::Cast( val1 )->Get( 0 );
			dim += 1;
		}
		psab = new SAFEARRAYBOUND[ dim ];
		pub  = new long[ dim ];
		pid  = new long[ dim ];

		if( !psab || !pub || !pid ){
			if( pub ) delete [] pub;
			if( psab ) delete [] psab;
			if( pid ) delete [] pid;
			V8Error( ERR_NOT_ENOUGH_MEMORY );
		}
		val1 = val;
		i = 0;
		while( val1->IsArray()){
			psab[ i ].cElements = Local<Array>::Cast( val1 )->Length();
			psab[ i ].lLbound = 0;
			pub[ i ] = psab[ i ].cElements;
			pid[ i ] = 0;
			i ++;
			val1 = Local<Array>::Cast( val1 )->Get( 0 );
		}
		/* Create and fill VARIANT array */
		psa = SafeArrayCreate( VT_VARIANT, dim, psab );
		if( psa == NULL ) hr = E_OUTOFMEMORY;
		else hr = SafeArrayLock( psa );
		
		if( SUCCEEDED( hr )){
			V8Array2SafeArray( Local<Array>::Cast( val ), psa, pub, pid, dim, 0 );
			hr = SafeArrayUnlock( psa );
		}
		if( pub ) delete [] pub;
		if( psab ) delete [] psab;
		if( pid ) delete [] pid;
		
		if( SUCCEEDED( hr )){
			V_VT( var ) = VT_VARIANT | VT_ARRAY;
			V_ARRAY( var ) = psa;
		}else if( psa != NULL ) SafeArrayDestroy( psa );
	}else if( val->IsString()){
		V_VT(var) = VT_BSTR;
		String::Value str( val );
		V_BSTR(var) = SysAllocString(( LPCWSTR )*str );
	}else if( val->IsInt32() || val->IsUint32() && val->Uint32Value() < 0x80000000 ){
		V_VT(var) = VT_I4;
		V_I4(var) = val->Int32Value();
	}else if( val->IsNumber()){
		V_VT(var) = VT_R8;
		V_R8(var) = val->NumberValue();
	}else if( val->IsBoolean()){
		V_VT(var) = VT_BOOL;
		V_BOOL(var) = val->IsTrue() ? VARIANT_TRUE : VARIANT_FALSE;
	}else if( val->IsFunction()){
		V_VT(var) = VT_DISPATCH;
		V_DISPATCH(var) = new ICallbackJSFunc( Local<Function>::Cast( val ));
	}else{
		V_VT(var) = VT_ERROR;
		V_ERROR(var) = DISP_E_PARAMNOTFOUND;
	}
}

Local<Value> COle::SafeArray2V8Array(
	VARIANT& variant,
	SAFEARRAY *psa,
	long *pLB, long *pUB, long *pID,
	int iMaxDim, int iDim
){
	Local<Array> ret = Array::New( Isolate::GetCurrent(), 0 );
	
	for( pID[ iDim ] = pLB[ iDim ]; pID[ iDim ] <= pUB[ iDim ]; ++pID[ iDim ]){
		if( iDim == iMaxDim - 1 ){
			// 要素をpush
			HRESULT hr = SafeArrayPtrOfIndex( psa, pID, &V_BYREF( &variant ));
			if( FAILED( hr )) break;
			ret->Set( ret->Length(), Variant2Val( &variant ));
		}else{
			// 子 array を push
			ret->Set( ret->Length(), SafeArray2V8Array( variant, psa, pLB, pUB, pID, iMaxDim, iDim + 1 ));
		}
	}
	return ret;
}

Local<Value> COle::Variant2Val( VARIANT *pvar ){
	Local<Value> ret;
	
	while( V_VT( pvar ) == ( VT_BYREF | VT_VARIANT )) pvar = V_VARIANTREF( pvar );
	
	if( V_ISARRAY( pvar )){
		SAFEARRAY *psa = V_ISBYREF( pvar ) ? *V_ARRAYREF( pvar ) : V_ARRAY( pvar );
		long i;
		long *pID, *pLB, *pUB;
		VARIANT variant;
		
		int dim = SafeArrayGetDim( psa );
		VariantInit( &variant );
		V_VT( &variant ) = ( V_VT( pvar ) & ~VT_ARRAY ) | VT_BYREF;
		
		pID = new long[ dim ];
		pLB = new long[ dim ];
		pUB = new long[ dim ];
		
		if( !pID || !pLB || !pUB ){
			if( pID ) delete [] pID;
			if( pLB ) delete [] pLB;
			if( pUB ) delete [] pUB;
			V8Error( ERR_NOT_ENOUGH_MEMORY );
			return Undefined( Isolate::GetCurrent());
		}
		
		for( i = 0; i < dim; ++i ){
			SafeArrayGetLBound( psa, i + 1, &pLB[ i ] );
			SafeArrayGetUBound( psa, i + 1, &pUB[ i ] );
		}
		
		HRESULT hr = SafeArrayLock( psa );
		if( SUCCEEDED( hr )){
			ret = SafeArray2V8Array( variant, psa, pLB, pUB, pID, dim, 0 );
			SafeArrayUnlock( psa );
		}
		
		if( pID ) delete [] pID;
		if( pLB ) delete [] pLB;
		if( pUB ) delete [] pUB;
		
		return ret;
	}
	
	switch( V_VT( pvar ) & ~VT_BYREF ){
	  case VT_EMPTY:
		return Undefined( Isolate::GetCurrent());
		
	  Case VT_NULL:
		return Null( Isolate::GetCurrent());
		
	  Case VT_UI1:
		ret = Int32::New( Isolate::GetCurrent(), V_ISBYREF( pvar ) ? *V_UI1REF( pvar ) : V_UI1( pvar ));
		
	  Case VT_I2:
		ret = Int32::New( Isolate::GetCurrent(), V_ISBYREF( pvar ) ? *V_I2REF( pvar ) : V_I2( pvar ));
		
	  Case VT_I4:
		ret = Int32::New( Isolate::GetCurrent(), V_ISBYREF( pvar ) ? *V_I4REF( pvar ) : V_I4( pvar ));
		
	  Case VT_R4:
		ret = Number::New( Isolate::GetCurrent(), V_ISBYREF( pvar ) ? *V_R4REF( pvar ) : V_R4( pvar ));
		
	  Case VT_R8:
		ret = Number::New( Isolate::GetCurrent(), V_ISBYREF( pvar ) ? *V_R8REF( pvar ) : V_R8( pvar ));
		
	  Case VT_BSTR: {
			uint16_t *str =  V_ISBYREF( pvar ) ? ( uint16_t *)*V_BSTRREF( pvar ) : ( uint16_t *)V_BSTR( pvar );
			if( str == NULL ) return Null( Isolate::GetCurrent());
			ret = String::NewFromTwoByte( Isolate::GetCurrent(), str );
		}
		
	  Case VT_ERROR:
		ret = Int32::New( Isolate::GetCurrent(), V_ISBYREF( pvar ) ? *V_ERRORREF( pvar ) : V_ERROR( pvar ));
		
	  Case VT_BOOL:
		ret = ( V_ISBYREF( pvar ) ? *V_BOOLREF( pvar ) : V_BOOL( pvar )) ? True( Isolate::GetCurrent()) : False( Isolate::GetCurrent());
		
	  Case VT_DISPATCH:
		ret = CreateActiveXObject( V_ISBYREF( pvar ) ? *V_DISPATCHREF( pvar ) : V_DISPATCH( pvar ));
		
	  Case VT_UNKNOWN:
		{
			/* get IDispatch interface from IUnknown interface */
			IUnknown *punk;
			IDispatch *pDispatch;
			HRESULT hr;
			
			punk = V_ISBYREF( pvar ) ? *V_UNKNOWNREF( pvar ) : V_UNKNOWN( pvar );
			
			if( punk != NULL ){
				hr = punk->QueryInterface( IID_IDispatch, ( void **)&pDispatch );
				if( SUCCEEDED( hr )) {
					ret = CreateActiveXObject( pDispatch );
				}
			}
			break;
		}
		
		/*
	  case VT_DATE:
		{
			DATE date;
			date = V_ISBYREF( pvar ) ? *V_DATEREF( pvar ) : V_DATE( pvar );
			ret =  date2time_str( date );
			break;
		}
	  case VT_CY:
		*/
		
	  default:
		{
			HRESULT hr;
			VARIANT variant;
			VariantInit( &variant );
			hr = VariantChangeTypeEx( &variant, pvar, LOCALE_SYSTEM_DEFAULT, 0, VT_BSTR );
			if( SUCCEEDED( hr ) && V_VT( &variant ) == VT_BSTR ){
				ret = String::NewFromTwoByte( Isolate::GetCurrent(), ( uint16_t *)V_BSTR( &variant ));
			}
			VariantClear( &variant );
			break;
		}
	}
	return ret;
}

void COle::Invoke(
	DISPID DispID,
	ReturnValue<Value>& Ret,
	const FunctionCallbackInfo<Value>& args,
	Local<Value>& value,
	UINT wFlags
){
	LCID	lcid = LOCALE_SYSTEM_DEFAULT;
	HRESULT hr;
	
	EXCEPINFO excepinfo;
	VARIANT result;
	VARIANTARG* realargs = NULL;
	unsigned int argErr = 0;
	unsigned int i;
	struct oleparam op;
	memset(&excepinfo, 0, sizeof(EXCEPINFO));
	
	VariantInit(&result);
	
	op.dp.rgvarg = NULL;
	op.dp.rgdispidNamedArgs = NULL;
	op.dp.cNamedArgs = 0;
	op.dp.cArgs = 0;
	
	/*--------------------------------------
	  non hash args ==> dispatch parameters 
	 ----------------------------------------*/
	if( &args != NULL && ( op.dp.cArgs = args.Length())){
		realargs		= ALLOCA_N(VARIANTARG, op.dp.cArgs);
		op.dp.rgvarg	= ALLOCA_N(VARIANTARG, op.dp.cArgs);
		
		for(i = 0; i < op.dp.cArgs; i++) {
			VariantInit(&realargs[ i ]);
			VariantInit(&op.dp.rgvarg[ i ]);
			
			Val2Variant( args[ op.dp.cArgs - i - 1 ], &realargs[ i ] );
			V_VT(&op.dp.rgvarg[ i ]) = VT_VARIANT | VT_BYREF;
			V_VARIANTREF(&op.dp.rgvarg[ i ]) = &realargs[ i ];
		}
	}
	
	/* apparent you need to call propput, you need this */
	else if (wFlags & DISPATCH_PROPERTYPUT) {
		op.dp.cNamedArgs = 1;
		op.dp.rgdispidNamedArgs = ALLOCA_N( DISPID, 1 );
		op.dp.rgdispidNamedArgs[0] = DISPID_PROPERTYPUT;
		
		op.dp.cArgs = 1;
		realargs = ALLOCA_N(VARIANTARG, op.dp.cArgs);
		op.dp.rgvarg  = ALLOCA_N(VARIANTARG, op.dp.cArgs);
		
		VariantInit(&realargs[0]);
		VariantInit(&op.dp.rgvarg[0]);
		
		Val2Variant( value, &realargs[ 0 ] );
		V_VT(&op.dp.rgvarg[0]) = VT_VARIANT | VT_BYREF;
		V_VARIANTREF(&op.dp.rgvarg[0]) = &realargs[0];
	}
	
	hr = m_pApp->Invoke( DispID, 
										 IID_NULL, lcid, wFlags, &op.dp, 
										 &result, &excepinfo, &argErr);
	if (FAILED(hr)) {
		/* retry to call args by value */
		if(op.dp.cArgs > 0) {
			for(i = 0; i < op.dp.cArgs; i++) {
				// ★prop セットの時ぬるぽ
				Val2Variant( args[ i ], &op.dp.rgvarg[ i ] );
			}
			memset(&excepinfo, 0, sizeof(EXCEPINFO));
			hr = m_pApp->Invoke( DispID, 
												 IID_NULL, lcid, wFlags,
												 &op.dp, NULL,
												 &excepinfo, &argErr);
			for(i = 0; i < op.dp.cArgs; i++) {
				VariantClear(&op.dp.rgvarg[ i ]);
			}
		}
		/* mega kludge. if a method in WORD is called and we ask
		 * for a result when one is not returned then
		 * hResult == DISP_E_EXCEPTION. this only happens on
		 * functions whose DISPID > 0x8000 */
		if (hr == DISP_E_EXCEPTION && DispID > 0x8000) {
			memset(&excepinfo, 0, sizeof(EXCEPINFO));
			hr = m_pApp->Invoke( DispID, 
												 IID_NULL, lcid, wFlags,
												 &op.dp, NULL,
												 &excepinfo, &argErr);
		
		}
	}
	/* clear dispatch parameter */
	for(i = 0; i < op.dp.cArgs; i++) {
		VariantClear(&op.dp.rgvarg[ i ]);
	}
	
	Local<Value> ret;
	if( FAILED( hr )) {
		ThrowHResultError( hr );
		ret = Undefined( Isolate::GetCurrent());
	}else{
		ret = Variant2Val( &result );
		VariantClear(&result);
	}
	
	delete [] realargs;
	delete [] op.dp.rgdispidNamedArgs;
	
	if( &Ret ) Ret.Set( ret );
}

/*** HRESULT のエラーメッセージを投げる *************************************/

void COle::ThrowHResultError( HRESULT hr ){
	LPWSTR pMsg;
	
	FormatMessageW(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
		NULL, hr, MAKELANGID( LANG_NEUTRAL, SUBLANG_DEFAULT ),
		( LPWSTR )&pMsg, 0, NULL
	);
	if( pMsg != NULL ){
		Isolate::GetCurrent()->ThrowException( Exception::Error( String::NewFromTwoByte( Isolate::GetCurrent(), ( uint16_t *)pMsg )));
		LocalFree( pMsg );
	}else{
		char szMsg[ 64 ];
		sprintf( szMsg, "OLE Error 0x%X", hr );
		Isolate::GetCurrent()->ThrowException( Exception::Error( String::NewFromOneByte( Isolate::GetCurrent(), ( uint8_t *)szMsg )));
	}
}

/*** JavaScript function callback *******************************************/

HRESULT STDMETHODCALLTYPE ICallbackJSFunc::Invoke(
	DISPID dispIdMember,
	REFIID riid,
	LCID lcid,
	WORD wFlags,
	DISPPARAMS *pDispParams,
	VARIANT *pVarResult,
	EXCEPINFO *pExcepInfo,
	UINT *puArgErr
){
	DebugMsgD( ">ICallbackJSFunc::Invoke\n" );
	
	Locker locker( m_pIsolate );
	Isolate::Scope IsolateScope( m_pIsolate );
	HandleScope handle_scope( m_pIsolate );
	Local<Context> context = Local<Context>::New( m_pIsolate, m_Context );
	Context::Scope context_scope( context );
	TryCatch try_catch;
	
	Local<Function>::New( m_pIsolate, m_CallbackFunc )
		->Call( context->Global(), 0, NULL );
	
	if( try_catch.HasCaught()){
		LPWSTR pMsg = CScript::ReportException( NULL, try_catch );
		fputws( pMsg, stdout );
		delete [] pMsg;
	}
	
	DebugMsgD( "<ICallbackJSFunc::Invoke\n" );
	return S_OK;
}
