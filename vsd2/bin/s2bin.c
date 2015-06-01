#include <stdio.h>
#include <ctype.h>

typedef	unsigned UINT;

#define SRAM_SIZE	( 20 * 1024 )

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

char	Mem[ SRAM_SIZE ] = { 0 };

int main( int argc, char **argv ){
	
	FILE	*fp;
	char	szBuf[ 256 ];
	char	*p;
	UINT	uAddr, uLen;
	UINT	uMaxAddr = 0;
	
	fp = fopen( argv[ 1 ], "r" );
	
	while( fgets( szBuf, 256, fp )) if( szBuf[ 1 ] == '3' ){
		
		uLen  = ToHex( szBuf + 2, 1 ) - 5;
		uAddr = ToHex( szBuf + 4, 4 ) - 0x20000000;
		
		p = szBuf + 12;
		
		while( uLen-- ){
			if( uAddr >= SRAM_SIZE ){
				printf( "addr:%08X\n", uAddr );
				break;
			}
			Mem[ uAddr++ ] = ToHex( p, 1 );
			p += 2;
			
			if( uAddr > uMaxAddr ) uMaxAddr = uAddr;
		}
	}
	
	fclose( fp );
	sprintf( szBuf, "%s.bin", argv[ 1 ] );
	fp = fopen( szBuf, "wb" );
	fwrite( &uMaxAddr, 1, 2, fp );
	fwrite( Mem, 1, uMaxAddr, fp );
	fclose( fp );
	
	return 0;
}
