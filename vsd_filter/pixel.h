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
	
	PIXEL_RABY(){}
	~PIXEL_RABY(){}
	
	static double GetY(  int r, int g, int b ){ return  0.299 * r + 0.587 * g + 0.114 * b; }
	static double GetCb( int r, int g, int b ){ return -0.169 * r - 0.331 * g + 0.500 * b; }
	static double GetCr( int r, int g, int b ){ return  0.500 * r - 0.419 * g - 0.081 * b; }
	
	static inline UINT Argb2Raby( UINT uColor ){
		int a = ( uColor >> 24 );
		int r = ( uColor >> 16 ) & 0xFF;
		int g = ( uColor >>  8 ) & 0xFF;
		int b = ( uColor       ) & 0xFF;
		
		return Argb2Raby( a, r, g, b );
	}
	
	static inline UINT Argb2Raby( int a, int r, int g, int b ){
		double dAlfa = ( 255 - a ) / 255.0;
		
		return (
			((( int )( GetCr( r, g, b ) * dAlfa )        ) << 24 ) |
			( a << 16 ) |
			((( int )( GetCb( r, g, b ) * dAlfa ) & 0xFF ) <<  8 ) |
			((( int )( GetY(  r, g, b ) * dAlfa ) & 0xFF )       )
		) ^ 0x80008000;
	}
	
	void Set( UINT uColor ){
		int a = ( uColor >> 24 );
		int r = ( uColor >> 16 ) & 0xFF;
		int g = ( uColor >>  8 ) & 0xFF;
		int b = ( uColor       ) & 0xFF;
		
		return Set( a, r, g, b );
	}
	
	void Set( int a, int r, int g, int b ){
		double dAlfa = ( 255 - a ) / 255.0;
		
		y	= ( int )( GetY(  r, g, b ) * dAlfa );
		cb	= ( int )( GetCb( r, g, b ) * dAlfa ) ^ 0x80;
		cr	= ( int )( GetCr( r, g, b ) * dAlfa ) ^ 0x80;
		alfa= a;
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

typedef	PIXEL_YCA PIXEL_YCA_ARG;

#else // !AVS_PLUGIN
typedef short	PIXEL_t;

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

typedef	PIXEL_YCA& PIXEL_YCA_ARG;

#endif // !AVS_PLUGIN
