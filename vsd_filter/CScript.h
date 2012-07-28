/*****************************************************************************
	
	VSD -- vehicle data logger system  Copyright(C) by DDS
	
	CScript.h - JavaScript
	
*****************************************************************************/

class CVsdFilter;

class CScript {
  public:
	CScript( CVsdFilter *pVsd );
	~CScript( void );
	
	BOOL Load( char *szFileName );
	BOOL Run( void );
	BOOL RunFunction( const char *szFunc );
	
	static CVsdFilter	*m_Vsd;	// ÉGÅc
	
  private:
	v8::Persistent<v8::Context> m_context;
	v8::Handle<v8::Script> m_script;
	v8::HandleScope m_handle_scope;
};
