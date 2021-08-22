// $Id$

#include "sci.h"
//#include <h83664f.h>
#include "3664s.h"

#define rx_buffer_end	RX_BUFFER_LENGTH
#define tx_buffer_end	TX_BUFFER_LENGTH

/* 受信/送信バッファ */
static UB rx_buffer[RX_BUFFER_LENGTH];
static UB tx_buffer[TX_BUFFER_LENGTH];

UCHAR	rx_idx_head;
UCHAR	rx_idx_tail;
UCHAR	tx_idx_head;
UCHAR	tx_idx_tail;

/* 受信エラー処理関数 */
void (*error_handler)(void);

void sci_flush_rx(void)
{
	rx_idx_head = rx_idx_tail = 0;
}

void sci_flush_tx(void)
{
	tx_idx_head = tx_idx_tail = 0;
}

/* 送信レジスタが空になるまで待つ */
void sci_wait_tx_empty(void)
{
	while(SCI3.SSR.BIT.TDRE == 0);
}

/* 受信レジスタがフルになるか、受信エラーが起きるまで待つ */
BOOL sci_wait_rx_full(void)
{
	while((SCI3.SSR.BYTE & 0x78) == 0);
	return SCI3.SSR.BIT.RDRF == 1;
}

/* mesc間なにもしない時間稼ぎ関数 */
void msecwait(int msec)
{
	/*1588は実測によって求めた値*/
	volatile int i, j;
	for(i = 0;i < msec; i++)for(j=0;j < 1588; j++);
}

/* シリアル通信の初期化38400baud */
void sci_init(VP_INT exinf)
{
	volatile int i;

//	rx_buffer_end = rx_buffer + RX_BUFFER_LENGTH;
//	tx_buffer_end = tx_buffer + TX_BUFFER_LENGTH;
	sci_flush_rx();
	sci_flush_tx();

	//SCI3.SCR3.BIT.TE = 0;    /* 送受信Off */
	//SCI3.SCR3.BIT.RE = 0;
	
	SCI3.SCR3.BYTE=0;         /*すべてのflag clear*/
	SCI3.SMR.BYTE=0;          /*Ascnc, 8bit , NoParity, stop1, 1/1*/
	SCI3.BRR=12;              /*2400=>207 4800=>103 9600=>51 19200=>25 38400=>12*/
	
	for(i = 0; i < 280; i++);
	
	SCI3.SCR3.BIT.RIE = 1;      /*受信割り込み可*/
		/* TDRが空の状態で送信割り込みを許可すると、直後に割り込みがかかって困る */
	SCI3.SCR3.BIT.RE = 1;
	IO.PMR1.BIT.TXD = 0;
	SCI3.SCR3.BIT.TIE = 0;
	
	error_handler = 0;
}

size_t sci_read(UB* data, size_t n)
{
	UB *data_begin, *data_end;

	if(!data){
		int szbuf = ( UINT )rx_idx_tail - rx_idx_head;
		if(szbuf < 0)szbuf += RX_BUFFER_LENGTH;
		return (size_t)szbuf;
	}

	data_begin = data;
	data_end = data + n;
	while(rx_idx_head != rx_idx_tail && data != data_end){
		*data++ = rx_buffer[ rx_idx_head++ ];
		if(rx_idx_head == rx_buffer_end)
			rx_idx_head = 0;
	}
	return data - data_begin;
}

size_t sci_write(UB* data, size_t n)
{
	UB *data_begin, *data_end;
	BOOL bTransmit;

	if(!data){
		int szbuf = ( UINT )tx_idx_tail - tx_idx_head;
		if(szbuf < 0)szbuf += RX_BUFFER_LENGTH;
		return (size_t)szbuf;
	}

	data_begin = data;
	data_end = data + n;
	bTransmit = tx_idx_head == tx_idx_tail;
	while(data != data_end){
		tx_buffer[ tx_idx_tail++ ] = *data++;
		if(tx_idx_tail == tx_buffer_end)
			tx_idx_tail = 0;
	}
	/* 送信レジスタが空の場合は送信開始 */
	if(bTransmit){
		SCI3.SCR3.BIT.TE = 1;
		SCI3.SCR3.BIT.TIE = 1;
		IO.PMR1.BIT.TXD = 1;
	}
	return data - data_begin;
}

void sci_int_handler(VP_INT exinf)
{
	/* オーバラン フレーム パリティ エラー */
	if(SCI3.SSR.BYTE & 0x38 ){
		if(error_handler)error_handler();
		SCI3.SSR.BYTE &= ~0x38;
	}
	/* 受信データフル */
	if(SCI3.SSR.BIT.RDRF){
		rx_buffer[ rx_idx_tail++ ] = SCI3.RDR;	/* 読むことでフラグをクリアする */
		if(rx_idx_tail == rx_buffer_end)
			rx_idx_tail = 0;
		//return;
	}
	/* 送信データエンプティ */
	if(SCI3.SSR.BIT.TDRE){
		/* バッファが空の場合は送信終了 */
		if(tx_idx_head == tx_idx_tail){
			/* TEND検出 */
			while(!SCI3.SSR.BIT.TEND);
			SCI3.SCR3.BIT.TE = 0;
			SCI3.SCR3.BIT.TIE = 0;
			IO.PMR1.BIT.TXD = 0;
		}
		else{
			SCI3.TDR = tx_buffer[ tx_idx_head++ ];	/* 書き込むことでフラグをクリアする */
			if(tx_idx_head == tx_buffer_end)
				tx_idx_head = 0;
		}
		//return;
	}
}
