/*****************************************************************************
	
	VSD -- vehicle data logger system  Copyright(C) by DDS
	
	CScript.h - JavaScript
	
*****************************************************************************/

#pragma once

class CVsdFilter;

class CScript {
  public:
	CScript( CVsdFilter *pVsd );
	~CScript( void );
	
	void Initialize( void );
	UINT RunFile( LPCWSTR szFileName );
	UINT Run( LPCWSTR szFunc );
	
	const char* ToCString( const v8::String::Utf8Value& value );
	void ReportException( v8::TryCatch* try_catch );
	
	static CVsdFilter	*m_Vsd;	// ÉGÅc
	
	LPWSTR m_szErrorMsg;
	UINT m_uError;
	
	LPCWSTR GetErrorMessage( void ){
		return m_szErrorMsg ? m_szErrorMsg : m_szErrorMsgID[ m_uError ];
	}
	
  private:
	v8::Persistent<v8::Context> m_Context;
	v8::Isolate	*m_pIsolate;
	
	static LPCWSTR m_szErrorMsgID[];
};
