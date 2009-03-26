/*****************************************************************************
	
	VSD -- vehicle data logger system  Copyright(C) by DDS
	
	vsd_filter_avs.cpp - vsd_filter for AviSynth
	$Id: CVsdImg.cpp 133 2009-03-24 15:11:10Z  $
	
*****************************************************************************/

#include <windows.h>
#include "avisynth.h"

#include "dds.h"
#include "dds_lib/dds_lib.h"
#include "CVsdFilter.h"

class CVsdFilterAvs : public GenericVideoFilter, CVsdFilter {
  public:
	CVsdFilterAvs::CVsdFilterAvs(
		PClip _child,
		const char *szParamFile,
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
};

/*** コンストラクタ・デストラクタ *******************************************/

CVsdFilterAvs::CVsdFilterAvs(
	PClip _child,
	const char *szParamFile,
	IScriptEnvironment* env
) : GenericVideoFilter( _child ){
	
	if( !vi.IsYUY2() ) env->ThrowError( "VSDFilter: requires YUY2 input.");
	
	// パラメータ初期化
	m_piParamT	= new int[ TRACK_N ];
	m_piParamC	= new int[ CHECK_N ];
	
	#define DEF_TRACKBAR( id, init, min, max, name )	m_piParamT[ id ] = init;
	#include "def_trackbar.h"
	
	#define DEF_CHECKBOX( id, init, name )	m_piParamC[ id ] = init;
	#include "def_checkbox.h"
	
	m_dVideoFPS = ( double )vi.fps_numerator / vi.fps_denominator;

	ReadLog( szParamFile );
}

CVsdFilterAvs::~CVsdFilterAvs(){
	delete [] m_piParamT;
	delete [] m_piParamC;
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
	
	int				i;
	double			dTime, dPrevTime;
	
	m_fBestTime	= -1;
	
	for( i = 0; i < m_iLapNum; ++i ){
		// ラップ検出
		dTime = m_Lap[ i ].iLogNum / m_dVideoFPS;
		
		m_Lap[ i ].uLap		= i;
		m_Lap[ i ].fTime	= i ? ( float )( dTime - dPrevTime ) : 0;
		
		if(
			i &&
			( m_fBestTime == -1 || m_fBestTime > m_Lap[ i ].fTime )
		){
			m_fBestTime			= m_Lap[ i ].fTime;
			m_iBestLapLogNum	= m_Lap[ i - 1 ].iLogNum;
		}
		
		dPrevTime = dTime;
	}
	m_Lap[ m_iLapNum ].iLogNum	= 0x7FFFFFFF;	// 番犬
	m_Lap[ m_iLapNum ].fTime	= 0;			// 番犬
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
    	args[1].AsString(),
    	env
    );
}

extern "C" __declspec( dllexport ) const char* __stdcall AvisynthPluginInit2( IScriptEnvironment* env ){
    env->AddFunction( "VSDFilter", "cs", Create_VSDFilter, 0 );
    return "`VSDFilter' vehicle data logger overlay plugin";
}
