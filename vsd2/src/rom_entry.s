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
__aeabi_memcpy = 0x08005249
	EXPORT __aeabi_memcpy4
__aeabi_memcpy4 = 0x08005263
	EXPORT __aeabi_memcpy8
__aeabi_memcpy8 = 0x08005263
;*** DblAdd.o ****************************************************************
	EXPORT __aeabi_dadd
__aeabi_dadd = 0x08005cc9
	EXPORT __iar_dadd
__iar_dadd = 0x08005cdb
;*** DblCmpGe.o **************************************************************
	EXPORT __aeabi_cdrcmple
__aeabi_cdrcmple = 0x08006451
;*** DblCmpLe.o **************************************************************
	EXPORT __aeabi_cdcmple
__aeabi_cdcmple = 0x08006421
;*** DblDiv.o ****************************************************************
	EXPORT __aeabi_ddiv
__aeabi_ddiv = 0x08004739
;*** DblMul.o ****************************************************************
	EXPORT __aeabi_dmul
__aeabi_dmul = 0x08005405
;*** DblSub.o ****************************************************************
	EXPORT __aeabi_dsub
__aeabi_dsub = 0x080055a9
	EXPORT __iar_dsub
__iar_dsub = 0x080055bb
	EXPORT __iar_dsubStart
__iar_dsubStart = 0x080055cf
;*** DblSubNrml.o ************************************************************
	EXPORT __iar_SubNrmlX
__iar_SubNrmlX = 0x0800634d
	EXPORT __iar_SubNrmlY
__iar_SubNrmlY = 0x08006369
;*** DblToI32.o **************************************************************
	EXPORT __aeabi_d2iz
__aeabi_d2iz = 0x08006225
	EXPORT __aeabi_d2uiz
__aeabi_d2uiz = 0x08006251
	EXPORT __iar_d2uiz
__iar_d2uiz = 0x0800625b
;*** I32DivMod.o *************************************************************
	EXPORT __aeabi_uidiv
__aeabi_uidiv = 0x08006619
	EXPORT __aeabi_uidivmod
__aeabi_uidivmod = 0x08006619
;*** I32ToDbl.o **************************************************************
	EXPORT __aeabi_i2d
__aeabi_i2d = 0x08006481
	EXPORT __aeabi_ui2d
__aeabi_ui2d = 0x08006495
	EXPORT __iar_ui2d
__iar_ui2d = 0x08006497
;*** I64DivMod.o *************************************************************
	EXPORT __aeabi_ldivmod
__aeabi_ldivmod = 0x08005de5
	EXPORT __aeabi_uldivmod
__aeabi_uldivmod = 0x08005df9
;*** I64DivZer.o *************************************************************
	EXPORT __aeabi_ldiv0
__aeabi_ldiv0 = 0x080066b5
;*** IntDivZer.o *************************************************************
	EXPORT __aeabi_idiv0
__aeabi_idiv0 = 0x080066b1
;*** cortexm3_macro.o ********************************************************
	EXPORT __BASEPRICONFIG
__BASEPRICONFIG = 0x080061b3
	EXPORT __DMB
__DMB = 0x08006171
	EXPORT __DSB
__DSB = 0x0800616b
	EXPORT __GetBASEPRI
__GetBASEPRI = 0x080061b9
	EXPORT __ISB
__ISB = 0x08006165
	EXPORT __MRS_CONTROL
__MRS_CONTROL = 0x0800617b
	EXPORT __MRS_MSP
__MRS_MSP = 0x08006197
	EXPORT __MRS_PSP
__MRS_PSP = 0x0800618b
	EXPORT __MSR_CONTROL
__MSR_CONTROL = 0x08006181
	EXPORT __MSR_MSP
__MSR_MSP = 0x0800619d
	EXPORT __MSR_PSP
__MSR_PSP = 0x08006191
	EXPORT __RESETFAULTMASK
__RESETFAULTMASK = 0x080061af
	EXPORT __RESETPRIMASK
__RESETPRIMASK = 0x080061a7
	EXPORT __REV_HalfWord
__REV_HalfWord = 0x080061bf
	EXPORT __REV_Word
__REV_Word = 0x080061c3
	EXPORT __SETFAULTMASK
__SETFAULTMASK = 0x080061ab
	EXPORT __SETPRIMASK
__SETPRIMASK = 0x080061a3
	EXPORT __SEV
__SEV = 0x08006161
	EXPORT __SVC
__SVC = 0x08006177
	EXPORT __WFE
__WFE = 0x0800615d
	EXPORT __WFI
__WFI = 0x08006159
;*** div.o *******************************************************************
	EXPORT div
div = 0x080065d9
;*** exit.o ******************************************************************
	EXPORT __exit
__exit = 0x08006641
	EXPORT exit
exit = 0x080066a1
;*** memchr.o ****************************************************************
	EXPORT __iar_Memchr
__iar_Memchr = 0x080065c1
	EXPORT memchr
memchr = 0x080065c1
;*** sprintf.o ***************************************************************
	EXPORT sprintf
sprintf = 0x080063ed
;*** stm32f10x_it.o **********************************************************
	EXPORT ADC_IRQHandler
ADC_IRQHandler = 0x08005b41
	EXPORT BusFaultException
BusFaultException = 0x08005ae1
	EXPORT CAN_RX1_IRQHandler
CAN_RX1_IRQHandler = 0x08005b55
	EXPORT CAN_SCE_IRQHandler
CAN_SCE_IRQHandler = 0x08005b59
	EXPORT DMAChannel1_IRQHandler
DMAChannel1_IRQHandler = 0x08005b25
	EXPORT DMAChannel2_IRQHandler
DMAChannel2_IRQHandler = 0x08005b29
	EXPORT DMAChannel3_IRQHandler
DMAChannel3_IRQHandler = 0x08005b2d
	EXPORT DMAChannel4_IRQHandler
DMAChannel4_IRQHandler = 0x08005b31
	EXPORT DMAChannel5_IRQHandler
DMAChannel5_IRQHandler = 0x08005b35
	EXPORT DMAChannel6_IRQHandler
DMAChannel6_IRQHandler = 0x08005b39
	EXPORT DMAChannel7_IRQHandler
DMAChannel7_IRQHandler = 0x08005b3d
	EXPORT DebugMonitor
DebugMonitor = 0x08005ae9
	EXPORT EXTI0_IRQHandler
EXTI0_IRQHandler = 0x08005b11
	EXPORT EXTI15_10_IRQHandler
EXTI15_10_IRQHandler = 0x08005bc5
	EXPORT EXTI1_IRQHandler
EXTI1_IRQHandler = 0x08005b15
	EXPORT EXTI2_IRQHandler
EXTI2_IRQHandler = 0x08005b19
	EXPORT EXTI3_IRQHandler
EXTI3_IRQHandler = 0x08005b1d
	EXPORT EXTI4_IRQHandler
EXTI4_IRQHandler = 0x08005b21
	EXPORT EXTI9_5_IRQHandler
EXTI9_5_IRQHandler = 0x08005b5d
	EXPORT FLASH_IRQHandler
FLASH_IRQHandler = 0x08005b09
	EXPORT HardFaultException
HardFaultException = 0x08005ad9
	EXPORT I2C1_ER_IRQHandler
I2C1_ER_IRQHandler = 0x08005b81
	EXPORT I2C1_EV_IRQHandler
I2C1_EV_IRQHandler = 0x08005b7d
	EXPORT I2C2_ER_IRQHandler
I2C2_ER_IRQHandler = 0x08005b89
	EXPORT I2C2_EV_IRQHandler
I2C2_EV_IRQHandler = 0x08005b85
	EXPORT MemManageException
MemManageException = 0x08005add
	EXPORT NMIException
NMIException = 0x08005ad5
	EXPORT PVD_IRQHandler
PVD_IRQHandler = 0x08005afd
	EXPORT PendSVC
PendSVC = 0x08005af1
	EXPORT RCC_IRQHandler
RCC_IRQHandler = 0x08005b0d
	EXPORT RTCAlarm_IRQHandler
RTCAlarm_IRQHandler = 0x08005bc9
	EXPORT RTC_IRQHandler
RTC_IRQHandler = 0x08005b05
	EXPORT SPI1_IRQHandler
SPI1_IRQHandler = 0x08005b8d
	EXPORT SPI2_IRQHandler
SPI2_IRQHandler = 0x08005b91
	EXPORT SVCHandler
SVCHandler = 0x08005aed
	EXPORT SysTickHandler
SysTickHandler = 0x08005af5
	EXPORT TAMPER_IRQHandler
TAMPER_IRQHandler = 0x08005b01
	EXPORT TIM1_BRK_IRQHandler
TIM1_BRK_IRQHandler = 0x08005b61
	EXPORT TIM1_CC_IRQHandler
TIM1_CC_IRQHandler = 0x08005b6d
	EXPORT TIM1_TRG_COM_IRQHandler
TIM1_TRG_COM_IRQHandler = 0x08005b69
	EXPORT TIM1_UP_IRQHandler
TIM1_UP_IRQHandler = 0x08005b65
	EXPORT TIM2_IRQHandler
TIM2_IRQHandler = 0x08005b71
	EXPORT TIM3_IRQHandler
TIM3_IRQHandler = 0x08005b75
	EXPORT TIM4_IRQHandler
TIM4_IRQHandler = 0x08005b79
;	EXPORT USART1_IRQHandler
;USART1_IRQHandler = 0x08005b95
	EXPORT USART2_IRQHandler
USART2_IRQHandler = 0x08005bbd
	EXPORT USART3_IRQHandler
USART3_IRQHandler = 0x08005bc1
	EXPORT USBWakeUp_IRQHandler
USBWakeUp_IRQHandler = 0x08005bcd
	EXPORT USB_HP_CAN_TX_IRQHandler
USB_HP_CAN_TX_IRQHandler = 0x08005b45
	EXPORT USB_LP_CAN_RX0_IRQHandler
USB_LP_CAN_RX0_IRQHandler = 0x08005b49
	EXPORT UsageFaultException
UsageFaultException = 0x08005ae5
	EXPORT WWDG_IRQHandler
WWDG_IRQHandler = 0x08005af9
;*** stm32f10x_rcc.o *********************************************************
	EXPORT ADCPrescTable
ADCPrescTable = 0x08006600
	EXPORT APBAHBPrescTable
APBAHBPrescTable = 0x080065f0
	EXPORT HSEStatus
HSEStatus = 0x20004f74
	EXPORT StartUpCounter
StartUpCounter = 0x20004f78
;*** strchr.o ****************************************************************
	EXPORT __iar_Strchr
__iar_Strchr = 0x0800662d
	EXPORT strchr
strchr = 0x0800662d
;*** strlen.o ****************************************************************
	EXPORT strlen
strlen = 0x08006661
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
Virtual_Com_Port_ConfigDescriptor = 0x08006308
	EXPORT Virtual_Com_Port_DeviceDescriptor
Virtual_Com_Port_DeviceDescriptor = 0x08006604
	EXPORT Virtual_Com_Port_StringLangID
Virtual_Com_Port_StringLangID = 0x080066a8
	EXPORT Virtual_Com_Port_StringProduct
Virtual_Com_Port_StringProduct = 0x08006384
	EXPORT Virtual_Com_Port_StringSerial
Virtual_Com_Port_StringSerial = 0x08006590
	EXPORT Virtual_Com_Port_StringVendor
Virtual_Com_Port_StringVendor = 0x080064dc
;*** usb_endp.o **************************************************************
	EXPORT EP1_IN_Callback
EP1_IN_Callback = 0x080063e1
	EXPORT EP3_OUT_Callback
EP3_OUT_Callback = 0x080063b9
	EXPORT buffer_out
buffer_out = 0x20004f84
	EXPORT count_in
count_in = 0x20004fc4
	EXPORT count_out
count_out = 0x20004f80
;*** usb_istr.o **************************************************************
	EXPORT USB_Istr
USB_Istr = 0x080061c9
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
Virtual_Com_Port_Data_Setup = 0x08004f89
	EXPORT Virtual_Com_Port_GetConfigDescriptor
Virtual_Com_Port_GetConfigDescriptor = 0x08005009
	EXPORT Virtual_Com_Port_GetDeviceDescriptor
Virtual_Com_Port_GetDeviceDescriptor = 0x08004ff9
	EXPORT Virtual_Com_Port_GetLineCoding
Virtual_Com_Port_GetLineCoding = 0x08005041
	EXPORT Virtual_Com_Port_GetStringDescriptor
Virtual_Com_Port_GetStringDescriptor = 0x08005019
	EXPORT Virtual_Com_Port_Get_Interface_Setting
Virtual_Com_Port_Get_Interface_Setting = 0x08005031
	EXPORT Virtual_Com_Port_NoData_Setup
Virtual_Com_Port_NoData_Setup = 0x08004fd9
	EXPORT Virtual_Com_Port_Reset
Virtual_Com_Port_Reset = 0x08004ea9
	EXPORT Virtual_Com_Port_SetLineCoding
Virtual_Com_Port_SetLineCoding = 0x08005049
	EXPORT Virtual_Com_Port_Status_In
Virtual_Com_Port_Status_In = 0x08004f71
	EXPORT Virtual_Com_Port_Status_Out
Virtual_Com_Port_Status_Out = 0x08004f85
	EXPORT Virtual_Com_Port_init
Virtual_Com_Port_init = 0x08004e59
	EXPORT linecoding
linecoding = 0x20004e8c
;*** usb_pwr.o ***************************************************************
	EXPORT PowerOn
PowerOn = 0x080064b1
	EXPORT bDeviceState
bDeviceState = 0x20004ffd
;*** xdnorm.o ****************************************************************
	EXPORT __iar_Dnorm
__iar_Dnorm = 0x080060ad
;*** xdscale.o ***************************************************************
	EXPORT __iar_Dscale
__iar_Dscale = 0x080059c1
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
;*** xsprout.o ***************************************************************
	EXPORT _SProut
_SProut = 0x08006681
	END
