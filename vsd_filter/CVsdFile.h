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
	int Seek( int iOffs, int iOrg );	// !js_func
	int IsEOF( void ); // !js_func
	
	// バイナリリーダ
	UCHAR *ReadBin( int iSize );
	
	int ReadChar	( void ){ return *( char *)ReadBin( 1 ); }	// !js_func
	int ReadUChar	( void ){ return *( UCHAR *)ReadBin( 1 ); }	// !js_func
	int ReadShortL	( void ){ return *( short *)ReadBin( 2 ); }	// !js_func
	int ReadUShortL	( void ){ return *( USHORT *)ReadBin( 2 ); } // !js_func
	int ReadIntL	( void ){ return *( int *)ReadBin( 4 ); }	// !js_func
	double ReadUIntL( void ){ return *( UINT *)ReadBin( 4 ); }	// !js_func
	double ReadFloat( void ){ return *( float *)ReadBin( 4 ); }	// !js_func
	double ReadDouble( void ){ return *( float *)ReadBin( 8 ); }	// !js_func
	
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
	
	static const int BUF_LEN = 1024;
	
  private:
	gzFile	m_gzfp;
	FILE	*m_fp;
	char	m_cBuf[ BUF_LEN ];
};
