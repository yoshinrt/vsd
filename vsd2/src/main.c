/******************** ( C ) COPYRIGHT 2007 STMicroelectronics ********************
* File Name          : main.c
* Author             : MCD Application Team
* Version            : V1.0
* Date               : 10/08/2007
* Description        : Virtual Com Port Demo main file
********************************************************************************
* THE PRESENT SOFTWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
* WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE TIME.
* AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY DIRECT,
* INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING FROM THE
* CONTENT OF SUCH SOFTWARE AND/OR THE USE MADE BY CUSTOMERS OF THE CODING
* INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
*******************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include <stdio.h>
#include <ST\iostm32f10xxB.h>
#include "stm32f10x_nvic.h"
#include "dds.h"
#include "usart.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Extern variables ----------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

void PutHex( UINT uNum ){
	UINT u;
	for( u = 0; u < 8; ++u ){
		UINT n = uNum >> 28;
		
		if( n > 9 ){
			UsartPutchar( n + ( 'A' - 10 ));
		}else{
			UsartPutchar( n + '0' );
		}
		uNum <<= 4;
	}
}

void timer( unsigned long i ){
	while( i-- );
}

__noreturn void main( void ){
	// ベクタテーブル再設定
	NVIC_SetVectorTable( NVIC_VectTab_RAM, 0 );
	
	UsartInit( 38400 );
	
	RCC_APB2ENR |= 0x10;     // CPIOCを使用できるようにする。
	GPIOC_CRL = 0x43444444;   // PC6を出力にする。　　
	GPIOC_ODR ^= 0x40;    // LEDの出力を反転させる。
	
	while(1){
		for(int t=0; t < 0x1000; t++){
			timer(1000);
		}
		
		UsartPutstr( "hgoefuga\n" );
		//printf( "hgoefuga\n" );
		GPIOC_ODR ^= 0x40;    // LEDの出力を反転させる。
		//GPIOA_ODR ^= ( 1 << 9 );    // LEDの出力を反転させる。
	}
}

#ifdef  DEBUG
/*******************************************************************************
* Function Name  : assert_failed
* Description    : Reports the name of the source file and the source line number
*                  where the assert_param error has occurred.
* Input          : - file: pointer to the source file name
*                  - line: assert_param error line source number
* Output         : None
* Return         : None
*******************************************************************************/

void assert_failed( u8* file, u32 line ){
	/* User can add his own implementation to report the file name and line number,
		 ex: printf( "Wrong parameters value: file %s on line %d\n\n", file, line ) */
	
	/* Infinite loop */
	while( 1 );
}
#endif
