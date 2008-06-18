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
#include <string.h>
#include <stdio.h>
#include "stm32f10x_lib.h"
#include "usb_lib.h"
#include "usb_desc.h"
#include "usb_regs.h"
#include "hw_config.h"
#include "msd.h"
#include "ff.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
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

__INTRINSIC putchar( int c ){
	if( c == '\n' ) putchar( '\r' );
	while( GetEPTxStatus( ENDP1 ) == EP_TX_VALID );
	UserToPMABufferCopy(( UCHAR *)&c, ENDP1_TXADDR, 1 );
	SetEPTxCount( ENDP1, 1 );
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

void dump( char *p ){
	int i;
	
	for( i = 0; i < 512; ++i ){
		if(( i & 0xF ) == 0 ){
			printf( "\n%04X:", i );
		}
		printf( "%02X ", p[ i ] );
	}
	printf( "\n" );
}

__noreturn void JumpTo( u32 uJmpAddr, u32 uSP ){
	asm( "MSR MSP, r1\nBX r0\n" );
}

__noreturn int main( void ){
	u32		uCnt;
	char	buf[ 512 ];
	int		i;
	
#ifdef DEBUG
	debug();
#endif
	
	printf( "init system\n" );
	MSD_Init();
	
#ifdef EXEC_SRAM
	unsigned long t;
	sMSD_CSD MSD_csd;
	int	res;
	
	RCC_APB2ENR |= 0x10;     // CPIOCを使用できるようにする。
	GPIOC_CRL = 0x43444444;   // PC6を出力にする。　　
	
	res = MSD_GetCSDRegister( &MSD_csd );
	
#if 0
	printf( "%04X:return code\n", res );
	printf( "%04X:CSD structure\n", MSD_csd.CSDStruct );
	printf( "%04X:System specification version\n", MSD_csd.SysSpecVersion );
	printf( "%04X:Reserved\n", MSD_csd.Reserved1 );
	printf( "%04X:Data read access-time 1\n", MSD_csd.TAAC );
	printf( "%04X:Data read access-time 2 in CLK cycles\n", MSD_csd.NSAC );
	printf( "%04X:Max. bus clock frequency\n", MSD_csd.MaxBusClkFrec );
	printf( "%04X:Card command classes\n", MSD_csd.CardComdClasses );
	printf( "%04X:Max. read data block length\n", MSD_csd.RdBlockLen );
	printf( "%04X:Partial blocks for read allowed\n", MSD_csd.PartBlockRead );
	printf( "%04X:Write block misalignment\n", MSD_csd.WrBlockMisalign );
	printf( "%04X:Read block misalignment\n", MSD_csd.RdBlockMisalign );
	printf( "%04X:DSR implemented\n", MSD_csd.DSRImpl );
	printf( "%04X:Reserved\n", MSD_csd.Reserved2 );
	printf( "%04X:Device Size\n", MSD_csd.DeviceSize );
	printf( "%04X:Max. read current @ VDD min\n", MSD_csd.MaxRdCurrentVDDMin );
	printf( "%04X:Max. read current @ VDD max\n", MSD_csd.MaxRdCurrentVDDMax );
	printf( "%04X:Max. write current @ VDD min\n", MSD_csd.MaxWrCurrentVDDMin );
	printf( "%04X:Max. write current @ VDD max\n", MSD_csd.MaxWrCurrentVDDMax );
	printf( "%04X:Device size multiplier\n", MSD_csd.DeviceSizeMul );
	printf( "%04X:Erase group size\n", MSD_csd.EraseGrSize );
	printf( "%04X:Erase group size multiplier\n", MSD_csd.EraseGrMul );
	printf( "%04X:Write protect group size\n", MSD_csd.WrProtectGrSize );
	printf( "%04X:Write protect group enable\n", MSD_csd.WrProtectGrEnable );
	printf( "%04X:Manufacturer default ECC\n", MSD_csd.ManDeflECC );
	printf( "%04X:Write speed factor\n", MSD_csd.WrSpeedFact );
	printf( "%04X:Max. write data block length\n", MSD_csd.MaxWrBlockLen );
	printf( "%04X:Partial blocks for write allowed\n", MSD_csd.WriteBlockPaPartial );
	printf( "%04X:Reserded\n", MSD_csd.Reserved3 );
	printf( "%04X:Content protection application\n", MSD_csd.ContentProtectAppli );
	printf( "%04X:File format group\n", MSD_csd.FileFormatGrouop );
	printf( "%04X:Copy flag (OTP)\n", MSD_csd.CopyFlag );
	printf( "%04X:Permanent write protection\n", MSD_csd.PermWrProtect );
	printf( "%04X:Temporary write protection\n", MSD_csd.TempWrProtect );
	printf( "%04X:File Format\n", MSD_csd.FileFormat );
	printf( "%04X:ECC code\n", MSD_csd.ECC );
	printf( "%04X:CRC\n", MSD_csd.CRC );
	printf( "%04X:always 1\n", MSD_csd.Reserved4 );
#endif

#if 1
	FIL	fp;
	FATFS fatfs;				/* File system object */
	
	res = f_mount( 0, &fatfs );
	printf( "f_mount:%d\n", res );
	
	res = f_open( &fp, "HOGE.TXT", FA_READ );
	printf( "f_open:%d\n", res );
	
	for( i = 0; i < 512; ++i ) buf[ i ] = 0xAA;
	
    UINT u;
	res = f_read( &fp, buf, 512, &u );
	printf( "f_read:%d: read=%d\n", res, u );
	
	res = f_close( &fp );
	printf( "f_close:%d\n", res );
	
	dump( buf );
	
#endif
	
	while(1){
		for(t=0; t < 0x1000; t++){
			timer(100);
		}
		
		GPIOC_ODR ^= 0x40;    // LEDの出力を反転させる。
	}
	
#else
	Set_System();
	Set_USBClock();
	USB_Interrupts_Config();
	USB_Init();
	DFU_Butn_Config();
	
	printf( "VSD2 - Vehicle Status Datalogger 2\n" );
	
	/* Test if PB.09 level is low (Key push-button on STM3210B-EVAL Board pressed) */
	
	for( uCnt = 0; uCnt < 0x4800; ){
		if( count_out != 0 ){
			memcpy(( void *)( 0x20000000 + uCnt ), buffer_out, count_out );
			uCnt += count_out;
			count_out = 0;
			
			if( DFU_Button_Read() == 0x00 ){
				printf( ":%08X\n" , 0x20000000 + uCnt );
				printf( szBuf );
			}
		}
	}
	
	printf( "\nstarting %X...\n", *( u32 *)0x20000004 );
	printf( szBuf );
	
	JumpTo( *( u32 *)0x20000004, *( u32 *)0x08003000 );
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
		 ex: printf( "Wrong parameters value: file %s on line %d\n\n", file, line ) */
	
	/* Infinite loop */
	while( 1 );
}
#endif

/******************* ( C ) COPYRIGHT 2007 STMicroelectronics *****END OF FILE****/
