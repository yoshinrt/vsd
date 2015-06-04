/*****************************************************************************
	
	VSD2 - vehicle data logger system2
	Copyright(C) by DDS
	
	main.c -- main routine
	
*****************************************************************************/

#include "dds.h"
#include <stdio.h>
#include "stm32f10x_rcc.h"
#include "stm32f10x_nvic.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_adc.h"
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
	/*** 初期化 *************************************************************/
	
	// USART buf
	USART_BUF_t	UsartBuf	= { 0 };
	
	#ifndef EXEC_SRAM
		Set_System();
		
		// SD カードが挿入されていたら，開発用に即 LoadSRecord()
		SdcInit();
		if( SdcInserted()){
			LedOn();
			LoadSRecord();
		}
	#else
		// ベクタテーブル再設定
		NVIC_SetVectorTable( NVIC_VectTab_RAM, 0 );
	#endif
	
	VSD_DATA_t	Vsd	= { 0 };
	g_pVsd = &Vsd;
	
	Vsd.uComputeMeterConst	= ( UINT )( TIMER_HZ * 3600.0 * 100 / PULSE_PER_1KM );
	Vsd.uMileage_0_400		= ( UINT )( PULSE_PER_1KM * 400 / 1000 + 0.5 );
	Vsd.uLogHz				= LOG_HZ;
	Vsd.uOutputPrevTime		= GetCurrentTime16();
	
	UsartInit( USART_BAUDRATE, &UsartBuf );
	
	AdcInit();
	TimerInit();
	PulseInit();
	
	/*** メインループ *******************************************************/
	
	// ★暫定
	//Vsd.Flags.bConnected = 1;
	
	while( 1 ){
		LedToggle();
		WaitStateChange( &Vsd );
		
		ComputeMeterSpeed( &Vsd );
		ComputeMeterTacho( &Vsd );
		Calibration( &Vsd );
		
		if( Vsd.Flags.bConnected ){
			OutputSerial( &Vsd );
			
			// 3秒接続断なら再起動
			if( ++Vsd.uConnectWDT > Vsd.uLogHz * 3 ){
				// ★リセット
			}
		}else{
			// ★ FW 再ダウンロード要求
			//putchar( 0xFF );
		}
	}
}
