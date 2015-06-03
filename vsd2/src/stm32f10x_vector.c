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
#include "stm32f10x_it.h"
#include "main2.h"

/* Private typedef -----------------------------------------------------------*/
typedef void( *intfunc )( void );
typedef union { intfunc __fun; void * __ptr; } intvec_elem;

/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/


#pragma language=extended
#pragma segment="CSTACK"

void __iar_program_start( void );

#pragma location = ".intvec"
/* STM32F10x Vector Table entries */
const intvec_elem __vector_table[] =
{
	{ .__ptr = __sfe( "CSTACK" ) },
	__iar_program_start,
	
	#define DEF_HANDLER( func )			func,
	#define DEF_HANDLER_NOP( func )		IntHandlerNop,
	#define DEF_HANDLER_LOOP( func )	IntHandlerLoop,
	#define DEF_HANDLER_RESERVED()		0,
	#include "def_int_handler.h"
};

/******************* (C) COPYRIGHT 2007 STMicroelectronics *****END OF FILE****/

