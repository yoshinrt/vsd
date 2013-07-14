/*****************************************************************************
	
	VSD -- vehicle data logger system  Copyright(C) by DDS
	
	vsd_filter_avs.cpp - vsd_filter for AviSynth
	
*****************************************************************************/

#include "StdAfx.h"

#include "CVsdFilter.h"
#include "rev_num.h"

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

/*** コンストラクタ・デストラクタ *******************************************/

// param 指定
CVsdFilter::CVsdFilter(
	PClip _child,
	AVSValue args,
	IScriptEnvironment* env
) : GenericVideoFilter( _child ){
	
	Constructor();	// 基本クラスのコンストラクタ
	
	const char *p;
	
	if( !vi.IsYUY2()) env->ThrowError( PROG_NAME ": requires YUY2 input.");
	
	// パラメータ初期化
	m_piParamT	= new int[ TRACK_N ];
	m_piParamC	= new int[ CHECK_N ];
	m_piParamS	= new int[ SHADOW_N ];
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
	
	// ラップチャート
	if( p = args[ ARGID_STRPARAM_LAPCHART ].AsString( NULL )){
		if( !LapChartRead( p )) env->ThrowError( PROG_NAME ": read lap chart \"%s\" failed.", p );
	}
	
	// ログリード
	if( p = args[ ARGID_STRPARAM_LOGFILE ].AsString( NULL )){
		const char *szReader = args[ ARGID_STRPARAM_LOGFILE_READER ].AsString( NULL );
		if( !ReadLog( m_VsdLog, p, szReader )) env->ThrowError( PROG_NAME ": read log \"%s\" failed.", p );
	}
	
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

CVsdFilter::~CVsdFilter(){
	Destructor();	// 基本クラスのデストラクタ
	
	delete [] m_piParamT;
	delete [] m_piParamC;
	delete [] m_piParamS;
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

inline void CVsdFilter::PutPixel( int iIndex, const PIXEL_YCA_ARG yc, int iAlfa ){
	
	m_pPlane[ iIndex + 0 ] = ( PIXEL_t )(
		yc.y + ((  m_pPlane[ iIndex + 0 ] * iAlfa ) >> 8 )
	);
	m_pPlane[ iIndex + 1 ] = ( PIXEL_t )(
		(( iIndex & 2 )? yc.cr : yc.cb ) + (((( int )m_pPlane[ iIndex + 1 ] - 0x80 ) * iAlfa ) >> 8 )
	);
}

void CVsdFilter::PutPixel( int x, int y, const PIXEL_YCA_ARG yc ){
	
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

void CVsdFilter::FillLine( int x1, int y1, int x2, const PIXEL_YCA_ARG yc ){
	
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

UINT CVsdFilter::PutImage0(
	int x, int y, CVsdImage &img,
	int ix_st, int iy_st, int ix_ed, int iy_ed
){
	#ifdef _OPENMP_AVS
		#pragma omp parallel for
	#endif
	for( int iy = iy_st; iy < iy_ed; ++iy, ++y ){
		
		int	iIndex = GetIndex( x, y );
		int ix = ix_st;
		int sx = x;
		
		// 先頭の半端な 1pixel 処理
		if( iIndex & 2 ){
			PIXEL_YCA yc( img.GetPixelRaw( ix, iy ));
			PutPixel( x, y, yc );
			iIndex += 2;
			++ix;
			++sx;
		}
		for( ; ix < ix_ed - 1; ix += 2, sx += 2, iIndex += 4 ){
			PIXEL_YCA yc0( img.GetPixelRaw( ix    , iy ));
			PIXEL_YCA yc1( img.GetPixelRaw( ix + 1, iy ));
			
			if( yc0.alfa == 0 && yc1.alfa == 0 ){
				// 2ピクセルが共に 100% 不透明
				yc0.cr	= ( yc0.cr + yc1.cr ) >> 1;
				yc0.cb	= ( yc0.cb + yc1.cb ) >> 1;
				yc0.alfa= yc1.y;
				
				*( UINT *)( m_pPlane + iIndex ) = yc0.ycbcr;
			}else{
				// そうではなかった
				PutPixel( sx,     y, yc0 );
				PutPixel( sx + 1, y, yc1 );
			}
		}
		// 後端の半端な 1pixel 処理
		if( ix < ix_ed ){
			PIXEL_YCA yc( img.GetPixelRaw( ix, iy ));
			PutPixel( sx, y, yc );
		}
	}
	
	return ERR_OK;
}

/*** エラーメッセージ *******************************************************/

void CVsdFilter::DispErrorMessage( LPCWSTR szMsg ){
	if( m_env ){
		char *p = NULL;
		m_env->ThrowError( "%s", StringNew( p, szMsg ));
		delete [] p;
	}else{
		MessageBoxW( NULL, szMsg, PROG_NAME_J_W, MB_ICONWARNING );
	}
}

/*** フレームをマーク *******************************************************/

void CVsdFilter::SetFrameMark( int iFrame ){
	m_iMark.push_back( iFrame );
};

int CVsdFilter::GetFrameMark( int iFrame ){
	UINT	u;
	for( u = 0; u < m_iMark.size(); ++u ){
		if( m_iMark[ u ] == iFrame ) return iFrame;
	}
	return -1;
}

/*** DrawSyncInfo ダミー ****************************************************/

void CVsdFilter::DrawSyncInfo( int x, int y, CVsdFont &Font, UINT uAlign ){}

/****************************************************************************/

PVideoFrame __stdcall CVsdFilter::GetFrame( int n, IScriptEnvironment* env ){
	m_SrcFrame = child->GetFrame( n, env );
	env->MakeWritable(&m_SrcFrame);
	
	m_iBytesPerLine		= m_SrcFrame->GetPitch();
	m_iFrameCnt			= n;
	m_env				= env;
	
	m_pPlane			= m_SrcFrame->GetWritePtr();
	
	DrawVSD();
	
	return m_SrcFrame;
}

AVSValue __cdecl Create_VSDFilter( AVSValue args, void* user_data, IScriptEnvironment* env ){
	return new CVsdFilter(
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
	
	return PROG_NAME_LONG " " PROG_REVISION_STR;
}
