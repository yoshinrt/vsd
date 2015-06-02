/*****************************************************************************
	
	VSD2 - vehicle data logger system2
	Copyright(C) by DDS
	
	def_int_handler.h -- interrupt handler definition
	
*****************************************************************************/

#ifndef DEF_HANDLER_NOP
	#define DEF_HANDLER_NOP( func )
#endif

#ifndef DEF_HANDLER_LOOP
	#define DEF_HANDLER_LOOP( func )
#endif

#ifndef DEF_HANDLER_RESERVED()
	#define DEF_HANDLER_RESERVED()
#endif

DEF_HANDLER_NOP(	NMIException )
DEF_HANDLER_LOOP(	HardFaultException )
DEF_HANDLER_LOOP(	MemManageException )
DEF_HANDLER_LOOP(	BusFaultException )
DEF_HANDLER_LOOP(	UsageFaultException )
DEF_HANDLER_RESERVED()
DEF_HANDLER_RESERVED()
DEF_HANDLER_RESERVED()
DEF_HANDLER_RESERVED()
DEF_HANDLER_NOP(	SVCHandler )
DEF_HANDLER_NOP(	DebugMonitor )
DEF_HANDLER_RESERVED()
DEF_HANDLER_NOP(	PendSVC )
DEF_HANDLER_NOP(	SysTickHandler )
DEF_HANDLER_NOP(	WWDG_IRQHandler )
DEF_HANDLER_NOP(	PVD_IRQHandler )
DEF_HANDLER_NOP(	TAMPER_IRQHandler )
DEF_HANDLER_NOP(	RTC_IRQHandler )
DEF_HANDLER_NOP(	FLASH_IRQHandler )
DEF_HANDLER_NOP(	RCC_IRQHandler )
DEF_HANDLER_NOP(	EXTI0_IRQHandler )
DEF_HANDLER_NOP(	EXTI1_IRQHandler )
DEF_HANDLER_NOP(	EXTI2_IRQHandler )
DEF_HANDLER_NOP(	EXTI3_IRQHandler )
DEF_HANDLER_NOP(	EXTI4_IRQHandler )
DEF_HANDLER_NOP(	DMAChannel1_IRQHandler )
DEF_HANDLER_NOP(	DMAChannel2_IRQHandler )
DEF_HANDLER_NOP(	DMAChannel3_IRQHandler )
DEF_HANDLER_NOP(	DMAChannel4_IRQHandler )
DEF_HANDLER_NOP(	DMAChannel5_IRQHandler )
DEF_HANDLER_NOP(	DMAChannel6_IRQHandler )
DEF_HANDLER_NOP(	DMAChannel7_IRQHandler )
DEF_HANDLER_NOP(	ADC_IRQHandler )
DEF_HANDLER_NOP(	USB_HP_CAN_TX_IRQHandler )
DEF_HANDLER(		USB_LP_CAN_RX0_IRQHandler )	// ÅöÇ†Ç∆Ç≈íºÇ∑
DEF_HANDLER_NOP(	CAN_RX1_IRQHandler )
DEF_HANDLER_NOP(	CAN_SCE_IRQHandler )
DEF_HANDLER_NOP(	EXTI9_5_IRQHandler )
DEF_HANDLER_NOP(	TIM1_BRK_IRQHandler )
DEF_HANDLER_NOP(	TIM1_UP_IRQHandler )
DEF_HANDLER_NOP(	TIM1_TRG_COM_IRQHandler )
DEF_HANDLER_NOP(	TIM1_CC_IRQHandler )
DEF_HANDLER_NOP(	TIM2_IRQHandler )
DEF_HANDLER_NOP(	TIM3_IRQHandler )
DEF_HANDLER_NOP(	TIM4_IRQHandler )
DEF_HANDLER_NOP(	I2C1_EV_IRQHandler )
DEF_HANDLER_NOP(	I2C1_ER_IRQHandler )
DEF_HANDLER_NOP(	I2C2_EV_IRQHandler )
DEF_HANDLER_NOP(	I2C2_ER_IRQHandler )
DEF_HANDLER_NOP(	SPI1_IRQHandler )
DEF_HANDLER_NOP(	SPI2_IRQHandler )
DEF_HANDLER(		USART1_IRQHandler )
DEF_HANDLER_NOP(	USART2_IRQHandler )
DEF_HANDLER_NOP(	USART3_IRQHandler )
DEF_HANDLER_NOP(	EXTI15_10_IRQHandler )
DEF_HANDLER_NOP(	RTCAlarm_IRQHandler )
DEF_HANDLER_NOP(	USBWakeUp_IRQHandler )

#undef DEF_HANDLER
#undef DEF_HANDLER_NOP
#undef DEF_HANDLER_LOOP
#undef DEF_HANDLER_RESERVED
