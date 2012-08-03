/*****************************************************************************
	
	VSD -- vehicle data logger system  Copyright(C) by DDS
	
	CScript.h - JavaScript
	
*****************************************************************************/

class CVsdFilter;

class CScript {
  public:
	CScript( CVsdFilter *pVsd );
	~CScript( void );
	
	UINT Initialize( char *szFileName );
	UINT Run( const char *szFunc );
	
	const char* ToCString( const v8::String::Utf8Value& value );
	void ReportException( v8::TryCatch* try_catch );
	
	static CVsdFilter	*m_Vsd;	// ÉGÅc
	
	char *m_szErrorMsg;
	BOOL m_bError;
	
  private:
	v8::Persistent<v8::Context> m_context;
	v8::HandleScope m_handle_scope;
};
