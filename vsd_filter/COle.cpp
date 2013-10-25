/*****************************************************************************
	
	VSD -- vehicle data logger system  Copyright( C ) by DDS
	
	COle.cpp - OLE class implementation
	
*****************************************************************************/

#include "StdAfx.h"
#include "CScript.h"
#include "COle.h"

/*** OLE インスタンス作成 ***************************************************/

UINT COle::CreateInstance( LPCWSTR strServer ){
	// Get CLSID for our server...
	CLSID clsid;
	HRESULT hr = CLSIDFromProgID( strServer, &clsid );
	
	if( FAILED( hr )) return ERR_OLE_NO_SERVER;
	
	// Start server and get IDispatch...
	hr = CoCreateInstance( clsid, NULL, CLSCTX_LOCAL_SERVER, IID_IDispatch, ( void ** )&m_pApp );
	if( FAILED( hr )) return ERR_OLE_CREATE_FAILED;
	
	return ERR_OK;
}

/*** JavaScript IF 初期化追加処理 *******************************************/

void COle::InitJS( v8::Local<v8::FunctionTemplate> tmpl ){
	v8::Handle<v8::ObjectTemplate> inst = tmpl->InstanceTemplate();
	// Default function 登録
	inst->SetCallAsFunctionHandler( COle::CallAsFunctionHandler, v8::Int32::New( 0 ));
}

/*** OLE function 追加 ******************************************************/

// 共通メソッド caller
v8::Handle<v8::Value> COle::OleFuncCaller(
	const v8::Arguments& args
){
	COle *obj = CScript::GetThis<COle>( args.This());
	if( !obj ) return v8::Undefined();
	
	return obj->Invoke(
		args.Data()->Int32Value(), args,
		v8::Local<v8::Value>(), args.This()->CreationContext(), DISPATCH_METHOD
	);
}

// 共通メソッド caller
v8::Handle<v8::Value> COle::CallAsFunctionHandler(
	const v8::Arguments& args
){
	COle *obj = CScript::GetThis<COle>( args.This());
	if( !obj ) return v8::Undefined();
	
	return obj->Invoke(
		args.Data()->Int32Value(), args,
		v8::Local<v8::Value>(), args.This()->CreationContext(), DISPATCH_PROPERTYGET
	);
}

/****************************************************************************/
// 共通プロパティセッタ
void COle::OleValueSetter(
	v8::Local<v8::String> propertyName,
	v8::Local<v8::Value> value,
	const v8::AccessorInfo& info
){
	COle *obj = CScript::GetThis<COle>( info.Holder());
	if( !obj ) return;
	
	obj->Invoke(
		info.Data()->Int32Value(), *( v8::Arguments *)NULL,
		value, info.This()->CreationContext(), DISPATCH_PROPERTYPUT
	);
}

// 共通プロパティゲッタ
v8::Handle<v8::Value> COle::OleValueGetter(
	v8::Local<v8::String> propertyName,
	const v8::AccessorInfo& info
){
	COle *obj = CScript::GetThis<COle>( info.Holder());
	if( !obj ) return v8::Undefined();
	
	return obj->Invoke(
		info.Data()->Int32Value(), *( v8::Arguments *)NULL,
		v8::Local<v8::Value>(), info.Holder()->CreationContext(), DISPATCH_PROPERTYGET
	);
}

void COle::AddOLEFunction( v8::Local<v8::Object> ThisObj ){
	UINT	uTypeInfoCnt;
	HRESULT	hr;
	
	if( m_pApp->GetTypeInfoCount( &uTypeInfoCnt ) != S_OK || uTypeInfoCnt == 0 ){
		return;
	}
	
	ITypeInfo	*pTypeInfo;
	TYPEATTR	*pTypeAttr;
	FUNCDESC	*pFuncDesc;
	VARDESC	*pVarDesc;
	BSTR		Name;
	
	for( UINT u = 0; u < uTypeInfoCnt; ++u ){
		if( FAILED( hr = m_pApp->GetTypeInfo( u, LOCALE_SYSTEM_DEFAULT, &pTypeInfo ))) break;
		
		if( FAILED( hr = pTypeInfo->GetTypeAttr( &pTypeAttr ))){
			pTypeInfo->Release();
			break;
		}
		
		// 変数一覧を取得
		for( u = 0; u < pTypeAttr->cVars; ++u ){
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
			//DebugMsgD( L"Var:%s\n", Name );
			
			SysFreeString( Name );
			pTypeInfo->ReleaseVarDesc( pVarDesc );
		}
		
		// function 一覧を取得
		for( u = 0; u < pTypeAttr->cFuncs; ++u ){
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
				//DebugMsgD( L"Func:%s\n", Name );
			}else{
				// var 追加
				ThisObj->SetAccessor(
					v8::String::New(( uint16_t *)Name ),
					OleValueGetter,
					OleValueSetter,
					v8::Int32::New( pFuncDesc->memid )
				);
				//DebugMsgD( L"Var:%s\n", Name );
			}
			SysFreeString( Name );
			pTypeInfo->ReleaseFuncDesc( pFuncDesc );
		}
		
		pTypeInfo->ReleaseTypeAttr( pTypeAttr );
		pTypeInfo->Release();
	}
}

/*** IDispatch -> ActiveXObject *********************************************/

v8::Handle<v8::Value> COle::CreateActiveXObject(
	IDispatch *pDispatch,
	v8::Handle<v8::Context> Context
){
	// ActiveXObject を New する
	v8::Local<v8::Function> hFunction = v8::Local<v8::Function>::Cast(
		Context->Global()->Get( v8::String::New( "ActiveXObject" ))
	);
	if( hFunction->IsUndefined()){
		return v8::Undefined(); // ★エラー処理追加
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

void COle::Val2Variant(
	v8::Local<v8::Value> val,
	VARIANT *var
){
#if 0
	struct oledata *pole;
	if(rb_obj_is_kind_of(val, cWIN32OLE)) {
		Data_Get_Struct(val, struct oledata, pole);
		OLE_ADDREF(pole->pDispatch);
		V_VT(var) = VT_DISPATCH;
		V_DISPATCH(var) = pole->pDispatch;
		return;
	}
	if (rb_obj_is_kind_of(val, rb_cTime)) {
		V_VT(var) = VT_DATE;
		V_DATE(var) = time_object2date(val);
		return;
	}
	if( val->IsArray()){
		VALUE val1;
		long dim = 0;
		int  i = 0;

		HRESULT hr;
		SAFEARRAYBOUND *psab;
		SAFEARRAY *psa;
		long	  *pub, *pid;

		val1 = val;
		while(TYPE(val1) == T_ARRAY) {
			val1 = rb_ary_entry(val1, 0);
			dim += 1;
		}
		psab = ALLOC_N(SAFEARRAYBOUND, dim);
		pub  = ALLOC_N(long, dim);
		pid  = ALLOC_N(long, dim);

		if(!psab || !pub || !pid) {
			if(pub) free(pub);
			if(psab) free(psab);
			if(pid) free(pid);
			rb_raise(rb_eRuntimeError, "memory allocate error");
		}
		val1 = val;
		i = 0;
		while(TYPE(val1) == T_ARRAY) {
			psab[i].cElements = RARRAY(val1)->len;
			psab[i].lLbound = 0;
			pub[i] = psab[i].cElements;
			pid[i] = 0;
			i ++;
			val1 = rb_ary_entry(val1, 0);
		}
		/* Create and fill VARIANT array */
		psa = SafeArrayCreate(VT_VARIANT, dim, psab);
		if (psa == NULL)
			hr = E_OUTOFMEMORY;
		else
			hr = SafeArrayLock(psa);
		if (SUCCEEDED(hr)) {
			ole_set_safe_array(dim-1, psa, pid, pub, val, dim-1);
			hr = SafeArrayUnlock(psa);
		}
		if(pub) free(pub);
		if(psab) free(psab);
		if(pid) free(pid);

		if (SUCCEEDED(hr)) {
			V_VT(var) = VT_VARIANT | VT_ARRAY;
			V_ARRAY(var) = psa;
		}
		else if (psa != NULL)
			SafeArrayDestroy(psa);
	}else
#endif
	if( val->IsString()){
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
	}else{
		V_VT(var) = VT_ERROR;
		V_ERROR(var) = DISP_E_PARAMNOTFOUND;
	}
}

v8::Handle<v8::Value> COle::Variant2Val( VARIANT *pvar, v8::Handle<v8::Context> Context ){
	v8::Handle<v8::Value> ret = v8::Undefined();
	
	HRESULT hr;
	while ( V_VT(pvar) == (VT_BYREF | VT_VARIANT) )
		pvar = V_VARIANTREF(pvar);

#if 0
	if(V_ISARRAY(pvar)) {
		SAFEARRAY *psa = V_ISBYREF(pvar) ? *V_ARRAYREF(pvar) : V_ARRAY(pvar);
		long i;
		long *pID, *pLB, *pUB;
		VARIANT variant;
		VALUE val;
		VALUE val2;

		int dim = SafeArrayGetDim(psa);
		VariantInit(&variant);
		V_VT(&variant) = (V_VT(pvar) & ~VT_ARRAY) | VT_BYREF;

		pID = ALLOC_N(long, dim);
		pLB = ALLOC_N(long, dim);
		pUB = ALLOC_N(long, dim);

		if(!pID || !pLB || !pUB) {
			if(pID) free(pID);
			if(pLB) free(pLB);
			if(pUB) free(pUB);
			rb_raise(rb_eRuntimeError, "memory allocate error");
		}

		ret = Qnil;

		for(i = 0; i < dim; ++i) {
			SafeArrayGetLBound(psa, i+1, &pLB[i]);
			SafeArrayGetLBound(psa, i+1, &pID[i]);
			SafeArrayGetUBound(psa, i+1, &pUB[i]);
		}

		hr = SafeArrayLock(psa);
		if (SUCCEEDED(hr)) {
			val2 = rb_ary_new();
			while (i >= 0) {
				hr = SafeArrayPtrOfIndex(psa, pID, &V_BYREF(&variant));
				if (FAILED(hr))
					break;

				val = ole_variant2val(&variant);
				rb_ary_push(val2, val);
				for (i = dim-1 ; i >= 0 ; --i) {
					if (++pID[i] <= pUB[i])
						break;

					pID[i] = pLB[i];
					if (i > 0) {
						if (ret == Qnil)
							ret = rb_ary_new();
						rb_ary_push(ret, val2);
						val2 = rb_ary_new();
					}
				}
			}
			SafeArrayUnlock(psa);
		}
		if(pID) free(pID);
		if(pLB) free(pLB);
		if(pUB) free(pUB);
		return (ret == Qnil) ? val2 : ret;
	}
#endif
	switch(V_VT(pvar) & ~VT_BYREF){
	case VT_EMPTY:
		break;
	case VT_NULL:
		break;
	case VT_UI1:
		if(V_ISBYREF(pvar)) 
			ret = v8::Integer::New(*V_UI1REF(pvar));
		else 
			ret = v8::Integer::New(V_UI1(pvar));
		break;

	case VT_I2:
		if(V_ISBYREF(pvar))
			ret = v8::Integer::New(*V_I2REF(pvar));
		else 
			ret = v8::Integer::New(V_I2(pvar));
		break;

	case VT_I4:
		if(V_ISBYREF(pvar))
			ret = v8::Integer::New(*V_I4REF(pvar));
		else 
			ret = v8::Integer::New(V_I4(pvar));
		break;

	case VT_R4:
		if(V_ISBYREF(pvar))
			ret = v8::Number::New(*V_R4REF(pvar));
		else
			ret = v8::Number::New(V_R4(pvar));
		break;

	case VT_R8:
		if(V_ISBYREF(pvar))
			ret = v8::Number::New(*V_R8REF(pvar));
		else
			ret = v8::Number::New(V_R8(pvar));
		break;

	case VT_BSTR:
	{
		if(V_ISBYREF(pvar))
			ret = v8::String::New(( uint16_t *)*V_BSTRREF(pvar));
		else
			ret = v8::String::New(( uint16_t *)V_BSTR(pvar));
		break;
	}

	case VT_ERROR:
		if(V_ISBYREF(pvar))
			ret = v8::Integer::New(*V_ERRORREF(pvar));
		else
			ret = v8::Integer::New(V_ERROR(pvar));
		break;

	case VT_BOOL:
		if (V_ISBYREF(pvar))
			ret = (*V_BOOLREF(pvar) ? v8::True() : v8::False());
		else
			ret = (V_BOOL(pvar) ? v8::True() : v8::False());
		break;

	case VT_DISPATCH:
	{
		IDispatch *pDispatch;

		if (V_ISBYREF(pvar))
			pDispatch = *V_DISPATCHREF(pvar);
		else
			pDispatch = V_DISPATCH(pvar);
		
		ret = CreateActiveXObject( pDispatch, Context );
		break;
	}

	case VT_UNKNOWN:
	{
		/* get IDispatch interface from IUnknown interface */
		IUnknown *punk;
		IDispatch *pDispatch;
		HRESULT hr;

		if (V_ISBYREF(pvar))
			punk = *V_UNKNOWNREF(pvar);
		else
			punk = V_UNKNOWN(pvar);

		if(punk != NULL) {
		   hr = punk->QueryInterface( IID_IDispatch, (void **)&pDispatch);
		   if(SUCCEEDED(hr)) {
			   ret = CreateActiveXObject( pDispatch, Context );
		   }
		}
		break;
	}

#if 0
	case VT_DATE:
	{
		DATE date;
		if(V_ISBYREF(pvar))
			date = *V_DATEREF(pvar);
		else
			date = V_DATE(pvar);

		ret =  date2time_str(date);
		break;
	}
	case VT_CY:
#endif
	default:
		{
		HRESULT hr;
		VARIANT variant;
		VariantInit(&variant);
		hr = VariantChangeTypeEx(&variant, pvar, 
								  LOCALE_SYSTEM_DEFAULT, 0, VT_BSTR);
		if (SUCCEEDED(hr) && V_VT(&variant) == VT_BSTR) {
			ret = v8::String::New(( uint16_t *)V_BSTR(&variant));
		}
		VariantClear(&variant);
		break;
		}
	}
	return ret;
}

v8::Handle<v8::Value> COle::Invoke(
	LPCWSTR wszPropName,
	const v8::Arguments& args,
	v8::Local<v8::Value> value,
	v8::Handle<v8::Context>	Context,
	UINT wFlags
){
	LCID	lcid = LOCALE_SYSTEM_DEFAULT;
	HRESULT hr;
	
	DISPID DispID;
	
	hr = m_pApp->GetIDsOfNames( IID_NULL, ( LPOLESTR *)&wszPropName, 1, lcid, &DispID );
	if(FAILED(hr)) {
		v8::ThrowException( v8::Exception::Error( v8::String::New(
			"Unknown property or method"
		)));
		return v8::Undefined();
	}
	
	return Invoke( DispID, args, value, Context, wFlags );
}

v8::Handle<v8::Value> COle::Invoke(
	DISPID DispID,
	const v8::Arguments& args,
	v8::Local<v8::Value> value,
	v8::Handle<v8::Context>	Context,
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
			VariantInit(&realargs[i]);
			VariantInit(&op.dp.rgvarg[i]);
			
			Val2Variant( args[ i ], &realargs[ i ] );
			V_VT(&op.dp.rgvarg[i]) = VT_VARIANT | VT_BYREF;
			V_VARIANTREF(&op.dp.rgvarg[i]) = &realargs[i];
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
				Val2Variant( args[ i ], &op.dp.rgvarg[i]);
			}
			memset(&excepinfo, 0, sizeof(EXCEPINFO));
			hr = m_pApp->Invoke( DispID, 
												 IID_NULL, lcid, wFlags,
												 &op.dp, NULL,
												 &excepinfo, &argErr);
			for(i = 0; i < op.dp.cArgs; i++) {
				VariantClear(&op.dp.rgvarg[i]);
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
		VariantClear(&op.dp.rgvarg[i]);
	}
	
	v8::Handle<v8::Value> ret;
	if (FAILED(hr)) {
		/*v = ole_excepinfo2msg(&excepinfo);
		ole_raise(hr, eWIN32OLE_RUNTIME_ERROR, "%s%s",
				  StringValuePtr(cmd), StringValuePtr(v));
		*/
		v8::ThrowException( v8::Exception::Error( v8::String::New(
			"OLE Error"
		)));
		ret = v8::Undefined();
	}else{
		ret = Variant2Val( &result, Context );
		VariantClear(&result);
	}
	
	delete [] realargs;
	delete [] op.dp.rgdispidNamedArgs;
	
	return ret;
}
