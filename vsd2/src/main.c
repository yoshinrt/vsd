/*****************************************************************************
	
	VSD2 - vehicle data logger system2
	Copyright(C) by DDS
	
	main.c -- main routine
	
*****************************************************************************/

#include <stdio.h>
#include <ST\iostm32f10xxB.h>
#include "stm32f10x_nvic.h"
//#define DEBUG
#include "dds.h"
#include "usart.h"

/*** macros *****************************************************************/

#define SRAM_TOP	0x20000000
#define SRAM_END	0x20005000

/*** const ******************************************************************/
/*** new type ***************************************************************/
/*** prototype **************************************************************/
/*** extern *****************************************************************/
/*** gloval vars ************************************************************/
/*** S レコードローダ *******************************************************/

UINT GetHex( UINT uBytes ){
	
	uBytes <<= 1;;
	UINT	uRet = 0;
	UINT	c;
	
	do{
		uRet <<= 4;
		c = GetcharWait();
		
		if( '0' <= c && c <= '9' )	uRet |= c - '0';
		else						uRet |= c - ( 'A' - 10 );
	}while( --uBytes );
	
	//DbgMsg(( "%02X ", uRet ));
	return uRet;
}

__noreturn void JumpTo( u32 uJmpAddr, u32 uSP ){
	asm( "MSR MSP, r1\nBX r0\n" );
}

__noreturn void LoadSRecord( void ){
	
	UINT	uAddr, uLen;
	UINT	c;
	
	while( 1 ){
		// 'S' までスキップ
		while( GetcharWait() != 'S' );
		
		// 終了ヘッダなら break;
		if(( c = GetcharWait()) == '7' ) break;
		
		if( c == '3' ){
			// データを書き込む
			uLen	= GetHex( 1 ) - 5;
			uAddr	= GetHex( 4 );
			
			DbgMsg(( "Addr:%X Len:%X\n", uAddr, uLen ));
			while( uLen-- ) *( UCHAR *)( uAddr++ ) = GetHex( 1 );
		}
	}
	
	printf( "starting %X...\n", *( u32 *)0x20000004 );
	JumpTo( *( u32 *)0x20000004, *( u32 *)0x08003000 );
}

/*** バイナリローダ *********************************************************/

__noreturn void LoadBin( void ){
	UINT uCnt;
	UINT uSize = GetcharWait() | ( GetcharWait() << 8 );
	
	DbgMsg(( "\nloading %d bytes\n", uSize ));
	
	for( uCnt = 0; uCnt < uSize; ++uCnt ){
		*( UCHAR *)( 0x20000000 + uCnt ) = GetcharWait();
	}
	
	DbgMsg(( "starting %X...\n", *( u32 *)0x20000004 ));
	JumpTo( *( u32 *)0x20000004, *( u32 *)0x08003000 );
}

/****************************************************************************/

void timer( unsigned long i ){
	while( i-- );
}

__noreturn void main( void ){
	// USART buf
	USART_BUF_t	UsartBuf = { 0 };
	
	RCC_APB2ENR |= 0x10;     // CPIOCを使用できるようにする。
	GPIOC_CRL = 0x43444444;   // PC6を出力にする。　　
	GPIOC_ODR ^= 0x40;    // LEDの出力を反転させる。
	
	// ベクタテーブル再設定
	#ifdef EXEC_SRAM
		NVIC_SetVectorTable( NVIC_VectTab_RAM, 0 );
	#endif
	
	UsartInit( 38400, &UsartBuf );
	printf( "USART test\n" );
	
	LoadSRecord();
}
