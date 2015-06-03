/*****************************************************************************
	
	VSD2 - vehicle data logger system2
	Copyright(C) by DDS
	
	main.c -- main routine
	
*****************************************************************************/

#include "dds.h"
#include <stdio.h>
#include <ST\iostm32f10xxB.h>
#include "stm32f10x_nvic.h"
#include "stm32f10x_gpio.h"
#include "hw_config.h"
#include "main2.h"
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

__noreturn void main( void ){
	
	#ifndef EXEC_SRAM
		Set_System();
		GPIOC->ODR ^= 0x40;    // LEDの出力を反転させる。
		UsartInit( 38400, NULL );
		printf( "Waiting for S record...\n" );
		LoadSRecord();
	#endif
	
	// USART buf
	USART_BUF_t	UsartBuf = { 0 };
	
	// ベクタテーブル再設定
	NVIC_SetVectorTable( NVIC_VectTab_RAM, 0 );
	
	UsartInit( 38400, &UsartBuf );
	TimerInit();
	PulseInit();
	
	
	UINT uPrevTime = GetCurrentTime16();
	UINT uTSC = 0;
	
	// デバッグ用
	UINT uTachoTime = GetCurrentTime();
	UINT uTacho = 1000;
	
	while( 1 ){
		// ログ周期待ち
		while((( GetCurrentTime16() - uPrevTime ) & 0xFFFF ) < ( TIMER_HZ / LOG_HZ )){
			
			if( uTacho ){
				UINT uTachoCntDiff = ( UINT )( TIMER_HZ * 30 ) / uTacho;
//printf( "%d %d %d\n", GetCurrentTime(), uTachoTime, uTachoCntDiff );
				// 割り込みエミュレーション
				if( GetCurrentTime() - uTachoTime > uTachoCntDiff ){
					GPIOC->ODR ^= 0x40;    // LEDの出力を反転させる。
					EXTI1_IRQHandler();
					uTachoTime += uTachoCntDiff;
				}
			}
		}
		uPrevTime += TIMER_HZ / LOG_HZ;
		
		ComputeMeterTacho();
		printf( "%d %d %d\n", g_Tacho.uVal, g_Speed.uVal, ++uTSC );
		
		char c = getchar();
		if( c == 'a' ) uTacho -= 100;
		if( c == 's' ) uTacho -= 50;
		if( c == 'd' ) uTacho += 50;
		if( c == 'f' ) uTacho += 100;
		if( c == 'z' ) LoadSRecord();
	}
}
