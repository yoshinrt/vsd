#ifndef _USART_H
#define _USART_H

#include "dds.h"
#include "stm32f10x_usart.h"

#define USART_TXBUF_SIZE	64
#define USART_RXBUF_SIZE	64

typedef struct {
	volatile USHORT	uTxBufRp, uTxBufWp;
	volatile USHORT	uRxBufRp, uRxBufWp;
	UCHAR	cTxBuf[ USART_TXBUF_SIZE ];
	UCHAR	cRxBuf[ USART_RXBUF_SIZE ];
} USART_BUF_t;

extern USART_BUF_t	*g_pUsartBuf;

void UsartInit( UINT uBaudRate, USART_BUF_t *pBuf );
int UsartPutcharUnbuffered( int c );
int UsartPutcharBuffered( int c );
int UsartGetcharUnbuffered( void );
int UsartGetcharBuffered( void );
int UsartGetcharWaitUnbuffered( void );
int UsartGetcharWaitBuffered( void );
int GetcharWait( void );
void UsartPutstr( char *szMsg );

#endif // _USART_H
