/*****************************************************************************
	
	VSD -- vehicle data logger system  Copyright(C) by DDS
	
	pixel.h - PIXEL_YC / PIXEL_YCA structure
	
*****************************************************************************/

#ifndef _PIXEL_H
#define _PIXEL_H

/*** new type ***************************************************************/

#ifdef AVS_PLUGIN
typedef UCHAR	PIXEL_t;

typedef struct {
	union {
		// yuv 個別指定
		struct {
			UCHAR	y;
			UCHAR	cb;
			UCHAR	y1;
			UCHAR	cr;
		};
		
		// ycb / ycr アクセス
		struct {
			USHORT	ycb;
			USHORT	ycr;
		};
		
		// yuv 全部指定
		UINT	ycbcr;
	};
	USHORT	alfa;
} PIXEL_YCA;

static inline Color2YCA( PIXEL_YCA &yca, int iColor ){
	hogefuga
}

#else // !AVS_PLUGIN
typedef short	PIXEL_t;

class PIXEL_YCA {
  public:
	short	y;		//	画素(輝度    )データ (     0 ～ 4096 )
	short	cb;		//	画素(色差(青))データ ( -2048 ～ 2048 )
	short	cr;		//	画素(色差(赤))データ ( -2048 ～ 2048 )
					//	画素データは範囲外に出ていることがあります
					//	また範囲内に収めなくてもかまいません
	USHORT	alfa;	// 0-256
	
	PIXEL_YCA(){}
	
	PIXEL_YCA( UINT uColor ){
		Set( uColor );
	}
	
	void Set( UINT uColor ){
		int a = ( uColor >> 24 ) & 0xFF;
		int r = ( uColor >> 16 ) & 0xFF;
		int g = ( uColor >>  8 ) & 0xFF;
		int b = ( uColor       ) & 0xFF;
		a += a >> 7;	// 四捨五入のつもり
		
		double dAlfa = ( 256 - a ) * ( 4096.0 / 256 / 255 );
		
		y		= ( int )(( 0.299 * r + 0.587 * g + 0.114 * b ) * dAlfa );
		cb		= ( int )((-0.169 * r - 0.331 * g + 0.500 * b ) * dAlfa );
		cr		= ( int )(( 0.500 * r - 0.419 * g - 0.081 * b ) * dAlfa );
		alfa	= a;
	}
};
#endif // !AVS_PLUGIN

#endif // _PIXEL_H
