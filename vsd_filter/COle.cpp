/*****************************************************************************
	
	VSD -- vehicle data logger system  Copyright( C ) by DDS
	
	COle.cpp - OLE class implementation
	
*****************************************************************************/

#include "StdAfx.h"

#include "COle.h"

/*** ファイルオープン *******************************************************/

// AutoWrap() - Automation helper function...
HRESULT COle::AutoWrap( int autoType, VARIANT *pvResult, LPOLESTR ptName, int cArgs... ){
	// Begin variable-argument list...
	va_list marker;
	va_start( marker, cArgs );
	
	// Variables used...
	DISPPARAMS dp = { NULL, NULL, 0, 0 };
	DISPID dispidNamed = DISPID_PROPERTYPUT;
	DISPID dispID;
	HRESULT hr;
	char szName[200];
	
	// Convert down to ANSI
	WideCharToMultiByte( CP_ACP, 0, ptName, -1, szName, 256, NULL, NULL );
	
	// Get DISPID for name passed...
	hr = m_pApp->GetIDsOfNames( IID_NULL, &ptName, 1, LOCALE_USER_DEFAULT, &dispID );
	if( FAILED( hr )){
		//sprintf( buf, "IDispatch::GetIDsOfNames( \"%s\" ) failed w/err 0x%08lx", szName, hr );
		//MessageBox( NULL, buf, "AutoWrap()", 0x10010 );
		//_exit( 0 );
		return hr;
	}
	
	// Allocate memory for arguments...
	VARIANT *pArgs = new VARIANT[cArgs+1];
	// Extract arguments...
	for( int i=0; i<cArgs; i++ ){
		pArgs[i] = va_arg( marker, VARIANT );
	}
	
	// Build DISPPARAMS
	dp.cArgs = cArgs;
	dp.rgvarg = pArgs;
	
	// Handle special-case for property-puts!
	if( autoType & DISPATCH_PROPERTYPUT ){
		dp.cNamedArgs = 1;
		dp.rgdispidNamedArgs = &dispidNamed;
	}
	
	// Make the call!
	hr = m_pApp->Invoke( dispID, IID_NULL, LOCALE_SYSTEM_DEFAULT, autoType, &dp, pvResult, NULL, NULL );
	if( FAILED( hr )){
		//sprintf( buf, "IDispatch::Invoke( \"%s\"=%08lx ) failed w/err 0x%08lx", szName, dispID, hr );
		//MessageBox( NULL, buf, "AutoWrap()", 0x10010 );
		//_exit( 0 );
		return hr;
	}
	// End variable-argument section...
	va_end( marker );
	
	delete [] pArgs;
	
	return hr;
}

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
	inst->SetNamedPropertyHandler( PropGetter, PropSetter );
}

/*** プロパティセット・ゲット ***********************************************/

v8::Handle<v8::Value> COle::PropGetter(
	v8::Local<v8::String> property,
	const v8::AccessorInfo& info
){
	v8::String::AsciiValue str( property );
	return v8::Undefined();
}

v8::Handle<v8::Value> COle::PropSetter(
	v8::Local<v8::String> property,
	v8::Local<v8::Value> value,
	const v8::AccessorInfo& info
){
	LPWSTR a = ( LPWSTR )*property;
	return v8::Undefined();
}

#if 0
	// Get CLSID for our server...
	CLSID clsid;
	HRESULT hr = CLSIDFromProgID( L"Excel.Application", &clsid );
	
	if( FAILED( hr )){
		::MessageBox( NULL, "CLSIDFromProgID() failed", "Error", 0x10010 );
		return -1;
	}
	
	// Start server and get IDispatch...
	hr = CoCreateInstance( clsid, NULL, CLSCTX_LOCAL_SERVER, IID_IDispatch, ( void ** )&m_pApp );
	if( FAILED( hr )){
		::MessageBox( NULL, "Excel not registered properly", "Error", 0x10010 );
		return -2;
	}
	
	// Make it visible ( i.e. app.visible = 1 )
	{
		VARIANT x;
		x.vt = VT_I4;
		x.lVal = 1;
		AutoWrap( DISPATCH_PROPERTYPUT, NULL, m_pApp, L"Visible", 1, x );
	}
	
	// Get Workbooks collection
	IDispatch *pXlBooks;
	{
		VARIANT result;
		VariantInit( &result );
		AutoWrap( DISPATCH_PROPERTYGET, &result, m_pApp, L"Workbooks", 0 );
		pXlBooks = result.pdispVal;
	}
	
	// Call Workbooks.Add() to get a new workbook...
	IDispatch *pXlBook;
	{
		VARIANT result;
		VariantInit( &result );
		AutoWrap( DISPATCH_PROPERTYGET, &result, pXlBooks, L"Add", 0 );
		pXlBook = result.pdispVal;
	}
	
	// Create a 15x15 safearray of variants...
	VARIANT arr;
	arr.vt = VT_ARRAY | VT_VARIANT;
	{
		SAFEARRAYBOUND sab[2];
		sab[0].lLbound = 1; sab[0].cElements = 15;
		sab[1].lLbound = 1; sab[1].cElements = 15;
		arr.parray = SafeArrayCreate( VT_VARIANT, 2, sab );
	}
	
	// Fill safearray with some values...
	for( int i=1; i<=15; i++ ){
		for( int j=1; j<=15; j++ ){
			// Create entry value for ( i,j )
			VARIANT tmp;
			tmp.vt = VT_I4;
			tmp.lVal = i*j;
			// Add to safearray...
			long indices[] = {i,j};
			SafeArrayPutElement( arr.parray, indices, ( void * )&tmp );
		}
	}
	
	// Get ActiveSheet object
	IDispatch *pXlSheet;
	{
		VARIANT result;
		VariantInit( &result );
		AutoWrap( DISPATCH_PROPERTYGET, &result, m_pApp, L"ActiveSheet", 0 );
		pXlSheet = result.pdispVal;
	}
	
	// Get Range object for the Range A1:O15...
	IDispatch *pXlRange;
	{
		VARIANT parm;
		parm.vt = VT_BSTR;
		parm.bstrVal = ::SysAllocString( L"A1:O15" );
		
		VARIANT result;
		VariantInit( &result );
		AutoWrap( DISPATCH_PROPERTYGET, &result, pXlSheet, L"Range", 1, parm );
		VariantClear( &parm );
		
		pXlRange = result.pdispVal;
	}
	
	// Set range with our safearray...
	AutoWrap( DISPATCH_PROPERTYPUT, NULL, pXlRange, L"Value", 1, arr );
	
	// Wait for user...
	::MessageBox( NULL, "All done.", "Notice", 0x10000 );
	
	// Set .Saved property of workbook to TRUE so we aren't prompted
	// to save when we tell Excel to quit...
	{
		VARIANT x;
		x.vt = VT_I4;
		x.lVal = 1;
		AutoWrap( DISPATCH_PROPERTYPUT, NULL, pXlBook, L"Saved", 1, x );
	}
	
	// Tell Excel to quit ( i.e. App.Quit )
	AutoWrap( DISPATCH_METHOD, NULL, m_pApp, L"Quit", 0 );
	
	// Release references...
	pXlRange->Release();
	pXlSheet->Release();
	pXlBook->Release();
	pXlBooks->Release();
	VariantClear( &arr );
#endif
