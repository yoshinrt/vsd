/************************************************************************/
/*      H8/300H Monitor Program (Normal Mode)           Ver. 2.0B       */
/*              Copyright (C) 2003 Renesas Technology Corp.             */
/************************************************************************/
#include "3664s.h"                              /*                      */
#pragma global_register(monitor=R5)             /* Return Address       */
void (*monitor)(void);                          /*                      */
/************************************************************************/
/*      User Initialize Module                                          */
/*              Input   R5  <-- Return Address                          */
/*              Output  Nothing                                         */
/*              Used Stack Area --> 0(0) Byte                           */
/************************************************************************/
#pragma noregsave(INITIALIZE)                   /* Non Register Save    */
void INITIALIZE(void)                           /*                      */
{                                               /*                      */
	// init Data, BSS
	
	unsigned *uSrc, *uDst;
	
	for(
		uSrc = __sectop( "D" ), uDst = __sectop( "R" );
		( unsigned )uSrc < ( unsigned )__secend( "D" );
		++uSrc, ++uDst
	){
		*uDst = *uSrc;
	}
	
	for(
		uDst = __sectop( "B" );
		( unsigned )uDst < ( unsigned )__secend( "B" );
		++uDst
	){
		*uDst = 0;
	}
	
	if( IO.PDR5.BIT.B5 ) monitor();			/* Goto Monitor Program */
	
	main();
}
