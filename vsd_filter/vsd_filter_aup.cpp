#include <windows.h>
#include <stdio.h>
#include <string.h>

#include "dds.h"
#include "filter.h"
#include "../vsd/main.h"
#include "dds_lib/dds_lib.h"
#include "CVsdImg.h"

/*** macros *****************************************************************/

#ifdef CIRCUIT_TOMO
	#define	FILE_EXT		"Pulse-Time Data (*.ptd)\0*.ptd\0Config File (*.cfg)\0*.cfg\0AllFile (*.*)\0*.*\0"
#else
	#define	FILE_EXT		"LogFile (*.log)\0*.log\0Config File (*.cfg)\0*.cfg\0AllFile (*.*)\0*.*\0"
#endif

#define	FILE_CFG_EXT		"Config File (*.cfg)\0*.cfg\0AllFile (*.*)\0*.*\0"

/****************************************************************************/
//---------------------------------------------------------------------
//		フィルタ構造体定義
//---------------------------------------------------------------------

// トラックバーの名前
TCHAR	*track_name[] = {
	#define DEF_TRACKBAR( id, init, min, max, name )	name,
	#include "def_trackbar.h"
};
// トラックバーの初期値
int		track_default[] = {
	#define DEF_TRACKBAR( id, init, min, max, name )	init,
	#include "def_trackbar.h"
};
// トラックバーの下限値
int		track_s[] = {
	#define DEF_TRACKBAR( id, init, min, max, name )	min,
	#include "def_trackbar.h"
};
// トラックバーの上限値
int		track_e[] = {
	#define DEF_TRACKBAR( id, init, min, max, name )	max,
	#include "def_trackbar.h"
};

enum {
	#define DEF_TRACKBAR( id, init, min, max, name )	id,
	#include "def_trackbar.h"
	TRACK_N
};

// チェックボックスの名前
TCHAR	*check_name[] = {
	#define DEF_CHECKBOX( id, init, name )	name,
	#include "def_checkbox.h"
};

// チェックボックスの初期値 (値は0か1)
int		check_default[] = {
	#define DEF_CHECKBOX( id, init, name )	init,
	#include "def_checkbox.h"
};

enum {
	#define DEF_CHECKBOX( id, init, name )	id,
	#include "def_checkbox.h"
	CHECK_N
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
	NULL /*func_init*/,			//	開始時に呼ばれる関数へのポインタ (NULLなら呼ばれません)
	NULL /*func_exit*/,			//	終了時に呼ばれる関数へのポインタ (NULLなら呼ばれません)
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
	
	delete [] g_VsdLog;
	delete [] g_Lap;
	return TRUE;
}
*/

/****************************************************************************/

BOOL func_WndProc( HWND hwnd,UINT message,WPARAM wparam,LPARAM lparam,void *editp,FILTER *filter ){
	//	TRUEを返すと全体が再描画される
	
	char	*szFileName[ BUF_SIZE ];
	
	//	編集中でなければ何もしない
	if( filter->exfunc->is_editing(editp) != TRUE ) return FALSE;
	
	int	iFrame;
	
	switch( message ) {
	  case WM_FILTER_IMPORT:
		if( filter->exfunc->dlg_get_load_name( szFileName, FILE_EXT, NULL ) != TRUE ) return FALSE;
		
		ReadLog( editp, filter, szFileName );
		
		// trackbar 設定
		track_e[ TRACK_LSt ] =
		track_e[ TRACK_LEd ] =
			#ifdef CIRCUIT_TOMO
				( int )( g_iVsdLogNum / LOG_FREQ + 1 );
			#else
				( g_iVsdLogNum + 99 ) / 100;
			#endif
		
		// 設定再描画
		filter->exfunc->filter_window_update( filter );
		
		#ifndef CIRCUIT_TOMO
			// log pos 更新
			func_update( filter, FILTER_UPDATE_STATUS_CHECK + CHECK_LOGPOS );
		#endif
		
	  Case WM_FILTER_EXPORT:
		if( filter->exfunc->dlg_get_save_name( szFileName, FILE_CFG_EXT, NULL ))
			SaveConfig( filter );
		
	  Case WM_FILTER_FILE_OPEN:
		// fps 取得
		FILE_INFO fi;
		filter->exfunc->get_file_info( editp, &fi );
		g_dVideoFPS  = ( double )fi.video_rate / fi.video_scale;
		
		// g_Lap 初期化
		if( g_Lap == NULL )	g_Lap = new LAP_t[ MAX_LAP ];
		g_iLapNum = 0;
		g_Lap[ 0 ].iLogNum	= 0x7FFFFFFF;	// 番犬
		g_Lap[ 0 ].fTime	= 0;			// 番犬
		
		g_bCalcLapTimeReq = TRUE;
		
		// trackbar 設定
		track_e[ TRACK_VSt ] =
		track_e[ TRACK_VEd ] = ( filter->exfunc->get_frame_n( editp ) + 99 ) / 100;
		
		// 設定再描画
		filter->exfunc->filter_window_update( filter );
		
	  Case WM_FILTER_FILE_CLOSE:
		delete [] g_VsdLog;
		g_VsdLog		= NULL;
		g_iVsdLogNum	= 0;
		
		delete [] g_Lap;
		g_Lap		= NULL;
		g_iLapNum	= 0;
		
	  Case WM_FILTER_MAIN_KEY_DOWN:
		switch( wparam ){
			
		  case 'M':
			// マーク
			FRAME_STATUS	fsp;
			iFrame = filter->exfunc->get_frame( editp );
			
			filter->exfunc->get_frame_status( editp, iFrame, &fsp );
			fsp.edit_flag ^= EDIT_FRAME_EDIT_FLAG_MARKFRAME;
			filter->exfunc->set_frame_status( editp, iFrame, &fsp );
			
			g_bCalcLapTimeReq = TRUE;
			return TRUE;
			
		  Case 'S':
			// フレーム数セット
			iFrame = filter->exfunc->get_frame( editp );
			
			filter->track[ TRACK_VSt ]	= filter->track[ TRACK_VEd ];
			filter->track[ TRACK_VSt2 ]	= filter->track[ TRACK_VEd2 ];
			
			filter->track[ TRACK_VEd ]	= iFrame / 100;
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
	
#ifndef CIRCUIT_TOMO
	if(
		status == ( FILTER_UPDATE_STATUS_CHECK + CHECK_LOGPOS ) &&
		fp->check[ CHECK_LOGPOS ]
	){
		fp->track[ TRACK_LSt  ] = g_iLogStart / 100;
		fp->track[ TRACK_LSt2 ] = g_iLogStart % 100;
		fp->track[ TRACK_LEd  ] = g_iLogStop  / 100;
		fp->track[ TRACK_LEd2 ] = g_iLogStop  % 100;
		
		// 設定再描画
		fp->exfunc->filter_window_update( fp );
	}
	
	// マップ回転
	if( status == ( FILTER_UPDATE_STATUS_TRACK + TRACK_MapAngle )){
		RotateMap( fp );
	}
#endif
	
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
