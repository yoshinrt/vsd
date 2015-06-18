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
	DS8	0x1C4

	PUBWEAK	LoadSGetcharWait
LoadSGetcharWait:	// 0x080048e1 @ diskio.o
	DS8	0x11C

	PUBWEAK	GetHex
GetHex:	// 0x080049fd @ diskio.o
	DS8	0x134

	PUBWEAK	LoadSRecordSub
LoadSRecordSub:	// 0x08004b31 @ diskio.o
	DS8	0x584

	PUBWEAK	LoadSRecord
LoadSRecord:	// 0x080050b5 @ diskio.o
	DS8	0xE6

	PUBWEAK	LoadBinSub
LoadBinSub:	// 0x0800519b @ diskio.o
	DS8	0x8A

	PUBWEAK	LoadBin
LoadBin:	// 0x08005225 @ diskio.o
	DS8	0xFC

	PUBWEAK	TimerInit
TimerInit:	// 0x08005321 @ diskio.o
	DS8	0x68

	PUBWEAK	GetCurrentTime
GetCurrentTime:	// 0x08005389 @ diskio.o
	DS8	0x44

	PUBWEAK	AdcInit
AdcInit:	// 0x080053cd @ diskio.o
	DS8	0x21C

	PUBWEAK	PulseInit
PulseInit:	// 0x080055e9 @ diskio.o
	DS8	0x190

	PUBWEAK	EXTI0_IRQHandler
EXTI0_IRQHandler:	// 0x08005779 @ diskio.o
	DS8	0x78

	PUBWEAK	EXTI1_IRQHandler
EXTI1_IRQHandler:	// 0x080057f1 @ diskio.o
	DS8	0x1E

	PUBWEAK	EXTI2_IRQHandler
EXTI2_IRQHandler:	// 0x0800580f @ diskio.o
	DS8	0x40

	PUBWEAK	ComputeMeterTacho
ComputeMeterTacho:	// 0x0800584f @ diskio.o
	DS8	0x54

	PUBWEAK	ComputeMeterSpeed
ComputeMeterSpeed:	// 0x080058a3 @ diskio.o
	DS8	0xAA

	PUBWEAK	SdcInit
SdcInit:	// 0x0800594d @ diskio.o
	DS8	0x46

	PUBWEAK	WdtInitSub
WdtInitSub:	// 0x08005993 @ diskio.o
	DS8	0x1A

	PUBWEAK	SerialPack
SerialPack:	// 0x080059ad @ diskio.o
	DS8	0x108

	PUBWEAK	OutputSerial
OutputSerial:	// 0x08005ab5 @ diskio.o
	DS8	0xAC

	PUBWEAK	InputSerial
InputSerial:	// 0x08005b61 @ diskio.o
	DS8	0x228

	PUBWEAK	CheckStartByGSensor
CheckStartByGSensor:	// 0x08005d89 @ diskio.o
	DS8	0x6C

	PUBWEAK	Calibration
Calibration:	// 0x08005df5 @ diskio.o
	DS8	0x28

	PUBWEAK	Initialize
Initialize:	// 0x08005e1d @ diskio.o
	DS8	0x2A0

	PUBWEAK	WaitStateChange
WaitStateChange:	// 0x080060bd @ diskio.o
	DS8	0x494

	PUBWEAK	MSD_Init
MSD_Init:	// 0x08006551 @ diskio.o
	DS8	0xF4

	PUBWEAK	MSD_WriteBlock
MSD_WriteBlock:	// 0x08006645 @ diskio.o
	DS8	0x188

	PUBWEAK	MSD_ReadBlock
MSD_ReadBlock:	// 0x080067cd @ diskio.o
	DS8	0x16C

	PUBWEAK	MSD_WriteBuffer
MSD_WriteBuffer:	// 0x08006939 @ diskio.o
	DS8	0x170

	PUBWEAK	MSD_ReadBuffer
MSD_ReadBuffer:	// 0x08006aa9 @ diskio.o
	DS8	0x150

	PUBWEAK	MSD_GetCSDRegister
MSD_GetCSDRegister:	// 0x08006bf9 @ diskio.o
	DS8	0x264

	PUBWEAK	MSD_GetCIDRegister
MSD_GetCIDRegister:	// 0x08006e5d @ diskio.o
	DS8	0x1B0

	PUBWEAK	MSD_WaitRdy
MSD_WaitRdy:	// 0x0800700d @ diskio.o
	DS8	0x30

	PUBWEAK	MSD_SendCmd
MSD_SendCmd:	// 0x0800703d @ diskio.o
	DS8	0x62

	PUBWEAK	MSD_GetDataResponse
MSD_GetDataResponse:	// 0x0800709f @ diskio.o
	DS8	0x42

	PUBWEAK	MSD_GetResponse2
MSD_GetResponse2:	// 0x080070e1 @ diskio.o
	DS8	0x24

	PUBWEAK	MSD_GetResponse
MSD_GetResponse:	// 0x08007105 @ diskio.o
	DS8	0x36

	PUBWEAK	MSD_GetStatus
MSD_GetStatus:	// 0x0800713b @ diskio.o
	DS8	0xAA

	PUBWEAK	MSD_GoIdleState
MSD_GoIdleState:	// 0x080071e5 @ diskio.o
	DS8	0x20E

	PUBWEAK	MSD_WriteByte
MSD_WriteByte:	// 0x080073f3 @ diskio.o
	DS8	0xE

	PUBWEAK	MSD_ReadByte
MSD_ReadByte:	// 0x08007401 @ diskio.o
	DS8	0x1A

	PUBWEAK	ADC_DeInit
ADC_DeInit:	// 0x0800741b @ diskio.o
	DS8	0x3E

	PUBWEAK	ADC_Init
ADC_Init:	// 0x08007459 @ diskio.o
	DS8	0x40

	PUBWEAK	ADC_StructInit
ADC_StructInit:	// 0x08007499 @ diskio.o
	DS8	0x12

	PUBWEAK	ADC_Cmd
ADC_Cmd:	// 0x080074ab @ diskio.o
	DS8	0x12

	PUBWEAK	ADC_DMACmd
ADC_DMACmd:	// 0x080074bd @ diskio.o
	DS8	0x14

	PUBWEAK	ADC_ITConfig
ADC_ITConfig:	// 0x080074d1 @ diskio.o
	DS8	0x12

	PUBWEAK	ADC_ResetCalibration
ADC_ResetCalibration:	// 0x080074e3 @ diskio.o
	DS8	0xA

	PUBWEAK	ADC_GetResetCalibrationStatus
ADC_GetResetCalibrationStatus:	// 0x080074ed @ diskio.o
	DS8	0xA

	PUBWEAK	ADC_StartCalibration
ADC_StartCalibration:	// 0x080074f7 @ diskio.o
	DS8	0xA

	PUBWEAK	ADC_GetCalibrationStatus
ADC_GetCalibrationStatus:	// 0x08007501 @ diskio.o
	DS8	0xA

	PUBWEAK	ADC_SoftwareStartConvCmd
ADC_SoftwareStartConvCmd:	// 0x0800750b @ diskio.o
	DS8	0x12

	PUBWEAK	ADC_GetSoftwareStartConvStatus
ADC_GetSoftwareStartConvStatus:	// 0x0800751d @ diskio.o
	DS8	0xA

	PUBWEAK	ADC_DiscModeChannelCountConfig
ADC_DiscModeChannelCountConfig:	// 0x08007527 @ diskio.o
	DS8	0x12

	PUBWEAK	ADC_DiscModeCmd
ADC_DiscModeCmd:	// 0x08007539 @ diskio.o
	DS8	0x12

	PUBWEAK	ADC_RegularChannelConfig
ADC_RegularChannelConfig:	// 0x0800754b @ diskio.o
	DS8	0x7E

	PUBWEAK	ADC_ExternalTrigConvCmd
ADC_ExternalTrigConvCmd:	// 0x080075c9 @ diskio.o
	DS8	0x12

	PUBWEAK	ADC_GetConversionValue
ADC_GetConversionValue:	// 0x080075db @ diskio.o
	DS8	0x6

	PUBWEAK	ADC_GetDualModeConversionValue
ADC_GetDualModeConversionValue:	// 0x080075e1 @ diskio.o
	DS8	0x8

	PUBWEAK	ADC_AutoInjectedConvCmd
ADC_AutoInjectedConvCmd:	// 0x080075e9 @ diskio.o
	DS8	0x12

	PUBWEAK	ADC_InjectedDiscModeCmd
ADC_InjectedDiscModeCmd:	// 0x080075fb @ diskio.o
	DS8	0x12

	PUBWEAK	ADC_ExternalTrigInjectedConvConfig
ADC_ExternalTrigInjectedConvConfig:	// 0x0800760d @ diskio.o
	DS8	0xC

	PUBWEAK	ADC_ExternalTrigInjectedConvCmd
ADC_ExternalTrigInjectedConvCmd:	// 0x08007619 @ diskio.o
	DS8	0x12

	PUBWEAK	ADC_SoftwareStartInjectedConvCmd
ADC_SoftwareStartInjectedConvCmd:	// 0x0800762b @ diskio.o
	DS8	0x12

	PUBWEAK	ADC_GetSoftwareStartInjectedConvCmdStatus
ADC_GetSoftwareStartInjectedConvCmdStatus:	// 0x0800763d @ diskio.o
	DS8	0xA

	PUBWEAK	ADC_InjectedChannelConfig
ADC_InjectedChannelConfig:	// 0x08007647 @ diskio.o
	DS8	0x54

	PUBWEAK	ADC_InjectedSequencerLengthConfig
ADC_InjectedSequencerLengthConfig:	// 0x0800769b @ diskio.o
	DS8	0x12

	PUBWEAK	ADC_SetInjectedOffset
ADC_SetInjectedOffset:	// 0x080076ad @ diskio.o
	DS8	0x4

	PUBWEAK	ADC_GetInjectedConversionValue
ADC_GetInjectedConversionValue:	// 0x080076b1 @ diskio.o
	DS8	0x8

	PUBWEAK	ADC_AnalogWatchdogCmd
ADC_AnalogWatchdogCmd:	// 0x080076b9 @ diskio.o
	DS8	0xE

	PUBWEAK	ADC_AnalogWatchdogThresholdsConfig
ADC_AnalogWatchdogThresholdsConfig:	// 0x080076c7 @ diskio.o
	DS8	0x6

	PUBWEAK	ADC_AnalogWatchdogSingleChannelConfig
ADC_AnalogWatchdogSingleChannelConfig:	// 0x080076cd @ diskio.o
	DS8	0xC

	PUBWEAK	ADC_TempSensorVrefintCmd
ADC_TempSensorVrefintCmd:	// 0x080076d9 @ diskio.o
	DS8	0x16

	PUBWEAK	ADC_GetFlagStatus
ADC_GetFlagStatus:	// 0x080076ef @ diskio.o
	DS8	0xE

	PUBWEAK	ADC_ClearFlag
ADC_ClearFlag:	// 0x080076fd @ diskio.o
	DS8	0xA

	PUBWEAK	ADC_GetITStatus
ADC_GetITStatus:	// 0x08007707 @ diskio.o
	DS8	0x18

	PUBWEAK	ADC_ClearITPendingBit
ADC_ClearITPendingBit:	// 0x0800771f @ diskio.o
	DS8	0xA

	PUBWEAK	FLASH_SetLatency
FLASH_SetLatency:	// 0x08007729 @ diskio.o
	DS8	0x14

	PUBWEAK	FLASH_HalfCycleAccessCmd
FLASH_HalfCycleAccessCmd:	// 0x0800773d @ diskio.o
	DS8	0x14

	PUBWEAK	FLASH_PrefetchBufferCmd
FLASH_PrefetchBufferCmd:	// 0x08007751 @ diskio.o
	DS8	0x14

	PUBWEAK	GPIO_DeInit
GPIO_DeInit:	// 0x08007765 @ diskio.o
	DS8	0x98

	PUBWEAK	GPIO_AFIODeInit
GPIO_AFIODeInit:	// 0x080077fd @ diskio.o
	DS8	0x18

	PUBWEAK	GPIO_Init
GPIO_Init:	// 0x08007815 @ diskio.o
	DS8	0xBE

	PUBWEAK	GPIO_StructInit
GPIO_StructInit:	// 0x080078d3 @ diskio.o
	DS8	0x10

	PUBWEAK	GPIO_ReadInputDataBit
GPIO_ReadInputDataBit:	// 0x080078e3 @ diskio.o
	DS8	0xE

	PUBWEAK	GPIO_ReadInputData
GPIO_ReadInputData:	// 0x080078f1 @ diskio.o
	DS8	0x6

	PUBWEAK	GPIO_ReadOutputDataBit
GPIO_ReadOutputDataBit:	// 0x080078f7 @ diskio.o
	DS8	0xE

	PUBWEAK	GPIO_ReadOutputData
GPIO_ReadOutputData:	// 0x08007905 @ diskio.o
	DS8	0x6

	PUBWEAK	GPIO_SetBits
GPIO_SetBits:	// 0x0800790b @ diskio.o
	DS8	0x4

	PUBWEAK	GPIO_ResetBits
GPIO_ResetBits:	// 0x0800790f @ diskio.o
	DS8	0x4

	PUBWEAK	GPIO_WriteBit
GPIO_WriteBit:	// 0x08007913 @ diskio.o
	DS8	0xA

	PUBWEAK	GPIO_Write
GPIO_Write:	// 0x0800791d @ diskio.o
	DS8	0x4

	PUBWEAK	GPIO_PinLockConfig
GPIO_PinLockConfig:	// 0x08007921 @ diskio.o
	DS8	0x10

	PUBWEAK	GPIO_EventOutputConfig
GPIO_EventOutputConfig:	// 0x08007931 @ diskio.o
	DS8	0x18

	PUBWEAK	GPIO_EventOutputCmd
GPIO_EventOutputCmd:	// 0x08007949 @ diskio.o
	DS8	0x8

	PUBWEAK	GPIO_PinRemapConfig
GPIO_PinRemapConfig:	// 0x08007951 @ diskio.o
	DS8	0x38

	PUBWEAK	GPIO_EXTILineConfig
GPIO_EXTILineConfig:	// 0x08007989 @ diskio.o
	DS8	0x26

	PUBWEAK	IWDG_WriteAccessCmd
IWDG_WriteAccessCmd:	// 0x080079af @ diskio.o
	DS8	0x8

	PUBWEAK	IWDG_SetPrescaler
IWDG_SetPrescaler:	// 0x080079b7 @ diskio.o
	DS8	0x8

	PUBWEAK	IWDG_SetReload
IWDG_SetReload:	// 0x080079bf @ diskio.o
	DS8	0x8

	PUBWEAK	IWDG_ReloadCounter
IWDG_ReloadCounter:	// 0x080079c7 @ diskio.o
	DS8	0xC

	PUBWEAK	IWDG_Enable
IWDG_Enable:	// 0x080079d3 @ diskio.o
	DS8	0xC

	PUBWEAK	IWDG_GetFlagStatus
IWDG_GetFlagStatus:	// 0x080079df @ diskio.o
	DS8	0x12

	PUBWEAK	NVIC_DeInit
NVIC_DeInit:	// 0x080079f1 @ diskio.o
	DS8	0x22

	PUBWEAK	NVIC_SCBDeInit
NVIC_SCBDeInit:	// 0x08007a13 @ diskio.o
	DS8	0x2E

	PUBWEAK	NVIC_PriorityGroupConfig
NVIC_PriorityGroupConfig:	// 0x08007a41 @ diskio.o
	DS8	0xE

	PUBWEAK	NVIC_Init
NVIC_Init:	// 0x08007a4f @ diskio.o
	DS8	0x88

	PUBWEAK	NVIC_StructInit
NVIC_StructInit:	// 0x08007ad7 @ diskio.o
	DS8	0xC

	PUBWEAK	NVIC_SETPRIMASK
NVIC_SETPRIMASK:	// 0x08007ae3 @ diskio.o
	DS8	0x4

	PUBWEAK	NVIC_RESETPRIMASK
NVIC_RESETPRIMASK:	// 0x08007ae7 @ diskio.o
	DS8	0x4

	PUBWEAK	NVIC_SETFAULTMASK
NVIC_SETFAULTMASK:	// 0x08007aeb @ diskio.o
	DS8	0x4

	PUBWEAK	NVIC_RESETFAULTMASK
NVIC_RESETFAULTMASK:	// 0x08007aef @ diskio.o
	DS8	0x4

	PUBWEAK	NVIC_BASEPRICONFIG
NVIC_BASEPRICONFIG:	// 0x08007af3 @ diskio.o
	DS8	0x6

	PUBWEAK	NVIC_GetBASEPRI
NVIC_GetBASEPRI:	// 0x08007af9 @ diskio.o
	DS8	0x4

	PUBWEAK	NVIC_GetCurrentPendingIRQChannel
NVIC_GetCurrentPendingIRQChannel:	// 0x08007afd @ diskio.o
	DS8	0xC

	PUBWEAK	NVIC_GetIRQChannelPendingBitStatus
NVIC_GetIRQChannelPendingBitStatus:	// 0x08007b09 @ diskio.o
	DS8	0x20

	PUBWEAK	NVIC_SetIRQChannelPendingBit
NVIC_SetIRQChannelPendingBit:	// 0x08007b29 @ diskio.o
	DS8	0x8

	PUBWEAK	NVIC_ClearIRQChannelPendingBit
NVIC_ClearIRQChannelPendingBit:	// 0x08007b31 @ diskio.o
	DS8	0x14

	PUBWEAK	NVIC_GetCurrentActiveHandler
NVIC_GetCurrentActiveHandler:	// 0x08007b45 @ diskio.o
	DS8	0xC

	PUBWEAK	NVIC_GetIRQChannelActiveBitStatus
NVIC_GetIRQChannelActiveBitStatus:	// 0x08007b51 @ diskio.o
	DS8	0x20

	PUBWEAK	NVIC_GetCPUID
NVIC_GetCPUID:	// 0x08007b71 @ diskio.o
	DS8	0x8

	PUBWEAK	NVIC_SetVectorTable
NVIC_SetVectorTable:	// 0x08007b79 @ diskio.o
	DS8	0x10

	PUBWEAK	NVIC_GenerateSystemReset
NVIC_GenerateSystemReset:	// 0x08007b89 @ diskio.o
	DS8	0xC

	PUBWEAK	NVIC_GenerateCoreReset
NVIC_GenerateCoreReset:	// 0x08007b95 @ diskio.o
	DS8	0xC

	PUBWEAK	NVIC_SystemLPConfig
NVIC_SystemLPConfig:	// 0x08007ba1 @ diskio.o
	DS8	0x14

	PUBWEAK	NVIC_SystemHandlerConfig
NVIC_SystemHandlerConfig:	// 0x08007bb5 @ diskio.o
	DS8	0x1E

	PUBWEAK	NVIC_SystemHandlerPriorityConfig
NVIC_SystemHandlerPriorityConfig:	// 0x08007bd3 @ diskio.o
	DS8	0x56

	PUBWEAK	NVIC_GetSystemHandlerPendingBitStatus
NVIC_GetSystemHandlerPendingBitStatus:	// 0x08007c29 @ diskio.o
	DS8	0x20

	PUBWEAK	NVIC_SetSystemHandlerPendingBit
NVIC_SetSystemHandlerPendingBit:	// 0x08007c49 @ diskio.o
	DS8	0x16

	PUBWEAK	NVIC_ClearSystemHandlerPendingBit
NVIC_ClearSystemHandlerPendingBit:	// 0x08007c5f @ diskio.o
	DS8	0x18

	PUBWEAK	NVIC_GetSystemHandlerActiveBitStatus
NVIC_GetSystemHandlerActiveBitStatus:	// 0x08007c77 @ diskio.o
	DS8	0x20

	PUBWEAK	NVIC_GetFaultHandlerSources
NVIC_GetFaultHandlerSources:	// 0x08007c97 @ diskio.o
	DS8	0x30

	PUBWEAK	NVIC_GetFaultAddress
NVIC_GetFaultAddress:	// 0x08007cc7 @ diskio.o
	DS8	0x12

	PUBWEAK	RCC_DeInit
RCC_DeInit:	// 0x08007cd9 @ diskio.o
	DS8	0x46

	PUBWEAK	RCC_HSEConfig
RCC_HSEConfig:	// 0x08007d1f @ diskio.o
	DS8	0x36

	PUBWEAK	RCC_WaitForHSEStartUp
RCC_WaitForHSEStartUp:	// 0x08007d55 @ diskio.o
	DS8	0x3A

	PUBWEAK	RCC_AdjustHSICalibrationValue
RCC_AdjustHSICalibrationValue:	// 0x08007d8f @ diskio.o
	DS8	0x12

	PUBWEAK	RCC_HSICmd
RCC_HSICmd:	// 0x08007da1 @ diskio.o
	DS8	0x8

	PUBWEAK	RCC_PLLConfig
RCC_PLLConfig:	// 0x08007da9 @ diskio.o
	DS8	0x12

	PUBWEAK	RCC_PLLCmd
RCC_PLLCmd:	// 0x08007dbb @ diskio.o
	DS8	0x8

	PUBWEAK	RCC_SYSCLKConfig
RCC_SYSCLKConfig:	// 0x08007dc3 @ diskio.o
	DS8	0x10

	PUBWEAK	RCC_GetSYSCLKSource
RCC_GetSYSCLKSource:	// 0x08007dd3 @ diskio.o
	DS8	0xC

	PUBWEAK	RCC_HCLKConfig
RCC_HCLKConfig:	// 0x08007ddf @ diskio.o
	DS8	0x10

	PUBWEAK	RCC_PCLK1Config
RCC_PCLK1Config:	// 0x08007def @ diskio.o
	DS8	0x10

	PUBWEAK	RCC_PCLK2Config
RCC_PCLK2Config:	// 0x08007dff @ diskio.o
	DS8	0x16

	PUBWEAK	RCC_ITConfig
RCC_ITConfig:	// 0x08007e15 @ diskio.o
	DS8	0x14

	PUBWEAK	RCC_USBCLKConfig
RCC_USBCLKConfig:	// 0x08007e29 @ diskio.o
	DS8	0x8

	PUBWEAK	RCC_ADCCLKConfig
RCC_ADCCLKConfig:	// 0x08007e31 @ diskio.o
	DS8	0x10

	PUBWEAK	RCC_LSEConfig
RCC_LSEConfig:	// 0x08007e41 @ diskio.o
	DS8	0x18

	PUBWEAK	RCC_LSICmd
RCC_LSICmd:	// 0x08007e59 @ diskio.o
	DS8	0x8

	PUBWEAK	RCC_RTCCLKConfig
RCC_RTCCLKConfig:	// 0x08007e61 @ diskio.o
	DS8	0xC

	PUBWEAK	RCC_RTCCLKCmd
RCC_RTCCLKCmd:	// 0x08007e6d @ diskio.o
	DS8	0xC

	PUBWEAK	RCC_GetClocksFreq
RCC_GetClocksFreq:	// 0x08007e79 @ diskio.o
	DS8	0x8C

	PUBWEAK	RCC_AHBPeriphClockCmd
RCC_AHBPeriphClockCmd:	// 0x08007f05 @ diskio.o
	DS8	0x14

	PUBWEAK	RCC_APB2PeriphClockCmd
RCC_APB2PeriphClockCmd:	// 0x08007f19 @ diskio.o
	DS8	0x14

	PUBWEAK	RCC_APB1PeriphClockCmd
RCC_APB1PeriphClockCmd:	// 0x08007f2d @ diskio.o
	DS8	0x14

	PUBWEAK	RCC_APB2PeriphResetCmd
RCC_APB2PeriphResetCmd:	// 0x08007f41 @ diskio.o
	DS8	0x12

	PUBWEAK	RCC_APB1PeriphResetCmd
RCC_APB1PeriphResetCmd:	// 0x08007f53 @ diskio.o
	DS8	0x14

	PUBWEAK	RCC_BackupResetCmd
RCC_BackupResetCmd:	// 0x08007f67 @ diskio.o
	DS8	0x8

	PUBWEAK	RCC_ClockSecuritySystemCmd
RCC_ClockSecuritySystemCmd:	// 0x08007f6f @ diskio.o
	DS8	0x8

	PUBWEAK	RCC_MCOConfig
RCC_MCOConfig:	// 0x08007f77 @ diskio.o
	DS8	0x6

	PUBWEAK	RCC_GetFlagStatus
RCC_GetFlagStatus:	// 0x08007f7d @ diskio.o
	DS8	0x24

	PUBWEAK	RCC_ClearFlag
RCC_ClearFlag:	// 0x08007fa1 @ diskio.o
	DS8	0xC

	PUBWEAK	RCC_GetITStatus
RCC_GetITStatus:	// 0x08007fad @ diskio.o
	DS8	0x10

	PUBWEAK	RCC_ClearITPendingBit
RCC_ClearITPendingBit:	// 0x08007fbd @ diskio.o
	DS8	0xC

	PUBWEAK	SPI_DeInit
SPI_DeInit:	// 0x08007fc9 @ diskio.o
	DS8	0x58

	PUBWEAK	SPI_Init
SPI_Init:	// 0x08008021 @ diskio.o
	DS8	0x2E

	PUBWEAK	SPI_StructInit
SPI_StructInit:	// 0x0800804f @ diskio.o
	DS8	0x18

	PUBWEAK	SPI_Cmd
SPI_Cmd:	// 0x08008067 @ diskio.o
	DS8	0x14

	PUBWEAK	SPI_ITConfig
SPI_ITConfig:	// 0x0800807b @ diskio.o
	DS8	0x1A

	PUBWEAK	SPI_DMACmd
SPI_DMACmd:	// 0x08008095 @ diskio.o
	DS8	0x10

	PUBWEAK	SPI_SendData
SPI_SendData:	// 0x080080a5 @ diskio.o
	DS8	0x4

	PUBWEAK	SPI_ReceiveData
SPI_ReceiveData:	// 0x080080a9 @ diskio.o
	DS8	0x4

	PUBWEAK	SPI_NSSInternalSoftwareConfig
SPI_NSSInternalSoftwareConfig:	// 0x080080ad @ diskio.o
	DS8	0x14

	PUBWEAK	SPI_SSOutputCmd
SPI_SSOutputCmd:	// 0x080080c1 @ diskio.o
	DS8	0x14

	PUBWEAK	SPI_DataSizeConfig
SPI_DataSizeConfig:	// 0x080080d5 @ diskio.o
	DS8	0x12

	PUBWEAK	SPI_TransmitCRC
SPI_TransmitCRC:	// 0x080080e7 @ diskio.o
	DS8	0xA

	PUBWEAK	SPI_CalculateCRC
SPI_CalculateCRC:	// 0x080080f1 @ diskio.o
	DS8	0x14

	PUBWEAK	SPI_GetCRC
SPI_GetCRC:	// 0x08008105 @ diskio.o
	DS8	0xC

	PUBWEAK	SPI_GetCRCPolynomial
SPI_GetCRCPolynomial:	// 0x08008111 @ diskio.o
	DS8	0x4

	PUBWEAK	SPI_BiDirectionalLineConfig
SPI_BiDirectionalLineConfig:	// 0x08008115 @ diskio.o
	DS8	0x16

	PUBWEAK	SPI_GetFlagStatus
SPI_GetFlagStatus:	// 0x0800812b @ diskio.o
	DS8	0xE

	PUBWEAK	SPI_ClearFlag
SPI_ClearFlag:	// 0x08008139 @ diskio.o
	DS8	0x22

	PUBWEAK	SPI_GetITStatus
SPI_GetITStatus:	// 0x0800815b @ diskio.o
	DS8	0x26

	PUBWEAK	SPI_ClearITPendingBit
SPI_ClearITPendingBit:	// 0x08008181 @ diskio.o
	DS8	0x2C

	PUBWEAK	TIM_DeInit
TIM_DeInit:	// 0x080081ad @ diskio.o
	DS8	0xF0

	PUBWEAK	TIM_TimeBaseInit
TIM_TimeBaseInit:	// 0x0800829d @ diskio.o
	DS8	0x1E

	PUBWEAK	TIM_OCInit
TIM_OCInit:	// 0x080082bb @ diskio.o
	DS8	0xE2

	PUBWEAK	TIM_ICInit
TIM_ICInit:	// 0x0800839d @ diskio.o
	DS8	0x124

	PUBWEAK	TIM_TimeBaseStructInit
TIM_TimeBaseStructInit:	// 0x080084c1 @ diskio.o
	DS8	0xC

	PUBWEAK	TIM_OCStructInit
TIM_OCStructInit:	// 0x080084cd @ diskio.o
	DS8	0xC

	PUBWEAK	TIM_ICStructInit
TIM_ICStructInit:	// 0x080084d9 @ diskio.o
	DS8	0x16

	PUBWEAK	TIM_Cmd
TIM_Cmd:	// 0x080084ef @ diskio.o
	DS8	0x14

	PUBWEAK	TIM_ITConfig
TIM_ITConfig:	// 0x08008503 @ diskio.o
	DS8	0x10

	PUBWEAK	TIM_DMAConfig
TIM_DMAConfig:	// 0x08008513 @ diskio.o
	DS8	0xC

	PUBWEAK	TIM_DMACmd
TIM_DMACmd:	// 0x0800851f @ diskio.o
	DS8	0x10

	PUBWEAK	TIM_InternalClockConfig
TIM_InternalClockConfig:	// 0x0800852f @ diskio.o
	DS8	0xC

	PUBWEAK	TIM_ITRxExternalClockConfig
TIM_ITRxExternalClockConfig:	// 0x0800853b @ diskio.o
	DS8	0x16

	PUBWEAK	TIM_TIxExternalClockConfig
TIM_TIxExternalClockConfig:	// 0x08008551 @ diskio.o
	DS8	0x7C

	PUBWEAK	TIM_ETRClockMode1Config
TIM_ETRClockMode1Config:	// 0x080085cd @ diskio.o
	DS8	0x3A

	PUBWEAK	TIM_ETRClockMode2Config
TIM_ETRClockMode2Config:	// 0x08008607 @ diskio.o
	DS8	0x1E

	PUBWEAK	TIM_ETRConfig
TIM_ETRConfig:	// 0x08008625 @ diskio.o
	DS8	0x16

	PUBWEAK	TIM_SelectInputTrigger
TIM_SelectInputTrigger:	// 0x0800863b @ diskio.o
	DS8	0xE

	PUBWEAK	TIM_PrescalerConfig
TIM_PrescalerConfig:	// 0x08008649 @ diskio.o
	DS8	0x14

	PUBWEAK	TIM_CounterModeConfig
TIM_CounterModeConfig:	// 0x0800865d @ diskio.o
	DS8	0xE

	PUBWEAK	TIM_ForcedOC1Config
TIM_ForcedOC1Config:	// 0x0800866b @ diskio.o
	DS8	0xE

	PUBWEAK	TIM_ForcedOC2Config
TIM_ForcedOC2Config:	// 0x08008679 @ diskio.o
	DS8	0x10

	PUBWEAK	TIM_ForcedOC3Config
TIM_ForcedOC3Config:	// 0x08008689 @ diskio.o
	DS8	0xE

	PUBWEAK	TIM_ForcedOC4Config
TIM_ForcedOC4Config:	// 0x08008697 @ diskio.o
	DS8	0x10

	PUBWEAK	TIM_ARRPreloadConfig
TIM_ARRPreloadConfig:	// 0x080086a7 @ diskio.o
	DS8	0x14

	PUBWEAK	TIM_SelectCCDMA
TIM_SelectCCDMA:	// 0x080086bb @ diskio.o
	DS8	0x12

	PUBWEAK	TIM_OC1PreloadConfig
TIM_OC1PreloadConfig:	// 0x080086cd @ diskio.o
	DS8	0xE

	PUBWEAK	TIM_OC2PreloadConfig
TIM_OC2PreloadConfig:	// 0x080086db @ diskio.o
	DS8	0x10

	PUBWEAK	TIM_OC3PreloadConfig
TIM_OC3PreloadConfig:	// 0x080086eb @ diskio.o
	DS8	0xE

	PUBWEAK	TIM_OC4PreloadConfig
TIM_OC4PreloadConfig:	// 0x080086f9 @ diskio.o
	DS8	0x10

	PUBWEAK	TIM_OC1FastConfig
TIM_OC1FastConfig:	// 0x08008709 @ diskio.o
	DS8	0xE

	PUBWEAK	TIM_OC2FastConfig
TIM_OC2FastConfig:	// 0x08008717 @ diskio.o
	DS8	0x10

	PUBWEAK	TIM_OC3FastConfig
TIM_OC3FastConfig:	// 0x08008727 @ diskio.o
	DS8	0xE

	PUBWEAK	TIM_OC4FastConfig
TIM_OC4FastConfig:	// 0x08008735 @ diskio.o
	DS8	0x10

	PUBWEAK	TIM_ClearOC1Ref
TIM_ClearOC1Ref:	// 0x08008745 @ diskio.o
	DS8	0xE

	PUBWEAK	TIM_ClearOC2Ref
TIM_ClearOC2Ref:	// 0x08008753 @ diskio.o
	DS8	0xE

	PUBWEAK	TIM_ClearOC3Ref
TIM_ClearOC3Ref:	// 0x08008761 @ diskio.o
	DS8	0xE

	PUBWEAK	TIM_ClearOC4Ref
TIM_ClearOC4Ref:	// 0x0800876f @ diskio.o
	DS8	0xE

	PUBWEAK	TIM_UpdateDisableConfig
TIM_UpdateDisableConfig:	// 0x0800877d @ diskio.o
	DS8	0x14

	PUBWEAK	TIM_EncoderInterfaceConfig
TIM_EncoderInterfaceConfig:	// 0x08008791 @ diskio.o
	DS8	0x32

	PUBWEAK	TIM_GenerateEvent
TIM_GenerateEvent:	// 0x080087c3 @ diskio.o
	DS8	0x8

	PUBWEAK	TIM_OC1PolarityConfig
TIM_OC1PolarityConfig:	// 0x080087cb @ diskio.o
	DS8	0xE

	PUBWEAK	TIM_OC2PolarityConfig
TIM_OC2PolarityConfig:	// 0x080087d9 @ diskio.o
	DS8	0x10

	PUBWEAK	TIM_OC3PolarityConfig
TIM_OC3PolarityConfig:	// 0x080087e9 @ diskio.o
	DS8	0x10

	PUBWEAK	TIM_OC4PolarityConfig
TIM_OC4PolarityConfig:	// 0x080087f9 @ diskio.o
	DS8	0x10

	PUBWEAK	TIM_UpdateRequestConfig
TIM_UpdateRequestConfig:	// 0x08008809 @ diskio.o
	DS8	0x14

	PUBWEAK	TIM_SelectHallSensor
TIM_SelectHallSensor:	// 0x0800881d @ diskio.o
	DS8	0x14

	PUBWEAK	TIM_SelectOnePulseMode
TIM_SelectOnePulseMode:	// 0x08008831 @ diskio.o
	DS8	0xE

	PUBWEAK	TIM_SelectOutputTrigger
TIM_SelectOutputTrigger:	// 0x0800883f @ diskio.o
	DS8	0xC

	PUBWEAK	TIM_SelectSlaveMode
TIM_SelectSlaveMode:	// 0x0800884b @ diskio.o
	DS8	0xE

	PUBWEAK	TIM_SelectMasterSlaveMode
TIM_SelectMasterSlaveMode:	// 0x08008859 @ diskio.o
	DS8	0xE

	PUBWEAK	TIM_SetCounter
TIM_SetCounter:	// 0x08008867 @ diskio.o
	DS8	0x4

	PUBWEAK	TIM_SetAutoreload
TIM_SetAutoreload:	// 0x0800886b @ diskio.o
	DS8	0x4

	PUBWEAK	TIM_SetCompare1
TIM_SetCompare1:	// 0x0800886f @ diskio.o
	DS8	0x4

	PUBWEAK	TIM_SetCompare2
TIM_SetCompare2:	// 0x08008873 @ diskio.o
	DS8	0x4

	PUBWEAK	TIM_SetCompare3
TIM_SetCompare3:	// 0x08008877 @ diskio.o
	DS8	0x4

	PUBWEAK	TIM_SetCompare4
TIM_SetCompare4:	// 0x0800887b @ diskio.o
	DS8	0x6

	PUBWEAK	TIM_SetIC1Prescaler
TIM_SetIC1Prescaler:	// 0x08008881 @ diskio.o
	DS8	0xE

	PUBWEAK	TIM_SetIC2Prescaler
TIM_SetIC2Prescaler:	// 0x0800888f @ diskio.o
	DS8	0x10

	PUBWEAK	TIM_SetIC3Prescaler
TIM_SetIC3Prescaler:	// 0x0800889f @ diskio.o
	DS8	0xE

	PUBWEAK	TIM_SetIC4Prescaler
TIM_SetIC4Prescaler:	// 0x080088ad @ diskio.o
	DS8	0x10

	PUBWEAK	TIM_SetClockDivision
TIM_SetClockDivision:	// 0x080088bd @ diskio.o
	DS8	0xA

	PUBWEAK	TIM_GetCapture1
TIM_GetCapture1:	// 0x080088c7 @ diskio.o
	DS8	0x4

	PUBWEAK	TIM_GetCapture2
TIM_GetCapture2:	// 0x080088cb @ diskio.o
	DS8	0x4

	PUBWEAK	TIM_GetCapture3
TIM_GetCapture3:	// 0x080088cf @ diskio.o
	DS8	0x4

	PUBWEAK	TIM_GetCapture4
TIM_GetCapture4:	// 0x080088d3 @ diskio.o
	DS8	0x6

	PUBWEAK	TIM_GetCounter
TIM_GetCounter:	// 0x080088d9 @ diskio.o
	DS8	0x4

	PUBWEAK	TIM_GetPrescaler
TIM_GetPrescaler:	// 0x080088dd @ diskio.o
	DS8	0x4

	PUBWEAK	TIM_GetFlagStatus
TIM_GetFlagStatus:	// 0x080088e1 @ diskio.o
	DS8	0xE

	PUBWEAK	TIM_ClearFlag
TIM_ClearFlag:	// 0x080088ef @ diskio.o
	DS8	0xA

	PUBWEAK	TIM_GetITStatus
TIM_GetITStatus:	// 0x080088f9 @ diskio.o
	DS8	0x14

	PUBWEAK	TIM_ClearITPendingBit
TIM_ClearITPendingBit:	// 0x0800890d @ diskio.o
	DS8	0x158

	PUBWEAK	USART_DeInit
USART_DeInit:	// 0x08008a65 @ diskio.o
	DS8	0x68

	PUBWEAK	USART_Init
USART_Init:	// 0x08008acd @ diskio.o
	DS8	0x10C

	PUBWEAK	USART_StructInit
USART_StructInit:	// 0x08008bd9 @ diskio.o
	DS8	0x20

	PUBWEAK	USART_Cmd
USART_Cmd:	// 0x08008bf9 @ diskio.o
	DS8	0x14

	PUBWEAK	USART_ITConfig
USART_ITConfig:	// 0x08008c0d @ diskio.o
	DS8	0x30

	PUBWEAK	USART_DMACmd
USART_DMACmd:	// 0x08008c3d @ diskio.o
	DS8	0x10

	PUBWEAK	USART_SetAddress
USART_SetAddress:	// 0x08008c4d @ diskio.o
	DS8	0x12

	PUBWEAK	USART_WakeUpConfig
USART_WakeUpConfig:	// 0x08008c5f @ diskio.o
	DS8	0x12

	PUBWEAK	USART_ReceiverWakeUpCmd
USART_ReceiverWakeUpCmd:	// 0x08008c71 @ diskio.o
	DS8	0x14

	PUBWEAK	USART_LINBreakDetectLengthConfig
USART_LINBreakDetectLengthConfig:	// 0x08008c85 @ diskio.o
	DS8	0x12

	PUBWEAK	USART_LINCmd
USART_LINCmd:	// 0x08008c97 @ diskio.o
	DS8	0x14

	PUBWEAK	USART_SendData
USART_SendData:	// 0x08008cab @ diskio.o
	DS8	0x8

	PUBWEAK	USART_ReceiveData
USART_ReceiveData:	// 0x08008cb3 @ diskio.o
	DS8	0x8

	PUBWEAK	USART_SendBreak
USART_SendBreak:	// 0x08008cbb @ diskio.o
	DS8	0xA

	PUBWEAK	USART_SetGuardTime
USART_SetGuardTime:	// 0x08008cc5 @ diskio.o
	DS8	0x10

	PUBWEAK	USART_SetPrescaler
USART_SetPrescaler:	// 0x08008cd5 @ diskio.o
	DS8	0x10

	PUBWEAK	USART_SmartCardCmd
USART_SmartCardCmd:	// 0x08008ce5 @ diskio.o
	DS8	0x14

	PUBWEAK	USART_SmartCardNACKCmd
USART_SmartCardNACKCmd:	// 0x08008cf9 @ diskio.o
	DS8	0x14

	PUBWEAK	USART_HalfDuplexCmd
USART_HalfDuplexCmd:	// 0x08008d0d @ diskio.o
	DS8	0x14

	PUBWEAK	USART_IrDAConfig
USART_IrDAConfig:	// 0x08008d21 @ diskio.o
	DS8	0x12

	PUBWEAK	USART_IrDACmd
USART_IrDACmd:	// 0x08008d33 @ diskio.o
	DS8	0x14

	PUBWEAK	USART_GetFlagStatus
USART_GetFlagStatus:	// 0x08008d47 @ diskio.o
	DS8	0xE

	PUBWEAK	USART_ClearFlag
USART_ClearFlag:	// 0x08008d55 @ diskio.o
	DS8	0xA

	PUBWEAK	USART_GetITStatus
USART_GetITStatus:	// 0x08008d5f @ diskio.o
	DS8	0x3E

	PUBWEAK	USART_ClearITPendingBit
USART_ClearITPendingBit:	// 0x08008d9d @ diskio.o
	DS8	0x12

	PUBWEAK	IntHandlerNop
IntHandlerNop:	// 0x08008daf @ diskio.o
	DS8	0x2

	PUBWEAK	IntHandlerReset
IntHandlerReset:	// 0x08008db1 @ diskio.o
	DS8	0x1C

	PUBWEAK	UsartInit
UsartInit:	// 0x08008dcd @ diskio.o
	DS8	0x10A

	PUBWEAK	USART1_IRQHandler
USART1_IRQHandler:	// 0x08008ed7 @ diskio.o
	DS8	0x54

	PUBWEAK	UsartPutcharUnbuffered
UsartPutcharUnbuffered:	// 0x08008f2b @ diskio.o
	DS8	0x10

	PUBWEAK	UsartPutcharBuffered
UsartPutcharBuffered:	// 0x08008f3b @ diskio.o
	DS8	0x2E

	PUBWEAK	putchar
putchar:	// 0x08008f69 @ diskio.o
	DS8	0x42

	PUBWEAK	UsartGetcharUnbuffered
UsartGetcharUnbuffered:	// 0x08008fab @ diskio.o
	DS8	0x16

	PUBWEAK	UsartGetcharBuffered
UsartGetcharBuffered:	// 0x08008fc1 @ diskio.o
	DS8	0x28

	PUBWEAK	getchar
getchar:	// 0x08008fe9 @ diskio.o
	DS8	0x38

	PUBWEAK	UsartGetcharWaitUnbuffered
UsartGetcharWaitUnbuffered:	// 0x08009021 @ diskio.o
	DS8	0x16

	PUBWEAK	UsartGetcharWaitBuffered
UsartGetcharWaitBuffered:	// 0x08009037 @ diskio.o
	DS8	0x22

	PUBWEAK	GetcharWait
GetcharWait:	// 0x08009059 @ diskio.o
	DS8	0x34

	PUBWEAK	UsartPutstrUnbuffered
UsartPutstrUnbuffered:	// 0x0800908d @ diskio.o
	DS8	0x4C

	PUBWEAK	__aeabi_memset
__aeabi_memset:	// 0x080090d9 @ ABImemset.o
	DS8	0x0

	PUBWEAK	__iar_Memset
__iar_Memset:	// 0x080090d9 @ ABImemset.o
	DS8	0x8

	PUBWEAK	__iar_Memset_word
__iar_Memset_word:	// 0x080090e1 @ ABImemset.o
	DS8	0x5E

	PUBWEAK	strncmp
strncmp:	// 0x0800913f @ strncmp.o
	DS8	0x2A

	PUBWEAK	__iar_unaligned_memcmp
__iar_unaligned_memcmp:	// 0x08009169 @ memcmp_unaligned.o
	DS8	0x62

	PUBWEAK	__iar_zero_init3
__iar_zero_init3:	// 0x080091cb @ zero_init3.o
	DS8	0x22

	PUBWEAK	__aeabi_memclr
__aeabi_memclr:	// 0x080091ed @ ABImemclr.o
	DS8	0x6

	PUBWEAK	_SProut
_SProut:	// 0x080091f3 @ xsprout.o
	DS8	0xA

	PUBWEAK	__aeabi_memclr4
__aeabi_memclr4:	// 0x080091fd @ ABImemclr4.o
	DS8	0x8

	PUBWEAK	__iar_unaligned___aeabi_memcpy
__iar_unaligned___aeabi_memcpy:	// 0x08009205 @ ABImemcpy_unaligned.o
	DS8	0x20

	PUBWEAK	__iar_unaligned___aeabi_memcpy4
__iar_unaligned___aeabi_memcpy4:	// 0x08009225 @ ABImemcpy_unaligned.o
	DS8	0x0

	PUBWEAK	__iar_unaligned___aeabi_memcpy8
__iar_unaligned___aeabi_memcpy8:	// 0x08009225 @ ABImemcpy_unaligned.o
	DS8	0x58

	PUBWEAK	JumpTo
JumpTo:	// 0x0800927d @ diskio.o
	DS8	0x8

	PUBWEAK	__aeabi_i2d
__aeabi_i2d:	// 0x08009285 @ I32ToDbl.o
	DS8	0x14

	PUBWEAK	__aeabi_ui2d
__aeabi_ui2d:	// 0x08009299 @ I32ToDbl.o
	DS8	0x4

	PUBWEAK	__iar_ui2d
__iar_ui2d:	// 0x0800929d @ I32ToDbl.o
	DS8	0x18

	PUBWEAK	__aeabi_ddiv
__aeabi_ddiv:	// 0x080092b5 @ DblDiv.o
	DS8	0x258

	PUBWEAK	__aeabi_d2iz
__aeabi_d2iz:	// 0x0800950d @ DblToI32.o
	DS8	0x2C

	PUBWEAK	__aeabi_d2uiz
__aeabi_d2uiz:	// 0x08009539 @ DblToI32.o
	DS8	0xC

	PUBWEAK	__iar_d2uiz
__iar_d2uiz:	// 0x08009545 @ DblToI32.o
	DS8	0x20

	PUBWEAK	__aeabi_dmul
__aeabi_dmul:	// 0x08009565 @ DblMul.o
	DS8	0x1A4

	PUBWEAK	__aeabi_dadd
__aeabi_dadd:	// 0x08009709 @ DblAdd.o
	DS8	0x14

	PUBWEAK	__iar_dadd
__iar_dadd:	// 0x0800971d @ DblAdd.o
	DS8	0xD4

	PUBWEAK	__WFI
__WFI:	// 0x080097f1 @ cortexm3_macro.o
	DS8	0x4

	PUBWEAK	__WFE
__WFE:	// 0x080097f5 @ cortexm3_macro.o
	DS8	0x4

	PUBWEAK	__SEV
__SEV:	// 0x080097f9 @ cortexm3_macro.o
	DS8	0x4

	PUBWEAK	__ISB
__ISB:	// 0x080097fd @ cortexm3_macro.o
	DS8	0x6

	PUBWEAK	__DSB
__DSB:	// 0x08009803 @ cortexm3_macro.o
	DS8	0x6

	PUBWEAK	__DMB
__DMB:	// 0x08009809 @ cortexm3_macro.o
	DS8	0x6

	PUBWEAK	__SVC
__SVC:	// 0x0800980f @ cortexm3_macro.o
	DS8	0x4

	PUBWEAK	__MRS_CONTROL
__MRS_CONTROL:	// 0x08009813 @ cortexm3_macro.o
	DS8	0x6

	PUBWEAK	__MSR_CONTROL
__MSR_CONTROL:	// 0x08009819 @ cortexm3_macro.o
	DS8	0xA

	PUBWEAK	__MRS_PSP
__MRS_PSP:	// 0x08009823 @ cortexm3_macro.o
	DS8	0x6

	PUBWEAK	__MSR_PSP
__MSR_PSP:	// 0x08009829 @ cortexm3_macro.o
	DS8	0x6

	PUBWEAK	__MRS_MSP
__MRS_MSP:	// 0x0800982f @ cortexm3_macro.o
	DS8	0x6

	PUBWEAK	__MSR_MSP
__MSR_MSP:	// 0x08009835 @ cortexm3_macro.o
	DS8	0x6

	PUBWEAK	__SETPRIMASK
__SETPRIMASK:	// 0x0800983b @ cortexm3_macro.o
	DS8	0x4

	PUBWEAK	__RESETPRIMASK
__RESETPRIMASK:	// 0x0800983f @ cortexm3_macro.o
	DS8	0x4

	PUBWEAK	__SETFAULTMASK
__SETFAULTMASK:	// 0x08009843 @ cortexm3_macro.o
	DS8	0x4

	PUBWEAK	__RESETFAULTMASK
__RESETFAULTMASK:	// 0x08009847 @ cortexm3_macro.o
	DS8	0x4

	PUBWEAK	__BASEPRICONFIG
__BASEPRICONFIG:	// 0x0800984b @ cortexm3_macro.o
	DS8	0x6

	PUBWEAK	__GetBASEPRI
__GetBASEPRI:	// 0x08009851 @ cortexm3_macro.o
	DS8	0x6

	PUBWEAK	__REV_HalfWord
__REV_HalfWord:	// 0x08009857 @ cortexm3_macro.o
	DS8	0x4

	PUBWEAK	__REV_Word
__REV_Word:	// 0x0800985b @ cortexm3_macro.o
	DS8	0x6

	PUBWEAK	__aeabi_memset4
__aeabi_memset4:	// 0x08009861 @ ABImemset48.o
	DS8	0x0

	PUBWEAK	__aeabi_memset8
__aeabi_memset8:	// 0x08009861 @ ABImemset48.o
	DS8	0x8

	PUBWEAK	__iar_Memset4_word
__iar_Memset4_word:	// 0x08009869 @ ABImemset48.o
	DS8	0x0

	PUBWEAK	__iar_Memset8_word
__iar_Memset8_word:	// 0x08009869 @ ABImemset48.o
	DS8	0x34

	PUBWEAK	__aeabi_dsub
__aeabi_dsub:	// 0x0800989d @ DblSub.o
	DS8	0x14

	PUBWEAK	__iar_dsub
__iar_dsub:	// 0x080098b1 @ DblSub.o
	DS8	0x12

	PUBWEAK	__iar_dsubStart
__iar_dsubStart:	// 0x080098c3 @ DblSub.o
	DS8	0x15A

	PUBWEAK	_PrintfFull
_PrintfFull:	// 0x08009a1d @ xprintffull.o
	DS8	0x58C

	PUBWEAK	LoadInt
LoadInt:	// 0x08009fa9 @ xprintffull.o
	DS8	0x6E

	PUBWEAK	_LitobFull
_LitobFull:	// 0x0800a017 @ xprintffull.o
	DS8	0x106

	PUBWEAK	_LdtobFull
_LdtobFull:	// 0x0800a11d @ xprintffull.o
	DS8	0x402

	PUBWEAK	_GenldFull
_GenldFull:	// 0x0800a51f @ xprintffull.o
	DS8	0x264

	PUBWEAK	_PutcharsFull
_PutcharsFull:	// 0x0800a783 @ xprintffull.o
	DS8	0x4A

	PUBWEAK	__iar_Strchr
__iar_Strchr:	// 0x0800a7cd @ strchr.o
	DS8	0x0

	PUBWEAK	strchr
strchr:	// 0x0800a7cd @ strchr.o
	DS8	0x18

	PUBWEAK	strlen
strlen:	// 0x0800a7e5 @ strlen.o
	DS8	0x38

	PUBWEAK	__iar_Memchr
__iar_Memchr:	// 0x0800a81d @ memchr.o
	DS8	0x0

	PUBWEAK	memchr
memchr:	// 0x0800a81d @ memchr.o
	DS8	0x58

	PUBWEAK	__aeabi_ldivmod
__aeabi_ldivmod:	// 0x0800a875 @ I64DivMod.o
	DS8	0x44

	PUBWEAK	__aeabi_uldivmod
__aeabi_uldivmod:	// 0x0800a8b9 @ I64DivMod.o
	DS8	0xEE

	PUBWEAK	__iar_Dnorm
__iar_Dnorm:	// 0x0800a9a7 @ xdnorm.o
	DS8	0x42

	PUBWEAK	__aeabi_cdcmple
__aeabi_cdcmple:	// 0x0800a9e9 @ DblCmpLe.o
	DS8	0x30

	PUBWEAK	__aeabi_cdrcmple
__aeabi_cdrcmple:	// 0x0800aa19 @ DblCmpGe.o
	DS8	0x30

	PUBWEAK	__iar_Dscale
__iar_Dscale:	// 0x0800aa49 @ xdscale.o
	DS8	0x110

	PUBWEAK	div
div:	// 0x0800ab59 @ div.o
	DS8	0x10

	PUBWEAK	__aeabi_ldiv0
__aeabi_ldiv0:	// 0x0800ab69 @ I64DivZer.o
	DS8	0xC0

	PUBWEAK	__iar_unaligned_strncpy
__iar_unaligned_strncpy:	// 0x0800ac29 @ strncpy_unaligned.o
	DS8	0x6C

	PUBWEAK	__iar_close_ttio
__iar_close_ttio:	// 0x0800ac95 @ iarttio.o
	DS8	0x30

	PUBWEAK	__iar_get_ttio
__iar_get_ttio:	// 0x0800acc5 @ iarttio.o
	DS8	0x3C

	PUBWEAK	__iar_lookup_ttioh
__iar_lookup_ttioh:	// 0x0800ad01 @ XShttio.o
	DS8	0x8

	PUBWEAK	__iar_unaligned_strcpy
__iar_unaligned_strcpy:	// 0x0800ad09 @ strcpy_unaligned.o
	DS8	0x44

	PUBWEAK	__iar_strcmp
__iar_strcmp:	// 0x0800ad4d @ strcmp_unaligned.o
	DS8	0x0

	PUBWEAK	__iar_unaligned_strcmp
__iar_unaligned_strcmp:	// 0x0800ad4d @ strcmp_unaligned.o
	DS8	0x44

	PUBWEAK	puts
puts:	// 0x0800ad91 @ puts.o
	DS8	0x36

	PUBWEAK	__write
__write:	// 0x0800adc7 @ write.o
	DS8	0x10

	PUBWEAK	__dwrite
__dwrite:	// 0x0800add7 @ dwrite.o
	DS8	0x1E

	PUBWEAK	__iar_sh_stdout
__iar_sh_stdout:	// 0x0800adf5 @ iarwstd.o
	DS8	0x20

	PUBWEAK	__iar_sh_write
__iar_sh_write:	// 0x0800ae15 @ iarwrite.o
	DS8	0x24

	PUBWEAK	sprintf
sprintf:	// 0x0800ae39 @ sprintf.o
	DS8	0x34

	PUBWEAK	__iar_copy_init3
__iar_copy_init3:	// 0x0800ae6d @ copy_init3.o
	DS8	0x2C

	PUBWEAK	strncat
strncat:	// 0x0800ae99 @ strncat.o
	DS8	0x50

	PUBWEAK	printf
printf:	// 0x0800aee9 @ printf.o
	DS8	0x50

	PUBWEAK	strcat
strcat:	// 0x0800af39 @ strcat.o
	DS8	0x36

	PUBWEAK	exit
exit:	// 0x0800af6f @ exit.o
	DS8	0x8

	PUBWEAK	__cexit_call_dtors
__cexit_call_dtors:	// 0x0800af77 @ cexit.o
	DS8	0x4

	PUBWEAK	__cexit_closeall
__cexit_closeall:	// 0x0800af7b @ cexit.o
	DS8	0x4

	PUBWEAK	__cexit_clearlocks
__cexit_clearlocks:	// 0x0800af7f @ cexit.o
	DS8	0xE

	PUBWEAK	__exit
__exit:	// 0x0800af8d @ exit.o
	DS8	0x14

	PUBWEAK	_Prout
_Prout:	// 0x0800afa1 @ xprout.o

	RSEG FLASH_DATA:DATA(1)
	DS8	0x0

	PUBWEAK	__iar_ttio_handles
__iar_ttio_handles:	// 0x20004fe4 @ XShttio.o
	DS8	0x10

	PUBWEAK	HSEStartUpStatus
HSEStartUpStatus:	// 0x20004ff4 @ diskio.o
	DS8	0x4

	PUBWEAK	g_pVsd
g_pVsd:	// 0x20004ff8 @ diskio.o
	DS8	0x4

	PUBWEAK	g_pUsartBuf
g_pUsartBuf:	// 0x20004ffc @ diskio.o
	END
