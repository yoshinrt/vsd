#include <stdio.h>
#include "stm32f10x_rcc.h"
#include "stm32f10x_gpio.h"
#include "dds.h"
#include "usart.h"

#define	DEFAULT_PORT	USART1

/*** 初期化 *****************************************************************/

void UsartInit( UINT uBaudRate ){
	// APB クロック
	RCC_APB2PeriphClockCmd( RCC_APB2Periph_AFIO | RCC_APB2Periph_GPIOA | RCC_APB2Periph_USART1, ENABLE );
	
	// GPIO設定
	GPIO_InitTypeDef GPIO_InitStruct;
	
	GPIO_StructInit( &GPIO_InitStruct );
	GPIO_InitStruct.GPIO_Pin   = GPIO_Pin_9;
	GPIO_InitStruct.GPIO_Mode  = GPIO_Mode_AF_PP;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init( GPIOA, &GPIO_InitStruct );
	
	GPIO_InitStruct.GPIO_Pin  = GPIO_Pin_10;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init( GPIOA, &GPIO_InitStruct );
	
	// パラメータ初期化
	USART_InitTypeDef	Param;
	USART_StructInit( &Param );
	Param.USART_BaudRate	= uBaudRate;
	Param.USART_Clock		= USART_Clock_Enable;
	
	// USART ハード初期化
	USART_DeInit( DEFAULT_PORT );
	USART_Init( DEFAULT_PORT, &Param );
	USART_Cmd( DEFAULT_PORT, ENABLE );
	
	// 割り込み設定
	//USART_ITConfig( DEFAULT_PORT
}

/*** 出力 *******************************************************************/

void UsartPutchar( UCHAR c ){
	if( c == '\n' ) UsartPutchar( '\r' );
	USART_SendData( DEFAULT_PORT, c );
}

void UsartPutstr( char *szMsg ){
	while( *szMsg ){
		while( USART_GetFlagStatus( DEFAULT_PORT, USART_FLAG_TXE ) == RESET );
		UsartPutchar( *szMsg );
		++szMsg;
	}
}
