/*****************************************************************************
	
	VSD -- vehicle data logger system  Copyright( C ) by DDS
	
	COle.cpp - OLE class implementation
	
*****************************************************************************/

#include "StdAfx.h"
#include "CVsdFilter.h"
#include "CScript.h"
#include "COle.h"

extern CVsdFilter	*g_Vsd;

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

void COle::InitJS( v8::Local<v8::FunctionTemplate> tmpl ){
	v8::Local<v8::ObjectTemplate> inst = tmpl->InstanceTemplate();
	// Default function 登録
	inst->SetCallAsFunctionHandler( COle::CallAsFunctionHandler, v8::Int32::New( 0 ));
}

/*** OLE function 追加 ******************************************************/

// 共通メソッド caller
v8::Handle<v8::Value> COle::OleFuncCaller(
	const v8::Arguments& args
){
	v8::EscapableHandleScope handle_scope;
	COle *obj = CScript::GetThis<COle>( args.This());
	if( !obj ) return v8::Undefined();
	
	return handle_scope.Close(
		obj->Invoke(
			args.This()->CreationContext(),
			args.Data()->Int32Value(), args,
			v8::Local<v8::Value>(), DISPATCH_METHOD
		)
	);
}

// 共通メソッド caller
v8::Handle<v8::Value> COle::CallAsFunctionHandler(
	const v8::Arguments& args
){
	v8::EscapableHandleScope handle_scope;
	COle *obj = CScript::GetThis<COle>( args.This());
	if( !obj ) return v8::Undefined();
	
	return handle_scope.Close(
		obj->Invoke(
			args.This()->CreationContext(),
			args.Data()->Int32Value(), args,
			v8::Local<v8::Value>(), DISPATCH_PROPERTYGET
		)
	);
}

/****************************************************************************/
// 共通プロパティセッタ
void COle::OleValueSetter(
	v8::Local<v8::String> propertyName,
	v8::Local<v8::Value> value,
	const v8::AccessorInfo& info
){
	v8::HandleScope handle_scope;
	COle *obj = CScript::GetThis<COle>( info.Holder());
	if( !obj ) return;
	
	obj->Invoke(
		info.This()->CreationContext(),
		info.Data()->Int32Value(), *( v8::Arguments *)NULL,
		value, DISPATCH_PROPERTYPUT
	);
}

// 共通プロパティゲッタ
v8::Handle<v8::Value> COle::OleValueGetter(
	v8::Local<v8::String> propertyName,
	const v8::AccessorInfo& info
){
	v8::EscapableHandleScope handle_scope;
	COle *obj = CScript::GetThis<COle>( info.Holder());
	if( !obj ) return v8::Undefined();
	
	return handle_scope.Close(
		obj->Invoke(
			info.Holder()->CreationContext(),
			info.Data()->Int32Value(), *( v8::Arguments *)NULL,
			v8::Local<v8::Value>(), DISPATCH_PROPERTYGET
		)
	);
}

void COle::AddOLEFunction( v8::Local<v8::Object> ThisObj ){
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

HRESULT COle::AddOLEFunction( v8::Local<v8::Object> ThisObj, ITypeInfo *pTypeInfo ){
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
			v8::String::New(( uint16_t *)Name ),
			OleValueGetter,
			OleValueSetter,
			v8::Int32::New( pVarDesc->memid )
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
				v8::String::New(( uint16_t *)Name ),
				v8::FunctionTemplate::New(
					OleFuncCaller,
					v8::Int32::New( pFuncDesc->memid )
				)->GetFunction()
			);
			DebugMsgD( L"Fuc:%08X:%s\n", pFuncDesc->memid, Name );
		}else{
			// var 追加
			ThisObj->SetAccessor(
				v8::String::New(( uint16_t *)Name ),
				OleValueGetter,
				OleValueSetter,
				v8::Int32::New( pFuncDesc->memid )
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

v8::Local<v8::Value> COle::CreateActiveXObject(
	IDispatch *pDispatch,
	v8::Local<v8::Context> Context
){
	// ActiveXObject を New する
	v8::Local<v8::Function> hFunction = v8::Local<v8::Function>::Cast(
		Context->Global()->Get( v8::String::New( "ActiveXObject" ))
	);
	if( hFunction->IsUndefined()){
		v8::ThrowException( v8::Exception::Error( v8::String::New(
			"Internal error : Can't call ActiveXOjbect constructor"
		)));
		return LocalUndefined();
	}
	
	v8::Local<v8::Object> JSObj = hFunction->NewInstance( 0, NULL );
	COle *obj = static_cast<COle *>(
		v8::Local<v8::External>::Cast( JSObj->GetInternalField( 0 ))->Value()
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
	v8::Local<v8::Context> Context,
	v8::Local<v8::Array> val,
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
			Val2Variant( val->Get( pID[ iDim ]), &variant, Context );
		}else{
			// 子 array を walk
			V8Array2SafeArray( Context, v8::Local<v8::Array>::Cast( val->Get( pID[ iDim ])), psa, pUB, pID, iMaxDim, iDim + 1 );
		}
	}
}

void COle::Val2Variant(
	v8::Local<v8::Value> val,
	VARIANT *var,
	v8::Local<v8::Context> Context
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
		v8::Local<v8::Value> val1;
		long dim = 0;
		int  i = 0;

		HRESULT hr;
		SAFEARRAYBOUND *psab;
		SAFEARRAY *psa;
		long	  *pub, *pid;

		val1 = val;
		while( val1->IsArray()){
			val1 = v8::Local<v8::Array>::Cast( val1 )->Get( 0 );
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
			psab[ i ].cElements = v8::Local<v8::Array>::Cast( val1 )->Length();
			psab[ i ].lLbound = 0;
			pub[ i ] = psab[ i ].cElements;
			pid[ i ] = 0;
			i ++;
			val1 = v8::Local<v8::Array>::Cast( val1 )->Get( 0 );
		}
		/* Create and fill VARIANT array */
		psa = SafeArrayCreate( VT_VARIANT, dim, psab );
		if( psa == NULL ) hr = E_OUTOFMEMORY;
		else hr = SafeArrayLock( psa );
		
		if( SUCCEEDED( hr )){
			V8Array2SafeArray( Context, v8::Local<v8::Array>::Cast( val ), psa, pub, pid, dim, 0 );
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
		v8::String::Value str( val );
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
		V_DISPATCH(var) = new ICallbackJSFunc(
			v8::Persistent<v8::Object>::New( Context->Global()),
			v8::Persistent<v8::Function>::Cast(
				v8::Persistent<v8::Value>::New( val )
			)
		);
	}else{
		V_VT(var) = VT_ERROR;
		V_ERROR(var) = DISP_E_PARAMNOTFOUND;
	}
}

v8::Local<v8::Value> COle::SafeArray2V8Array(
	v8::Local<v8::Context> Context,
	VARIANT& variant,
	SAFEARRAY *psa,
	long *pLB, long *pUB, long *pID,
	int iMaxDim, int iDim
){
	v8::Local<v8::Array> ret = v8::Array::New( 0 );
	
	for( pID[ iDim ] = pLB[ iDim ]; pID[ iDim ] <= pUB[ iDim ]; ++pID[ iDim ]){
		if( iDim == iMaxDim - 1 ){
			// 要素をpush
			HRESULT hr = SafeArrayPtrOfIndex( psa, pID, &V_BYREF( &variant ));
			if( FAILED( hr )) break;
			ret->Set( ret->Length(), Variant2Val( &variant, Context ));
		}else{
			// 子 array を push
			ret->Set( ret->Length(), SafeArray2V8Array( Context, variant, psa, pLB, pUB, pID, iMaxDim, iDim + 1 ));
		}
	}
	return ret;
}

v8::Local<v8::Value> COle::Variant2Val( VARIANT *pvar, v8::Local<v8::Context> Context ){
	v8::Local<v8::Value> ret;
	
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
			return LocalUndefined();
		}
		
		for( i = 0; i < dim; ++i ){
			SafeArrayGetLBound( psa, i + 1, &pLB[ i ] );
			SafeArrayGetUBound( psa, i + 1, &pUB[ i ] );
		}
		
		HRESULT hr = SafeArrayLock( psa );
		if( SUCCEEDED( hr )){
			ret = SafeArray2V8Array( Context, variant, psa, pLB, pUB, pID, dim, 0 );
			SafeArrayUnlock( psa );
		}
		
		if( pID ) delete [] pID;
		if( pLB ) delete [] pLB;
		if( pUB ) delete [] pUB;
		
		return ret;
	}
	
	switch( V_VT( pvar ) & ~VT_BYREF ){
	  case VT_EMPTY:
		return v8::Local<v8::Value>( *LocalUndefined());
		
	  Case VT_NULL:
		return v8::Local<v8::Value>( *v8::Null());
		
	  Case VT_UI1:
		ret = v8::Int32::New( V_ISBYREF( pvar ) ? *V_UI1REF( pvar ) : V_UI1( pvar ));
		
	  Case VT_I2:
		ret = v8::Int32::New( V_ISBYREF( pvar ) ? *V_I2REF( pvar ) : V_I2( pvar ));
		
	  Case VT_I4:
		ret = v8::Int32::New( V_ISBYREF( pvar ) ? *V_I4REF( pvar ) : V_I4( pvar ));
		
	  Case VT_R4:
		ret = v8::Number::New( V_ISBYREF( pvar ) ? *V_R4REF( pvar ) : V_R4( pvar ));
		
	  Case VT_R8:
		ret = v8::Number::New( V_ISBYREF( pvar ) ? *V_R8REF( pvar ) : V_R8( pvar ));
		
	  Case VT_BSTR:
		ret = v8::String::New( V_ISBYREF( pvar ) ? ( uint16_t *)*V_BSTRREF( pvar ) : ( uint16_t *)V_BSTR( pvar ));
		
	  Case VT_ERROR:
		ret = v8::Int32::New( V_ISBYREF( pvar ) ? *V_ERRORREF( pvar ) : V_ERROR( pvar ));
		
	  Case VT_BOOL:
		ret = v8::Local<v8::Value>( *(
			( V_ISBYREF( pvar ) ? *V_BOOLREF( pvar ) : V_BOOL( pvar )) ? v8::True() : v8::False()
		));
		
	  Case VT_DISPATCH:
		ret = CreateActiveXObject( V_ISBYREF( pvar ) ? *V_DISPATCHREF( pvar ) : V_DISPATCH( pvar ), Context );
		
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
					ret = CreateActiveXObject( pDispatch, Context );
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
				ret = v8::String::New(( uint16_t *)V_BSTR( &variant ));
			}
			VariantClear( &variant );
			break;
		}
	}
	return ret;
}

v8::Local<v8::Value> COle::Invoke(
	v8::Local<v8::Context>	Context,
	DISPID DispID,
	const v8::Arguments& args,
	v8::Local<v8::Value> value,
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
			
			Val2Variant( args[ op.dp.cArgs - i - 1 ], &realargs[ i ], Context );
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
		
		Val2Variant( value, &realargs[ 0 ], Context );
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
				Val2Variant( args[ i ], &op.dp.rgvarg[ i ], Context );
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
	
	v8::Local<v8::Value> ret;
	if( FAILED( hr )) {
		ThrowHResultError( hr );
		ret = LocalUndefined();
	}else{
		ret = Variant2Val( &result, Context );
		VariantClear(&result);
	}
	
	delete [] realargs;
	delete [] op.dp.rgdispidNamedArgs;
	
	return ret;
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
		v8::ThrowException( v8::Exception::Error( v8::String::New(( uint16_t *)pMsg )));
		LocalFree( pMsg );
	}else{
		char szMsg[ 64 ];
		sprintf( szMsg, "OLE Error 0x%X", hr );
		v8::ThrowException( v8::Exception::Error( v8::String::New( szMsg )));
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
	//CSemaphoreLock sem;
	
	DebugMsgD( ">ICallbackJSFunc::Invoke\n" );
	
	v8::Isolate::Scope IsolateScope( g_Vsd->m_Script->m_pIsolate );
	v8::HandleScope handle_scope;
	v8::Context::Scope context_scope( g_Vsd->m_Script->m_Context );
	v8::TryCatch try_catch;
	
	m_CallbackFunc->Call( m_Global, 0, NULL );
	
	if( try_catch.HasCaught()){
		LPWSTR pMsg = CScript::ReportException( NULL, try_catch );
		fputws( pMsg, stdout );
		delete [] pMsg;
	}
	
	DebugMsgD( "<ICallbackJSFunc::Invoke\n" );
	return S_OK;
}
