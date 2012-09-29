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
		// yuv 全部指定
		UINT	ycbcr;
		
		// ycb / ycr アクセス
		struct {
			USHORT	ycb;
			USHORT	ycr;
		};
		
		// yuv 個別指定
		struct {
			UCHAR	y;
			UCHAR	cb;		// -128〜127 ではなくて 0〜255
			UCHAR	alfa;
			UCHAR	cr;		// -128〜127 ではなくて 0〜255
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
	short	y;		//	画素(輝度    )データ (     0 〜 4096 )
	short	cb;		//	画素(色差(青))データ ( -2048 〜 2048 )
	short	cr;		//	画素(色差(赤))データ ( -2048 〜 2048 )
					//	画素データは範囲外に出ていることがあります
					//	また範囲内に収めなくてもかまいません
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
