/*****************************************************************************
	
	VSD -- vehicle data logger system  Copyright(C) by DDS
	
	vsd_filter.cpp - VSD filter for AviUti
	$Id$
	
*****************************************************************************/

#include <windows.h>
#include <stdio.h>
#include <math.h>
#include <string.h>

#include "dds.h"
#include "../vsd/main.h"
#include "filter.h"
#include "dds_lib/dds_lib.h"
#include "CVsdLog.h"
#include "CVsdFilter.h"

/*** macros *****************************************************************/

#ifdef GPS_ONLY
	#define CONFIG_EXT		"cfg"
	#define	FILE_EXT		"log/config file\0*.nme*; *.dp3; *." CONFIG_EXT "\0AllFile (*.*)\0*.*\0"
#else
	#define CONFIG_EXT		"avs"
	#define	FILE_EXT		"log/config file\0*.log; *.nme*; *.gz; *." CONFIG_EXT "\0AllFile (*.*)\0*.*\0"
#endif

#define	FILE_CFG_EXT	"Config File (*." CONFIG_EXT ")\0*." CONFIG_EXT "\0AllFile (*.*)\0*.*\0"

/*** new type ***************************************************************/

/*** gloval var *************************************************************/

HINSTANCE		g_hInst 	= NULL;

/****************************************************************************/
//---------------------------------------------------------------------
//		フィルタ構造体定義
//---------------------------------------------------------------------

// トラックバーの名前
TCHAR	*track_name[] = {
	#define DEF_TRACKBAR( id, init, min, max, name, conf_name )	name,
	#include "def_trackbar.h"
};
// トラックバーの初期値
int		track_default[] = {
	#define DEF_TRACKBAR( id, init, min, max, name, conf_name )	init,
	#include "def_trackbar.h"
};
// トラックバーの下限値
int		track_s[] = {
	#define DEF_TRACKBAR( id, init, min, max, name, conf_name )	min,
	#include "def_trackbar.h"
};
// トラックバーの上限値
int		track_e[] = {
	#define DEF_TRACKBAR( id, init, min, max, name, conf_name )	max,
	#include "def_trackbar.h"
};

// チェックボックスの名前
TCHAR	*check_name[] = {
	#define DEF_CHECKBOX( id, init, name, conf_name )	name,
	#include "def_checkbox.h"
};

// チェックボックスの初期値 (値は0か1)
int		check_default[] = {
	#define DEF_CHECKBOX( id, init, name, conf_name )	init,
	#include "def_checkbox.h"
};

// トラックバーの初期値
int		shadow_default[] = {
	#define DEF_SHADOW( id, init, conf_name )	init,
	#include "def_shadow.h"
};

FILTER_DLL filter = {
	FILTER_FLAG_EX_INFORMATION | FILTER_FLAG_IMPORT | FILTER_FLAG_EXPORT | FILTER_FLAG_MAIN_MESSAGE,
								//	フィルタのフラグ
								//	FILTER_FLAG_ALWAYS_ACTIVE		: フィルタを常にアクティブにします
								//	FILTER_FLAG_CONFIG_POPUP		: 設定をポップアップメニューにします
								//	FILTER_FLAG_CONFIG_CHECK		: 設定をチェックボックスメニューにします
								//	FILTER_FLAG_CONFIG_RADIO		: 設定をラジオボタンメニューにします
								//	FILTER_FLAG_EX_DATA				: 拡張データを保存出来るようにします。
								//	FILTER_FLAG_PRIORITY_HIGHEST	: フィルタのプライオリティを常に最上位にします
								//	FILTER_FLAG_PRIORITY_LOWEST		: フィルタのプライオリティを常に最下位にします
								//	FILTER_FLAG_WINDOW_THICKFRAME	: サイズ変更可能なウィンドウを作ります
								//	FILTER_FLAG_WINDOW_SIZE			: 設定ウィンドウのサイズを指定出来るようにします
								//	FILTER_FLAG_DISP_FILTER			: 表示フィルタにします
								//	FILTER_FLAG_EX_INFORMATION		: フィルタの拡張情報を設定できるようにします
								//	FILTER_FLAG_NO_CONFIG			: 設定ウィンドウを表示しないようにします
								//	FILTER_FLAG_AUDIO_FILTER		: オーディオフィルタにします
								//	FILTER_FLAG_RADIO_BUTTON		: チェックボックスをラジオボタンにします
								//	FILTER_FLAG_WINDOW_HSCROLL		: 水平スクロールバーを持つウィンドウを作ります
								//	FILTER_FLAG_WINDOW_VSCROLL		: 垂直スクロールバーを持つウィンドウを作ります
								//	FILTER_FLAG_IMPORT				: インポートメニューを作ります
								//	FILTER_FLAG_EXPORT				: エクスポートメニューを作ります
	0,0,						//	設定ウインドウのサイズ (FILTER_FLAG_WINDOW_SIZEが立っている時に有効)
	"VSDメーター合成",			//	フィルタの名前
	TRACK_N,					//	トラックバーの数 (0なら名前初期値等もNULLでよい)
	track_name,					//	トラックバーの名前郡へのポインタ
	track_default,				//	トラックバーの初期値郡へのポインタ
	track_s, track_e,			//	トラックバーの数値の下限上限 (NULLなら全て0～256)
	CHECK_N,					//	チェックボックスの数 (0なら名前初期値等もNULLでよい)
	check_name,					//	チェックボックスの名前郡へのポインタ
	check_default,				//	チェックボックスの初期値郡へのポインタ
	func_proc,					//	フィルタ処理関数へのポインタ (NULLなら呼ばれません)
	NULL, //func_init,			//	開始時に呼ばれる関数へのポインタ (NULLなら呼ばれません)
	NULL, //func_exit,			//	終了時に呼ばれる関数へのポインタ (NULLなら呼ばれません)
	func_update,				//	設定が変更されたときに呼ばれる関数へのポインタ (NULLなら呼ばれません)
	func_WndProc,				//	設定ウィンドウにウィンドウメッセージが来た時に呼ばれる関数へのポインタ (NULLなら呼ばれません)
	NULL,NULL,					//	システムで使いますので使用しないでください
	NULL,						//  拡張データ領域へのポインタ (FILTER_FLAG_EX_DATAが立っている時に有効)
	NULL,						//  拡張データサイズ (FILTER_FLAG_EX_DATAが立っている時に有効)
	NULL,						//  フィルタ情報へのポインタ (FILTER_FLAG_EX_INFORMATIONが立っている時に有効)
	NULL,						//	セーブが開始される直前に呼ばれる関数へのポインタ (NULLなら呼ばれません)
	NULL,						//	セーブが終了した直前に呼ばれる関数へのポインタ (NULLなら呼ばれません)
};


//---------------------------------------------------------------------
//		フィルタ構造体のポインタを渡す関数
//---------------------------------------------------------------------

EXTERN_C FILTER_DLL __declspec(dllexport) * __stdcall GetFilterTable( void ){
	return &filter;
}

//---------------------------------------------------------------------
//		初期化
//---------------------------------------------------------------------

/*
BOOL func_init( FILTER *fp ){
	return TRUE;
}
*/

//---------------------------------------------------------------------
//		終了
//---------------------------------------------------------------------

/*
BOOL func_exit( FILTER *fp ){
	return TRUE;
}
*/

//---------------------------------------------------------------------
//		フィルタ処理関数
//---------------------------------------------------------------------

/*** CVsdFilterAvu クラス ***************************************************/

class CVsdFilterAvu : public CVsdFilter {
	
  public:
	void	*editp;
	FILTER	*filter;
	FILTER_PROC_INFO *fpip;
	
	int GetIndex( int x, int y ){ return fpip->max_w * y + x; }
	BOOL ConfigSave( const char *szFileName );
	
	// 仮想関数
	virtual void PutPixel( int x, int y, const PIXEL_YC &yc, UINT uFlag );
	
	virtual int	GetWidth( void ){ return fpip->w ; }
	virtual int	GetHeight( void ){ return fpip->h ; }
	virtual int	GetFrameMax( void ){ return fpip->frame_n ; }
	virtual int	GetFrameCnt( void ){ return fpip->frame ; }
	
	virtual void SetFrameMark( int iFrame );
	virtual int  GetFrameMark( int iFrame );
	
	virtual char *GetVideoFileName( char *szFileName );
};

CVsdFilterAvu	*g_Vsd;

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

void CVsdFilterAvu::PutPixel( int x, int y, const PIXEL_YC &yc, UINT uFlag ){
	
	if( uFlag & IMG_POLYGON ){
		// ポリゴン描画
		if( x > m_Polygon[ y ].iRight ) m_Polygon[ y ].iRight = x;
		if( x < m_Polygon[ y ].iLeft  ) m_Polygon[ y ].iLeft  = x;
	}else{
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
		}
	}
}

/*** フレームをマーク *******************************************************/

void CVsdFilterAvu::SetFrameMark( int iFrame ){
	FRAME_STATUS	fsp;
	
	filter->exfunc->get_frame_status( editp, iFrame, &fsp );
	fsp.edit_flag |= EDIT_FRAME_EDIT_FLAG_MARKFRAME;
	filter->exfunc->set_frame_status( editp, iFrame, &fsp );
}

int CVsdFilterAvu::GetFrameMark( int iFrame ){
	
	FRAME_STATUS	fsp;
	
	for( ; iFrame < GetFrameMax(); ++iFrame ){
		filter->exfunc->get_frame_status( editp, iFrame, &fsp );
		
		if( fsp.edit_flag & EDIT_FRAME_EDIT_FLAG_MARKFRAME ){
			return iFrame;
		}
	}
	return -1;
}

/*** 編集ビデオファイル名取得 ***********************************************/

char *CVsdFilterAvu::GetVideoFileName( char *szFileName ){
	FILE_INFO	fi;
	filter->exfunc->get_file_info( editp, &fi );
	return strcpy( szFileName, fi.name );
}

/*** config セーブ **********************************************************/

BOOL CVsdFilterAvu::ConfigSave( const char *szFileName ){
	FILE	*fp;
	int		i;
	
	if(( fp = fopen( szFileName, "w" )) == NULL ) return FALSE;
	
	char szBuf[ BUF_SIZE ];
	
	FILE_INFO fi;
	filter->exfunc->get_file_info( editp, &fi );
	
	fprintf( fp,
		"DirectShowSource( \"%s\", pixel_type=\"YUY2\", fps=%d.0/%d )\n"
		"VSDFilter",
		GetVideoFileName( szBuf ),
		fi.video_rate, fi.video_scale
	);
	
	char cSep = '(';
	if( m_szLogFile ){
		fprintf( fp, "%c \\\n\tlog_file=\"%s\"", cSep, m_szLogFile );
		cSep = ',';
	}
	
	if( m_szGPSLogFile ){
		fprintf( fp, "%c \\\n\tgps_file=\"%s\"", cSep, m_szGPSLogFile );
		cSep = ',';
	}
	
	for( i = 0; i < TRACK_N; ++i ){
		if( m_szTrackbarName[ i ] == NULL ) continue;
		
		fprintf( fp, "%c \\\n\t%s=%d", cSep, m_szTrackbarName[ i ],
			( i <= TRACK_GEd ) ? m_piParamT[ i ] * 100 + m_piParamT[ i + 1 ] :
			m_piParamT[ i ]
		);
		cSep = ',';
	}
	
	for( i = 0; i < CHECK_N; ++i ){
		if( m_szCheckboxName[ i ] == NULL ) continue;
		
		fprintf(
			fp, ", \\\n\t%s=%d", m_szCheckboxName[ i ], m_piParamC[ i ]
		);
	}
	
	// 手動ラップ計測マーク出力
	if( m_iLapMode != LAPMODE_MAGNET && m_iLapNum ){
		FRAME_STATUS	fsp;
		FILE_INFO		fip;
		BOOL			bFirst = TRUE;
		
		filter->exfunc->get_file_info( editp, &fip );
		
		// マークされているフレーム# を求める  GetFrameMax() はなぜか×
		for( i = 0; i < fip.frame_n; ++i ){
			filter->exfunc->get_frame_status( editp, i, &fsp );
			if( fsp.edit_flag & EDIT_FRAME_EDIT_FLAG_MARKFRAME ){
				fprintf( fp, "%s%u", bFirst ? ", \\\n\tmark=\"" : ",", i );
				bFirst = FALSE;
			}
		}
		fputc( '"', fp );
	}
	
	fprintf( fp, " \\\n)\n" );
	
	fclose( fp );
	return TRUE;
}

/*** func_proc **************************************************************/

BOOL func_proc( FILTER *fp, FILTER_PROC_INFO *fpip ){
//
//	fp->track[n]			: トラックバーの数値
//	fp->check[n]			: チェックボックスの数値
//	fpip->w 				: 実際の画像の横幅
//	fpip->h 				: 実際の画像の縦幅
//	fpip->w					: 画像領域の横幅
//	fpip->h					: 画像領域の縦幅
//	fpip->ycp_edit			: 画像領域へのポインタ
//	fpip->ycp_temp			: テンポラリ領域へのポインタ
//	fpip->ycp_edit[n].y		: 画素(輝度    )データ (     0 ～ 4095 )
//	fpip->ycp_edit[n].cb	: 画素(色差(青))データ ( -2048 ～ 2047 )
//	fpip->ycp_edit[n].cr	: 画素(色差(赤))データ ( -2048 ～ 2047 )
//
//  画素データは範囲外に出ていることがあります。
//  また範囲内に収めなくてもかまいません。
//
//	画像サイズを変えたいときは fpip->w や fpip->h を変えます。
//
//	テンポラリ領域に処理した画像を格納したいときは
//	fpip->ycp_edit と fpip->ycp_temp を入れ替えます。
//
	
	if( !g_Vsd ) return 0;
	// クラスに変換
	g_Vsd->filter	= fp;
	g_Vsd->fpip		= fpip;
	
	return g_Vsd->DrawVSD();
}

/*** WndProc ****************************************************************/

BOOL func_WndProc( HWND hwnd,UINT message,WPARAM wparam,LPARAM lparam,void *editp,FILTER *filter ){
	
	TCHAR	szBuf[ MAX_PATH ];
	TCHAR	szBuf2[ MAX_PATH ];
	int		iFrame;
	
	//	TRUEを返すと全体が再描画される
	
	//	編集中でなければ何もしない
	if( filter->exfunc->is_editing( editp ) != TRUE ) return FALSE;
	
	switch( message ) {
	  case WM_FILTER_IMPORT:
		if( filter->exfunc->dlg_get_load_name( szBuf, FILE_EXT, NULL )){
			
			#ifdef GPS_ONLY
				// config ロード
				// .log.gz かもしれないので，拡張子を削除してみる
				ChangeExt( szBuf2, ( char *)szBuf, NULL );
				
				// .avs ロード
				g_Vsd->ConfigLoad( ChangeExt( szBuf2, ( char *)szBuf2, CONFIG_EXT ));
				
				// .nmea ロード
				if( g_Vsd->GPSLogLoad( szBuf )){
					// trackbar 設定
					track_e[ TRACK_GSt ] =
					track_e[ TRACK_GEd ] = ( g_Vsd->m_GPSLog->m_iCnt + 99 ) / 100;
				}
			#else
				// config ロード
				// .log.gz かもしれないので，拡張子を削除してみる
				ChangeExt( szBuf, ( char *)szBuf, NULL );
				
				// .avs ロード
				g_Vsd->ConfigLoad( ChangeExt( szBuf2, ( char *)szBuf, CONFIG_EXT ));
				
				// .nmea ロード
				if(
					g_Vsd->GPSLogLoad( ChangeExt( szBuf2, ( char *)szBuf, "nmea.gz" )) ||
					g_Vsd->GPSLogLoad( ChangeExt( szBuf2, ( char *)szBuf, "nmea" )) ||
					g_Vsd->GPSLogLoad( ChangeExt( szBuf2, ( char *)szBuf, "dp3" ))
				){
					// trackbar 設定
					track_e[ TRACK_GSt ] =
					track_e[ TRACK_GEd ] = ( g_Vsd->m_GPSLog->m_iCnt + 99 ) / 100;
				}
				
				// .log ロード
				if(
					g_Vsd->ReadLog( ChangeExt( szBuf2, ( char *)szBuf, "log.gz" )) ||
					g_Vsd->ReadLog( ChangeExt( szBuf2, ( char *)szBuf, "log" ))
				){
					// trackbar 設定
					track_e[ TRACK_LSt ] =
					track_e[ TRACK_LEd ] = ( g_Vsd->m_VsdLog->m_iCnt + 99 ) / 100;
				}
			#endif
			
			// 設定再描画
			filter->exfunc->filter_window_update( filter );
			
			#ifndef GPS_ONLY
				// log pos 更新
				func_update( filter, FILTER_UPDATE_STATUS_CHECK + CHECK_LOGPOS );
			#endif
		}
		return TRUE;
		
	  Case WM_FILTER_EXPORT:
		if( filter->exfunc->dlg_get_save_name( szBuf, FILE_CFG_EXT, NULL ))
			return g_Vsd->ConfigSave( szBuf );
		
	  Case WM_FILTER_FILE_OPEN:
		
		g_Vsd = new CVsdFilterAvu;
		
		g_Vsd->m_piParamT	= filter->track;
		g_Vsd->m_piParamC	= filter->check;
		g_Vsd->m_piParamS	= shadow_default;
		g_Vsd->filter		= filter;
		g_Vsd->editp		= editp;
		
		// fps 取得
		FILE_INFO fi;
		filter->exfunc->get_file_info( editp, &fi );
		g_Vsd->m_dVideoFPS  = ( double )fi.video_rate / fi.video_scale;
		
		g_Vsd->m_bCalcLapTimeReq = TRUE;
		
		// trackbar 設定
		track_e[ TRACK_VSt ] =
		track_e[ TRACK_VEd ] = ( filter->exfunc->get_frame_n( editp ) + 99 ) / 100;
		
		// 設定再描画
		filter->exfunc->filter_window_update( filter );
		
	  Case WM_FILTER_FILE_CLOSE:
		delete g_Vsd;
		g_Vsd = NULL;
		
	  Case WM_FILTER_MAIN_KEY_DOWN:
		switch( wparam ){
			
		  case 'M':
			// マーク
			FRAME_STATUS	fsp;
			iFrame = filter->exfunc->get_frame( editp );
			
			filter->exfunc->get_frame_status( editp, iFrame, &fsp );
			fsp.edit_flag ^= EDIT_FRAME_EDIT_FLAG_MARKFRAME;
			filter->exfunc->set_frame_status( editp, iFrame, &fsp );
			
			g_Vsd->m_bCalcLapTimeReq = TRUE;
			return TRUE;
			
		  Case 'S':
			// フレーム数セット
			iFrame = filter->exfunc->get_frame( editp );
			
			filter->track[ TRACK_VSt  ]	= filter->track[ TRACK_VEd  ];
			filter->track[ TRACK_VSt2 ]	= filter->track[ TRACK_VEd2 ];
			
			filter->track[ TRACK_VEd  ]	= iFrame / 100;
			filter->track[ TRACK_VEd2 ]	= iFrame % 100;
			
			// 設定再描画
			filter->exfunc->filter_window_update( filter );
		}
	}
	
	return FALSE;
}

/*** スライダバー連動 *******************************************************/

BOOL func_update( FILTER *fp, int status ){
	static	BOOL	bReEnter = FALSE;
	
	if( bReEnter ) return TRUE;
	
	bReEnter = TRUE;
	
	if(
		g_Vsd && (
			status >= FILTER_UPDATE_STATUS_TRACK + TRACK_VSt  &&
			status <= FILTER_UPDATE_STATUS_TRACK + TRACK_GSt2 ||
			status == FILTER_UPDATE_STATUS_TRACK + TRACK_SLineWidth
		)
	) g_Vsd->m_bCalcLapTimeReq = TRUE;
	
	#ifndef GPS_ONLY
		if(
			status == ( FILTER_UPDATE_STATUS_CHECK + CHECK_LOGPOS ) &&
			fp->check[ CHECK_LOGPOS ]
		){
			fp->track[ TRACK_LSt  ] = g_Vsd->m_iLogStart / 100;
			fp->track[ TRACK_LSt2 ] = g_Vsd->m_iLogStart % 100;
			fp->track[ TRACK_LEd  ] = g_Vsd->m_iLogStop  / 100;
			fp->track[ TRACK_LEd2 ] = g_Vsd->m_iLogStop  % 100;
			
			// 設定再描画
			fp->exfunc->filter_window_update( fp );
		}
	#endif
	
	// マップ回転
	if( status == ( FILTER_UPDATE_STATUS_TRACK + TRACK_MapAngle )){
		if( g_Vsd->m_VsdLog )
			g_Vsd->m_VsdLog->RotateMap( fp->track[ TRACK_MapAngle ] * ( -ToRAD / 10 ));
		if( g_Vsd->m_GPSLog )
			g_Vsd->m_GPSLog->RotateMap( fp->track[ TRACK_MapAngle ] * ( -ToRAD / 10 ));
	}
	
	bReEnter = FALSE;
	
	return TRUE;
}

/****************************************************************************/

BOOL WINAPI DllMain(
	HINSTANCE	hinstDLL,	// handle to DLL module
	DWORD		fdwReason,	// reason for calling function
	LPVOID		lpvReserved	// reserved
){
	if( fdwReason == DLL_PROCESS_ATTACH ){
		g_hInst = hinstDLL;
	}
	return TRUE;
}
