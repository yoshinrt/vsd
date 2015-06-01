#include <stdio.h>
#include "stm32f10x_rcc.h"
#include "stm32f10x_gpio.h"
#include "dds.h"
#include "usart.h"

#define	DEFAULT_PORT	USART1

/*** バッファ ***************************************************************/

#define TXBUF_SIZE	64
#define RXBUF_SIZE	64

UCHAR	g_cTxBuf[ TXBUF_SIZE ];
volatile USHORT	g_uTxBufRp = 0, g_uTxBufWp = 0;
UCHAR	g_cRxBuf[ TXBUF_SIZE ];
volatile USHORT	g_uRxBufRp = 0, g_uRxBufWp = 0;

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
	
	// USART ハード初期化
	USART_DeInit( DEFAULT_PORT );
	USART_Init( DEFAULT_PORT, &Param );
	USART_Cmd( DEFAULT_PORT, ENABLE );
	
	// 割り込み設定
	USART_ITConfig( DEFAULT_PORT, USART_IT_RXNE, ENABLE );
}

/*** 割り込みハンドラ *******************************************************/

void USART1_IRQHandler( void ){
	
	// 受信バッファフル
	if( DEFAULT_PORT->SR & ( 1 << 5 )){
		UINT uWp = g_uRxBufWp;
		g_cRxBuf[ uWp ] = USART_ReceiveData( DEFAULT_PORT );
		
		uWp = ( uWp + 1 ) & ( RXBUF_SIZE - 1 );
		g_uRxBufWp = uWp;
		
		// RxBuf フルなら，割込み停止
		/*
		uWp = ( uWp + 1 ) & ( RXBUF_SIZE - 1 );
		if( uWp == g_uRxBufRp ){
			USART_ITConfig( DEFAULT_PORT, USART_IT_RXNE, DISABLE );
		}*/
	}
	
	// 送信バッファエンプティ
	if( DEFAULT_PORT->SR & ( 1 << 7 )){
		UINT uRp = g_uTxBufRp;
		
		if( uRp != g_uTxBufWp ){
			USART_SendData( DEFAULT_PORT, g_cTxBuf[ uRp ]);
			uRp = ( uRp + 1 ) & ( TXBUF_SIZE - 1 );
			g_uTxBufRp = uRp;
			
			// 送信データが無くなったので割り込み禁止
			if( uRp == g_uTxBufWp ){
				USART_ITConfig( DEFAULT_PORT, USART_IT_TXE, DISABLE );
			}
		}
	}
}

/*** 出力 *******************************************************************/

void UsartPutchar( UCHAR c ){
	UINT uWp = g_uTxBufWp;
	UINT uNextWp = ( uWp + 1 ) & ( TXBUF_SIZE - 1 );
	
	// 送信バッファ Full なので待ち
	while( uNextWp == g_uTxBufRp );
	
	g_cTxBuf[ uWp ] = c;
	g_uTxBufWp = uNextWp;
	
	// tx 割り込み許可
	USART_ITConfig( DEFAULT_PORT, USART_IT_TXE, ENABLE );
}

int UsartGetchar( void ){
	UINT uRp = g_uRxBufRp;
	if( uRp == g_uRxBufWp ) return EOF;
	
	int iRet = g_cRxBuf[ uRp ];
	g_uRxBufRp = ( uRp + 1 ) & ( RXBUF_SIZE - 1 );
	
	// rx 割込み許可
	//USART_ITConfig( DEFAULT_PORT, USART_IT_RXNE, ENABLE );
	return iRet;
}

void UsartPutstr( char *szMsg ){
	while( *szMsg ){
		UsartPutchar( *szMsg );
		++szMsg;
	}
}
