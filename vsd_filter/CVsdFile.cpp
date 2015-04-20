/*****************************************************************************
	
	VSD -- vehicle data logger system  Copyright(C) by DDS
	
	CVsdFile.cpp - CVsdFile class implementation
	
*****************************************************************************/

#include "StdAfx.h"
#include "CVsdFile.h"

#ifdef PUBLIC_MODE
	BOOL IsFileWriteEnabled( void );
#endif

/*** ファイルオープン *******************************************************/

int CVsdFile::Open( LPCWSTR szFile, LPCWSTR szMode ){
	
	LPCWSTR pMode = szMode;
	
	#ifdef PUBLIC_MODE
		// ファイルライト不許可時は失敗コードを返す
		if(
			wcschr( szMode, L'w' ) != NULL &&
			!IsFileWriteEnabled()
		){
			return 1;
		}
	#endif
	
	// gz open
	if( *pMode == 'z' ){
		++pMode;
		char *pFile = NULL;
		char *pMode = NULL;
		
		StringNew( pFile, szFile );
		StringNew( pMode, szMode );
		
		m_gzfp = gzopen( pFile, pMode );
		delete [] pFile;
		delete [] pMode;
		return m_gzfp != NULL ? 0 : 1;
	}
	
	// zip open
	if( *pMode == 'Z' ){
		char *pFile = NULL;
		
		StringNew( pFile, szFile );
		m_unzfp = unzOpen( pFile );
		delete [] pFile;
		
		return m_unzfp != NULL ? 0 : 1;
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
	}else if( m_fp ){
		fclose( m_fp );
		m_fp = NULL;
	}else if( m_unzfp ){
		if( m_bZipCurrentFileOpen ) unzCloseCurrentFile( m_unzfp );
		unzClose( m_unzfp );
		m_unzfp = NULL;
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

/*** ラインリード ***********************************************************/

int CVsdFile::WriteLine( char *str ){
	
	if( m_gzfp ){
		return gzputs( m_gzfp, str );
	}else if( m_fp ){
		return fputs( str, m_fp );
	}

	return -1;
}

/*** seek *******************************************************************/

int CVsdFile::Seek( int iOffs, int iOrg ){
	if( m_gzfp ){
		return gzseek( m_gzfp, iOffs, iOrg ) >= 0 ? 0 : -1;
	}else if( m_fp ){
		return fseek( m_fp, iOffs, iOrg );
	}
	
	return -1;
}

/*** バイナリリード *********************************************************/

UCHAR *CVsdFile::ReadBin( int iSize ){
	static UCHAR cBuf[ 8 ];
	
	*( int *)cBuf = *( int *)( cBuf + 4 ) = 0;
	
	if( m_gzfp ){
		gzread( m_gzfp, cBuf, iSize );
	}else if( m_fp ){
		fread( cBuf, 1, iSize, m_fp );
	}
	return cBuf;
}

/*** バイナリリード *********************************************************/

int CVsdFile::WriteBin( void *pBuf, int iSize ){
	if( m_gzfp ){
		return gzread( m_gzfp, pBuf, iSize );
	}else if( m_fp ){
		return fwrite( pBuf, 1, iSize, m_fp );
	}
	return 0;
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

/*** zip 次のファイル *******************************************************/

v8::Handle<v8::Value> CVsdFile::ZipNextFile( void ){
	unz_file_info fileInfo;

	#define MAX_PATH_LEN	256
	char szFileName[ MAX_PATH_LEN ];
	
	// zip じゃなければ ret
	if( !m_unzfp ) return v8::Undefined();
	
	// ファイルを開いていたら，閉じる
	if( m_bZipCurrentFileOpen ){
		unzCloseCurrentFile( m_unzfp );
		
		// ファイル全部処理した
		if( unzGoToNextFile( m_unzfp ) == UNZ_END_OF_LIST_OF_FILE ){
			return v8::Undefined();
		}
	}
	m_bZipCurrentFileOpen = FALSE;
	
	if(
		unzGetCurrentFileInfo(
			m_unzfp, &fileInfo,
			szFileName, MAX_PATH_LEN - 1,
			NULL, 0, NULL, 0
		) != UNZ_OK
	){
		return v8::Undefined();
	}
	
	if( unzOpenCurrentFile( m_unzfp ) == UNZ_OK ){
		m_bZipCurrentFileOpen = TRUE;
		return v8::String::New( szFileName );
	}
	
	return v8::Undefined();
}
