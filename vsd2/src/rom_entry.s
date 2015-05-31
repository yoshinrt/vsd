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
;*** cortexm3_macro.o ********************************************************
	EXPORT __BASEPRICONFIG
__BASEPRICONFIG = 0x080063e3
	EXPORT __DMB
__DMB = 0x080063a1
	EXPORT __DSB
__DSB = 0x0800639b
	EXPORT __GetBASEPRI
__GetBASEPRI = 0x080063e9
	EXPORT __ISB
__ISB = 0x08006395
	EXPORT __MRS_CONTROL
__MRS_CONTROL = 0x080063ab
	EXPORT __MRS_MSP
__MRS_MSP = 0x080063c7
	EXPORT __MRS_PSP
__MRS_PSP = 0x080063bb
	EXPORT __MSR_CONTROL
__MSR_CONTROL = 0x080063b1
	EXPORT __MSR_MSP
__MSR_MSP = 0x080063cd
	EXPORT __MSR_PSP
__MSR_PSP = 0x080063c1
	EXPORT __RESETFAULTMASK
__RESETFAULTMASK = 0x080063df
	EXPORT __RESETPRIMASK
__RESETPRIMASK = 0x080063d7
	EXPORT __REV_HalfWord
__REV_HalfWord = 0x080063ef
	EXPORT __REV_Word
__REV_Word = 0x080063f3
	EXPORT __SETFAULTMASK
__SETFAULTMASK = 0x080063db
	EXPORT __SETPRIMASK
__SETPRIMASK = 0x080063d3
	EXPORT __SEV
__SEV = 0x08006391
	EXPORT __SVC
__SVC = 0x080063a7
	EXPORT __WFE
__WFE = 0x0800638d
	EXPORT __WFI
__WFI = 0x08006389
;*** div.o *******************************************************************
	EXPORT div
div = 0x08006895
;*** exit.o ******************************************************************
	EXPORT __exit
__exit = 0x080068fd
	EXPORT exit
exit = 0x08006961
;*** hw_config.o *************************************************************
	EXPORT HSEStartUpStatus
HSEStartUpStatus = 0x20004ffa
	EXPORT Set_System
Set_System = 0x080049c1
	EXPORT Set_USBClock
Set_USBClock = 0x08004a8d
	EXPORT USART_Config
USART_Config = 0x08004b49
	EXPORT USART_Config_Default
USART_Config_Default = 0x08004b09
	EXPORT USART_InitStructure
USART_InitStructure = 0x20004f1c
	EXPORT USART_To_USB_Send_Data
USART_To_USB_Send_Data = 0x08004bd9
	EXPORT USB_Cable_Config
USB_Cable_Config = 0x08004ae9
	EXPORT USB_Interrupts_Config
USB_Interrupts_Config = 0x08004aa1
	EXPORT buffer_in
buffer_in = 0x20004f34
;*** memchr.o ****************************************************************
	EXPORT __iar_Memchr
__iar_Memchr = 0x08006865
	EXPORT memchr
memchr = 0x08006865
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
;*** stm32f10x_rcc.o *********************************************************
	EXPORT ADCPrescTable
ADCPrescTable = 0x080068bc
	EXPORT APBAHBPrescTable
APBAHBPrescTable = 0x080068ac
	EXPORT HSEStatus
HSEStatus = 0x20004f74
	EXPORT StartUpCounter
StartUpCounter = 0x20004f78
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
	EXPORT NoData_Setup0
NoData_Setup0 = 0x08004371
;*** usb_desc.o **************************************************************
	EXPORT Virtual_Com_Port_ConfigDescriptor
Virtual_Com_Port_ConfigDescriptor = 0x080065a8
	EXPORT Virtual_Com_Port_DeviceDescriptor
Virtual_Com_Port_DeviceDescriptor = 0x080068c0
	EXPORT Virtual_Com_Port_StringLangID
Virtual_Com_Port_StringLangID = 0x08006968
	EXPORT Virtual_Com_Port_StringProduct
Virtual_Com_Port_StringProduct = 0x08006624
	EXPORT Virtual_Com_Port_StringSerial
Virtual_Com_Port_StringSerial = 0x08006834
	EXPORT Virtual_Com_Port_StringVendor
Virtual_Com_Port_StringVendor = 0x08006748
;*** usb_endp.o **************************************************************
	EXPORT EP1_IN_Callback
EP1_IN_Callback = 0x08006681
	EXPORT EP3_OUT_Callback
EP3_OUT_Callback = 0x08006659
	EXPORT buffer_out
buffer_out = 0x20004f84
	EXPORT count_in
count_in = 0x20004fc4
	EXPORT count_out
count_out = 0x20004f80
;*** usb_istr.o **************************************************************
	EXPORT USB_Istr
USB_Istr = 0x0800645d
	EXPORT bIntPackSOF
bIntPackSOF = 0x20004ff6
	EXPORT pEpInt_IN
pEpInt_IN = 0x20004e54
	EXPORT pEpInt_OUT
pEpInt_OUT = 0x20004e70
	EXPORT wIstr
wIstr = 0x20004ff8
;*** usb_prop.o **************************************************************
	EXPORT Config_Descriptor
Config_Descriptor = 0x20004ef4
	EXPORT Device_Descriptor
Device_Descriptor = 0x20004eec
	EXPORT Device_Property
Device_Property = 0x20004e98
	EXPORT Device_Table
Device_Table = 0x20004e94
	EXPORT Request
Request = 0x20004ffc
	EXPORT String_Descriptor
String_Descriptor = 0x20004efc
	EXPORT User_Standard_Requests
User_Standard_Requests = 0x20004ec8
	EXPORT Virtual_Com_Port_Data_Setup
Virtual_Com_Port_Data_Setup = 0x080051a9
	EXPORT Virtual_Com_Port_GetConfigDescriptor
Virtual_Com_Port_GetConfigDescriptor = 0x08005229
	EXPORT Virtual_Com_Port_GetDeviceDescriptor
Virtual_Com_Port_GetDeviceDescriptor = 0x08005219
	EXPORT Virtual_Com_Port_GetLineCoding
Virtual_Com_Port_GetLineCoding = 0x08005261
	EXPORT Virtual_Com_Port_GetStringDescriptor
Virtual_Com_Port_GetStringDescriptor = 0x08005239
	EXPORT Virtual_Com_Port_Get_Interface_Setting
Virtual_Com_Port_Get_Interface_Setting = 0x08005251
	EXPORT Virtual_Com_Port_NoData_Setup
Virtual_Com_Port_NoData_Setup = 0x080051f9
	EXPORT Virtual_Com_Port_Reset
Virtual_Com_Port_Reset = 0x080050c9
	EXPORT Virtual_Com_Port_SetLineCoding
Virtual_Com_Port_SetLineCoding = 0x08005269
	EXPORT Virtual_Com_Port_Status_In
Virtual_Com_Port_Status_In = 0x08005191
	EXPORT Virtual_Com_Port_Status_Out
Virtual_Com_Port_Status_Out = 0x080051a5
	EXPORT Virtual_Com_Port_init
Virtual_Com_Port_init = 0x08005079
	EXPORT linecoding
linecoding = 0x20004e8c
;*** usb_pwr.o ***************************************************************
	EXPORT PowerOn
PowerOn = 0x0800671d
	EXPORT bDeviceState
bDeviceState = 0x20004ffd
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
