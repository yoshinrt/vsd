;*****************************************************************************
;	
;	VSD2 - vehicle data logger system2
;	Copyright(C) by DDS
;	
;	rom_entry.s -- rom entry point table
;	$Id$
;	
;*****************************************************************************

	RSEG ROM_CODE:CODE(2)
;*** ABImemcpy.o *************************************************************
	EXPORT __aeabi_memcpy
__aeabi_memcpy = 0x08005475
	EXPORT __aeabi_memcpy4
__aeabi_memcpy4 = 0x0800548f
	EXPORT __aeabi_memcpy8
__aeabi_memcpy8 = 0x0800548f
;*** DblAdd.o ****************************************************************
	EXPORT __aeabi_dadd
__aeabi_dadd = 0x08005ef9
	EXPORT __iar_dadd
__iar_dadd = 0x08005f0b
;*** DblCmpGe.o **************************************************************
	EXPORT __aeabi_cdrcmple
__aeabi_cdrcmple = 0x080066bd
;*** DblCmpLe.o **************************************************************
	EXPORT __aeabi_cdcmple
__aeabi_cdcmple = 0x0800668d
;*** DblDiv.o ****************************************************************
	EXPORT __aeabi_ddiv
__aeabi_ddiv = 0x08004739
;*** DblMul.o ****************************************************************
	EXPORT __aeabi_dmul
__aeabi_dmul = 0x08005631
;*** DblSub.o ****************************************************************
	EXPORT __aeabi_dsub
__aeabi_dsub = 0x080057d5
	EXPORT __iar_dsub
__iar_dsub = 0x080057e7
	EXPORT __iar_dsubStart
__iar_dsubStart = 0x080057fb
;*** DblSubNrml.o ************************************************************
	EXPORT __iar_SubNrmlX
__iar_SubNrmlX = 0x080065ed
	EXPORT __iar_SubNrmlY
__iar_SubNrmlY = 0x08006609
;*** DblToI32.o **************************************************************
	EXPORT __aeabi_d2iz
__aeabi_d2iz = 0x080064b9
	EXPORT __aeabi_d2uiz
__aeabi_d2uiz = 0x080064e5
	EXPORT __iar_d2uiz
__iar_d2uiz = 0x080064ef
;*** I32DivMod.o *************************************************************
	EXPORT __aeabi_uidiv
__aeabi_uidiv = 0x080068d5
	EXPORT __aeabi_uidivmod
__aeabi_uidivmod = 0x080068d5
;*** I32ToDbl.o **************************************************************
	EXPORT __aeabi_i2d
__aeabi_i2d = 0x080066ed
	EXPORT __aeabi_ui2d
__aeabi_ui2d = 0x08006701
	EXPORT __iar_ui2d
__iar_ui2d = 0x08006703
;*** I64DivMod.o *************************************************************
	EXPORT __aeabi_ldivmod
__aeabi_ldivmod = 0x08006015
	EXPORT __aeabi_uldivmod
__aeabi_uldivmod = 0x08006029
;*** I64DivZer.o *************************************************************
	EXPORT __aeabi_ldiv0
__aeabi_ldiv0 = 0x08006975
;*** IntDivZer.o *************************************************************
	EXPORT __aeabi_idiv0
__aeabi_idiv0 = 0x08006971
;*** div.o *******************************************************************
	EXPORT div
div = 0x08006895
;*** exit.o ******************************************************************
	EXPORT __exit
__exit = 0x080068fd
	EXPORT exit
exit = 0x08006961
;*** memchr.o ****************************************************************
	EXPORT __iar_Memchr
__iar_Memchr = 0x08006865
	EXPORT memchr
memchr = 0x08006865
;*** stm32f10x_flash.o *******************************************************
	EXPORT FLASH_PrefetchBufferCmd
FLASH_PrefetchBufferCmd = 0x0800677d
	EXPORT FLASH_SetLatency
FLASH_SetLatency = 0x08006771
;*** stm32f10x_gpio.o ********************************************************
	EXPORT GPIO_Init
GPIO_Init = 0x08006179
	EXPORT GPIO_ReadInputDataBit
GPIO_ReadInputDataBit = 0x08006219
	EXPORT GPIO_ResetBits
GPIO_ResetBits = 0x0800622d
	EXPORT GPIO_SetBits
GPIO_SetBits = 0x08006229
;*** stm32f10x_it.o **********************************************************
	EXPORT ADC_IRQHandler
ADC_IRQHandler = 0x08005e69
	EXPORT BusFaultException
BusFaultException = 0x08005e09
	EXPORT CAN_RX1_IRQHandler
CAN_RX1_IRQHandler = 0x08005e7d
	EXPORT CAN_SCE_IRQHandler
CAN_SCE_IRQHandler = 0x08005e81
	EXPORT DMAChannel1_IRQHandler
DMAChannel1_IRQHandler = 0x08005e4d
	EXPORT DMAChannel2_IRQHandler
DMAChannel2_IRQHandler = 0x08005e51
	EXPORT DMAChannel3_IRQHandler
DMAChannel3_IRQHandler = 0x08005e55
	EXPORT DMAChannel4_IRQHandler
DMAChannel4_IRQHandler = 0x08005e59
	EXPORT DMAChannel5_IRQHandler
DMAChannel5_IRQHandler = 0x08005e5d
	EXPORT DMAChannel6_IRQHandler
DMAChannel6_IRQHandler = 0x08005e61
	EXPORT DMAChannel7_IRQHandler
DMAChannel7_IRQHandler = 0x08005e65
	EXPORT DebugMonitor
DebugMonitor = 0x08005e11
	EXPORT EXTI0_IRQHandler
EXTI0_IRQHandler = 0x08005e39
	EXPORT EXTI15_10_IRQHandler
EXTI15_10_IRQHandler = 0x08005eed
	EXPORT EXTI1_IRQHandler
EXTI1_IRQHandler = 0x08005e3d
	EXPORT EXTI2_IRQHandler
EXTI2_IRQHandler = 0x08005e41
	EXPORT EXTI3_IRQHandler
EXTI3_IRQHandler = 0x08005e45
	EXPORT EXTI4_IRQHandler
EXTI4_IRQHandler = 0x08005e49
	EXPORT EXTI9_5_IRQHandler
EXTI9_5_IRQHandler = 0x08005e85
	EXPORT FLASH_IRQHandler
FLASH_IRQHandler = 0x08005e31
	EXPORT HardFaultException
HardFaultException = 0x08005e01
	EXPORT I2C1_ER_IRQHandler
I2C1_ER_IRQHandler = 0x08005ea9
	EXPORT I2C1_EV_IRQHandler
I2C1_EV_IRQHandler = 0x08005ea5
	EXPORT I2C2_ER_IRQHandler
I2C2_ER_IRQHandler = 0x08005eb1
	EXPORT I2C2_EV_IRQHandler
I2C2_EV_IRQHandler = 0x08005ead
	EXPORT MemManageException
MemManageException = 0x08005e05
	EXPORT NMIException
NMIException = 0x08005dfd
	EXPORT PVD_IRQHandler
PVD_IRQHandler = 0x08005e25
	EXPORT PendSVC
PendSVC = 0x08005e19
	EXPORT RCC_IRQHandler
RCC_IRQHandler = 0x08005e35
	EXPORT RTCAlarm_IRQHandler
RTCAlarm_IRQHandler = 0x08005ef1
	EXPORT RTC_IRQHandler
RTC_IRQHandler = 0x08005e2d
	EXPORT SPI1_IRQHandler
SPI1_IRQHandler = 0x08005eb5
	EXPORT SPI2_IRQHandler
SPI2_IRQHandler = 0x08005eb9
	EXPORT SVCHandler
SVCHandler = 0x08005e15
	EXPORT SysTickHandler
SysTickHandler = 0x08005e1d
	EXPORT TAMPER_IRQHandler
TAMPER_IRQHandler = 0x08005e29
	EXPORT TIM1_BRK_IRQHandler
TIM1_BRK_IRQHandler = 0x08005e89
	EXPORT TIM1_CC_IRQHandler
TIM1_CC_IRQHandler = 0x08005e95
	EXPORT TIM1_TRG_COM_IRQHandler
TIM1_TRG_COM_IRQHandler = 0x08005e91
	EXPORT TIM1_UP_IRQHandler
TIM1_UP_IRQHandler = 0x08005e8d
	EXPORT TIM2_IRQHandler
TIM2_IRQHandler = 0x08005e99
	EXPORT TIM3_IRQHandler
TIM3_IRQHandler = 0x08005e9d
	EXPORT TIM4_IRQHandler
TIM4_IRQHandler = 0x08005ea1
	EXPORT USART1_IRQHandler
USART1_IRQHandler = 0x08005ebd
	EXPORT USART2_IRQHandler
USART2_IRQHandler = 0x08005ee5
	EXPORT USART3_IRQHandler
USART3_IRQHandler = 0x08005ee9
	EXPORT USBWakeUp_IRQHandler
USBWakeUp_IRQHandler = 0x08005ef5
	EXPORT USB_HP_CAN_TX_IRQHandler
USB_HP_CAN_TX_IRQHandler = 0x08005e6d
	EXPORT USB_LP_CAN_RX0_IRQHandler
USB_LP_CAN_RX0_IRQHandler = 0x08005e71
	EXPORT UsageFaultException
UsageFaultException = 0x08005e0d
	EXPORT WWDG_IRQHandler
WWDG_IRQHandler = 0x08005e21
;*** stm32f10x_nvic.o ********************************************************
	EXPORT NVIC_Init
NVIC_Init = 0x08006241
	EXPORT NVIC_PriorityGroupConfig
NVIC_PriorityGroupConfig = 0x08006231
	EXPORT NVIC_SetVectorTable
NVIC_SetVectorTable = 0x080062c5
;*** stm32f10x_rcc.o *********************************************************
	EXPORT ADCPrescTable
ADCPrescTable = 0x080068bc
	EXPORT APBAHBPrescTable
APBAHBPrescTable = 0x080068ac
	EXPORT HSEStatus
HSEStatus = 0x20004f74
	EXPORT RCC_ADCCLKConfig
RCC_ADCCLKConfig = 0x08004d59
	EXPORT RCC_APB1PeriphClockCmd
RCC_APB1PeriphClockCmd = 0x08004e0d
	EXPORT RCC_APB2PeriphClockCmd
RCC_APB2PeriphClockCmd = 0x08004df5
	EXPORT RCC_DeInit
RCC_DeInit = 0x08004c35
	EXPORT RCC_GetClocksFreq
RCC_GetClocksFreq = 0x08004d6d
	EXPORT RCC_GetFlagStatus
RCC_GetFlagStatus = 0x08004e25
	EXPORT RCC_GetSYSCLKSource
RCC_GetSYSCLKSource = 0x08004d15
	EXPORT RCC_HCLKConfig
RCC_HCLKConfig = 0x08004d21
	EXPORT RCC_HSEConfig
RCC_HSEConfig = 0x08004c81
	EXPORT RCC_PCLK1Config
RCC_PCLK1Config = 0x08004d31
	EXPORT RCC_PCLK2Config
RCC_PCLK2Config = 0x08004d39
	EXPORT RCC_PLLCmd
RCC_PLLCmd = 0x08004d01
	EXPORT RCC_PLLConfig
RCC_PLLConfig = 0x08004ce9
	EXPORT RCC_SYSCLKConfig
RCC_SYSCLKConfig = 0x08004d0d
	EXPORT RCC_USBCLKConfig
RCC_USBCLKConfig = 0x08004d4d
	EXPORT RCC_WaitForHSEStartUp
RCC_WaitForHSEStartUp = 0x08004cbd
	EXPORT StartUpCounter
StartUpCounter = 0x20004f78
;*** stm32f10x_usart.o *******************************************************
	EXPORT USART_ClearITPendingBit
USART_ClearITPendingBit = 0x08005bdd
	EXPORT USART_Cmd
USART_Cmd = 0x08005b49
	EXPORT USART_GetITStatus
USART_GetITStatus = 0x08005ba1
	EXPORT USART_ITConfig
USART_ITConfig = 0x08005b65
	EXPORT USART_Init
USART_Init = 0x08005ab1
	EXPORT USART_ReceiveData
USART_ReceiveData = 0x08005b99
;*** strchr.o ****************************************************************
	EXPORT __iar_Strchr
__iar_Strchr = 0x080068e9
	EXPORT strchr
strchr = 0x080068e9
;*** strlen.o ****************************************************************
	EXPORT strlen
strlen = 0x0800691d
;*** usb_core.o **************************************************************
	EXPORT DataStageIn
DataStageIn = 0x080042f1
	EXPORT DataStageOut
DataStageOut = 0x0800425d
	EXPORT Data_Setup0
Data_Setup0 = 0x0800443d
	EXPORT In0_Process
In0_Process = 0x08004625
	EXPORT NOP_Process
NOP_Process = 0x08004735
	EXPORT NoData_Setup0
NoData_Setup0 = 0x08004371
	EXPORT Out0_Process
Out0_Process = 0x0800467d
	EXPORT Post0_Process
Post0_Process = 0x080046ad
	EXPORT SetDeviceAddress
SetDeviceAddress = 0x080046f5
	EXPORT Setup0_Process
Setup0_Process = 0x080045a9
	EXPORT Standard_ClearFeature
Standard_ClearFeature = 0x080040f5
	EXPORT Standard_GetConfiguration
Standard_GetConfiguration = 0x08003fb9
	EXPORT Standard_GetDescriptorData
Standard_GetDescriptorData = 0x08004241
	EXPORT Standard_GetInterface
Standard_GetInterface = 0x08004001
	EXPORT Standard_GetStatus
Standard_GetStatus = 0x08004069
	EXPORT Standard_SetConfiguration
Standard_SetConfiguration = 0x08003fd1
	EXPORT Standard_SetDeviceFeature
Standard_SetDeviceFeature = 0x08004221
	EXPORT Standard_SetEndPointFeature
Standard_SetEndPointFeature = 0x080041b1
	EXPORT Standard_SetInterface
Standard_SetInterface = 0x08004025
	EXPORT StatusInfo
StatusInfo = 0x20004f7c
;*** xdnorm.o ****************************************************************
	EXPORT __iar_Dnorm
__iar_Dnorm = 0x080062dd
;*** xdscale.o ***************************************************************
	EXPORT __iar_Dscale
__iar_Dscale = 0x08005bed
;*** xprintffull.o ***********************************************************
	EXPORT _GenldDefault
_GenldDefault = 0x08003d05
	EXPORT _LdtobDefault
_LdtobDefault = 0x08003899
	EXPORT _LitobDefault
_LitobDefault = 0x08003799
	EXPORT _Printf
_Printf = 0x080030ed
	EXPORT _PutcharsDefault
_PutcharsDefault = 0x08003f7d
	END
