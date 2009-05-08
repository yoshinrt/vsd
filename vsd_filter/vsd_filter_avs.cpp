/*****************************************************************************
	
	VSD -- vehicle data logger system  Copyright(C) by DDS
	
	vsd_filter_avs.cpp - vsd_filter for AviSynth
	$Id$
	
*****************************************************************************/

#include <windows.h>
#include "avisynth.h"

#include "dds.h"
#include "dds_lib/dds_lib.h"
#include "CVsdFilter.h"

#define PROG_NAME	"VSDFilter"

/****************************************************************************/

enum {
	ARGID_CLIP,
	ARGID_LOG_FILE,
	ARGID_PARAM_FILE,
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
	virtual void PutPixel( int x, int y, const PIXEL_YC &yc, UINT uFlag );
	
	virtual int	GetWidth( void )	{ return m_iWidth; }
	virtual int	GetHeight( void )	{ return m_iHeight; }
	virtual int	GetFrameMax( void )	{ return vi.num_frames; }
	virtual int	GetFrameCnt( void )	{ return m_iFrameCnt; }
	
	virtual void SetFrameMark( int iFrame );
	virtual void CalcLapTime( void );
	
	// パラメータ
	int	m_iWidth, m_iHeight, m_iFrameCnt, m_iFrameMax;
	int m_iBytesPerLine;
	
	BYTE	*m_pPlane;
	const char *m_szMark;
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
	m_dVideoFPS = ( double )vi.fps_numerator / vi.fps_denominator;
	
	// パラメータ初期値
	#define DEF_TRACKBAR( id, init, min, max, name, conf_name )	m_piParamT[ id ] = init;
	#include "def_trackbar.h"
	
	#define DEF_CHECKBOX( id, init, name, conf_name )	m_piParamC[ id ] = init;
	#include "def_checkbox.h"
	
	#define DEF_SHADOW( id, init, conf_name )	m_piParamS[ id ] = init;
	#include "def_shadow.h"
	
	// パラメータファイルにより初期化
	if( p = args[ ARGID_PARAM_FILE ].AsString( NULL )) if( !ConfigLoad( p ))
		env->ThrowError( PROG_NAME ": read cfg \"%s\" failed.", p );
	
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
	
	m_piParamT[ TRACK_VSt2 ] = m_piParamT[ TRACK_VSt ] % 100;
	m_piParamT[ TRACK_VSt  ] /= 100;
	m_piParamT[ TRACK_VEd2 ] = m_piParamT[ TRACK_VEd ] % 100;
	m_piParamT[ TRACK_VEd  ] /= 100;
	m_piParamT[ TRACK_LSt2 ] = m_piParamT[ TRACK_LSt ] % 100;
	m_piParamT[ TRACK_LSt  ] /= 100;
	m_piParamT[ TRACK_LEd2 ] = m_piParamT[ TRACK_LEd ] % 100;
	m_piParamT[ TRACK_LEd  ] /= 100;
	
	// mark= 引数処理
	if( p = args[ ARGID_MARK ].AsString( NULL )) ParseMarkStr( p );
	
	// ログリード
	if( p = args[ ARGID_LOG_FILE ].AsString( NULL )) if( !ReadLog( p ))
		env->ThrowError( PROG_NAME ": read log \"%s\" failed.", p );
}

CVsdFilterAvs::~CVsdFilterAvs(){
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

void CVsdFilterAvs::PutPixel( int x, int y, const PIXEL_YC &yc, UINT uFlag ){
	
	if( uFlag & IMG_POLYGON ){
		// ポリゴン描画
		if( x > m_Polygon[ y ].iRight ) m_Polygon[ y ].iRight = x;
		if( x < m_Polygon[ y ].iLeft  ) m_Polygon[ y ].iLeft  = x;
	}else{
		if( 0 <= x && x < GetWidth() && 0 <= y && y < GetHeight() ){
			int	iIndex	= GetIndex( x, y );
			
			if( uFlag & IMG_ALFA ){
				m_pPlane[ iIndex + 0 ] = ( yc.y + m_pPlane[ iIndex + 0 ] ) / 2;
				m_pPlane[ iIndex + 1 ] = ((( x & 1 )? yc.cr : yc.cb ) + m_pPlane[ iIndex + 1 ] ) / 2;
			}else{
				*( USHORT *)( m_pPlane + iIndex ) = ( x & 1 ) ? yc.ycr : yc.ycb;
				m_pPlane[ iIndex ] = yc.y;
			}
		}
	}
}

/*** フレームをマーク *******************************************************/

void CVsdFilterAvs::SetFrameMark( int iFrame ){
	m_Lap[ m_iLapNum++ ].iLogNum = iFrame;
};

/*** ラップタイム再計算 *****************************************************/

void CVsdFilterAvs::CalcLapTime( void ){
	
	int	i;
	int	iTime, iPrevTime;
	
	m_iBestTime	= BESTLAP_NONE;
	
	for( i = 0; i < m_iLapNum; ++i ){
		// ラップ検出
		iTime = ( int )( m_Lap[ i ].iLogNum * 1000.0 / m_dVideoFPS );
		
		m_Lap[ i ].uLap		= i;
		m_Lap[ i ].iTime	= i ? iTime - iPrevTime : 0;
		
		if(
			i &&
			( m_iBestTime == BESTLAP_NONE || m_iBestTime > m_Lap[ i ].iTime )
		){
			m_iBestTime			= m_Lap[ i ].iTime;
			m_iBestLapLogNum	= m_Lap[ i - 1 ].iLogNum;
		}
		
		iPrevTime = iTime;
	}
	m_Lap[ m_iLapNum ].iLogNum	= 0x7FFFFFFF;	// 番犬
	m_Lap[ m_iLapNum ].iTime	= 0;			// 番犬
}

/****************************************************************************/

PVideoFrame __stdcall CVsdFilterAvs::GetFrame( int n, IScriptEnvironment* env ){
	PVideoFrame src = child->GetFrame( n, env );
	env->MakeWritable(&src);
	
	m_iBytesPerLine		= src->GetPitch();
	m_iWidth			= src->GetRowSize()>>1; //case of YUY2
	m_iHeight			= src->GetHeight();
	m_iFrameCnt			= n;
	
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
		"c[log_file]s[param_file]s"
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
	
	return "`VSDFilter' vehicle data logger overlay plugin";
}
