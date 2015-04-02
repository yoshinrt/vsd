/*****************************************************************************
	
	VSD -- vehicle data logger system  Copyright(C) by DDS
	
	pixel.h - PIXEL_YC / PIXEL_YCA structure
	
*****************************************************************************/

#pragma once 

									//   AARRGGBB
#define	color_black			PIXEL_YCA( 0x00000000 )
#define	color_white			PIXEL_YCA( 0x00FFFFFF )
#define	color_blue			PIXEL_YCA( 0x000000FF )
#define	color_red			PIXEL_YCA( 0x00FF0000 )
#define	color_cyan			PIXEL_YCA( 0x0000FFFF )
#define	color_orange		PIXEL_YCA( 0x00FF4000 )
#define	color_masenta		PIXEL_YCA( 0x00FF00FF )
#define	color_transparent	PIXEL_YCA( 0xFF000000 )

/*** new type ***************************************************************/

#define PIXEL	PIXEL_RABY
#include "pixel_base.h"

/****************************************************************************/

#ifdef AVS_PLUGIN
typedef UCHAR		PIXEL_t;
typedef PIXEL_RABY	PIXEL_YCA;
typedef	PIXEL_YCA	PIXEL_YCA_ARG;

#else // !AVS_PLUGIN
typedef short	PIXEL_t;

#define PIXEL	PIXEL_YCA
#define PIXEL_AVU
#include "pixel_base.h"

typedef	PIXEL_YCA& PIXEL_YCA_ARG;

#endif // !AVS_PLUGIN
