#ifndef _USART_H
#define _USART_H

#include "stm32f10x_usart.h"
#include "dds.h"

#define USART_TXBUF_SIZE	64
#define USART_RXBUF_SIZE	64

typedef struct {
	volatile USHORT	uTxBufRp, uTxBufWp;
	volatile USHORT	uRxBufRp, uRxBufWp;
	UCHAR	cTxBuf[ USART_TXBUF_SIZE ];
	UCHAR	cRxBuf[ USART_RXBUF_SIZE ];
} USART_BUF_t;

void UsartInit( UINT uBaudRate, USART_BUF_t *pBuf );
int GetcharWait( void );
void UsartPutstr( char *szMsg );

#endif // _USART_H
