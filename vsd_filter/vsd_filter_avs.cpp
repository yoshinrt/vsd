/*****************************************************************************
	
	VSD -- vehicle data logger system  Copyright(C) by DDS
	
	vsd_filter_avs.cpp - vsd_filter for AviSynth
	$Id: CVsdImg.cpp 133 2009-03-24 15:11:10Z  $
	
*****************************************************************************/

#include <windows.h>
#include "avisynth.h"

#include "dds.h"
#include "filter.h"
#include "dds_lib/dds_lib.h"
#include "CVsdFilter.h"

class CVsdFilterAvs : public GenericVideoFilter, CVsdFilter {
  public:
	CVsdFilterAvs( PClip _child, IScriptEnvironment* env );
	~CVsdFilterAvs();
	PVideoFrame __stdcall GetFrame( int n, IScriptEnvironment* env );
	
	/////////////
	
	int GetIndex( int x, int y ){ return m_iBytesPerLine * y + x * 2; }
	
	PIXEL_YC hoge;
	
	// 仮想関数
	virtual void PutPixel( int x, int y, const PIXEL_YC &yc, UINT uFlag );
	virtual PIXEL_YC &GetPixel( int x, int y, UINT uFlag );
	
	virtual int	GetWidth( void )	{ return m_iWidth; }
	virtual int	GetHeight( void )	{ return m_iHeight; }
	virtual int	GetFrameMax( void )	{ return vi.num_frames; }
	virtual int	GetFrameCnt( void )	{ return m_iFrameCnt; }
	
	virtual void SetFrameMark( int iFrame ){};
	virtual void CalcLapTime( void ){};
	
	// パラメータ
	int	m_iWidth, m_iHeight, m_iFrameCnt, m_iFrameMax;
	int m_iBytesPerLine;
	
	BYTE	*m_pPlane;
};

/*** コンストラクタ・デストラクタ *******************************************/

CVsdFilterAvs::CVsdFilterAvs( PClip _child, IScriptEnvironment* env ) : GenericVideoFilter( _child ){
	// パラメータ初期化
	m_piParamT	= new int[ TRACK_N ];
	m_piParamC	= new int[ CHECK_N ];
	
	#define DEF_TRACKBAR( id, init, min, max, name )	m_piParamT[ id ] = init;
	#include "def_trackbar.h"
	
	#define DEF_CHECKBOX( id, init, name )	m_piParamC[ id ] = init;
	#include "def_checkbox.h"
	
	ReadLog( "D:\\Video\\その他\\車\\ELISE\\20080502_美浜\\vsd20080502_162104-9.log" );
	DebugMsgD( "init completed\n" );
	
	hoge.y = 0;
	hoge.cr = 128;
	hoge.cb = 128;
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

/*
inline void CVsdFilter::PutPixel( int x, int y, short iY, short iCr, short iCb ){
	int	iIndex = GetIndex( x, y );
	
	fpip->ycp_edit[ iIndex ].y  = iY;
	fpip->ycp_edit[ iIndex ].cr = iCr;
	fpip->ycp_edit[ iIndex ].cb = iCb;
}
*/

void CVsdFilterAvs::PutPixel( int x, int y, const PIXEL_YC &yc, UINT uFlag ){
	
	if( uFlag & IMG_POLYGON ){
		// ポリゴン描画
		if( x > m_Polygon[ y ].iRight ) m_Polygon[ y ].iRight = x;
		if( x < m_Polygon[ y ].iLeft  ) m_Polygon[ y ].iLeft  = x;
	}else{
		/*
		PIXEL_YC	*ycp = fpip->ycp_edit;
		
		if( uFlag & IMG_ALFA && yc.y == -1 ) return;
		
		if( 0 <= x && x < fpip->max_w && 0 <= y && y < fpip->max_h ){
			if( uFlag & IMG_ALFA ){
				int	iIndex = GetIndex( x, y );
				
				ycp[ iIndex ].y  = ( yc.y  + ycp[ iIndex ].y  ) / 2;
				ycp[ iIndex ].cr = ( yc.cr + ycp[ iIndex ].cr ) / 2;
				ycp[ iIndex ].cb = ( yc.cb + ycp[ iIndex ].cb ) / 2;
			}else{
				ycp[ GetIndex( x, y ) ] = yc;
			}
		}*/
		int	iIndex = GetIndex( x, y );
		m_pPlane[ iIndex + 0 ] = yc.y  >> 4;
		if( x & 1 ){
			m_pPlane[ iIndex + 1 ] = ( yc.cr >> 4 ) + 0x80;
		}else{
			m_pPlane[ iIndex + 1 ] = ( yc.cb >> 4 ) + 0x80;
		}
	}
}

PIXEL_YC &CVsdFilterAvs::GetPixel( int x, int y, UINT uFlag ){
	//PIXEL_YC	*ycp = fpip->ycp_edit;
	//return	ycp[ GetIndex( x, y ) ];
	return hoge;
}

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
    return new CVsdFilterAvs( args[0].AsClip(), env );  
}

extern "C" __declspec( dllexport ) const char* __stdcall AvisynthPluginInit2( IScriptEnvironment* env ){
    env->AddFunction( "VSDFilter", "c", Create_VSDFilter, 0 );
    return "`VSDFilter' vehicle data logger overlay plugin";
}
