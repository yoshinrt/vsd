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
/*** const ******************************************************************/
/*** new type ***************************************************************/
/*** prototype **************************************************************/
/*** extern *****************************************************************/
/*** gloval vars ************************************************************/
/****************************************************************************/

__noreturn void main( void ){
	#ifndef EXEC_SRAM
		Set_System();
		GPIOC->ODR ^= 0x40;    // LEDの出力を反転させる。
		UsartInit( USART_BAUDRATE, NULL );
		LoadSRecord();
	#endif
	
	// USART buf
	USART_BUF_t	UsartBuf = { 0 };
	
	// ベクタテーブル再設定
	NVIC_SetVectorTable( NVIC_VectTab_RAM, 0 );
	
	UsartInit( USART_BAUDRATE, &UsartBuf );
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
