// $Id$

#ifndef SCI_H
#define SCI_H

//#include <stddef.h>
//#include <itron.h>

#include "dds.h"

typedef int				VP_INT;
typedef unsigned char	UB;
typedef int				size_t;

/* バッファがオーバランしたときの動作は保証されません */
#define RX_BUFFER_LENGTH	32		/* 受信バッファの長さ */
#define TX_BUFFER_LENGTH	48		/* 送信バッファの長さ */

void sci_init(VP_INT exinf);			/* 初期化 */
void sci_int_handler(VP_INT exinf);
void sci_flush_rx(void);				/* 受信バッファクリア */
void sci_flush_tx(void);				/* 送信バッファクリア */
size_t sci_read(UB* data, size_t n);	/* 受信バッファから読み込む */
	/* dataに0を指定すると、受信バッファにたまっているデータのバイト数を返す */
size_t sci_write(UB* data, size_t n);	/* 送信バッファに書き込む */
	/* dataに0を指定すると、送信バッファにたまっているデータのバイト数を返す */

#endif
