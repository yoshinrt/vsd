/******************** (C) COPYRIGHT 2007 STMicroelectronics ********************
* File Name          : stm32f10x_vector.c
* Author             : MCD Application Team
* Version            : V1.0
* Date               : 10/08/2007
* Description        : This file contains the vector table for STM32F10x.
*                      After Reset the Cortex-M3 processor is in Thread mode,
*                      priority is Privileged, and the Stack is set to Main.
********************************************************************************
* THE PRESENT SOFTWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
* WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE TIME.
* AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY DIRECT,
* INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING FROM THE
* CONTENT OF SUCH SOFTWARE AND/OR THE USE MADE BY CUSTOMERS OF THE CODING
* INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
*******************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include "dds.h"
#include "stm32f10x_lib.h"
#include "stm32f10x_nvic.h"
#include "main.h"

/* Private typedef -----------------------------------------------------------*/
typedef void( *intfunc )( void );
typedef union { intfunc __fun; void * __ptr; } intvec_elem;

/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/

#define DEF_HANDLER( func ) void func( void );
#include "def_int_handler.h"

/* Private functions ---------------------------------------------------------*/

#ifndef EXEC_SRAM
void IntHandlerNop( void ){}
void IntHandlerReset( void ){
	NVIC_GenerateSystemReset();
}
#endif

#pragma language=extended
#pragma segment="CSTACK"

void __iar_program_start( void );

/* STM32F10x Vector Table entries */
#pragma location = ".intvec"
#ifdef EXEC_SRAM
// エントリアドレスのみが有効なダミーのベクタテーブル
const intvec_elem __vector_table[] =
{
	__iar_program_start,
	__iar_program_start,
};
#else
const intvec_elem __vector_table[] =
{
	{ .__ptr = __sfe( "CSTACK" ) },
	__iar_program_start,
	
	#define DEF_HANDLER( func )			func,
	#define DEF_HANDLER_NOP( func )		IntHandlerReset, //IntHandlerNop,
	#define DEF_HANDLER_LOOP( func )	IntHandlerReset,
	#define DEF_HANDLER_RESERVED()		0,
	#include "def_int_handler.h"
};
#endif

/******************* (C) COPYRIGHT 2007 STMicroelectronics *****END OF FILE****/

