/*****************************************************************************
	
	VSD2 - vehicle data logger system2
	Copyright(C) by DDS
	
	uart.c -- USART1 driver
	
*****************************************************************************/

#include <stdio.h>
#include "stm32f10x_rcc.h"
#include "stm32f10x_gpio.h"
#include "dds.h"
#include "usart.h"

#define	DEFAULT_PORT	USART1

//#define NO_INT

/*** バッファ ***************************************************************/

USART_BUF_t	*g_pUsartBuf;

/*** 初期化 *****************************************************************/

void UsartInit( UINT uBaudRate, USART_BUF_t *pBuf ){
	g_pUsartBuf = pBuf;
	
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
	
	// USART ハード初期化
	USART_DeInit( DEFAULT_PORT );
	USART_Init( DEFAULT_PORT, &Param );
	USART_Cmd( DEFAULT_PORT, ENABLE );
	
	// 割り込み設定
	#ifdef NO_INT
		USART_ITConfig( DEFAULT_PORT, USART_IT_RXNE, ENABLE );
	#endif
}

/*** 割り込みハンドラ *******************************************************/

void USART1_IRQHandler( void ){
	
	// 受信バッファフル
	if( DEFAULT_PORT->SR & ( 1 << 5 )){
		UINT uWp = g_pUsartBuf->uRxBufWp;
		g_pUsartBuf->cRxBuf[ uWp ] = USART_ReceiveData( DEFAULT_PORT );
		
		uWp = ( uWp + 1 ) & ( USART_RXBUF_SIZE - 1 );
		g_pUsartBuf->uRxBufWp = uWp;
		
		// RxBuf フルなら，割込み停止
		/*
		uWp = ( uWp + 1 ) & ( USART_RXBUF_SIZE - 1 );
		if( uWp == g_pUsartBuf->uRxBufRp ){
			USART_ITConfig( DEFAULT_PORT, USART_IT_RXNE, DISABLE );
		}*/
	}
	
	// 送信バッファエンプティ
	if( DEFAULT_PORT->SR & ( 1 << 7 )){
		UINT uRp = g_pUsartBuf->uTxBufRp;
		
		if( uRp != g_pUsartBuf->uTxBufWp ){
			USART_SendData( DEFAULT_PORT, g_pUsartBuf->cTxBuf[ uRp ]);
			uRp = ( uRp + 1 ) & ( USART_TXBUF_SIZE - 1 );
			g_pUsartBuf->uTxBufRp = uRp;
			
			// 送信データが無くなったので割り込み禁止
			if( uRp == g_pUsartBuf->uTxBufWp ){
				USART_ITConfig( DEFAULT_PORT, USART_IT_TXE, DISABLE );
			}
		}
	}
}

/*** 1文字入出力 ************************************************************/

int putchar( int c ){
#ifdef NO_INT
	while( !( DEFAULT_PORT->SR & ( 1 << 7 )));
	USART_SendData( DEFAULT_PORT, c );
	return c;
}

int putchar_int( int c ){
#endif
	UINT uWp = g_pUsartBuf->uTxBufWp;
	UINT uNextWp = ( uWp + 1 ) & ( USART_TXBUF_SIZE - 1 );
	
	// 送信バッファ Full なので待ち
	while( uNextWp == g_pUsartBuf->uTxBufRp );
	
	g_pUsartBuf->cTxBuf[ uWp ] = c;
	g_pUsartBuf->uTxBufWp = uNextWp;
	
	// tx 割り込み許可
	USART_ITConfig( DEFAULT_PORT, USART_IT_TXE, ENABLE );
	
	return c;
}

int getchar( void ){
#ifdef NO_INT
	if( DEFAULT_PORT->SR & ( 1 << 5 )){
		return USART_ReceiveData( DEFAULT_PORT );
	}
	return EOF;
}

int getchar_int( void ){
#endif
	UINT uRp = g_pUsartBuf->uRxBufRp;
	if( uRp == g_pUsartBuf->uRxBufWp ) return EOF;
	
	int iRet = g_pUsartBuf->cRxBuf[ uRp ];
	g_pUsartBuf->uRxBufRp = ( uRp + 1 ) & ( USART_RXBUF_SIZE - 1 );
	
	// rx 割込み許可
	//USART_ITConfig( DEFAULT_PORT, USART_IT_RXNE, ENABLE );
	return iRet;
}

int GetcharWait( void ){
	int c;
	while(( c = getchar()) == EOF ) /*_WFI*/;
	return c;
}
