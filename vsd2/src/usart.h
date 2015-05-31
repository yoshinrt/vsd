#ifndef _USART_H
#define _USART_H

#include "stm32f10x_usart.h"
#include "dds.h"

void UsartInit( UINT uBaudRate );
void UsartPutchar( UCHAR c );
void UsartPutstr( char *szMsg );

#endif // _USART_H
