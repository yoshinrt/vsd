/*****************************************************************************
	
	VSD -- vehicle data logger system  Copyright(C) by DDS
	
	vsd_filter_avs.cpp - vsd_filter for AviSynth
	
*****************************************************************************/

#include "StdAfx.h"

#include "avisynth.h"
#include "CVsdFilter.h"

/****************************************************************************/

enum {
	ARGID_CLIP,
	#define DEF_STR_PARAM( id, var, init, conf_name )			ARGID_ ## id,
	#include "def_str_param.h"
	#define DEF_TRACKBAR( id, init, min, max, name, conf_name )	ARGID_ ## id,
	#define DEF_TRACKBAR_N( id, init, min, max, name, conf_name )
	#include "def_trackbar.h"
	#define DEF_CHECKBOX( id, init, name, conf_name )			ARGID_ ## id,
	#define DEF_CHECKBOX_N( id, init, name, conf_name )
	#include "def_checkbox.h"
	#define DEF_SHADOW( id, init, conf_name )					ARGID_ ## id,
	#include "def_shadow.h"
	ARGID_MARK,
	ARGID_NUM
};

/****************************************************************************/

class CVsdFilterAvs : public GenericVideoFilter, CVsdFilter {
  public:
	CVsdFilterAvs(
		PClip _child,
		AVSValue args,
		IScriptEnvironment* env
	);
	
	~CVsdFilterAvs();
	PVideoFrame __stdcall GetFrame( int n, IScriptEnvironment* env );
	
	/////////////
	
	int GetIndex( int x, int y ){ return m_iBytesPerLine * y + x * 2; }
	
	// 仮想関数
	void         PutPixel( int iIndex, const PIXEL_YCA_ARG yc, int iAlfa );
	virtual void PutPixel( int x, int y, const PIXEL_YCA_ARG yc );
	virtual void FillLine( int x1, int y1, int x2, const PIXEL_YCA_ARG yc );
	virtual UINT PutImage( int x, int y, CVsdImage &img, UINT uAlign );
	
	virtual int	GetWidth( void )	{ return m_iWidth; }
	virtual int	GetHeight( void )	{ return m_iHeight; }
	virtual int	GetFrameMax( void )	{ return vi.num_frames; }
	virtual int	GetFrameCnt( void )	{ return m_iFrameCnt; }
	virtual double	GetFPS( void )	{ return ( double )vi.fps_numerator / vi.fps_denominator; }
	
	virtual void SetFrameMark( int iFrame );
	virtual int  GetFrameMark( int iFrame );
	
	void DispErrorMessage( LPCWSTR szMsg );
	
	// パラメータ
	int	m_iWidth, m_iHeight, m_iFrameCnt, m_iFrameMax;
	int m_iBytesPerLine;
	int *m_piMark;
	int m_iMarkCnt;
	
	BYTE	*m_pPlane;
	const char *m_szMark;
	IScriptEnvironment* m_env;
};

/*** コンストラクタ・デストラクタ *******************************************/

// param 指定
CVsdFilterAvs::CVsdFilterAvs(
	PClip _child,
	AVSValue args,
	IScriptEnvironment* env
) : GenericVideoFilter( _child ){
	
	const char *p;
	
	if( !vi.IsYUY2()) env->ThrowError( PROG_NAME ": requires YUY2 input.");
	
	// パラメータ初期化
	m_piParamT	= new int[ TRACK_N ];
	m_piParamC	= new int[ CHECK_N ];
	m_piParamS	= new int[ SHADOW_N ];
	m_piMark	= new int[ MAX_LAP ];
	m_iMarkCnt	= 0;
	
	m_env = NULL;
	
	// パラメータ初期値
	#define DEF_TRACKBAR( id, init, min, max, name, conf_name )	m_piParamT[ id ] = init;
	#include "def_trackbar.h"
	
	#define DEF_CHECKBOX( id, init, name, conf_name )	m_piParamC[ id ] = init;
	#include "def_checkbox.h"
	
	#define DEF_SHADOW( id, init, conf_name )	m_piParamS[ id ] = init;
	#include "def_shadow.h"
	
	// 引数指定により初期化
	#define DEF_TRACKBAR( id, init, min, max, name, conf_name ) \
		if( args[ ARGID_ ## id ].Defined()) m_piParamT[ id ] = args[ ARGID_ ## id ].AsInt();
	#define DEF_TRACKBAR_N( id, init, min, max, name, conf_name )
	#include "def_trackbar.h"
	
	#define DEF_CHECKBOX( id, init, name, conf_name ) \
		if( args[ ARGID_ ## id ].Defined()) m_piParamC[ id ] = args[ ARGID_ ## id ].AsInt();
	#define DEF_CHECKBOX_N( id, init, name, conf_name )
	#include "def_checkbox.h"
	
	#define DEF_SHADOW( id, init, conf_name ) \
		if( args[ ARGID_ ## id ].Defined()) m_piParamS[ id ] = args[ ARGID_ ## id ].AsInt();
	#include "def_shadow.h"
	
	// mark= 引数処理
	if( p = args[ ARGID_MARK ].AsString( NULL )) ParseMarkStr( p );
	
	// ログリード
	#ifndef GPS_ONLY
		if( p = args[ ARGID_STRPARAM_LOGFILE ].AsString( NULL )){
			const char *szReader = args[ ARGID_STRPARAM_LOGFILE_READER ].AsString( NULL );
			if( !ReadLog( m_VsdLog, p, szReader )) env->ThrowError( PROG_NAME ": read log \"%s\" failed.", p );
		}
	#endif
	
	// GPS ログリード
	if( p = args[ ARGID_STRPARAM_GPSFILE ].AsString( NULL )){
			const char *szReader = args[ ARGID_STRPARAM_GPSFILE_READER ].AsString( NULL );
		if( !ReadLog( m_GPSLog, p, szReader )) env->ThrowError( PROG_NAME ": read GPS log \"%s\" failed.", p );
	}
	
	// スキンロード
	if( p = args[ ARGID_STRPARAM_SKINFILE ].AsString( NULL )){
		SetSkinFile( p );
	}else{
		SetSkinFile( DEFAULT_SKIN );
	}
}

CVsdFilterAvs::~CVsdFilterAvs(){
	delete [] m_piParamT;
	delete [] m_piParamC;
	delete [] m_piParamS;
	delete [] m_piMark;
}

/*** PutPixel ***************************************************************/

/* 変換式
Y  =  0.299R+0.587G+0.114B
Cr =  0.500R-0.419G-0.081B
Cb = -0.169R-0.332G+0.500B

R = Y+1.402Cr 
G = Y-0.714Cr-0.344Cb 
B = Y+1.772Cb 
*/

inline void CVsdFilterAvs::PutPixel( int iIndex, const PIXEL_YCA_ARG yc, int iAlfa ){
	
	m_pPlane[ iIndex + 0 ] = ( PIXEL_t )(
		yc.y + ((  m_pPlane[ iIndex + 0 ] * iAlfa ) >> 8 )
	);
	m_pPlane[ iIndex + 1 ] = ( PIXEL_t )(
		(( iIndex & 2 )? yc.cr : yc.cb ) + (((( int )m_pPlane[ iIndex + 1 ] - 0x80 ) * iAlfa ) >> 8 )
	);
}

inline void CVsdFilterAvs::PutPixel( int x, int y, const PIXEL_YCA_ARG yc ){
	
	int	iIndex	= GetIndex( x, y );
	
	PIXEL_YCA yca = yc;
	int iAlfa = yca.alfa;
	yca.alfa = yca.y;
	
	if( iAlfa ){
		iAlfa += iAlfa >> 7;
		PutPixel( iIndex, yc, iAlfa );
	}else{
		*( USHORT *)( m_pPlane + iIndex ) = ( x & 1 ) ? yca.ycr : yca.ycb;
	}
}

inline void CVsdFilterAvs::FillLine( int x1, int y1, int x2, const PIXEL_YCA_ARG yc ){
	
	int iIndex = GetIndex( x1, y1 );
	
	PIXEL_YCA yca = yc;
	int iAlfa = yca.alfa;
	yca.alfa = yca.y;
	
	if( iAlfa ){
		iAlfa += iAlfa >> 7;
		for( int x = x1; x <= x2; ++x, iIndex += 2 ){
			PutPixel( iIndex, yc, iAlfa );
		}
	}else{
		// x1, x2 が半端な pixel なら，それだけ先に処理
		if( x1 & 1 ){
			*( USHORT *)( m_pPlane + iIndex ) = yca.ycr;
			++x1;
			iIndex += 2;
		}
		if( !( x2 & 1 )){
			*( USHORT *)( m_pPlane + GetIndex( x2, y1 )) = yca.ycb;
			--x2;
		}
		for( int x = x1; x <= x2; x += 2, iIndex += 4 ){
			*( UINT *)( m_pPlane + iIndex ) = yca.ycbcr;
		}
	}
}

/*** PutImage ***************************************************************/

UINT CVsdFilterAvs::PutImage(
	int x, int y, CVsdImage &img, UINT uAlign
){
	if( uAlign & ALIGN_HCENTER ){
		x -= img.m_iWidth / 2;
	}else if( uAlign & ALIGN_RIGHT ){
		x -= img.m_iWidth;
	}
	
	if( uAlign & ALIGN_VCENTER ){
		y -= img.m_iHeight / 2;
	}else if( uAlign & ALIGN_BOTTOM ){
		y -= img.m_iHeight;
	}
	
	int xst = ( -x <= img.m_iOffsX ) ? img.m_iOffsX : -x;
	int yst = ( -y <= img.m_iOffsY ) ? img.m_iOffsY : -y;
	int xed = x + img.m_iOffsX + img.m_iRawWidth  <= GetWidth()  ? img.m_iOffsX + img.m_iRawWidth  : GetWidth()  - x;
	int yed = y + img.m_iOffsY + img.m_iRawHeight <= GetHeight() ? img.m_iOffsY + img.m_iRawHeight : GetHeight() - y;
	
	#ifdef _OPENMP_AVS
		#pragma omp parallel for
	#endif
	for( int y1 = yst; y1 < yed; ++y1 ){
		
		int	iIndex = GetIndex( x + xst, y + y1 );
		int x1 = xst;
		
		// 先頭の半端な 1pixel 処理
		if( iIndex & 2 ){
			PIXEL_YCA yc( img.GetPixel0( x1, y1 ));
			PutPixel( x1, y1, yc );
			iIndex += 2;
			++x1;
		}
		for( ; x1 < xed - 1; x1 += 2, iIndex += 4 ){
			PIXEL_YCA yc0( img.GetPixel0( x1    , y1 ));
			PIXEL_YCA yc1( img.GetPixel0( x1 + 1, y1 ));
			
			if( yc0.alfa == 0 && yc1.alfa == 0 ){
				// 2ピクセルが共に 100% 不透明
				yc0.cr	= ( yc0.cr + yc1.cr ) >> 1;
				yc0.cb	= ( yc0.cb + yc1.cb ) >> 1;
				yc0.alfa= yc1.y;
				
				*( UINT *)( m_pPlane + iIndex ) = yc0.ycbcr;
			}else{
				// そうではなかった
				PutPixel( x + x1,     y + y1, yc0 );
				PutPixel( x + x1 + 1, y + y1, yc1 );
			}
		}
		// 後端の半端な 1pixel 処理
		if( x1 < xed ){
			PIXEL_YCA yc( img.GetPixel0( x1, y1 ));
			PutPixel( x + x1, y + y1, yc );
		}
	}
	
	return ERR_OK;
}

/*** エラーメッセージ *******************************************************/

void CVsdFilterAvs::DispErrorMessage( LPCWSTR szMsg ){
	if( m_env ){
		char *p = NULL;
		m_env->ThrowError( "%s", StringNew( p, szMsg ));
		delete [] p;
	}else{
		MessageBoxW( NULL, szMsg, PROG_NAME_J_W, MB_ICONWARNING );
	}
}

/*** フレームをマーク *******************************************************/

void CVsdFilterAvs::SetFrameMark( int iFrame ){
	m_piMark[ m_iMarkCnt++ ] = iFrame;
	m_piMark[ m_iMarkCnt   ] = -1;
};

int CVsdFilterAvs::GetFrameMark( int iFrame ){
	int	i;
	for( i = 0; m_piMark[ i ] < iFrame && m_piMark[ i ] >= 0; ++i );
	return m_piMark[ i ];
}

/****************************************************************************/

PVideoFrame __stdcall CVsdFilterAvs::GetFrame( int n, IScriptEnvironment* env ){
	PVideoFrame src = child->GetFrame( n, env );
	env->MakeWritable(&src);
	
	m_iBytesPerLine		= src->GetPitch();
	m_iWidth			= src->GetRowSize()>>1; //case of YUY2
	m_iHeight			= src->GetHeight();
	m_iFrameCnt			= n;
	m_env				= env;
	
	m_pPlane			= src->GetWritePtr();
	
	DrawVSD();
	
	return src;
}

AVSValue __cdecl Create_VSDFilter( AVSValue args, void* user_data, IScriptEnvironment* env ){
	return new CVsdFilterAvs(
		args[0].AsClip(),
		args,
		env
	);
}

extern "C" __declspec( dllexport ) const char* __stdcall AvisynthPluginInit2( IScriptEnvironment* env ){
	
	env->AddFunction( "VSDFilter",
		"c"
		#define DEF_STR_PARAM( id, var, init, conf_name )			"[" conf_name "]s"
		#include "def_str_param.h"
		#define DEF_TRACKBAR( id, init, min, max, name, conf_name )	"[" conf_name "]i"
		#define DEF_TRACKBAR_N( id, init, min, max, name, conf_name )
		#include "def_trackbar.h"
		#define DEF_CHECKBOX( id, init, name, conf_name )			"[" conf_name "]i"
		#define DEF_CHECKBOX_N( id, init, name, conf_name )
		#include "def_checkbox.h"
		#define DEF_SHADOW( id, init, conf_name )					"[" conf_name "]i"
		#include "def_shadow.h"
		"[mark]s",
		Create_VSDFilter, 0
	);
	
	return PROG_NAME_LONG " " PROG_VERSION;
}
