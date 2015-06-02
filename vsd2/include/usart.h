#ifndef _USART_H
#define _USART_H

#include "stm32f10x_usart.h"
#include "dds.h"

#define USART_TXBUF_SIZE	64
#define USART_RXBUF_SIZE	64

typedef struct USART_BUF_t {
	volatile USHORT	uTxBufRp, uTxBufWp;
	volatile USHORT	uRxBufRp, uRxBufWp;
	UCHAR	cTxBuf[ USART_TXBUF_SIZE ];
	UCHAR	cRxBuf[ USART_TXBUF_SIZE ];
};

void UsartInit( UINT uBaudRate, USART_BUF_t *pBuf );
void UsartPutchar( UCHAR c );
int UsartGetchar( void );
int UsartGetcharWait( void );
void UsartPutstr( char *szMsg );

#endif // _USART_H
