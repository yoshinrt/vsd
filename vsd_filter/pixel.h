/*****************************************************************************
	
	VSD -- vehicle data logger system  Copyright(C) by DDS
	
	pixel.h - PIXEL_YC / PIXEL_YCA structure
	
*****************************************************************************/

#pragma once 

/*** new type ***************************************************************/

#ifdef AVS_PLUGIN
typedef UCHAR	PIXEL_t;

class PIXEL_YCA {
  public:
	union {
		// yuv ‘S•”Žw’è
		UINT	ycbcr;
		
		// ycb / ycr ƒAƒNƒZƒX
		struct {
			USHORT	ycb;
			USHORT	ycr;
		};
		
		// yuv ŒÂ•ÊŽw’è
		struct {
			UCHAR	y;
			UCHAR	cb;		// -128`127 ‚Å‚Í‚È‚­‚Ä 0`255
			UCHAR	alfa;
			UCHAR	cr;		// -128`127 ‚Å‚Í‚È‚­‚Ä 0`255
		};
	};
	
	PIXEL_YCA(){};
	
	PIXEL_YCA( UINT uColor ){
		Set( uColor );
	}
	
	void Set( UINT uColor ){
		ycbcr = uColor;
	}
};

#define PIXEL_YCA_ARG	PIXEL_YCA

#else // !AVS_PLUGIN
typedef short	PIXEL_t;

#define PIXEL_YCA_ARG	PIXEL_YCA&

class PIXEL_YCA {
  public:
	short	y;		//	‰æ‘f(‹P“x    )ƒf[ƒ^ (     0 ` 4096 )
	short	cb;		//	‰æ‘f(F·(Â))ƒf[ƒ^ ( -2048 ` 2048 )
	short	cr;		//	‰æ‘f(F·(Ô))ƒf[ƒ^ ( -2048 ` 2048 )
					//	‰æ‘fƒf[ƒ^‚Í”ÍˆÍŠO‚Éo‚Ä‚¢‚é‚±‚Æ‚ª‚ ‚è‚Ü‚·
					//	‚Ü‚½”ÍˆÍ“à‚ÉŽû‚ß‚È‚­‚Ä‚à‚©‚Ü‚¢‚Ü‚¹‚ñ
	USHORT	alfa;	// 0-256
	
	PIXEL_YCA(){}
	
	PIXEL_YCA( UINT uColor ){
		Set( uColor );
	}
	
	void Set( UINT uColor ){
		uColor ^= 0x80008000;
		
		int ir = ( char )( uColor >> 24 );
		int ia = ( uColor >> 16 ) & 0xFF;
		int ib = ( char )( uColor >>  8 );
		int iy = ( uColor       ) & 0xFF;
		
		y		= ( iy << 4 ) + ( iy >> 4 );
		cb		= ( ib << 4 ) + ( ib >> 4 );
		cr		= ( ir << 4 ) + ( ir >> 4 );
		alfa	= ia + ( ia >> 7 );
	}
};
#endif // !AVS_PLUGIN
