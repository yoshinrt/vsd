/*****************************************************************************
	
	VSD -- vehicle data logger system  Copyright(C) by DDS
	
	CVsdFile.h - JavaScript File
	
*****************************************************************************/

#pragma once

class CVsdFile {
  public:
	CVsdFile(){
		m_gzfp	= NULL;
		m_fp	= NULL;
	}
	
	~CVsdFile(){
		Close();
	}
	
	int Open( LPCWSTR szFile, LPCWSTR szMode );	// !js_func
	void Close( void ); // !js_func
	char *ReadLine( void ); // !js_func
	int IsEOF( void ); // !js_func
	
	static const int BUF_LEN = 1024;
	
  private:
	gzFile	m_gzfp;
	FILE	*m_fp;
	char	m_cBuf[ BUF_LEN ];
};
