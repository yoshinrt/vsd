/*****************************************************************************
	
	VSD -- vehicle data logger system  Copyright(C) by DDS
	
	CVsdFile.cpp - CVsdFile class implementation
	
*****************************************************************************/

#include "StdAfx.h"
#include "CVsdFile.h"
#include "CScript.h"

#ifdef PUBLIC_MODE
	BOOL IsFileWriteEnabled( void );
#endif

#define V8ErrorClosedHandle()		V8Error( "operation not permitted on closed handle" )
#define V8ErrorZipNotSupported()	V8Error( "operation not supported on zip-mode" )
#define V8ErrorZipNotOpened()		V8Error( "file in zip not opened, execute ZipNextFile() first." )

/*** ファイルオープン *******************************************************/

int CVsdFile::Open( LPCWSTR szFile, LPCWSTR szMode ){
	
	LPCWSTR pMode = szMode;
	
	#ifdef PUBLIC_MODE
		// ファイルライト不許可時は失敗コードを返す
		if(
			wcschr( szMode, L'w' ) != NULL &&
			!IsFileWriteEnabled()
		){
			V8Error( "write operation not permitted by user" );
			return -1;
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
		
		if( m_gzfp ) m_uMode = MODE_GZIP;
	}
	
	// zip open
	else if( *pMode == 'Z' ){
		char *pFile = NULL;
		
		StringNew( pFile, szFile );
		m_unzfp = unzOpen( pFile );
		delete [] pFile;
		
		if( m_unzfp ) m_uMode = MODE_ZIP;
	}
	
	// 普通 fopen
	else{
		m_fp = _wfopen( szFile, szMode );
	}
	
	if( m_fp == NULL ) V8Error( ERR_CANT_OPEN_FILE );
	return 0;
}

/*** ファイルクローズ *******************************************************/

void CVsdFile::Close( void ){
	if( m_fp ) switch( m_uMode ){
		case MODE_NORMAL:		fclose( m_fp );
		Case MODE_GZIP:			gzclose( m_gzfp );
		
		Case MODE_ZIP_OPENED:	unzCloseCurrentFile( m_unzfp );
		case MODE_ZIP:			unzClose( m_unzfp );
	}
	m_fp = NULL;
}

/*** ラインリード ***********************************************************/

v8::Handle<v8::Value> CVsdFile::ReadLine( void ){
	*m_cBuf = '\0';
	
	if( !m_fp ){
		V8ErrorClosedHandle();
		return v8::Undefined();
	}
	
	switch( m_uMode ){
		case MODE_NORMAL:		fgets( m_cBuf, FILEBUF_LEN, m_fp );
		Case MODE_GZIP:			gzgets( m_gzfp, m_cBuf, FILEBUF_LEN );
		
		Case MODE_ZIP: {
			V8ErrorZipNotOpened();
			return v8::Undefined();
		}
		
		Case MODE_ZIP_OPENED: {
			
			// \n サーチ
			char *p = ( char *)memchr( m_cBuf + m_uBufPtr, '\n', m_uBufSize - m_uBufPtr );
			
			// 見つからないので，buf read する
			if( !p ){
				ReadZip();
				p = ( char *)memchr( m_cBuf + m_uBufPtr, '\n', m_uBufSize - m_uBufPtr );
				
				// 再び見つからなかったので，p は buf 終端を指す
				if( !p ) p = m_cBuf + m_uBufSize - 1;
			}
			
			// 文字列長付きで v8::String 変換
			int iStr = m_uBufPtr;
			m_uBufPtr = ( p - m_cBuf + 1 );
			return v8::String::New( m_cBuf + iStr, m_uBufPtr - iStr );
		}
	}
	return v8::String::New( m_cBuf );
}

/*** ラインライト ***********************************************************/

int CVsdFile::WriteLine( char *str ){
	
	if( m_fp ) switch( m_uMode ){
		case MODE_NORMAL:		return fputs( str, m_fp );
		case MODE_GZIP:			return gzputs( m_gzfp, str );
		
		case MODE_ZIP:
		case MODE_ZIP_OPENED:	V8ErrorZipNotSupported();
	}
	else V8ErrorClosedHandle();
	
	return 0;
}

/*** seek *******************************************************************/

int CVsdFile::Seek( int iOffs, int iOrg ){
	
	if( m_fp ) switch( m_uMode ){
		case MODE_NORMAL:		return fseek( m_fp, iOffs, iOrg );
		case MODE_GZIP:			return gzseek( m_gzfp, iOffs, iOrg ) >= 0 ? 0 : -1;
		
		case MODE_ZIP:
		case MODE_ZIP_OPENED:	V8ErrorZipNotSupported();
	}
	else V8ErrorClosedHandle();
	
	return 0;
}

/*** バイナリリード *********************************************************/

UCHAR *CVsdFile::ReadBin( int iSize ){
	static UCHAR cBuf[ 8 ];
	
	*( int *)cBuf = *( int *)( cBuf + 4 ) = 0;
	
	if( m_fp )	switch( m_uMode ){
		case MODE_NORMAL:		fread( cBuf, 1, iSize, m_fp );
		Case MODE_GZIP:			gzread( m_gzfp, cBuf, iSize );
		Case MODE_ZIP:			V8ErrorZipNotOpened();
		Case MODE_ZIP_OPENED: {
			// buf 残量が足りなかったら，読む
			if( iSize > ( int )( m_uBufSize - m_uBufPtr )) ReadZip();
			if( iSize > ( int )( m_uBufSize - m_uBufPtr )) iSize = m_uBufSize - m_uBufPtr;
			
			// リードバッファにコピー
			memcpy( cBuf, m_cBuf + m_uBufPtr, iSize );
			m_uBufPtr += iSize;
		}
	}
	return cBuf;
}

/*** バイナリライト *********************************************************/

int CVsdFile::WriteBin( void *pBuf, int iSize ){
	if( m_fp ) switch( m_uMode ){
		case MODE_NORMAL:		return fwrite( pBuf, 1, iSize, m_fp );
		case MODE_GZIP:			return gzread( m_gzfp, pBuf, iSize );
		
		case MODE_ZIP:
		case MODE_ZIP_OPENED:	V8ErrorZipNotSupported();
	}
	else V8ErrorClosedHandle();
	return 0;
}

/*** eof チェック ***********************************************************/

int CVsdFile::IsEOF( void ){
	if( m_fp ) switch( m_uMode ){
		case MODE_NORMAL:		return feof( m_fp );
		case MODE_GZIP:			return gzeof( m_gzfp );
		
		case MODE_ZIP:			V8ErrorZipNotSupported();
		
		case MODE_ZIP_OPENED: return
			!( m_uFlag & FLAG_EOF ) ? 0 :
			m_uBufPtr == m_uBufSize ? -1 : 0;
	}
	else V8ErrorClosedHandle();
	return 0;
}

/*** zip 次のファイル *******************************************************/

v8::Handle<v8::Value> CVsdFile::ZipNextFile( void ){
	unz_file_info fileInfo;
	
	LPWSTR	wszFileName = NULL;
	
	// zip じゃなければ ret
	if( !( MODE_ZIP <= m_uMode && m_uMode <= MODE_ZIP_OPENED )){
		V8Error( "not opened by zip-mode" );
		return v8::Undefined();
	}
	
	m_uFlag &= ~FLAG_EOF;	// eof クリア
	
	// ファイルを開いていたら，閉じる
	if( m_uMode == MODE_ZIP_OPENED ){
		unzCloseCurrentFile( m_unzfp );
	}
	m_uMode = MODE_ZIP;
	
	while( 1 ){
		if( m_uFlag & FLAG_2ND_FILE ){
			// ファイル全部処理した
			if( unzGoToNextFile( m_unzfp ) == UNZ_END_OF_LIST_OF_FILE ){
				return v8::Undefined();
			}
		}
		m_uFlag |= FLAG_2ND_FILE;
		
		// 書庫内ファイル名取得
		if(
			unzGetCurrentFileInfo(
				m_unzfp, &fileInfo,
				m_cBuf, FILEBUF_LEN - 1,
				NULL, 0, NULL, 0
			) != UNZ_OK
		){
			V8Error( "zlib: unzGetCurrentFileInfo failed (maybe wrong zip format?)" );
			return v8::Undefined();
		}
		
		// unicode 変換
		StringNew( wszFileName, m_cBuf );
		
		// '/' で終わっていたら dir なのでスキップ
		int iLen = wcslen( wszFileName ) - 1;
		if( iLen >= 0 && wszFileName[ iLen ] != '/' ) break;
	}
	
	// 書庫内ファイルオープン
	if( unzOpenCurrentFile( m_unzfp ) == UNZ_OK ){
		m_uMode = MODE_ZIP_OPENED;
		
		v8::Local<v8::String> v8strFileName = v8::String::New(( uint16_t *)wszFileName );
		delete [] wszFileName;
		
		return v8strFileName;
	}
	
	V8Error( "zlib: can't open file in zip (maybe wrong zip format?)" );
	return v8::Undefined();
}

/*** zip ファイルの body リード *********************************************/

int CVsdFile::ReadZip( void ){
	
	// 残っているデータを配列先頭に move
	if( m_uBufPtr != 0 && m_uBufPtr != m_uBufSize ){
		memmove( m_cBuf, m_cBuf + m_uBufPtr, m_uBufSize - m_uBufPtr );
	}
	m_uBufSize -= m_uBufPtr;
	m_uBufPtr = 0;
	
	// リード
	int iBufRemained = FILEBUF_LEN - m_uBufSize;
	int iReadSize = unzReadCurrentFile( m_unzfp, m_cBuf + m_uBufSize, iBufRemained );
	m_uBufSize += iReadSize;
	
	if( iReadSize < iBufRemained ) m_uFlag |= FLAG_EOF;
	
	return iReadSize;
}
