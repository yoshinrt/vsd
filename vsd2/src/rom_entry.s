;*****************************************************************************
;	
;	VSD2 - vehicle data logger system2
;	Copyright(C) by DDS
;	
;	rom_entry.s -- rom entry point table
;	
;*****************************************************************************

	RSEG FLASH_CODE:DATA(1)
	DS8	0x1

	PUBWEAK	_PrintfFull
_PrintfFull:	// 0x080030ed @ xprintffull.o
	DS8	0x58C

	PUBWEAK	LoadInt
LoadInt:	// 0x08003679 @ xprintffull.o
	DS8	0x6E

	PUBWEAK	_LitobFull
_LitobFull:	// 0x080036e7 @ xprintffull.o
	DS8	0x106

	PUBWEAK	_LdtobFull
_LdtobFull:	// 0x080037ed @ xprintffull.o
	DS8	0x402

	PUBWEAK	_GenldFull
_GenldFull:	// 0x08003bef @ xprintffull.o
	DS8	0x264

	PUBWEAK	_PutcharsFull
_PutcharsFull:	// 0x08003e53 @ xprintffull.o
	DS8	0x48

	PUBWEAK	__iar_zero_init3
__iar_zero_init3:	// 0x08003e9b @ zero_init3.o
	DS8	0x22

	PUBWEAK	__iar_Strchr
__iar_Strchr:	// 0x08003ebd @ strchr.o
	DS8	0x0

	PUBWEAK	strchr
strchr:	// 0x08003ebd @ strchr.o
	DS8	0x16

	PUBWEAK	_SProut
_SProut:	// 0x08003ed3 @ xsprout.o
	DS8	0xA

	PUBWEAK	strlen
strlen:	// 0x08003edd @ strlen.o
	DS8	0x38

	PUBWEAK	__iar_Memchr
__iar_Memchr:	// 0x08003f15 @ memchr.o
	DS8	0x0

	PUBWEAK	memchr
memchr:	// 0x08003f15 @ memchr.o
	DS8	0x58

	PUBWEAK	__aeabi_uldivmod
__aeabi_uldivmod:	// 0x08003f6d @ I64DivMod.o
	DS8	0xF0

	PUBWEAK	__iar_unaligned___aeabi_memcpy
__iar_unaligned___aeabi_memcpy:	// 0x0800405d @ ABImemcpy_unaligned.o
	DS8	0x20

	PUBWEAK	__iar_unaligned___aeabi_memcpy4
__iar_unaligned___aeabi_memcpy4:	// 0x0800407d @ ABImemcpy_unaligned.o
	DS8	0x0

	PUBWEAK	__iar_unaligned___aeabi_memcpy8
__iar_unaligned___aeabi_memcpy8:	// 0x0800407d @ ABImemcpy_unaligned.o
	DS8	0x56

	PUBWEAK	__iar_Dnorm
__iar_Dnorm:	// 0x080040d3 @ xdnorm.o
	DS8	0x42

	PUBWEAK	__aeabi_cdcmple
__aeabi_cdcmple:	// 0x08004115 @ DblCmpLe.o
	DS8	0x30

	PUBWEAK	__aeabi_cdrcmple
__aeabi_cdrcmple:	// 0x08004145 @ DblCmpGe.o
	DS8	0x30

	PUBWEAK	__iar_Dscale
__iar_Dscale:	// 0x08004175 @ xdscale.o
	DS8	0x110

	PUBWEAK	__aeabi_d2iz
__aeabi_d2iz:	// 0x08004285 @ DblToI32.o
	DS8	0x2C

	PUBWEAK	__aeabi_d2uiz
__aeabi_d2uiz:	// 0x080042b1 @ DblToI32.o
	DS8	0xC

	PUBWEAK	__iar_d2uiz
__iar_d2uiz:	// 0x080042bd @ DblToI32.o
	DS8	0x20

	PUBWEAK	__aeabi_i2d
__aeabi_i2d:	// 0x080042dd @ I32ToDbl.o
	DS8	0x14

	PUBWEAK	__aeabi_ui2d
__aeabi_ui2d:	// 0x080042f1 @ I32ToDbl.o
	DS8	0x4

	PUBWEAK	__iar_ui2d
__iar_ui2d:	// 0x080042f5 @ I32ToDbl.o
	DS8	0x18

	PUBWEAK	__aeabi_dsub
__aeabi_dsub:	// 0x0800430d @ DblSub.o
	DS8	0x14

	PUBWEAK	__iar_dsub
__iar_dsub:	// 0x08004321 @ DblSub.o
	DS8	0x12

	PUBWEAK	__iar_dsubStart
__iar_dsubStart:	// 0x08004333 @ DblSub.o
	DS8	0x15A

	PUBWEAK	__aeabi_dmul
__aeabi_dmul:	// 0x0800448d @ DblMul.o
	DS8	0x1A4

	PUBWEAK	__aeabi_ddiv
__aeabi_ddiv:	// 0x08004631 @ DblDiv.o
	DS8	0x256

	PUBWEAK	div
div:	// 0x08004887 @ div.o
	DS8	0xE

	PUBWEAK	__aeabi_ldiv0
__aeabi_ldiv0:	// 0x08004895 @ I64DivZer.o
	DS8	0x4

	PUBWEAK	__iar_dadd
__iar_dadd:	// 0x08004899 @ DblAdd.o
	DS8	0xD4

	PUBWEAK	USART_DeInit
USART_DeInit:	// 0x0800496d @ stm32f10x_usart.o
	DS8	0x58

	PUBWEAK	USART_Init
USART_Init:	// 0x080049c5 @ stm32f10x_usart.o
	DS8	0x98

	PUBWEAK	USART_StructInit
USART_StructInit:	// 0x08004a5d @ stm32f10x_usart.o
	DS8	0x20

	PUBWEAK	USART_Cmd
USART_Cmd:	// 0x08004a7d @ stm32f10x_usart.o
	DS8	0x14

	PUBWEAK	USART_ITConfig
USART_ITConfig:	// 0x08004a91 @ stm32f10x_usart.o
	DS8	0x30

	PUBWEAK	USART_SendData
USART_SendData:	// 0x08004ac1 @ stm32f10x_usart.o
	DS8	0x8

	PUBWEAK	USART_ReceiveData
USART_ReceiveData:	// 0x08004ac9 @ stm32f10x_usart.o
	DS8	0x8

	PUBWEAK	RCC_GetClocksFreq
RCC_GetClocksFreq:	// 0x08004ad1 @ stm32f10x_rcc.o
	DS8	0x7A

	PUBWEAK	RCC_APB2PeriphClockCmd
RCC_APB2PeriphClockCmd:	// 0x08004b4b @ stm32f10x_rcc.o
	DS8	0x12

	PUBWEAK	RCC_APB2PeriphResetCmd
RCC_APB2PeriphResetCmd:	// 0x08004b5d @ stm32f10x_rcc.o
	DS8	0x12

	PUBWEAK	RCC_APB1PeriphResetCmd
RCC_APB1PeriphResetCmd:	// 0x08004b6f @ stm32f10x_rcc.o
	DS8	0x29

	PUBWEAK	APBAHBPrescTable
APBAHBPrescTable:	// 0x08004b98 @ stm32f10x_rcc.o
	DS8	0x10

	PUBWEAK	ADCPrescTable
ADCPrescTable:	// 0x08004ba8 @ stm32f10x_rcc.o
	DS8	0x5

	PUBWEAK	UsartInit
UsartInit:	// 0x08004bad @ usart.o
	DS8	0x7E

	PUBWEAK	USART1_IRQHandler
USART1_IRQHandler:	// 0x08004c2b @ usart.o
	DS8	0x64

	PUBWEAK	putchar
putchar:	// 0x08004c8f @ usart.o
	DS8	0x16

	PUBWEAK	getchar
getchar:	// 0x08004ca5 @ usart.o
	DS8	0x20

	PUBWEAK	GetcharWait
GetcharWait:	// 0x08004cc5 @ usart.o
	DS8	0x10

	PUBWEAK	GPIO_Init
GPIO_Init:	// 0x08004cd5 @ stm32f10x_gpio.o
	DS8	0xB8

	PUBWEAK	GPIO_StructInit
GPIO_StructInit:	// 0x08004d8d @ stm32f10x_gpio.o
	DS8	0x10

	PUBWEAK	GetHex
GetHex:	// 0x08004d9d @ main.o
	DS8	0x24

	PUBWEAK	JumpTo
JumpTo:	// 0x08004dc1 @ main.o
	DS8	0x8

	PUBWEAK	LoadSRecord
LoadSRecord:	// 0x08004dc9 @ main.o
	DS8	0xBC

	PUBWEAK	printf
printf:	// 0x08004e85 @ printf.o
	DS8	0x24

	PUBWEAK	__aeabi_memclr4
__aeabi_memclr4:	// 0x08004ea9 @ ABImemclr4.o
	DS8	0x8

	PUBWEAK	__iar_Memset4_word
__iar_Memset4_word:	// 0x08004eb1 @ ABImemset48.o
	DS8	0x0

	PUBWEAK	__iar_Memset8_word
__iar_Memset8_word:	// 0x08004eb1 @ ABImemset48.o
	DS8	0x34

	PUBWEAK	__WFI
__WFI:	// 0x08004ee5 @ cortexm3_macro.o
	DS8	0x4

	PUBWEAK	__WFE
__WFE:	// 0x08004ee9 @ cortexm3_macro.o
	DS8	0x4

	PUBWEAK	__SEV
__SEV:	// 0x08004eed @ cortexm3_macro.o
	DS8	0x4

	PUBWEAK	__ISB
__ISB:	// 0x08004ef1 @ cortexm3_macro.o
	DS8	0x6

	PUBWEAK	__DSB
__DSB:	// 0x08004ef7 @ cortexm3_macro.o
	DS8	0x6

	PUBWEAK	__DMB
__DMB:	// 0x08004efd @ cortexm3_macro.o
	DS8	0x6

	PUBWEAK	__SVC
__SVC:	// 0x08004f03 @ cortexm3_macro.o
	DS8	0x4

	PUBWEAK	__MRS_CONTROL
__MRS_CONTROL:	// 0x08004f07 @ cortexm3_macro.o
	DS8	0x6

	PUBWEAK	__MSR_CONTROL
__MSR_CONTROL:	// 0x08004f0d @ cortexm3_macro.o
	DS8	0xA

	PUBWEAK	__MRS_PSP
__MRS_PSP:	// 0x08004f17 @ cortexm3_macro.o
	DS8	0x6

	PUBWEAK	__MSR_PSP
__MSR_PSP:	// 0x08004f1d @ cortexm3_macro.o
	DS8	0x6

	PUBWEAK	__MRS_MSP
__MRS_MSP:	// 0x08004f23 @ cortexm3_macro.o
	DS8	0x6

	PUBWEAK	__MSR_MSP
__MSR_MSP:	// 0x08004f29 @ cortexm3_macro.o
	DS8	0x6

	PUBWEAK	__SETPRIMASK
__SETPRIMASK:	// 0x08004f2f @ cortexm3_macro.o
	DS8	0x4

	PUBWEAK	__RESETPRIMASK
__RESETPRIMASK:	// 0x08004f33 @ cortexm3_macro.o
	DS8	0x4

	PUBWEAK	__SETFAULTMASK
__SETFAULTMASK:	// 0x08004f37 @ cortexm3_macro.o
	DS8	0x4

	PUBWEAK	__RESETFAULTMASK
__RESETFAULTMASK:	// 0x08004f3b @ cortexm3_macro.o
	DS8	0x4

	PUBWEAK	__BASEPRICONFIG
__BASEPRICONFIG:	// 0x08004f3f @ cortexm3_macro.o
	DS8	0x6

	PUBWEAK	__GetBASEPRI
__GetBASEPRI:	// 0x08004f45 @ cortexm3_macro.o
	DS8	0x6

	PUBWEAK	__REV_HalfWord
__REV_HalfWord:	// 0x08004f4b @ cortexm3_macro.o
	DS8	0x4

	PUBWEAK	__REV_Word
__REV_Word:	// 0x08004f4f @ cortexm3_macro.o
	DS8	0x6

	PUBWEAK	sprintf
sprintf:	// 0x08004f55 @ sprintf.o
	DS8	0x86

	PUBWEAK	exit
exit:	// 0x08004fdb @ exit.o
	DS8	0x12

	PUBWEAK	__exit
__exit:	// 0x08004fed @ exit.o
	DS8	0x14

	PUBWEAK	_Prout
_Prout:	// 0x08005001 @ xprout.o
	DS8	0x22

	PUBWEAK	IntHandlerNop
IntHandlerNop:	// 0x08005023 @ stm32f10x_it.o
	DS8	0x2

	PUBWEAK	IntHandlerLoop
IntHandlerLoop:	// 0x08005025 @ stm32f10x_it.o

	RSEG FLASH_DATA:DATA(1)
	DS8	0x0

	PUBWEAK	g_pUsartBuf
g_pUsartBuf:	// 0x20004ffc @ usart.o
	DS8	0x2000C004

	PUBWEAK	_A_GPIOC_CRL
_A_GPIOC_CRL:	// 0x40011000 @ main.o
	DS8	0xC

	PUBWEAK	_A_GPIOC_ODR
_A_GPIOC_ODR:	// 0x4001100c @ main.o
	DS8	0x1000C

	PUBWEAK	_A_RCC_APB2ENR
_A_RCC_APB2ENR:	// 0x40021018 @ main.o
	END
