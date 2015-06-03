/*****************************************************************************
	
	VSD2 - vehicle data logger system2
	Copyright(C) by DDS
	
	main.c -- main routine
	
*****************************************************************************/

#include "dds.h"
#include <stdio.h>
#include <ST\iostm32f10xxB.h>
#include "stm32f10x_nvic.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_adc.h"
#include "hw_config.h"
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
	#ifndef EXEC_SRAM
		Set_System();
		GPIOC->ODR ^= 0x40;    // LEDの出力を反転させる。
		UsartInit( USART_BAUDRATE, NULL );
		LoadSRecord();
	#endif
	
	// USART buf
	USART_BUF_t	UsartBuf = { 0 };
	
	// ベクタテーブル再設定
	NVIC_SetVectorTable( NVIC_VectTab_RAM, 0 );
	
	UsartInit( USART_BAUDRATE, &UsartBuf );
	TimerInit();
	PulseInit();
	
	printf( "ADC init..." );
	AdcInit();
	printf( "done.\n" );
	
	while( 1 ){
		AdcConversion();
		
		printf( "%X %X %X %04X %04X %04X %04X\n",
			ADC1->JSQR,
			ADC1->CR1,
			ADC1->CR2,
			ADC_GetInjectedConversionValue( ADC1, ADC_InjectedChannel_1 ),
			ADC_GetInjectedConversionValue( ADC1, ADC_InjectedChannel_2 ),
			ADC_GetInjectedConversionValue( ADC1, ADC_InjectedChannel_3 ),
			ADC_GetInjectedConversionValue( ADC1, ADC_InjectedChannel_4 )
		);
		
		char c = getchar();
		if( c == 'z' ) LoadSRecord();
	}
}
