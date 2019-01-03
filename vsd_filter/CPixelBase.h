/*****************************************************************************
	
	VSD -- vehicle data logger system  Copyright(C) by DDS
	
	CPixelBase.h - PIXEL_YC / CPixel structure
	
*****************************************************************************/

/*** new type ***************************************************************/

class CPixelBase {
  public:
	#ifdef PIXEL_AVU
		short	y;		//	画素(輝度    )データ (     0 ～ 4096 )
		short	cb;		//	画素(色差(青))データ ( -2048 ～ 2048 )
		short	cr;		//	画素(色差(赤))データ ( -2048 ～ 2048 )
						//	画素データは範囲外に出ていることがあります
						//	また範囲内に収めなくてもかまいません
		USHORT	alfa;	// 0-256
		
		void Set( int a, int r, int g, int b ){
			int iAlfa = 256 - ( a + ( a >> 7 ));
			
			y	= ( GetY(  r, g, b ) * iAlfa ) >> 14;
			cb	= ( GetCb( r, g, b ) * iAlfa ) >> 14;
			cr	= ( GetCr( r, g, b ) * iAlfa ) >> 14;
			alfa= a + ( a >> 7 );
		}
		
		CPixelBase( CPixelImg yc ){
			int iy = yc.y;
			int ib = ( char )( yc.cb ^ 0x80 );
			int ir = ( char )( yc.cr ^ 0x80 );
			int ia = yc.alfa;
			
			y		= ( iy << 4 ) + ( iy >> 4 );
			cb		= ( ib << 4 ) + ( ib >> 4 );
			cr		= ( ir << 4 ) + ( ir >> 4 );
			alfa	= ia + ( ia >> 7 );
		}
	#else
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
		
		void Set( int a, int r, int g, int b ){
			int iAlfa = 256 - ( a + ( a >> 7 ));
			
			y	=  ( GetY(  r, g, b ) * iAlfa ) >> 18;
			cb	= (( GetCb( r, g, b ) * iAlfa ) >> 18 ) ^ 0x80;
			cr	= (( GetCr( r, g, b ) * iAlfa ) >> 18 ) ^ 0x80;
			alfa= a;
		}
	#endif
	
	CPixelBase(){}
	CPixelBase( UINT uColor ){ Set( uColor ); }
	
	void Set( UINT uColor ){
		int a = ( uColor >> 24 );
		int r = ( uColor >> 16 ) & 0xFF;
		int g = ( uColor >>  8 ) & 0xFF;
		int b = ( uColor       ) & 0xFF;
		
		return Set( a, r, g, b );
	}
	
	// new & ブレンド
	CPixelBase( CPixelBase yc0, CPixelBase yc1, UINT uAlfa, UINT uAlfaMax ){
		UINT uAlfaM = uAlfaMax - uAlfa;
		
		y	= ( yc0.y  * uAlfaM + yc1.y  * uAlfa ) / uAlfaMax;
		cb	= ( yc0.cb * uAlfaM + yc1.cb * uAlfa ) / uAlfaMax;
		cr	= ( yc0.cr * uAlfaM + yc1.cr * uAlfa ) / uAlfaMax;
		alfa= ( yc0.alfa * uAlfaM + yc1.alfa * uAlfa ) / uAlfaMax;
	}
	
	// ITU-R BT.601 / ITU-R BT.709
  private:
	#ifdef PIXEL_AVU
		// フルレンジ
		static int GetY(  int r, int g, int b ){ return  306 * r + 601 * g + 117 * b; }
		static int GetCb( int r, int g, int b ){ return -173 * r - 339 * g + 512 * b; }
		static int GetCr( int r, int g, int b ){ return  512 * r - 429 * g -  83 * b; }
	#else
		// Y[16:235], UV[16:240]
		static int GetY(  int r, int g, int b ){ return  263 * r + 517 * g + 100 * b + 16 * 1024; }
		static int GetCb( int r, int g, int b ){ return -152 * r - 298 * g + 450 * b; }
		static int GetCr( int r, int g, int b ){ return  450 * r - 377 * g -  73 * b; }
	#endif
};

#undef PIXEL_AVU
#undef CPixelBase
