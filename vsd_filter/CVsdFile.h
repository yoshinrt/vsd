/*****************************************************************************
	
	VSD -- vehicle data logger system  Copyright(C) by DDS
	
	CVsdFile.h - JavaScript File
	
*****************************************************************************/

#pragma once

#include "unzip.h"

class CVsdFile {
  public:
	CVsdFile(){
		m_fp		= NULL;
		m_uMode		= MODE_NORMAL;
		m_uBufSize	= 0;
		m_uBufPtr	= 0;
		m_uFlag		= 0;
	}
	
	~CVsdFile(){
		Close();
	}
	
	v8::Handle<v8::Value> Open( LPCWSTR szFile, LPCWSTR szMode );	// !js_func
	void Close( void ); // !js_func
	v8::Handle<v8::Value> ReadLine( void ); // !js_func
	v8::Handle<v8::Value> WriteLine( char *str ); // !js_func
	v8::Handle<v8::Value> Seek( int iOffs, int iOrg );	// !js_func
	v8::Handle<v8::Value> IsEOF( void ); // !js_func
	
	v8::Handle<v8::Value> ZipNextFile( void ); // !js_func
	int ReadZip( void );
	
	// バイナリアクセス
	UCHAR *ReadBin( int iSize );
	v8::Handle<v8::Value> WriteBin( void *pBuf, int iSize );
	
	int ReadChar	( void ){ return *( char *)ReadBin( 1 ); }	// !js_func
	int ReadUChar	( void ){ return *( UCHAR *)ReadBin( 1 ); }	// !js_func
	int ReadShortL	( void ){ return *( short *)ReadBin( 2 ); }	// !js_func
	int ReadUShortL	( void ){ return *( USHORT *)ReadBin( 2 ); } // !js_func
	int ReadIntL	( void ){ return *( int *)ReadBin( 4 ); }	// !js_func
	double ReadUIntL( void ){ return *( UINT *)ReadBin( 4 ); }	// !js_func
	double ReadFloat( void ){ return *( float *)ReadBin( 4 ); }	// !js_func
	double ReadDouble( void ){ return *( double *)ReadBin( 8 ); }	// !js_func
	
	int ReadShortB	( void ){	// !js_func
		UCHAR *p = ReadBin( 2 );
		return ( short )(( p[ 0 ] << 8 ) + p[ 1 ] );
	}
	int ReadUShortB	( void ){	// !js_func
		UCHAR *p = ReadBin( 2 );
		return ( p[ 0 ] << 8 ) + p[ 1 ];
	}
	
	int ReadIntB( void ){	// !js_func
		UCHAR *p = ReadBin( 4 );
		return	( int )(
			( p[ 0 ] << 24 ) |
			( p[ 1 ] << 16 ) |
			( p[ 2 ] <<  8 ) |
			( p[ 3 ]       )
		);
	}
	double ReadUIntB( void ){	// !js_func
		UCHAR *p = ReadBin( 4 );
		return	( UINT )(
			( p[ 0 ] << 24 ) |
			( p[ 1 ] << 16 ) |
			( p[ 2 ] <<  8 ) |
			( p[ 3 ]       )
		);
	}
	
	v8::Handle<v8::Value> WriteChar	( int iVal ){ return WriteBin( &iVal, 1 ); }	// !js_func
	v8::Handle<v8::Value> WriteShortL	( int iVal ){ return WriteBin( &iVal, 2 ); }	// !js_func
	
	v8::Handle<v8::Value> WriteIntL( double dVal ){	// !js_func
		dVal += ( double )( 1 << 31 ) * 2;
		UINT uTmp = ( UINT )fmod( dVal, ( double )( 1 << 31 ) * 2 );
		return WriteBin( &uTmp, 4 );
	}
	
	v8::Handle<v8::Value> WriteFloat ( double dVal ){	// !js_func
		float fVal = ( float )dVal;
		return WriteBin( &fVal, 4 );
	}
	v8::Handle<v8::Value> WriteDouble( double dVal ){ return WriteBin( &dVal, 8 ); }	// !js_func
	
	v8::Handle<v8::Value> WriteShortB( int iVal ){	// !js_func
		int iTmp = (( UINT )iVal >> 8 ) | (( iVal & 0xFF ) << 8 );
		return WriteShortL( iTmp );
	}
	
	v8::Handle<v8::Value> WriteIntB( double dVal ){	// !js_func
		dVal += ( double )( 1 << 31 ) * 2;
		UINT uTmp = ( UINT )fmod( dVal, ( double )( 1 << 31 ) * 2 );
		UINT uTmp2 =
			(( uTmp              ) >> 24 ) |
			(( uTmp & 0x00FF0000 ) >>  8 ) |
			(( uTmp & 0x0000FF00 ) <<  8 ) |
			(( uTmp              ) << 24 );
		return WriteBin( &uTmp2, 4 );
	}
	
	static const int FILEBUF_LEN = 10240;
	
  private:
	union {
		gzFile	m_gzfp;
		FILE	*m_fp;
		unzFile	m_unzfp;
	};
	
	// zip 用
	UINT	m_uBufSize;
	UINT	m_uBufPtr;
	
	USHORT	m_uMode;
	
	enum {
		MODE_NORMAL,
		MODE_GZIP,
		MODE_ZIP,			// 内部 file を open していない
		MODE_ZIP_OPENED,	// 内部 file を open 中
	};
	
	USHORT	m_uFlag;
	
	enum {
		FLAG_EOF		= 1 << 0,
		FLAG_2ND_FILE	= 1 << 1,
	};
	
	char	m_cBuf[ FILEBUF_LEN ];
};
