/*****************************************************************************
	
	VSD -- vehicle data logger system  Copyright(C) by DDS
	
	CVsdFile.cpp - CVsdFile class implementation
	
*****************************************************************************/

#include "StdAfx.h"

#include "dds.h"
#include "dds_lib/dds_lib.h"
#include "CVsdFile.h"

/*** ファイルオープン *******************************************************/

int CVsdFile::Open( LPCWSTR szFile, LPCWSTR szMode ){
	
	LPCWSTR pMode = szMode;
	
	// gz open
	if( *pMode == 'z' ){
		++pMode;
		//m_gzfp = gzopen( szFile, szMode );
		return m_gzfp != NULL ? 0 : 1;
	}
	
	// 普通 fopen
	m_fp = _wfopen( szFile, szMode );
	return m_fp != NULL ? 0 : 1;
}

/*** ファイルクローズ *******************************************************/

void CVsdFile::Close( void ){
	if( m_gzfp ){
		gzclose( m_gzfp );
		m_gzfp = NULL;
	}
	
	if( m_fp ){
		fclose( m_fp );
		m_fp = NULL;
	}
}

/*** ラインリード ***********************************************************/

char *CVsdFile::ReadLine( void ){
	*m_cBuf = '\0';
	
	if( m_gzfp ){
		gzgets( m_gzfp, m_cBuf, BUF_LEN );
	}else if( m_fp ){
		fgets( m_cBuf, BUF_LEN, m_fp );
	}

	return m_cBuf;
}

/*** eof チェック ***********************************************************/

int CVsdFile::IsEOF( void ){
	if( m_gzfp ){
		return gzeof( m_gzfp );
	}else if( m_fp ){
		return feof( m_fp );
	}
	return 1;
}
