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

	PUBWEAK	disk_initialize
disk_initialize:	// 0x080030ed @ diskio.o
	DS8	0x4

	PUBWEAK	disk_status
disk_status:	// 0x080030f1 @ diskio.o
	DS8	0x4

	PUBWEAK	disk_read
disk_read:	// 0x080030f5 @ diskio.o
	DS8	0x16

	PUBWEAK	disk_write
disk_write:	// 0x0800310b @ diskio.o
	DS8	0x16

	PUBWEAK	disk_ioctl
disk_ioctl:	// 0x08003121 @ diskio.o
	DS8	0x4C

	PUBWEAK	get_fattime
get_fattime:	// 0x0800316d @ diskio.o
	DS8	0xD18

	PUBWEAK	f_mount
f_mount:	// 0x08003e85 @ diskio.o
	DS8	0x28

	PUBWEAK	f_open
f_open:	// 0x08003ead @ diskio.o
	DS8	0x306

	PUBWEAK	f_read
f_read:	// 0x080041b3 @ diskio.o
	DS8	0x152

	PUBWEAK	f_write
f_write:	// 0x08004305 @ diskio.o
	DS8	0x2AA

	PUBWEAK	f_sync
f_sync:	// 0x080045af @ diskio.o
	DS8	0x15C

	PUBWEAK	f_close
f_close:	// 0x0800470b @ diskio.o
	DS8	0x12

	PUBWEAK	Set_System
Set_System:	// 0x0800471d @ diskio.o
	DS8	0x1B8

	PUBWEAK	LoadSGetcharWait
LoadSGetcharWait:	// 0x080048d5 @ diskio.o
	DS8	0x11C

	PUBWEAK	GetHex
GetHex:	// 0x080049f1 @ diskio.o
	DS8	0x134

	PUBWEAK	LoadSRecordSub
LoadSRecordSub:	// 0x08004b25 @ diskio.o
	DS8	0x584

	PUBWEAK	LoadSRecord
LoadSRecord:	// 0x080050a9 @ diskio.o
	DS8	0xE6

	PUBWEAK	LoadBinSub
LoadBinSub:	// 0x0800518f @ diskio.o
	DS8	0x8A

	PUBWEAK	LoadBin
LoadBin:	// 0x08005219 @ diskio.o
	DS8	0xFC

	PUBWEAK	TimerInit
TimerInit:	// 0x08005315 @ diskio.o
	DS8	0x64

	PUBWEAK	GetCurrentTime
GetCurrentTime:	// 0x08005379 @ diskio.o
	DS8	0x48

	PUBWEAK	AdcInit
AdcInit:	// 0x080053c1 @ diskio.o
	DS8	0x21C

	PUBWEAK	PulseInit
PulseInit:	// 0x080055dd @ diskio.o
	DS8	0x18C

	PUBWEAK	EXTI0_IRQHandler
EXTI0_IRQHandler:	// 0x08005769 @ diskio.o
	DS8	0x78

	PUBWEAK	EXTI1_IRQHandler
EXTI1_IRQHandler:	// 0x080057e1 @ diskio.o
	DS8	0x1E

	PUBWEAK	EXTI2_IRQHandler
EXTI2_IRQHandler:	// 0x080057ff @ diskio.o
	DS8	0x40

	PUBWEAK	ComputeMeterTacho
ComputeMeterTacho:	// 0x0800583f @ diskio.o
	DS8	0x54

	PUBWEAK	ComputeMeterSpeed
ComputeMeterSpeed:	// 0x08005893 @ diskio.o
	DS8	0xAA

	PUBWEAK	SdcInit
SdcInit:	// 0x0800593d @ diskio.o
	DS8	0x46

	PUBWEAK	WdtInitSub
WdtInitSub:	// 0x08005983 @ diskio.o
	DS8	0x1A

	PUBWEAK	WdtInit
WdtInit:	// 0x0800599d @ diskio.o
	DS8	0x78

	PUBWEAK	SerialPack
SerialPack:	// 0x08005a15 @ diskio.o
	DS8	0x104

	PUBWEAK	OutputSerial
OutputSerial:	// 0x08005b19 @ diskio.o
	DS8	0xA4

	PUBWEAK	InputSerial
InputSerial:	// 0x08005bbd @ diskio.o
	DS8	0x1D4

	PUBWEAK	CheckStartByGSensor
CheckStartByGSensor:	// 0x08005d91 @ diskio.o
	DS8	0x70

	PUBWEAK	Calibration
Calibration:	// 0x08005e01 @ diskio.o
	DS8	0x28

	PUBWEAK	Initialize
Initialize:	// 0x08005e29 @ diskio.o
	DS8	0x29C

	PUBWEAK	WaitStateChange
WaitStateChange:	// 0x080060c5 @ diskio.o
	DS8	0x530

	PUBWEAK	MSD_Init
MSD_Init:	// 0x080065f5 @ diskio.o
	DS8	0xF4

	PUBWEAK	MSD_WriteBlock
MSD_WriteBlock:	// 0x080066e9 @ diskio.o
	DS8	0x194

	PUBWEAK	MSD_ReadBlock
MSD_ReadBlock:	// 0x0800687d @ diskio.o
	DS8	0x154

	PUBWEAK	MSD_WriteBuffer
MSD_WriteBuffer:	// 0x080069d1 @ diskio.o
	DS8	0x16C

	PUBWEAK	MSD_ReadBuffer
MSD_ReadBuffer:	// 0x08006b3d @ diskio.o
	DS8	0x15C

	PUBWEAK	MSD_GetCSDRegister
MSD_GetCSDRegister:	// 0x08006c99 @ diskio.o
	DS8	0x258

	PUBWEAK	MSD_GetCIDRegister
MSD_GetCIDRegister:	// 0x08006ef1 @ diskio.o
	DS8	0x1B0

	PUBWEAK	MSD_WaitRdy
MSD_WaitRdy:	// 0x080070a1 @ diskio.o
	DS8	0x30

	PUBWEAK	MSD_SendCmd
MSD_SendCmd:	// 0x080070d1 @ diskio.o
	DS8	0x62

	PUBWEAK	MSD_GetDataResponse
MSD_GetDataResponse:	// 0x08007133 @ diskio.o
	DS8	0x46

	PUBWEAK	MSD_GetResponse2
MSD_GetResponse2:	// 0x08007179 @ diskio.o
	DS8	0x24

	PUBWEAK	MSD_GetResponse
MSD_GetResponse:	// 0x0800719d @ diskio.o
	DS8	0x36

	PUBWEAK	MSD_GetStatus
MSD_GetStatus:	// 0x080071d3 @ diskio.o
	DS8	0xAA

	PUBWEAK	MSD_GoIdleState
MSD_GoIdleState:	// 0x0800727d @ diskio.o
	DS8	0x20E

	PUBWEAK	MSD_WriteByte
MSD_WriteByte:	// 0x0800748b @ diskio.o
	DS8	0xE

	PUBWEAK	MSD_ReadByte
MSD_ReadByte:	// 0x08007499 @ diskio.o
	DS8	0x1A

	PUBWEAK	ADC_DeInit
ADC_DeInit:	// 0x080074b3 @ diskio.o
	DS8	0x3E

	PUBWEAK	ADC_Init
ADC_Init:	// 0x080074f1 @ diskio.o
	DS8	0x40

	PUBWEAK	ADC_StructInit
ADC_StructInit:	// 0x08007531 @ diskio.o
	DS8	0x12

	PUBWEAK	ADC_Cmd
ADC_Cmd:	// 0x08007543 @ diskio.o
	DS8	0x12

	PUBWEAK	ADC_DMACmd
ADC_DMACmd:	// 0x08007555 @ diskio.o
	DS8	0x14

	PUBWEAK	ADC_ITConfig
ADC_ITConfig:	// 0x08007569 @ diskio.o
	DS8	0x12

	PUBWEAK	ADC_ResetCalibration
ADC_ResetCalibration:	// 0x0800757b @ diskio.o
	DS8	0xA

	PUBWEAK	ADC_GetResetCalibrationStatus
ADC_GetResetCalibrationStatus:	// 0x08007585 @ diskio.o
	DS8	0xA

	PUBWEAK	ADC_StartCalibration
ADC_StartCalibration:	// 0x0800758f @ diskio.o
	DS8	0xA

	PUBWEAK	ADC_GetCalibrationStatus
ADC_GetCalibrationStatus:	// 0x08007599 @ diskio.o
	DS8	0xA

	PUBWEAK	ADC_SoftwareStartConvCmd
ADC_SoftwareStartConvCmd:	// 0x080075a3 @ diskio.o
	DS8	0x12

	PUBWEAK	ADC_GetSoftwareStartConvStatus
ADC_GetSoftwareStartConvStatus:	// 0x080075b5 @ diskio.o
	DS8	0xA

	PUBWEAK	ADC_DiscModeChannelCountConfig
ADC_DiscModeChannelCountConfig:	// 0x080075bf @ diskio.o
	DS8	0x12

	PUBWEAK	ADC_DiscModeCmd
ADC_DiscModeCmd:	// 0x080075d1 @ diskio.o
	DS8	0x12

	PUBWEAK	ADC_RegularChannelConfig
ADC_RegularChannelConfig:	// 0x080075e3 @ diskio.o
	DS8	0x7E

	PUBWEAK	ADC_ExternalTrigConvCmd
ADC_ExternalTrigConvCmd:	// 0x08007661 @ diskio.o
	DS8	0x12

	PUBWEAK	ADC_GetConversionValue
ADC_GetConversionValue:	// 0x08007673 @ diskio.o
	DS8	0x6

	PUBWEAK	ADC_GetDualModeConversionValue
ADC_GetDualModeConversionValue:	// 0x08007679 @ diskio.o
	DS8	0x8

	PUBWEAK	ADC_AutoInjectedConvCmd
ADC_AutoInjectedConvCmd:	// 0x08007681 @ diskio.o
	DS8	0x12

	PUBWEAK	ADC_InjectedDiscModeCmd
ADC_InjectedDiscModeCmd:	// 0x08007693 @ diskio.o
	DS8	0x12

	PUBWEAK	ADC_ExternalTrigInjectedConvConfig
ADC_ExternalTrigInjectedConvConfig:	// 0x080076a5 @ diskio.o
	DS8	0xC

	PUBWEAK	ADC_ExternalTrigInjectedConvCmd
ADC_ExternalTrigInjectedConvCmd:	// 0x080076b1 @ diskio.o
	DS8	0x12

	PUBWEAK	ADC_SoftwareStartInjectedConvCmd
ADC_SoftwareStartInjectedConvCmd:	// 0x080076c3 @ diskio.o
	DS8	0x12

	PUBWEAK	ADC_GetSoftwareStartInjectedConvCmdStatus
ADC_GetSoftwareStartInjectedConvCmdStatus:	// 0x080076d5 @ diskio.o
	DS8	0xA

	PUBWEAK	ADC_InjectedChannelConfig
ADC_InjectedChannelConfig:	// 0x080076df @ diskio.o
	DS8	0x54

	PUBWEAK	ADC_InjectedSequencerLengthConfig
ADC_InjectedSequencerLengthConfig:	// 0x08007733 @ diskio.o
	DS8	0x12

	PUBWEAK	ADC_SetInjectedOffset
ADC_SetInjectedOffset:	// 0x08007745 @ diskio.o
	DS8	0x4

	PUBWEAK	ADC_GetInjectedConversionValue
ADC_GetInjectedConversionValue:	// 0x08007749 @ diskio.o
	DS8	0x8

	PUBWEAK	ADC_AnalogWatchdogCmd
ADC_AnalogWatchdogCmd:	// 0x08007751 @ diskio.o
	DS8	0xE

	PUBWEAK	ADC_AnalogWatchdogThresholdsConfig
ADC_AnalogWatchdogThresholdsConfig:	// 0x0800775f @ diskio.o
	DS8	0x6

	PUBWEAK	ADC_AnalogWatchdogSingleChannelConfig
ADC_AnalogWatchdogSingleChannelConfig:	// 0x08007765 @ diskio.o
	DS8	0xC

	PUBWEAK	ADC_TempSensorVrefintCmd
ADC_TempSensorVrefintCmd:	// 0x08007771 @ diskio.o
	DS8	0x16

	PUBWEAK	ADC_GetFlagStatus
ADC_GetFlagStatus:	// 0x08007787 @ diskio.o
	DS8	0xE

	PUBWEAK	ADC_ClearFlag
ADC_ClearFlag:	// 0x08007795 @ diskio.o
	DS8	0xA

	PUBWEAK	ADC_GetITStatus
ADC_GetITStatus:	// 0x0800779f @ diskio.o
	DS8	0x18

	PUBWEAK	ADC_ClearITPendingBit
ADC_ClearITPendingBit:	// 0x080077b7 @ diskio.o
	DS8	0xA

	PUBWEAK	FLASH_SetLatency
FLASH_SetLatency:	// 0x080077c1 @ diskio.o
	DS8	0x14

	PUBWEAK	FLASH_HalfCycleAccessCmd
FLASH_HalfCycleAccessCmd:	// 0x080077d5 @ diskio.o
	DS8	0x14

	PUBWEAK	FLASH_PrefetchBufferCmd
FLASH_PrefetchBufferCmd:	// 0x080077e9 @ diskio.o
	DS8	0x14

	PUBWEAK	GPIO_DeInit
GPIO_DeInit:	// 0x080077fd @ diskio.o
	DS8	0x98

	PUBWEAK	GPIO_AFIODeInit
GPIO_AFIODeInit:	// 0x08007895 @ diskio.o
	DS8	0x18

	PUBWEAK	GPIO_Init
GPIO_Init:	// 0x080078ad @ diskio.o
	DS8	0xBE

	PUBWEAK	GPIO_StructInit
GPIO_StructInit:	// 0x0800796b @ diskio.o
	DS8	0x10

	PUBWEAK	GPIO_ReadInputDataBit
GPIO_ReadInputDataBit:	// 0x0800797b @ diskio.o
	DS8	0xE

	PUBWEAK	GPIO_ReadInputData
GPIO_ReadInputData:	// 0x08007989 @ diskio.o
	DS8	0x6

	PUBWEAK	GPIO_ReadOutputDataBit
GPIO_ReadOutputDataBit:	// 0x0800798f @ diskio.o
	DS8	0xE

	PUBWEAK	GPIO_ReadOutputData
GPIO_ReadOutputData:	// 0x0800799d @ diskio.o
	DS8	0x6

	PUBWEAK	GPIO_SetBits
GPIO_SetBits:	// 0x080079a3 @ diskio.o
	DS8	0x4

	PUBWEAK	GPIO_ResetBits
GPIO_ResetBits:	// 0x080079a7 @ diskio.o
	DS8	0x4

	PUBWEAK	GPIO_WriteBit
GPIO_WriteBit:	// 0x080079ab @ diskio.o
	DS8	0xA

	PUBWEAK	GPIO_Write
GPIO_Write:	// 0x080079b5 @ diskio.o
	DS8	0x4

	PUBWEAK	GPIO_PinLockConfig
GPIO_PinLockConfig:	// 0x080079b9 @ diskio.o
	DS8	0x10

	PUBWEAK	GPIO_EventOutputConfig
GPIO_EventOutputConfig:	// 0x080079c9 @ diskio.o
	DS8	0x18

	PUBWEAK	GPIO_EventOutputCmd
GPIO_EventOutputCmd:	// 0x080079e1 @ diskio.o
	DS8	0x8

	PUBWEAK	GPIO_PinRemapConfig
GPIO_PinRemapConfig:	// 0x080079e9 @ diskio.o
	DS8	0x38

	PUBWEAK	GPIO_EXTILineConfig
GPIO_EXTILineConfig:	// 0x08007a21 @ diskio.o
	DS8	0x26

	PUBWEAK	IWDG_WriteAccessCmd
IWDG_WriteAccessCmd:	// 0x08007a47 @ diskio.o
	DS8	0x8

	PUBWEAK	IWDG_SetPrescaler
IWDG_SetPrescaler:	// 0x08007a4f @ diskio.o
	DS8	0x8

	PUBWEAK	IWDG_SetReload
IWDG_SetReload:	// 0x08007a57 @ diskio.o
	DS8	0x8

	PUBWEAK	IWDG_ReloadCounter
IWDG_ReloadCounter:	// 0x08007a5f @ diskio.o
	DS8	0xC

	PUBWEAK	IWDG_Enable
IWDG_Enable:	// 0x08007a6b @ diskio.o
	DS8	0xC

	PUBWEAK	IWDG_GetFlagStatus
IWDG_GetFlagStatus:	// 0x08007a77 @ diskio.o
	DS8	0x12

	PUBWEAK	NVIC_DeInit
NVIC_DeInit:	// 0x08007a89 @ diskio.o
	DS8	0x22

	PUBWEAK	NVIC_SCBDeInit
NVIC_SCBDeInit:	// 0x08007aab @ diskio.o
	DS8	0x2E

	PUBWEAK	NVIC_PriorityGroupConfig
NVIC_PriorityGroupConfig:	// 0x08007ad9 @ diskio.o
	DS8	0xE

	PUBWEAK	NVIC_Init
NVIC_Init:	// 0x08007ae7 @ diskio.o
	DS8	0x88

	PUBWEAK	NVIC_StructInit
NVIC_StructInit:	// 0x08007b6f @ diskio.o
	DS8	0xC

	PUBWEAK	NVIC_SETPRIMASK
NVIC_SETPRIMASK:	// 0x08007b7b @ diskio.o
	DS8	0x4

	PUBWEAK	NVIC_RESETPRIMASK
NVIC_RESETPRIMASK:	// 0x08007b7f @ diskio.o
	DS8	0x4

	PUBWEAK	NVIC_SETFAULTMASK
NVIC_SETFAULTMASK:	// 0x08007b83 @ diskio.o
	DS8	0x4

	PUBWEAK	NVIC_RESETFAULTMASK
NVIC_RESETFAULTMASK:	// 0x08007b87 @ diskio.o
	DS8	0x4

	PUBWEAK	NVIC_BASEPRICONFIG
NVIC_BASEPRICONFIG:	// 0x08007b8b @ diskio.o
	DS8	0x6

	PUBWEAK	NVIC_GetBASEPRI
NVIC_GetBASEPRI:	// 0x08007b91 @ diskio.o
	DS8	0x4

	PUBWEAK	NVIC_GetCurrentPendingIRQChannel
NVIC_GetCurrentPendingIRQChannel:	// 0x08007b95 @ diskio.o
	DS8	0xC

	PUBWEAK	NVIC_GetIRQChannelPendingBitStatus
NVIC_GetIRQChannelPendingBitStatus:	// 0x08007ba1 @ diskio.o
	DS8	0x20

	PUBWEAK	NVIC_SetIRQChannelPendingBit
NVIC_SetIRQChannelPendingBit:	// 0x08007bc1 @ diskio.o
	DS8	0x8

	PUBWEAK	NVIC_ClearIRQChannelPendingBit
NVIC_ClearIRQChannelPendingBit:	// 0x08007bc9 @ diskio.o
	DS8	0x14

	PUBWEAK	NVIC_GetCurrentActiveHandler
NVIC_GetCurrentActiveHandler:	// 0x08007bdd @ diskio.o
	DS8	0xC

	PUBWEAK	NVIC_GetIRQChannelActiveBitStatus
NVIC_GetIRQChannelActiveBitStatus:	// 0x08007be9 @ diskio.o
	DS8	0x20

	PUBWEAK	NVIC_GetCPUID
NVIC_GetCPUID:	// 0x08007c09 @ diskio.o
	DS8	0x8

	PUBWEAK	NVIC_SetVectorTable
NVIC_SetVectorTable:	// 0x08007c11 @ diskio.o
	DS8	0x10

	PUBWEAK	NVIC_GenerateSystemReset
NVIC_GenerateSystemReset:	// 0x08007c21 @ diskio.o
	DS8	0xC

	PUBWEAK	NVIC_GenerateCoreReset
NVIC_GenerateCoreReset:	// 0x08007c2d @ diskio.o
	DS8	0xC

	PUBWEAK	NVIC_SystemLPConfig
NVIC_SystemLPConfig:	// 0x08007c39 @ diskio.o
	DS8	0x14

	PUBWEAK	NVIC_SystemHandlerConfig
NVIC_SystemHandlerConfig:	// 0x08007c4d @ diskio.o
	DS8	0x1E

	PUBWEAK	NVIC_SystemHandlerPriorityConfig
NVIC_SystemHandlerPriorityConfig:	// 0x08007c6b @ diskio.o
	DS8	0x56

	PUBWEAK	NVIC_GetSystemHandlerPendingBitStatus
NVIC_GetSystemHandlerPendingBitStatus:	// 0x08007cc1 @ diskio.o
	DS8	0x20

	PUBWEAK	NVIC_SetSystemHandlerPendingBit
NVIC_SetSystemHandlerPendingBit:	// 0x08007ce1 @ diskio.o
	DS8	0x16

	PUBWEAK	NVIC_ClearSystemHandlerPendingBit
NVIC_ClearSystemHandlerPendingBit:	// 0x08007cf7 @ diskio.o
	DS8	0x18

	PUBWEAK	NVIC_GetSystemHandlerActiveBitStatus
NVIC_GetSystemHandlerActiveBitStatus:	// 0x08007d0f @ diskio.o
	DS8	0x20

	PUBWEAK	NVIC_GetFaultHandlerSources
NVIC_GetFaultHandlerSources:	// 0x08007d2f @ diskio.o
	DS8	0x30

	PUBWEAK	NVIC_GetFaultAddress
NVIC_GetFaultAddress:	// 0x08007d5f @ diskio.o
	DS8	0x12

	PUBWEAK	RCC_DeInit
RCC_DeInit:	// 0x08007d71 @ diskio.o
	DS8	0x46

	PUBWEAK	RCC_HSEConfig
RCC_HSEConfig:	// 0x08007db7 @ diskio.o
	DS8	0x36

	PUBWEAK	RCC_WaitForHSEStartUp
RCC_WaitForHSEStartUp:	// 0x08007ded @ diskio.o
	DS8	0x32

	PUBWEAK	RCC_AdjustHSICalibrationValue
RCC_AdjustHSICalibrationValue:	// 0x08007e1f @ diskio.o
	DS8	0x12

	PUBWEAK	RCC_HSICmd
RCC_HSICmd:	// 0x08007e31 @ diskio.o
	DS8	0x8

	PUBWEAK	RCC_PLLConfig
RCC_PLLConfig:	// 0x08007e39 @ diskio.o
	DS8	0x12

	PUBWEAK	RCC_PLLCmd
RCC_PLLCmd:	// 0x08007e4b @ diskio.o
	DS8	0x8

	PUBWEAK	RCC_SYSCLKConfig
RCC_SYSCLKConfig:	// 0x08007e53 @ diskio.o
	DS8	0x10

	PUBWEAK	RCC_GetSYSCLKSource
RCC_GetSYSCLKSource:	// 0x08007e63 @ diskio.o
	DS8	0xC

	PUBWEAK	RCC_HCLKConfig
RCC_HCLKConfig:	// 0x08007e6f @ diskio.o
	DS8	0x10

	PUBWEAK	RCC_PCLK1Config
RCC_PCLK1Config:	// 0x08007e7f @ diskio.o
	DS8	0x10

	PUBWEAK	RCC_PCLK2Config
RCC_PCLK2Config:	// 0x08007e8f @ diskio.o
	DS8	0x16

	PUBWEAK	RCC_ITConfig
RCC_ITConfig:	// 0x08007ea5 @ diskio.o
	DS8	0x14

	PUBWEAK	RCC_USBCLKConfig
RCC_USBCLKConfig:	// 0x08007eb9 @ diskio.o
	DS8	0x8

	PUBWEAK	RCC_ADCCLKConfig
RCC_ADCCLKConfig:	// 0x08007ec1 @ diskio.o
	DS8	0x10

	PUBWEAK	RCC_LSEConfig
RCC_LSEConfig:	// 0x08007ed1 @ diskio.o
	DS8	0x18

	PUBWEAK	RCC_LSICmd
RCC_LSICmd:	// 0x08007ee9 @ diskio.o
	DS8	0x8

	PUBWEAK	RCC_RTCCLKConfig
RCC_RTCCLKConfig:	// 0x08007ef1 @ diskio.o
	DS8	0xC

	PUBWEAK	RCC_RTCCLKCmd
RCC_RTCCLKCmd:	// 0x08007efd @ diskio.o
	DS8	0xC

	PUBWEAK	RCC_GetClocksFreq
RCC_GetClocksFreq:	// 0x08007f09 @ diskio.o
	DS8	0x8C

	PUBWEAK	RCC_AHBPeriphClockCmd
RCC_AHBPeriphClockCmd:	// 0x08007f95 @ diskio.o
	DS8	0x14

	PUBWEAK	RCC_APB2PeriphClockCmd
RCC_APB2PeriphClockCmd:	// 0x08007fa9 @ diskio.o
	DS8	0x14

	PUBWEAK	RCC_APB1PeriphClockCmd
RCC_APB1PeriphClockCmd:	// 0x08007fbd @ diskio.o
	DS8	0x14

	PUBWEAK	RCC_APB2PeriphResetCmd
RCC_APB2PeriphResetCmd:	// 0x08007fd1 @ diskio.o
	DS8	0x12

	PUBWEAK	RCC_APB1PeriphResetCmd
RCC_APB1PeriphResetCmd:	// 0x08007fe3 @ diskio.o
	DS8	0x14

	PUBWEAK	RCC_BackupResetCmd
RCC_BackupResetCmd:	// 0x08007ff7 @ diskio.o
	DS8	0x8

	PUBWEAK	RCC_ClockSecuritySystemCmd
RCC_ClockSecuritySystemCmd:	// 0x08007fff @ diskio.o
	DS8	0x6

	PUBWEAK	RCC_MCOConfig
RCC_MCOConfig:	// 0x08008005 @ diskio.o
	DS8	0x6

	PUBWEAK	RCC_GetFlagStatus
RCC_GetFlagStatus:	// 0x0800800b @ diskio.o
	DS8	0x24

	PUBWEAK	RCC_ClearFlag
RCC_ClearFlag:	// 0x0800802f @ diskio.o
	DS8	0xC

	PUBWEAK	RCC_GetITStatus
RCC_GetITStatus:	// 0x0800803b @ diskio.o
	DS8	0x10

	PUBWEAK	RCC_ClearITPendingBit
RCC_ClearITPendingBit:	// 0x0800804b @ diskio.o
	DS8	0xA

	PUBWEAK	SPI_DeInit
SPI_DeInit:	// 0x08008055 @ diskio.o
	DS8	0x5C

	PUBWEAK	SPI_Init
SPI_Init:	// 0x080080b1 @ diskio.o
	DS8	0x2E

	PUBWEAK	SPI_StructInit
SPI_StructInit:	// 0x080080df @ diskio.o
	DS8	0x18

	PUBWEAK	SPI_Cmd
SPI_Cmd:	// 0x080080f7 @ diskio.o
	DS8	0x14

	PUBWEAK	SPI_ITConfig
SPI_ITConfig:	// 0x0800810b @ diskio.o
	DS8	0x1A

	PUBWEAK	SPI_DMACmd
SPI_DMACmd:	// 0x08008125 @ diskio.o
	DS8	0x10

	PUBWEAK	SPI_SendData
SPI_SendData:	// 0x08008135 @ diskio.o
	DS8	0x4

	PUBWEAK	SPI_ReceiveData
SPI_ReceiveData:	// 0x08008139 @ diskio.o
	DS8	0x4

	PUBWEAK	SPI_NSSInternalSoftwareConfig
SPI_NSSInternalSoftwareConfig:	// 0x0800813d @ diskio.o
	DS8	0x14

	PUBWEAK	SPI_SSOutputCmd
SPI_SSOutputCmd:	// 0x08008151 @ diskio.o
	DS8	0x14

	PUBWEAK	SPI_DataSizeConfig
SPI_DataSizeConfig:	// 0x08008165 @ diskio.o
	DS8	0x12

	PUBWEAK	SPI_TransmitCRC
SPI_TransmitCRC:	// 0x08008177 @ diskio.o
	DS8	0xA

	PUBWEAK	SPI_CalculateCRC
SPI_CalculateCRC:	// 0x08008181 @ diskio.o
	DS8	0x14

	PUBWEAK	SPI_GetCRC
SPI_GetCRC:	// 0x08008195 @ diskio.o
	DS8	0xC

	PUBWEAK	SPI_GetCRCPolynomial
SPI_GetCRCPolynomial:	// 0x080081a1 @ diskio.o
	DS8	0x4

	PUBWEAK	SPI_BiDirectionalLineConfig
SPI_BiDirectionalLineConfig:	// 0x080081a5 @ diskio.o
	DS8	0x16

	PUBWEAK	SPI_GetFlagStatus
SPI_GetFlagStatus:	// 0x080081bb @ diskio.o
	DS8	0xE

	PUBWEAK	SPI_ClearFlag
SPI_ClearFlag:	// 0x080081c9 @ diskio.o
	DS8	0x22

	PUBWEAK	SPI_GetITStatus
SPI_GetITStatus:	// 0x080081eb @ diskio.o
	DS8	0x26

	PUBWEAK	SPI_ClearITPendingBit
SPI_ClearITPendingBit:	// 0x08008211 @ diskio.o
	DS8	0x2C

	PUBWEAK	TIM_DeInit
TIM_DeInit:	// 0x0800823d @ diskio.o
	DS8	0xF0

	PUBWEAK	TIM_TimeBaseInit
TIM_TimeBaseInit:	// 0x0800832d @ diskio.o
	DS8	0x1E

	PUBWEAK	TIM_OCInit
TIM_OCInit:	// 0x0800834b @ diskio.o
	DS8	0xE2

	PUBWEAK	TIM_ICInit
TIM_ICInit:	// 0x0800842d @ diskio.o
	DS8	0x124

	PUBWEAK	TIM_TimeBaseStructInit
TIM_TimeBaseStructInit:	// 0x08008551 @ diskio.o
	DS8	0xC

	PUBWEAK	TIM_OCStructInit
TIM_OCStructInit:	// 0x0800855d @ diskio.o
	DS8	0xC

	PUBWEAK	TIM_ICStructInit
TIM_ICStructInit:	// 0x08008569 @ diskio.o
	DS8	0x16

	PUBWEAK	TIM_Cmd
TIM_Cmd:	// 0x0800857f @ diskio.o
	DS8	0x14

	PUBWEAK	TIM_ITConfig
TIM_ITConfig:	// 0x08008593 @ diskio.o
	DS8	0x10

	PUBWEAK	TIM_DMAConfig
TIM_DMAConfig:	// 0x080085a3 @ diskio.o
	DS8	0xC

	PUBWEAK	TIM_DMACmd
TIM_DMACmd:	// 0x080085af @ diskio.o
	DS8	0x10

	PUBWEAK	TIM_InternalClockConfig
TIM_InternalClockConfig:	// 0x080085bf @ diskio.o
	DS8	0xC

	PUBWEAK	TIM_ITRxExternalClockConfig
TIM_ITRxExternalClockConfig:	// 0x080085cb @ diskio.o
	DS8	0x16

	PUBWEAK	TIM_TIxExternalClockConfig
TIM_TIxExternalClockConfig:	// 0x080085e1 @ diskio.o
	DS8	0x7C

	PUBWEAK	TIM_ETRClockMode1Config
TIM_ETRClockMode1Config:	// 0x0800865d @ diskio.o
	DS8	0x3A

	PUBWEAK	TIM_ETRClockMode2Config
TIM_ETRClockMode2Config:	// 0x08008697 @ diskio.o
	DS8	0x1E

	PUBWEAK	TIM_ETRConfig
TIM_ETRConfig:	// 0x080086b5 @ diskio.o
	DS8	0x16

	PUBWEAK	TIM_SelectInputTrigger
TIM_SelectInputTrigger:	// 0x080086cb @ diskio.o
	DS8	0xE

	PUBWEAK	TIM_PrescalerConfig
TIM_PrescalerConfig:	// 0x080086d9 @ diskio.o
	DS8	0x14

	PUBWEAK	TIM_CounterModeConfig
TIM_CounterModeConfig:	// 0x080086ed @ diskio.o
	DS8	0xE

	PUBWEAK	TIM_ForcedOC1Config
TIM_ForcedOC1Config:	// 0x080086fb @ diskio.o
	DS8	0xE

	PUBWEAK	TIM_ForcedOC2Config
TIM_ForcedOC2Config:	// 0x08008709 @ diskio.o
	DS8	0x10

	PUBWEAK	TIM_ForcedOC3Config
TIM_ForcedOC3Config:	// 0x08008719 @ diskio.o
	DS8	0xE

	PUBWEAK	TIM_ForcedOC4Config
TIM_ForcedOC4Config:	// 0x08008727 @ diskio.o
	DS8	0x10

	PUBWEAK	TIM_ARRPreloadConfig
TIM_ARRPreloadConfig:	// 0x08008737 @ diskio.o
	DS8	0x14

	PUBWEAK	TIM_SelectCCDMA
TIM_SelectCCDMA:	// 0x0800874b @ diskio.o
	DS8	0x12

	PUBWEAK	TIM_OC1PreloadConfig
TIM_OC1PreloadConfig:	// 0x0800875d @ diskio.o
	DS8	0xE

	PUBWEAK	TIM_OC2PreloadConfig
TIM_OC2PreloadConfig:	// 0x0800876b @ diskio.o
	DS8	0x10

	PUBWEAK	TIM_OC3PreloadConfig
TIM_OC3PreloadConfig:	// 0x0800877b @ diskio.o
	DS8	0xE

	PUBWEAK	TIM_OC4PreloadConfig
TIM_OC4PreloadConfig:	// 0x08008789 @ diskio.o
	DS8	0x10

	PUBWEAK	TIM_OC1FastConfig
TIM_OC1FastConfig:	// 0x08008799 @ diskio.o
	DS8	0xE

	PUBWEAK	TIM_OC2FastConfig
TIM_OC2FastConfig:	// 0x080087a7 @ diskio.o
	DS8	0x10

	PUBWEAK	TIM_OC3FastConfig
TIM_OC3FastConfig:	// 0x080087b7 @ diskio.o
	DS8	0xE

	PUBWEAK	TIM_OC4FastConfig
TIM_OC4FastConfig:	// 0x080087c5 @ diskio.o
	DS8	0x10

	PUBWEAK	TIM_ClearOC1Ref
TIM_ClearOC1Ref:	// 0x080087d5 @ diskio.o
	DS8	0xE

	PUBWEAK	TIM_ClearOC2Ref
TIM_ClearOC2Ref:	// 0x080087e3 @ diskio.o
	DS8	0xE

	PUBWEAK	TIM_ClearOC3Ref
TIM_ClearOC3Ref:	// 0x080087f1 @ diskio.o
	DS8	0xE

	PUBWEAK	TIM_ClearOC4Ref
TIM_ClearOC4Ref:	// 0x080087ff @ diskio.o
	DS8	0xE

	PUBWEAK	TIM_UpdateDisableConfig
TIM_UpdateDisableConfig:	// 0x0800880d @ diskio.o
	DS8	0x14

	PUBWEAK	TIM_EncoderInterfaceConfig
TIM_EncoderInterfaceConfig:	// 0x08008821 @ diskio.o
	DS8	0x32

	PUBWEAK	TIM_GenerateEvent
TIM_GenerateEvent:	// 0x08008853 @ diskio.o
	DS8	0x8

	PUBWEAK	TIM_OC1PolarityConfig
TIM_OC1PolarityConfig:	// 0x0800885b @ diskio.o
	DS8	0xE

	PUBWEAK	TIM_OC2PolarityConfig
TIM_OC2PolarityConfig:	// 0x08008869 @ diskio.o
	DS8	0x10

	PUBWEAK	TIM_OC3PolarityConfig
TIM_OC3PolarityConfig:	// 0x08008879 @ diskio.o
	DS8	0x10

	PUBWEAK	TIM_OC4PolarityConfig
TIM_OC4PolarityConfig:	// 0x08008889 @ diskio.o
	DS8	0x10

	PUBWEAK	TIM_UpdateRequestConfig
TIM_UpdateRequestConfig:	// 0x08008899 @ diskio.o
	DS8	0x14

	PUBWEAK	TIM_SelectHallSensor
TIM_SelectHallSensor:	// 0x080088ad @ diskio.o
	DS8	0x14

	PUBWEAK	TIM_SelectOnePulseMode
TIM_SelectOnePulseMode:	// 0x080088c1 @ diskio.o
	DS8	0xE

	PUBWEAK	TIM_SelectOutputTrigger
TIM_SelectOutputTrigger:	// 0x080088cf @ diskio.o
	DS8	0xC

	PUBWEAK	TIM_SelectSlaveMode
TIM_SelectSlaveMode:	// 0x080088db @ diskio.o
	DS8	0xE

	PUBWEAK	TIM_SelectMasterSlaveMode
TIM_SelectMasterSlaveMode:	// 0x080088e9 @ diskio.o
	DS8	0xE

	PUBWEAK	TIM_SetCounter
TIM_SetCounter:	// 0x080088f7 @ diskio.o
	DS8	0x4

	PUBWEAK	TIM_SetAutoreload
TIM_SetAutoreload:	// 0x080088fb @ diskio.o
	DS8	0x4

	PUBWEAK	TIM_SetCompare1
TIM_SetCompare1:	// 0x080088ff @ diskio.o
	DS8	0x4

	PUBWEAK	TIM_SetCompare2
TIM_SetCompare2:	// 0x08008903 @ diskio.o
	DS8	0x4

	PUBWEAK	TIM_SetCompare3
TIM_SetCompare3:	// 0x08008907 @ diskio.o
	DS8	0x4

	PUBWEAK	TIM_SetCompare4
TIM_SetCompare4:	// 0x0800890b @ diskio.o
	DS8	0x6

	PUBWEAK	TIM_SetIC1Prescaler
TIM_SetIC1Prescaler:	// 0x08008911 @ diskio.o
	DS8	0xE

	PUBWEAK	TIM_SetIC2Prescaler
TIM_SetIC2Prescaler:	// 0x0800891f @ diskio.o
	DS8	0x10

	PUBWEAK	TIM_SetIC3Prescaler
TIM_SetIC3Prescaler:	// 0x0800892f @ diskio.o
	DS8	0xE

	PUBWEAK	TIM_SetIC4Prescaler
TIM_SetIC4Prescaler:	// 0x0800893d @ diskio.o
	DS8	0x10

	PUBWEAK	TIM_SetClockDivision
TIM_SetClockDivision:	// 0x0800894d @ diskio.o
	DS8	0xA

	PUBWEAK	TIM_GetCapture1
TIM_GetCapture1:	// 0x08008957 @ diskio.o
	DS8	0x4

	PUBWEAK	TIM_GetCapture2
TIM_GetCapture2:	// 0x0800895b @ diskio.o
	DS8	0x4

	PUBWEAK	TIM_GetCapture3
TIM_GetCapture3:	// 0x0800895f @ diskio.o
	DS8	0x4

	PUBWEAK	TIM_GetCapture4
TIM_GetCapture4:	// 0x08008963 @ diskio.o
	DS8	0x6

	PUBWEAK	TIM_GetCounter
TIM_GetCounter:	// 0x08008969 @ diskio.o
	DS8	0x4

	PUBWEAK	TIM_GetPrescaler
TIM_GetPrescaler:	// 0x0800896d @ diskio.o
	DS8	0x4

	PUBWEAK	TIM_GetFlagStatus
TIM_GetFlagStatus:	// 0x08008971 @ diskio.o
	DS8	0xE

	PUBWEAK	TIM_ClearFlag
TIM_ClearFlag:	// 0x0800897f @ diskio.o
	DS8	0xA

	PUBWEAK	TIM_GetITStatus
TIM_GetITStatus:	// 0x08008989 @ diskio.o
	DS8	0x14

	PUBWEAK	TIM_ClearITPendingBit
TIM_ClearITPendingBit:	// 0x0800899d @ diskio.o
	DS8	0x158

	PUBWEAK	USART_DeInit
USART_DeInit:	// 0x08008af5 @ diskio.o
	DS8	0x6C

	PUBWEAK	USART_Init
USART_Init:	// 0x08008b61 @ diskio.o
	DS8	0x10C

	PUBWEAK	USART_StructInit
USART_StructInit:	// 0x08008c6d @ diskio.o
	DS8	0x20

	PUBWEAK	USART_Cmd
USART_Cmd:	// 0x08008c8d @ diskio.o
	DS8	0x14

	PUBWEAK	USART_ITConfig
USART_ITConfig:	// 0x08008ca1 @ diskio.o
	DS8	0x30

	PUBWEAK	USART_DMACmd
USART_DMACmd:	// 0x08008cd1 @ diskio.o
	DS8	0x10

	PUBWEAK	USART_SetAddress
USART_SetAddress:	// 0x08008ce1 @ diskio.o
	DS8	0x12

	PUBWEAK	USART_WakeUpConfig
USART_WakeUpConfig:	// 0x08008cf3 @ diskio.o
	DS8	0x12

	PUBWEAK	USART_ReceiverWakeUpCmd
USART_ReceiverWakeUpCmd:	// 0x08008d05 @ diskio.o
	DS8	0x14

	PUBWEAK	USART_LINBreakDetectLengthConfig
USART_LINBreakDetectLengthConfig:	// 0x08008d19 @ diskio.o
	DS8	0x12

	PUBWEAK	USART_LINCmd
USART_LINCmd:	// 0x08008d2b @ diskio.o
	DS8	0x14

	PUBWEAK	USART_SendData
USART_SendData:	// 0x08008d3f @ diskio.o
	DS8	0x8

	PUBWEAK	USART_ReceiveData
USART_ReceiveData:	// 0x08008d47 @ diskio.o
	DS8	0x8

	PUBWEAK	USART_SendBreak
USART_SendBreak:	// 0x08008d4f @ diskio.o
	DS8	0xA

	PUBWEAK	USART_SetGuardTime
USART_SetGuardTime:	// 0x08008d59 @ diskio.o
	DS8	0x10

	PUBWEAK	USART_SetPrescaler
USART_SetPrescaler:	// 0x08008d69 @ diskio.o
	DS8	0x10

	PUBWEAK	USART_SmartCardCmd
USART_SmartCardCmd:	// 0x08008d79 @ diskio.o
	DS8	0x14

	PUBWEAK	USART_SmartCardNACKCmd
USART_SmartCardNACKCmd:	// 0x08008d8d @ diskio.o
	DS8	0x14

	PUBWEAK	USART_HalfDuplexCmd
USART_HalfDuplexCmd:	// 0x08008da1 @ diskio.o
	DS8	0x14

	PUBWEAK	USART_IrDAConfig
USART_IrDAConfig:	// 0x08008db5 @ diskio.o
	DS8	0x12

	PUBWEAK	USART_IrDACmd
USART_IrDACmd:	// 0x08008dc7 @ diskio.o
	DS8	0x14

	PUBWEAK	USART_GetFlagStatus
USART_GetFlagStatus:	// 0x08008ddb @ diskio.o
	DS8	0xE

	PUBWEAK	USART_ClearFlag
USART_ClearFlag:	// 0x08008de9 @ diskio.o
	DS8	0xA

	PUBWEAK	USART_GetITStatus
USART_GetITStatus:	// 0x08008df3 @ diskio.o
	DS8	0x3E

	PUBWEAK	USART_ClearITPendingBit
USART_ClearITPendingBit:	// 0x08008e31 @ diskio.o
	DS8	0x12

	PUBWEAK	IntHandlerNop
IntHandlerNop:	// 0x08008e43 @ diskio.o
	DS8	0x2

	PUBWEAK	IntHandlerReset
IntHandlerReset:	// 0x08008e45 @ diskio.o
	DS8	0x18

	PUBWEAK	UsartInit
UsartInit:	// 0x08008e5d @ diskio.o
	DS8	0x10A

	PUBWEAK	USART1_IRQHandler
USART1_IRQHandler:	// 0x08008f67 @ diskio.o
	DS8	0x54

	PUBWEAK	UsartPutcharUnbuffered
UsartPutcharUnbuffered:	// 0x08008fbb @ diskio.o
	DS8	0x10

	PUBWEAK	UsartPutcharBuffered
UsartPutcharBuffered:	// 0x08008fcb @ diskio.o
	DS8	0x2E

	PUBWEAK	putchar
putchar:	// 0x08008ff9 @ diskio.o
	DS8	0x42

	PUBWEAK	UsartGetcharUnbuffered
UsartGetcharUnbuffered:	// 0x0800903b @ diskio.o
	DS8	0x16

	PUBWEAK	UsartGetcharBuffered
UsartGetcharBuffered:	// 0x08009051 @ diskio.o
	DS8	0x28

	PUBWEAK	getchar
getchar:	// 0x08009079 @ diskio.o
	DS8	0x38

	PUBWEAK	UsartGetcharWaitUnbuffered
UsartGetcharWaitUnbuffered:	// 0x080090b1 @ diskio.o
	DS8	0x16

	PUBWEAK	UsartGetcharWaitBuffered
UsartGetcharWaitBuffered:	// 0x080090c7 @ diskio.o
	DS8	0x22

	PUBWEAK	GetcharWait
GetcharWait:	// 0x080090e9 @ diskio.o
	DS8	0x34

	PUBWEAK	UsartPutstrUnbuffered
UsartPutstrUnbuffered:	// 0x0800911d @ diskio.o
	DS8	0x4C

	PUBWEAK	__aeabi_memset
__aeabi_memset:	// 0x08009169 @ ABImemset.o
	DS8	0x0

	PUBWEAK	__iar_Memset
__iar_Memset:	// 0x08009169 @ ABImemset.o
	DS8	0x8

	PUBWEAK	__iar_Memset_word
__iar_Memset_word:	// 0x08009171 @ ABImemset.o
	DS8	0x5E

	PUBWEAK	__iar_zero_init3
__iar_zero_init3:	// 0x080091cf @ zero_init3.o
	DS8	0x22

	PUBWEAK	__iar_unaligned_memcmp
__iar_unaligned_memcmp:	// 0x080091f1 @ memcmp_unaligned.o
	DS8	0x62

	PUBWEAK	_SProut
_SProut:	// 0x08009253 @ xsprout.o
	DS8	0xA

	PUBWEAK	__aeabi_memclr
__aeabi_memclr:	// 0x0800925d @ ABImemclr.o
	DS8	0x8

	PUBWEAK	__aeabi_memclr4
__aeabi_memclr4:	// 0x08009265 @ ABImemclr4.o
	DS8	0x8

	PUBWEAK	__iar_unaligned___aeabi_memcpy
__iar_unaligned___aeabi_memcpy:	// 0x0800926d @ ABImemcpy_unaligned.o
	DS8	0x20

	PUBWEAK	__iar_unaligned___aeabi_memcpy4
__iar_unaligned___aeabi_memcpy4:	// 0x0800928d @ ABImemcpy_unaligned.o
	DS8	0x0

	PUBWEAK	__iar_unaligned___aeabi_memcpy8
__iar_unaligned___aeabi_memcpy8:	// 0x0800928d @ ABImemcpy_unaligned.o
	DS8	0x58

	PUBWEAK	JumpTo
JumpTo:	// 0x080092e5 @ diskio.o
	DS8	0x8

	PUBWEAK	__WFI
__WFI:	// 0x080092ed @ cortexm3_macro.o
	DS8	0x4

	PUBWEAK	__WFE
__WFE:	// 0x080092f1 @ cortexm3_macro.o
	DS8	0x4

	PUBWEAK	__SEV
__SEV:	// 0x080092f5 @ cortexm3_macro.o
	DS8	0x4

	PUBWEAK	__ISB
__ISB:	// 0x080092f9 @ cortexm3_macro.o
	DS8	0x6

	PUBWEAK	__DSB
__DSB:	// 0x080092ff @ cortexm3_macro.o
	DS8	0x6

	PUBWEAK	__DMB
__DMB:	// 0x08009305 @ cortexm3_macro.o
	DS8	0x6

	PUBWEAK	__SVC
__SVC:	// 0x0800930b @ cortexm3_macro.o
	DS8	0x4

	PUBWEAK	__MRS_CONTROL
__MRS_CONTROL:	// 0x0800930f @ cortexm3_macro.o
	DS8	0x6

	PUBWEAK	__MSR_CONTROL
__MSR_CONTROL:	// 0x08009315 @ cortexm3_macro.o
	DS8	0xA

	PUBWEAK	__MRS_PSP
__MRS_PSP:	// 0x0800931f @ cortexm3_macro.o
	DS8	0x6

	PUBWEAK	__MSR_PSP
__MSR_PSP:	// 0x08009325 @ cortexm3_macro.o
	DS8	0x6

	PUBWEAK	__MRS_MSP
__MRS_MSP:	// 0x0800932b @ cortexm3_macro.o
	DS8	0x6

	PUBWEAK	__MSR_MSP
__MSR_MSP:	// 0x08009331 @ cortexm3_macro.o
	DS8	0x6

	PUBWEAK	__SETPRIMASK
__SETPRIMASK:	// 0x08009337 @ cortexm3_macro.o
	DS8	0x4

	PUBWEAK	__RESETPRIMASK
__RESETPRIMASK:	// 0x0800933b @ cortexm3_macro.o
	DS8	0x4

	PUBWEAK	__SETFAULTMASK
__SETFAULTMASK:	// 0x0800933f @ cortexm3_macro.o
	DS8	0x4

	PUBWEAK	__RESETFAULTMASK
__RESETFAULTMASK:	// 0x08009343 @ cortexm3_macro.o
	DS8	0x4

	PUBWEAK	__BASEPRICONFIG
__BASEPRICONFIG:	// 0x08009347 @ cortexm3_macro.o
	DS8	0x6

	PUBWEAK	__GetBASEPRI
__GetBASEPRI:	// 0x0800934d @ cortexm3_macro.o
	DS8	0x6

	PUBWEAK	__REV_HalfWord
__REV_HalfWord:	// 0x08009353 @ cortexm3_macro.o
	DS8	0x4

	PUBWEAK	__REV_Word
__REV_Word:	// 0x08009357 @ cortexm3_macro.o
	DS8	0x6

	PUBWEAK	__aeabi_memset4
__aeabi_memset4:	// 0x0800935d @ ABImemset48.o
	DS8	0x0

	PUBWEAK	__aeabi_memset8
__aeabi_memset8:	// 0x0800935d @ ABImemset48.o
	DS8	0x8

	PUBWEAK	__iar_Memset4_word
__iar_Memset4_word:	// 0x08009365 @ ABImemset48.o
	DS8	0x0

	PUBWEAK	__iar_Memset8_word
__iar_Memset8_word:	// 0x08009365 @ ABImemset48.o
	DS8	0x34

	PUBWEAK	_PrintfFull
_PrintfFull:	// 0x08009399 @ xprintffull.o
	DS8	0x58C

	PUBWEAK	LoadInt
LoadInt:	// 0x08009925 @ xprintffull.o
	DS8	0x6E

	PUBWEAK	_LitobFull
_LitobFull:	// 0x08009993 @ xprintffull.o
	DS8	0x106

	PUBWEAK	_LdtobFull
_LdtobFull:	// 0x08009a99 @ xprintffull.o
	DS8	0x402

	PUBWEAK	_GenldFull
_GenldFull:	// 0x08009e9b @ xprintffull.o
	DS8	0x264

	PUBWEAK	_PutcharsFull
_PutcharsFull:	// 0x0800a0ff @ xprintffull.o
	DS8	0x4A

	PUBWEAK	__iar_Strchr
__iar_Strchr:	// 0x0800a149 @ strchr.o
	DS8	0x0

	PUBWEAK	strchr
strchr:	// 0x0800a149 @ strchr.o
	DS8	0x18

	PUBWEAK	strlen
strlen:	// 0x0800a161 @ strlen.o
	DS8	0x38

	PUBWEAK	__iar_Memchr
__iar_Memchr:	// 0x0800a199 @ memchr.o
	DS8	0x0

	PUBWEAK	memchr
memchr:	// 0x0800a199 @ memchr.o
	DS8	0x58

	PUBWEAK	__aeabi_ldivmod
__aeabi_ldivmod:	// 0x0800a1f1 @ I64DivMod.o
	DS8	0x44

	PUBWEAK	__aeabi_uldivmod
__aeabi_uldivmod:	// 0x0800a235 @ I64DivMod.o
	DS8	0xEE

	PUBWEAK	__iar_Dnorm
__iar_Dnorm:	// 0x0800a323 @ xdnorm.o
	DS8	0x42

	PUBWEAK	__aeabi_cdcmple
__aeabi_cdcmple:	// 0x0800a365 @ DblCmpLe.o
	DS8	0x30

	PUBWEAK	__aeabi_cdrcmple
__aeabi_cdrcmple:	// 0x0800a395 @ DblCmpGe.o
	DS8	0x30

	PUBWEAK	__iar_Dscale
__iar_Dscale:	// 0x0800a3c5 @ xdscale.o
	DS8	0x110

	PUBWEAK	__aeabi_d2iz
__aeabi_d2iz:	// 0x0800a4d5 @ DblToI32.o
	DS8	0x2C

	PUBWEAK	__aeabi_d2uiz
__aeabi_d2uiz:	// 0x0800a501 @ DblToI32.o
	DS8	0xC

	PUBWEAK	__iar_d2uiz
__iar_d2uiz:	// 0x0800a50d @ DblToI32.o
	DS8	0x20

	PUBWEAK	__aeabi_i2d
__aeabi_i2d:	// 0x0800a52d @ I32ToDbl.o
	DS8	0x14

	PUBWEAK	__aeabi_ui2d
__aeabi_ui2d:	// 0x0800a541 @ I32ToDbl.o
	DS8	0x4

	PUBWEAK	__iar_ui2d
__iar_ui2d:	// 0x0800a545 @ I32ToDbl.o
	DS8	0x18

	PUBWEAK	__aeabi_dsub
__aeabi_dsub:	// 0x0800a55d @ DblSub.o
	DS8	0x14

	PUBWEAK	__iar_dsub
__iar_dsub:	// 0x0800a571 @ DblSub.o
	DS8	0x12

	PUBWEAK	__iar_dsubStart
__iar_dsubStart:	// 0x0800a583 @ DblSub.o
	DS8	0x15A

	PUBWEAK	__aeabi_dmul
__aeabi_dmul:	// 0x0800a6dd @ DblMul.o
	DS8	0x1A4

	PUBWEAK	__aeabi_ddiv
__aeabi_ddiv:	// 0x0800a881 @ DblDiv.o
	DS8	0x256

	PUBWEAK	div
div:	// 0x0800aad7 @ div.o
	DS8	0xE

	PUBWEAK	__aeabi_ldiv0
__aeabi_ldiv0:	// 0x0800aae5 @ I64DivZer.o
	DS8	0x4

	PUBWEAK	__aeabi_dadd
__aeabi_dadd:	// 0x0800aae9 @ DblAdd.o
	DS8	0x14

	PUBWEAK	__iar_dadd
__iar_dadd:	// 0x0800aafd @ DblAdd.o
	DS8	0x194

	PUBWEAK	__iar_close_ttio
__iar_close_ttio:	// 0x0800ac91 @ iarttio.o
	DS8	0x30

	PUBWEAK	__iar_get_ttio
__iar_get_ttio:	// 0x0800acc1 @ iarttio.o
	DS8	0x3C

	PUBWEAK	__iar_lookup_ttioh
__iar_lookup_ttioh:	// 0x0800acfd @ XShttio.o
	DS8	0x8

	PUBWEAK	puts
puts:	// 0x0800ad05 @ puts.o
	DS8	0x36

	PUBWEAK	__write
__write:	// 0x0800ad3b @ write.o
	DS8	0x10

	PUBWEAK	__dwrite
__dwrite:	// 0x0800ad4b @ dwrite.o
	DS8	0x1E

	PUBWEAK	__iar_sh_stdout
__iar_sh_stdout:	// 0x0800ad69 @ iarwstd.o
	DS8	0x20

	PUBWEAK	__iar_sh_write
__iar_sh_write:	// 0x0800ad89 @ iarwrite.o
	DS8	0x24

	PUBWEAK	sprintf
sprintf:	// 0x0800adad @ sprintf.o
	DS8	0x34

	PUBWEAK	__iar_copy_init3
__iar_copy_init3:	// 0x0800ade1 @ copy_init3.o
	DS8	0x54

	PUBWEAK	printf
printf:	// 0x0800ae35 @ printf.o
	DS8	0x6A

	PUBWEAK	exit
exit:	// 0x0800ae9f @ exit.o
	DS8	0x8

	PUBWEAK	__cexit_call_dtors
__cexit_call_dtors:	// 0x0800aea7 @ cexit.o
	DS8	0x4

	PUBWEAK	__cexit_closeall
__cexit_closeall:	// 0x0800aeab @ cexit.o
	DS8	0x4

	PUBWEAK	__cexit_clearlocks
__cexit_clearlocks:	// 0x0800aeaf @ cexit.o
	DS8	0xE

	PUBWEAK	__exit
__exit:	// 0x0800aebd @ exit.o
	DS8	0x14

	PUBWEAK	_Prout
_Prout:	// 0x0800aed1 @ xprout.o

	RSEG FLASH_DATA:DATA(1)
	DS8	0x0

	PUBWEAK	__iar_ttio_handles
__iar_ttio_handles:	// 0x20004fe0 @ XShttio.o
	DS8	0x10

	PUBWEAK	HSEStartUpStatus
HSEStartUpStatus:	// 0x20004ff0 @ diskio.o
	DS8	0x8

	PUBWEAK	g_pVsd
g_pVsd:	// 0x20004ff8 @ diskio.o
	DS8	0x4

	PUBWEAK	g_pUsartBuf
g_pUsartBuf:	// 0x20004ffc @ diskio.o
	END
