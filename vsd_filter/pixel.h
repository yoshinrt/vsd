/*****************************************************************************
	
	VSD -- vehicle data logger system  Copyright(C) by DDS
	
	pixel.h - PIXEL_YC / PIXEL_YCA structure
	
*****************************************************************************/

#pragma once 

/*** new type ***************************************************************/

typedef UINT	tRABY;

class PIXEL_RABY {
  public:
	union {
		UINT	raby;
		struct {
			UCHAR	y;	// UCHAR
			UCHAR	b;	// * alfa 済み，signed だけど +128 offset 済み
			UCHAR	a;	// * alfa 済み，UCHAR
			UCHAR	r;	// * alfa 済み，signed だけど +128 offset 済み
		};
	};
	
	PIXEL_RABY(){}
	~PIXEL_RABY(){}
	
	static inline UINT Argb2Raby( UINT uColor ){
		int a = ( uColor >> 24 );
		int r = ( uColor >> 16 ) & 0xFF;
		int g = ( uColor >>  8 ) & 0xFF;
		int b = ( uColor       ) & 0xFF;
		
		return Argb2Raby( a, r, g, b );
	}
	
	static inline UINT Argb2Raby(
		int a, int r, int g, int b
	){
		double dAlfa = ( 255 - a ) / 255.0;
		
		return (
			((( int )(( 0.500 * r - 0.419 * g - 0.081 * b ) * dAlfa )        ) << 24 ) |
			( a << 16 ) |
			((( int )((-0.169 * r - 0.331 * g + 0.500 * b ) * dAlfa ) & 0xFF ) <<  8 ) |
			((( int )(( 0.299 * r + 0.587 * g + 0.114 * b ) * dAlfa ) & 0xFF )       )
		) ^ 0x80008000;
	}
};

/****************************************************************************/

											//   AARRGGBB
#define	color_black		PIXEL_RABY::Argb2Raby( 0x00000000 )
#define	color_white		PIXEL_RABY::Argb2Raby( 0x00FFFFFF )
#define	color_blue		PIXEL_RABY::Argb2Raby( 0x000000FF )
#define	color_red		PIXEL_RABY::Argb2Raby( 0x00FF0000 )
#define	color_cyan		PIXEL_RABY::Argb2Raby( 0x0000FFFF )
#define	color_orange	PIXEL_RABY::Argb2Raby( 0x00FF4000 )
#define	color_gray_a	PIXEL_RABY::Argb2Raby( 0x80404040 )
#define	color_black_a	PIXEL_RABY::Argb2Raby( 0x40000000 )
#define	color_green		PIXEL_RABY::Argb2Raby( 0x0000FF00 )
#define	color_masenta	PIXEL_RABY::Argb2Raby( 0x00FF00FF )

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
			UCHAR	cb;		// -128～127 ではなくて 0～255
			UCHAR	alfa;
			UCHAR	cr;		// -128～127 ではなくて 0～255
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
