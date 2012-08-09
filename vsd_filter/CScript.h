/*****************************************************************************
	
	VSD -- vehicle data logger system  Copyright(C) by DDS
	
	CScript.h - JavaScript
	
*****************************************************************************/

class CVsdFilter;

class CScript {
  public:
	CScript( CVsdFilter *pVsd );
	~CScript( void );
	
	UINT CompileRun( const char *szScript, const char *szFileName );
	void Initialize( void );
	UINT RunFile( char *szFileName );
	UINT Run( const char *szFunc );
	
	const char* ToCString( const v8::String::Utf8Value& value );
	void ReportException( v8::TryCatch* try_catch );
	
	static CVsdFilter	*m_Vsd;	// ÉGÅc
	
	char *m_szErrorMsg;
	UINT m_uError;
	
	const char *GetErrorMessage( void ){
		return m_szErrorMsg ? m_szErrorMsg : m_szErrorMsgID[ m_uError ];
	}
	
  private:
	v8::Persistent<v8::Context> m_Context;
	v8::Isolate	*m_pIsolate;
	
	static const char *m_szErrorMsgID[];
};
