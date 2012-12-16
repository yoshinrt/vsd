#include <stdio.h>
#include <ctype.h>

typedef	unsigned UINT;

UINT ToHex( char *p, UINT uBytes ){
	
	UINT	uRet = 0;
	uBytes *= 2;
	
	while( uBytes-- ){
		uRet <<= 4;
		
		if( isdigit( *p )){
			uRet |= *p - '0';
		}else{
			uRet |= *p - ( 'A' - 10 );
		}
		++p;
	}
	
//	printf( "%02X ", uRet );
	return uRet;
}

int main( int argc, char **argv ){
	
	FILE	*fp;
	char	szBuf[ 256 ];
	char	Mem[ 0x4800 ];
	char	*p;
	UINT	uAddr, uLen;
	
	for( uAddr = 0; uAddr < 0x4800; ++uAddr ) Mem[ uAddr ] = 0;
	fp = fopen( argv[ 1 ], "r" );
	
	while( fgets( szBuf, 256, fp )) if( szBuf[ 1 ] == '3' ){
		
		uLen  = ToHex( szBuf + 2, 1 ) - 5;
		uAddr = ToHex( szBuf + 4, 4 ) - 0x20000000;
		
		p = szBuf + 12;
		
		while( uLen-- ){
			if( uAddr >= 0x4800 ){
				printf( "addr:%08X\n", uAddr );
				break;
			}
			Mem[ uAddr++ ] = ToHex( p, 1 );
			p += 2;
		}
	}
	
	fclose( fp );
	sprintf( szBuf, "%s.bin", argv[ 1 ] );
	fp = fopen( szBuf, "wb" );
	fwrite( Mem, 1, 0x4800, fp );
	fclose( fp );
	
	return 0;
}
