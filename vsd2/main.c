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
	char	buf[ 256 ];
	int		i;
	
#ifdef DEBUG
	debug();
#endif
	
	USBComPuts( "init system" EOL );
	MSD_Init();
	
#ifdef EXEC_SRAM
	unsigned long t;
	sMSD_CSD MSD_csd;
	int	res;
	
	RCC_APB2ENR |= 0x10;     // CPIOCを使用できるようにする。
	GPIOC_CRL = 0x43444444;   // PC6を出力にする。　　
	
	res = MSD_GetCSDRegister( &MSD_csd );
	
	sprintf( szBuf, "%04X:return code\n", res ); USBComPuts( szBuf );
	sprintf( szBuf, "%04X:CSD structure\n", MSD_csd.CSDStruct ); USBComPuts( szBuf );
	sprintf( szBuf, "%04X:System specification version\n", MSD_csd.SysSpecVersion ); USBComPuts( szBuf );
	sprintf( szBuf, "%04X:Reserved\n", MSD_csd.Reserved1 ); USBComPuts( szBuf );
	sprintf( szBuf, "%04X:Data read access-time 1\n", MSD_csd.TAAC ); USBComPuts( szBuf );
	sprintf( szBuf, "%04X:Data read access-time 2 in CLK cycles\n", MSD_csd.NSAC ); USBComPuts( szBuf );
	sprintf( szBuf, "%04X:Max. bus clock frequency\n", MSD_csd.MaxBusClkFrec ); USBComPuts( szBuf );
	sprintf( szBuf, "%04X:Card command classes\n", MSD_csd.CardComdClasses ); USBComPuts( szBuf );
	sprintf( szBuf, "%04X:Max. read data block length\n", MSD_csd.RdBlockLen ); USBComPuts( szBuf );
	sprintf( szBuf, "%04X:Partial blocks for read allowed\n", MSD_csd.PartBlockRead ); USBComPuts( szBuf );
	sprintf( szBuf, "%04X:Write block misalignment\n", MSD_csd.WrBlockMisalign ); USBComPuts( szBuf );
	sprintf( szBuf, "%04X:Read block misalignment\n", MSD_csd.RdBlockMisalign ); USBComPuts( szBuf );
	sprintf( szBuf, "%04X:DSR implemented\n", MSD_csd.DSRImpl ); USBComPuts( szBuf );
	sprintf( szBuf, "%04X:Reserved\n", MSD_csd.Reserved2 ); USBComPuts( szBuf );
	sprintf( szBuf, "%04X:Device Size\n", MSD_csd.DeviceSize ); USBComPuts( szBuf );
	sprintf( szBuf, "%04X:Max. read current @ VDD min\n", MSD_csd.MaxRdCurrentVDDMin ); USBComPuts( szBuf );
	sprintf( szBuf, "%04X:Max. read current @ VDD max\n", MSD_csd.MaxRdCurrentVDDMax ); USBComPuts( szBuf );
	sprintf( szBuf, "%04X:Max. write current @ VDD min\n", MSD_csd.MaxWrCurrentVDDMin ); USBComPuts( szBuf );
	sprintf( szBuf, "%04X:Max. write current @ VDD max\n", MSD_csd.MaxWrCurrentVDDMax ); USBComPuts( szBuf );
	sprintf( szBuf, "%04X:Device size multiplier\n", MSD_csd.DeviceSizeMul ); USBComPuts( szBuf );
	sprintf( szBuf, "%04X:Erase group size\n", MSD_csd.EraseGrSize ); USBComPuts( szBuf );
	sprintf( szBuf, "%04X:Erase group size multiplier\n", MSD_csd.EraseGrMul ); USBComPuts( szBuf );
	sprintf( szBuf, "%04X:Write protect group size\n", MSD_csd.WrProtectGrSize ); USBComPuts( szBuf );
	sprintf( szBuf, "%04X:Write protect group enable\n", MSD_csd.WrProtectGrEnable ); USBComPuts( szBuf );
	sprintf( szBuf, "%04X:Manufacturer default ECC\n", MSD_csd.ManDeflECC ); USBComPuts( szBuf );
	sprintf( szBuf, "%04X:Write speed factor\n", MSD_csd.WrSpeedFact ); USBComPuts( szBuf );
	sprintf( szBuf, "%04X:Max. write data block length\n", MSD_csd.MaxWrBlockLen ); USBComPuts( szBuf );
	sprintf( szBuf, "%04X:Partial blocks for write allowed\n", MSD_csd.WriteBlockPaPartial ); USBComPuts( szBuf );
	sprintf( szBuf, "%04X:Reserded\n", MSD_csd.Reserved3 ); USBComPuts( szBuf );
	sprintf( szBuf, "%04X:Content protection application\n", MSD_csd.ContentProtectAppli ); USBComPuts( szBuf );
	sprintf( szBuf, "%04X:File format group\n", MSD_csd.FileFormatGrouop ); USBComPuts( szBuf );
	sprintf( szBuf, "%04X:Copy flag (OTP)\n", MSD_csd.CopyFlag ); USBComPuts( szBuf );
	sprintf( szBuf, "%04X:Permanent write protection\n", MSD_csd.PermWrProtect ); USBComPuts( szBuf );
	sprintf( szBuf, "%04X:Temporary write protection\n", MSD_csd.TempWrProtect ); USBComPuts( szBuf );
	sprintf( szBuf, "%04X:File Format\n", MSD_csd.FileFormat ); USBComPuts( szBuf );
	sprintf( szBuf, "%04X:ECC code\n", MSD_csd.ECC ); USBComPuts( szBuf );
	sprintf( szBuf, "%04X:CRC\n", MSD_csd.CRC ); USBComPuts( szBuf );
	sprintf( szBuf, "%04X:always 1\n", MSD_csd.Reserved4 ); USBComPuts( szBuf );
	
#if 1
	FIL	fp;
	FATFS fatfs;				/* File system object */
	
	res = f_mount( 0, &fatfs );
	sprintf( szBuf, "f_mount:%d\n", res ); USBComPuts( szBuf );
	
	res = f_open( &fp, "hoge.txt", FA_READ );
	sprintf( szBuf, "f_open:%d\n", res ); USBComPuts( szBuf );
	
	for( i = 0; i < 512; ++i ) buf[ i ] = 0xAA;
	
	res = f_read( &fp, buf, 256, &i );
	sprintf( szBuf, "f_read:%d: read=%d\n", res, i ); USBComPuts( szBuf );
	
	res = f_close( &fp );
	sprintf( szBuf, "f_close:%d\n", res ); USBComPuts( szBuf );
	
	for( i = 0; i < 512; ++i ){
		if(( i & 0xF ) == 0 ){
			sprintf( szBuf, EOL "\n%04X:", i );
			USBComPuts( szBuf );
		}
		sprintf( szBuf, "%02X ", buf[ i ] );
		USBComPuts( szBuf );
	}
	
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
