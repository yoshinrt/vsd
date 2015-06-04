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

	PUBWEAK	NvicIntEnable
NvicIntEnable:	// 0x0800496d @ main2.o
	DS8	0xE

	PUBWEAK	NvicIntDisable
NvicIntDisable:	// 0x0800497b @ main2.o
	DS8	0x16

	PUBWEAK	JumpTo
JumpTo:	// 0x08004991 @ main2.o
	DS8	0x8

	PUBWEAK	GetHex
GetHex:	// 0x08004999 @ main2.o
	DS8	0x22

	PUBWEAK	LoadSRecordSub
LoadSRecordSub:	// 0x080049bb @ main2.o
	DS8	0x3C

	PUBWEAK	LoadSRecord
LoadSRecord:	// 0x080049f7 @ main2.o
	DS8	0x1A

	PUBWEAK	LoadBinSub
LoadBinSub:	// 0x08004a11 @ main2.o
	DS8	0x3C

	PUBWEAK	LoadBin
LoadBin:	// 0x08004a4d @ main2.o
	DS8	0x30

	PUBWEAK	TimerInit
TimerInit:	// 0x08004a7d @ main2.o
	DS8	0x4A

	PUBWEAK	GetCurrentTime
GetCurrentTime:	// 0x08004ac7 @ main2.o
	DS8	0x1A

	PUBWEAK	GetCurrentTime16
GetCurrentTime16:	// 0x08004ae1 @ main2.o
	DS8	0x8

	PUBWEAK	AdcInit
AdcInit:	// 0x08004ae9 @ main2.o
	DS8	0x10C

	PUBWEAK	PulseInit
PulseInit:	// 0x08004bf5 @ main2.o
	DS8	0xB2

	PUBWEAK	EXTI0_IRQHandler
EXTI0_IRQHandler:	// 0x08004ca7 @ main2.o
	DS8	0x4E

	PUBWEAK	EXTI1_IRQHandler
EXTI1_IRQHandler:	// 0x08004cf5 @ main2.o
	DS8	0x1E

	PUBWEAK	EXTI2_IRQHandler
EXTI2_IRQHandler:	// 0x08004d13 @ main2.o
	DS8	0x2C

	PUBWEAK	ComputeMeterTacho
ComputeMeterTacho:	// 0x08004d3f @ main2.o
	DS8	0x4A

	PUBWEAK	ComputeMeterSpeed
ComputeMeterSpeed:	// 0x08004d89 @ main2.o
	DS8	0x8E

	PUBWEAK	SdcInit
SdcInit:	// 0x08004e17 @ main2.o
	DS8	0x2E

	PUBWEAK	SdcInserted
SdcInserted:	// 0x08004e45 @ main2.o
	DS8	0x10

	PUBWEAK	SerialPack
SerialPack:	// 0x08004e55 @ main2.o
	DS8	0x2C

	PUBWEAK	OutputSerial
OutputSerial:	// 0x08004e81 @ main2.o
	DS8	0x54

	PUBWEAK	InputSerial
InputSerial:	// 0x08004ed5 @ main2.o
	DS8	0xBA

	PUBWEAK	CheckStartByGSensor
CheckStartByGSensor:	// 0x08004f8f @ main2.o
	DS8	0x60

	PUBWEAK	Calibration
Calibration:	// 0x08004fef @ main2.o
	DS8	0x20

	PUBWEAK	WaitStateChange
WaitStateChange:	// 0x0800500f @ main2.o
	DS8	0x14A

	PUBWEAK	UsartInit
UsartInit:	// 0x08005159 @ usart.o
	DS8	0x9E

	PUBWEAK	USART1_IRQHandler
USART1_IRQHandler:	// 0x080051f7 @ usart.o
	DS8	0x64

	PUBWEAK	UsartPutcharUnbuffered
UsartPutcharUnbuffered:	// 0x0800525b @ usart.o
	DS8	0x16

	PUBWEAK	UsartPutcharBuffered
UsartPutcharBuffered:	// 0x08005271 @ usart.o
	DS8	0x2E

	PUBWEAK	putchar
putchar:	// 0x0800529f @ usart.o
	DS8	0xE

	PUBWEAK	UsartGetcharUnbuffered
UsartGetcharUnbuffered:	// 0x080052ad @ usart.o
	DS8	0x12

	PUBWEAK	UsartGetcharBuffered
UsartGetcharBuffered:	// 0x080052bf @ usart.o
	DS8	0x1E

	PUBWEAK	getchar
getchar:	// 0x080052dd @ usart.o
	DS8	0x1C

	PUBWEAK	UsartGetcharWaitUnbuffered
UsartGetcharWaitUnbuffered:	// 0x080052f9 @ usart.o
	DS8	0xE

	PUBWEAK	UsartPutstrUnbuffered
UsartPutstrUnbuffered:	// 0x08005307 @ usart.o
	DS8	0x16

	PUBWEAK	RCC_DeInit
RCC_DeInit:	// 0x0800531d @ stm32f10x_rcc.o
	DS8	0x40

	PUBWEAK	RCC_HSEConfig
RCC_HSEConfig:	// 0x0800535d @ stm32f10x_rcc.o
	DS8	0x32

	PUBWEAK	RCC_WaitForHSEStartUp
RCC_WaitForHSEStartUp:	// 0x0800538f @ stm32f10x_rcc.o
	DS8	0x28

	PUBWEAK	RCC_PLLConfig
RCC_PLLConfig:	// 0x080053b7 @ stm32f10x_rcc.o
	DS8	0x10

	PUBWEAK	RCC_PLLCmd
RCC_PLLCmd:	// 0x080053c7 @ stm32f10x_rcc.o
	DS8	0x6

	PUBWEAK	RCC_SYSCLKConfig
RCC_SYSCLKConfig:	// 0x080053cd @ stm32f10x_rcc.o
	DS8	0xE

	PUBWEAK	RCC_GetSYSCLKSource
RCC_GetSYSCLKSource:	// 0x080053db @ stm32f10x_rcc.o
	DS8	0xA

	PUBWEAK	RCC_HCLKConfig
RCC_HCLKConfig:	// 0x080053e5 @ stm32f10x_rcc.o
	DS8	0xA

	PUBWEAK	RCC_PCLK1Config
RCC_PCLK1Config:	// 0x080053ef @ stm32f10x_rcc.o
	DS8	0xE

	PUBWEAK	RCC_PCLK2Config
RCC_PCLK2Config:	// 0x080053fd @ stm32f10x_rcc.o
	DS8	0x10

	PUBWEAK	RCC_ADCCLKConfig
RCC_ADCCLKConfig:	// 0x0800540d @ stm32f10x_rcc.o
	DS8	0xA

	PUBWEAK	RCC_GetClocksFreq
RCC_GetClocksFreq:	// 0x08005417 @ stm32f10x_rcc.o
	DS8	0x7A

	PUBWEAK	RCC_APB2PeriphClockCmd
RCC_APB2PeriphClockCmd:	// 0x08005491 @ stm32f10x_rcc.o
	DS8	0x12

	PUBWEAK	RCC_APB1PeriphClockCmd
RCC_APB1PeriphClockCmd:	// 0x080054a3 @ stm32f10x_rcc.o
	DS8	0x12

	PUBWEAK	RCC_APB2PeriphResetCmd
RCC_APB2PeriphResetCmd:	// 0x080054b5 @ stm32f10x_rcc.o
	DS8	0x12

	PUBWEAK	RCC_APB1PeriphResetCmd
RCC_APB1PeriphResetCmd:	// 0x080054c7 @ stm32f10x_rcc.o
	DS8	0x12

	PUBWEAK	RCC_GetFlagStatus
RCC_GetFlagStatus:	// 0x080054d9 @ stm32f10x_rcc.o
	DS8	0x53

	PUBWEAK	APBAHBPrescTable
APBAHBPrescTable:	// 0x0800552c @ stm32f10x_rcc.o
	DS8	0x10

	PUBWEAK	ADCPrescTable
ADCPrescTable:	// 0x0800553c @ stm32f10x_rcc.o
	DS8	0x5

	PUBWEAK	TIM_DeInit
TIM_DeInit:	// 0x08005541 @ stm32f10x_tim.o
	DS8	0x50

	PUBWEAK	GPIO_Init
GPIO_Init:	// 0x08005591 @ stm32f10x_gpio.o
	DS8	0xB8

	PUBWEAK	GPIO_StructInit
GPIO_StructInit:	// 0x08005649 @ stm32f10x_gpio.o
	DS8	0x10

	PUBWEAK	GPIO_EXTILineConfig
GPIO_EXTILineConfig:	// 0x08005659 @ stm32f10x_gpio.o
	DS8	0x28

	PUBWEAK	ADC_Init
ADC_Init:	// 0x08005681 @ stm32f10x_adc.o
	DS8	0x3C

	PUBWEAK	ADC_StructInit
ADC_StructInit:	// 0x080056bd @ stm32f10x_adc.o
	DS8	0x12

	PUBWEAK	ADC_Cmd
ADC_Cmd:	// 0x080056cf @ stm32f10x_adc.o
	DS8	0x12

	PUBWEAK	ADC_ResetCalibration
ADC_ResetCalibration:	// 0x080056e1 @ stm32f10x_adc.o
	DS8	0xA

	PUBWEAK	ADC_GetResetCalibrationStatus
ADC_GetResetCalibrationStatus:	// 0x080056eb @ stm32f10x_adc.o
	DS8	0x6

	PUBWEAK	ADC_StartCalibration
ADC_StartCalibration:	// 0x080056f1 @ stm32f10x_adc.o
	DS8	0xA

	PUBWEAK	ADC_GetCalibrationStatus
ADC_GetCalibrationStatus:	// 0x080056fb @ stm32f10x_adc.o
	DS8	0xA

	PUBWEAK	ADC_ExternalTrigInjectedConvConfig
ADC_ExternalTrigInjectedConvConfig:	// 0x08005705 @ stm32f10x_adc.o
	DS8	0xC

	PUBWEAK	ADC_SoftwareStartInjectedConvCmd
ADC_SoftwareStartInjectedConvCmd:	// 0x08005711 @ stm32f10x_adc.o
	DS8	0x12

	PUBWEAK	ADC_InjectedChannelConfig
ADC_InjectedChannelConfig:	// 0x08005723 @ stm32f10x_adc.o
	DS8	0x5E

	PUBWEAK	ADC_InjectedSequencerLengthConfig
ADC_InjectedSequencerLengthConfig:	// 0x08005781 @ stm32f10x_adc.o
	DS8	0x12

	PUBWEAK	ADC_GetInjectedConversionValue
ADC_GetInjectedConversionValue:	// 0x08005793 @ stm32f10x_adc.o
	DS8	0x12

	PUBWEAK	ADC_GetFlagStatus
ADC_GetFlagStatus:	// 0x080057a5 @ stm32f10x_adc.o
	DS8	0x10

	PUBWEAK	NVIC_PriorityGroupConfig
NVIC_PriorityGroupConfig:	// 0x080057b5 @ stm32f10x_nvic.o
	DS8	0xA

	PUBWEAK	NVIC_Init
NVIC_Init:	// 0x080057bf @ stm32f10x_nvic.o
	DS8	0x7E

	PUBWEAK	NVIC_SetVectorTable
NVIC_SetVectorTable:	// 0x0800583d @ stm32f10x_nvic.o
	DS8	0x28

	PUBWEAK	USART_DeInit
USART_DeInit:	// 0x08005865 @ stm32f10x_usart.o
	DS8	0x58

	PUBWEAK	USART_Init
USART_Init:	// 0x080058bd @ stm32f10x_usart.o
	DS8	0x98

	PUBWEAK	USART_StructInit
USART_StructInit:	// 0x08005955 @ stm32f10x_usart.o
	DS8	0x20

	PUBWEAK	USART_Cmd
USART_Cmd:	// 0x08005975 @ stm32f10x_usart.o
	DS8	0x14

	PUBWEAK	USART_ITConfig
USART_ITConfig:	// 0x08005989 @ stm32f10x_usart.o
	DS8	0x30

	PUBWEAK	USART_SendData
USART_SendData:	// 0x080059b9 @ stm32f10x_usart.o
	DS8	0x8

	PUBWEAK	USART_ReceiveData
USART_ReceiveData:	// 0x080059c1 @ stm32f10x_usart.o
	DS8	0xF0

	PUBWEAK	FLASH_SetLatency
FLASH_SetLatency:	// 0x08005ab1 @ stm32f10x_flash.o
	DS8	0x14

	PUBWEAK	FLASH_PrefetchBufferCmd
FLASH_PrefetchBufferCmd:	// 0x08005ac5 @ stm32f10x_flash.o
	DS8	0xBC

	PUBWEAK	__aeabi_memclr4
__aeabi_memclr4:	// 0x08005b81 @ ABImemclr4.o
	DS8	0x8

	PUBWEAK	__iar_Memset4_word
__iar_Memset4_word:	// 0x08005b89 @ ABImemset48.o
	DS8	0x0

	PUBWEAK	__iar_Memset8_word
__iar_Memset8_word:	// 0x08005b89 @ ABImemset48.o
	DS8	0x34

	PUBWEAK	__WFI
__WFI:	// 0x08005bbd @ cortexm3_macro.o
	DS8	0x4

	PUBWEAK	__WFE
__WFE:	// 0x08005bc1 @ cortexm3_macro.o
	DS8	0x4

	PUBWEAK	__SEV
__SEV:	// 0x08005bc5 @ cortexm3_macro.o
	DS8	0x4

	PUBWEAK	__ISB
__ISB:	// 0x08005bc9 @ cortexm3_macro.o
	DS8	0x6

	PUBWEAK	__DSB
__DSB:	// 0x08005bcf @ cortexm3_macro.o
	DS8	0x6

	PUBWEAK	__DMB
__DMB:	// 0x08005bd5 @ cortexm3_macro.o
	DS8	0x6

	PUBWEAK	__SVC
__SVC:	// 0x08005bdb @ cortexm3_macro.o
	DS8	0x4

	PUBWEAK	__MRS_CONTROL
__MRS_CONTROL:	// 0x08005bdf @ cortexm3_macro.o
	DS8	0x6

	PUBWEAK	__MSR_CONTROL
__MSR_CONTROL:	// 0x08005be5 @ cortexm3_macro.o
	DS8	0xA

	PUBWEAK	__MRS_PSP
__MRS_PSP:	// 0x08005bef @ cortexm3_macro.o
	DS8	0x6

	PUBWEAK	__MSR_PSP
__MSR_PSP:	// 0x08005bf5 @ cortexm3_macro.o
	DS8	0x6

	PUBWEAK	__MRS_MSP
__MRS_MSP:	// 0x08005bfb @ cortexm3_macro.o
	DS8	0x6

	PUBWEAK	__MSR_MSP
__MSR_MSP:	// 0x08005c01 @ cortexm3_macro.o
	DS8	0x6

	PUBWEAK	__SETPRIMASK
__SETPRIMASK:	// 0x08005c07 @ cortexm3_macro.o
	DS8	0x4

	PUBWEAK	__RESETPRIMASK
__RESETPRIMASK:	// 0x08005c0b @ cortexm3_macro.o
	DS8	0x4

	PUBWEAK	__SETFAULTMASK
__SETFAULTMASK:	// 0x08005c0f @ cortexm3_macro.o
	DS8	0x4

	PUBWEAK	__RESETFAULTMASK
__RESETFAULTMASK:	// 0x08005c13 @ cortexm3_macro.o
	DS8	0x4

	PUBWEAK	__BASEPRICONFIG
__BASEPRICONFIG:	// 0x08005c17 @ cortexm3_macro.o
	DS8	0x6

	PUBWEAK	__GetBASEPRI
__GetBASEPRI:	// 0x08005c1d @ cortexm3_macro.o
	DS8	0x6

	PUBWEAK	__REV_HalfWord
__REV_HalfWord:	// 0x08005c23 @ cortexm3_macro.o
	DS8	0x4

	PUBWEAK	__REV_Word
__REV_Word:	// 0x08005c27 @ cortexm3_macro.o
	DS8	0x6

	PUBWEAK	__iar_close_ttio
__iar_close_ttio:	// 0x08005c2d @ iarttio.o
	DS8	0x30

	PUBWEAK	__iar_get_ttio
__iar_get_ttio:	// 0x08005c5d @ iarttio.o
	DS8	0x3C

	PUBWEAK	__iar_lookup_ttioh
__iar_lookup_ttioh:	// 0x08005c99 @ XShttio.o
	DS8	0x8

	PUBWEAK	puts
puts:	// 0x08005ca1 @ puts.o
	DS8	0x36

	PUBWEAK	__write
__write:	// 0x08005cd7 @ write.o
	DS8	0x10

	PUBWEAK	__dwrite
__dwrite:	// 0x08005ce7 @ dwrite.o
	DS8	0x1E

	PUBWEAK	__iar_sh_stdout
__iar_sh_stdout:	// 0x08005d05 @ iarwstd.o
	DS8	0x20

	PUBWEAK	__iar_sh_write
__iar_sh_write:	// 0x08005d25 @ iarwrite.o
	DS8	0x24

	PUBWEAK	sprintf
sprintf:	// 0x08005d49 @ sprintf.o
	DS8	0x34

	PUBWEAK	__iar_copy_init3
__iar_copy_init3:	// 0x08005d7d @ copy_init3.o
	DS8	0x54

	PUBWEAK	printf
printf:	// 0x08005dd1 @ printf.o
	DS8	0x6A

	PUBWEAK	exit
exit:	// 0x08005e3b @ exit.o
	DS8	0x12

	PUBWEAK	__exit
__exit:	// 0x08005e4d @ exit.o
	DS8	0x14

	PUBWEAK	_Prout
_Prout:	// 0x08005e61 @ xprout.o
	DS8	0x2C

	PUBWEAK	IntHandlerNop
IntHandlerNop:	// 0x08005e8d @ stm32f10x_it.o
	DS8	0x2

	PUBWEAK	IntHandlerLoop
IntHandlerLoop:	// 0x08005e8f @ stm32f10x_it.o

	RSEG FLASH_DATA:DATA(1)
	DS8	0x0

	PUBWEAK	__iar_ttio_handles
__iar_ttio_handles:	// 0x20004fe4 @ XShttio.o
	DS8	0x8

	PUBWEAK	g_pVsd
g_pVsd:	// 0x20004fec @ main2.o
	DS8	0x4

	PUBWEAK	HSEStatus
HSEStatus:	// 0x20004ff0 @ stm32f10x_rcc.o
	DS8	0x4

	PUBWEAK	StartUpCounter
StartUpCounter:	// 0x20004ff4 @ stm32f10x_rcc.o
	DS8	0x8

	PUBWEAK	g_pUsartBuf
g_pUsartBuf:	// 0x20004ffc @ usart.o
	END
