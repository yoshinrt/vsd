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
#include <ST\iostm32f10x.h>
#include "stm32f10x_lib.h"
#include "usb_lib.h"
#include "usb_desc.h"
#include "usb_regs.h"
#include "hw_config.h"
#include <string.h>
#include <stdio.h>

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/

#define EOL		"\n"

/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Extern variables ----------------------------------------------------------*/
extern u32 count_out;
extern u32 count_in;
extern u8 buffer_out[VIRTUAL_COM_PORT_DATA_SIZE];

/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

/*** USB com port puts ********************************************************/

void USBComPuts( char* buf ){
	while( GetEPTxStatus( ENDP1 ) == EP_TX_VALID );
	count_in = strlen( buf );
	UserToPMABufferCopy(( u8 * )buf, ENDP1_TXADDR, count_in );
	SetEPTxCount( ENDP1, count_in );
	SetEPTxValid( ENDP1 );
}

/*******************************************************************************
* Function Name  : DFU_Button_Config.
* Description    : Configures the DFU selector Button to enter DFU Mode.
* Input          : None.
* Output         : None.
* Return         : None.
*******************************************************************************/
void DFU_Button_Config(void){
  GPIO_InitTypeDef GPIO_InitStructure;
  
  /* Enable GPIOB clock */
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
  
  /* Configure PD.09 as input floating (Key push-button on Eval Board) */
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
  GPIO_Init(GPIOB, &GPIO_InitStructure);
}

/*******************************************************************************
* Function Name  : DFU_Button_Read.
* Description    : Reads the DFU selector Button to enter DFU Mode.
* Input          : None.
* Output         : None.
* Return         : Status
*******************************************************************************/
u8 DFU_Button_Read (void){
	return GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_9);
}

/*******************************************************************************
* Function Name  : main.
* Description    : Main routine.
* Input          : None.
* Output         : None.
* Return         : None.
*******************************************************************************/

void timer(unsigned long i){
	while(i--) ;
}

int main( void ){
	u32		uCnt;
	char	szBuf[ 256 ];
	char	Data_Buffer[ 512 ];
	int		i;
	
#ifdef DEBUG
	debug();
#endif
	
	USBComPuts( "init system" EOL );
	MSD_Init();
	
#ifdef EXEC_SRAM
	unsigned long t;
	
	RCC_APB2ENR |= 0x10;     // CPIOC���g�p�ł���悤�ɂ���B
	GPIOC_CRL = 0x43444444;   // PC6���o�͂ɂ���B�@�@
	
	for( i = 0; i < 512; ++i ) Data_Buffer[ i ] = 0xAA;
	
	MSD_ReadBlock(Data_Buffer, 0, 512);
	
	for( i = 0; i < 512; ++i ){
		if(( i & 0xF ) == 0 ){
			sprintf( szBuf, EOL "\n%04X:", i );
			USBComPuts( szBuf );
		}
		sprintf( szBuf, "%02X ", Data_Buffer[ i ] );
		USBComPuts( szBuf );
	}
	
	while(1){
		for(t=0; t < 0x1000; t++){
			timer(100);
		}
		
		GPIOC_ODR ^= 0x40;    // LED�̏o�͂𔽓]������B
	}
	
#else
	Set_System();
	Set_USBClock();
	USB_Interrupts_Config();
	USB_Init();
	DFU_Butn_Config();
	
	USBComPuts( "VSD2 - Vehicle Status Datalogger 2" EOL );
	
	/* Test if PB.09 level is low (Key push-button on STM3210B-EVAL Board pressed) */
	
	for( uCnt = 0; uCnt < 0x4800; ){
		if( count_out != 0 ){
			memcpy(( void *)( 0x20000000 + uCnt ), buffer_out, count_out );
			uCnt += count_out;
			count_out = 0;
			
			if( DFU_Button_Read() == 0x00 ){
				sprintf( szBuf, ":%08X\r" , 0x20000000 + uCnt );
				USBComPuts( szBuf );
			}
		}
	}
	
	sprintf( szBuf, EOL "starting %X..." EOL, *( u32 *)0x20000004 );
	USBComPuts( szBuf );
	( **( void ( ** )( void ))0x20000004 )();
#endif
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
		 ex: printf( "Wrong parameters value: file %s on line %d\r\n", file, line ) */
	
	/* Infinite loop */
	while( 1 );
}
#endif

/******************* ( C ) COPYRIGHT 2007 STMicroelectronics *****END OF FILE****/