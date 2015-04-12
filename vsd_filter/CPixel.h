/*****************************************************************************
	
	VSD -- vehicle data logger system  Copyright(C) by DDS
	
	CPixel.h - CPixel structure
	
*****************************************************************************/

#pragma once 

								//    AARRGGBB
#define	color_black			CPixel( 0x00000000 )
#define	color_white			CPixel( 0x00FFFFFF )
#define	color_blue			CPixel( 0x000000FF )
#define	color_red			CPixel( 0x00FF0000 )
#define	color_cyan			CPixel( 0x0000FFFF )
#define	color_orange		CPixel( 0x00FF4000 )
#define	color_transparent	CPixel( 0xFF000000 )

/*** new type ***************************************************************/

#define CPixelBase	CPixelImg
#include "CPixelBase.h"

/****************************************************************************/

#ifdef AVS_PLUGIN
	typedef UCHAR		PIXEL_t;
	typedef CPixelImg	CPixel;
	typedef	const CPixel	CPixelArg;
	
#else // !AVS_PLUGIN
	typedef short	PIXEL_t;
	
	#define CPixelBase	CPixel
	#define PIXEL_AVU
	#include "CPixelBase.h"
	
	typedef	const CPixel& CPixelArg;
#endif // !AVS_PLUGIN
