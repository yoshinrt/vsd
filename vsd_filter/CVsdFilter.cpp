 /*****************************************************************************
	
	VSD -- vehicle data logger system  Copyright(C) by DDS
	
	CVsdFilter.cpp - CVsdFilter class implementation
	
*****************************************************************************/

#include "StdAfx.h"

#include "dds.h"
#include "../vsd/main.h"
#include "dds_lib/dds_lib.h"

#ifndef AVS_PLUGIN
	#include "filter.h"
#endif
#include "CVsdLog.h"
#include "CVsdFont.h"
#include "CScript.h"
#include "pixel.h"
#include "CVsdImage.h"
#include "CVsdFilter.h"

/*** macros *****************************************************************/

#define SLineWidth		( m_piParamT[ TRACK_SLineWidth ] / 10.0 )

/*** static member **********************************************************/

HINSTANCE	CVsdFilter::m_hInst 	= NULL;

/*** コンストラクタ *********************************************************/

CVsdFilter::CVsdFilter (){
	
	m_VsdLog 			= NULL;
	m_GPSLog 			= NULL;
	m_LapLog			= NULL;
	
	m_bCalcLapTimeReq	= FALSE;
	
	m_Polygon			= NULL;	// DrawPolygon 用バッファ
	m_pFont				= NULL;
	
	m_szLogFile			= NULL;
	m_szGPSLogFile		= NULL;
	m_szSkinFile		= NULL;
	m_szSkinDirA		= NULL;
	m_szPluginDirA		= NULL;
	m_szSkinDirW		= NULL;
	m_szPluginDirW		= NULL;
	
	// str param に初期値設定
	#define DEF_STR_PARAM( id, var, init, conf_name ) StringNew( var, init );
	#include "def_str_param.h"
	
	// plugin dll path 取得
	SetPluginDir();
	
	// SkinDir セット
	SetSkinFile( m_szSkinFile );
	
	m_Script	= NULL;
	
	m_iWidth	=
	m_iHeight	= 0;
}

/*** デストラクタ ***********************************************************/

CVsdFilter::~CVsdFilter (){
	delete m_VsdLog;
	delete m_GPSLog;
	delete m_LapLog;
	delete [] m_szLogFile;
	delete [] m_szGPSLogFile;
	delete [] m_szSkinFile;
	delete [] m_szSkinDirA;
	delete [] m_szPluginDirA;
	delete [] m_szSkinDirW;
	delete [] m_szPluginDirW;
	delete [] m_Polygon;
	delete m_pFont;
	delete m_Script;
}

/*** ログリード ************************************************************/

int CVsdFilter::ReadLog( const char *szFileName ){
	if( m_VsdLog ) delete m_VsdLog;
	m_VsdLog = new CVsdLog( this );
	if( m_LapLog ) delete m_LapLog;
	m_LapLog = NULL;
	int iRet = m_VsdLog->ReadLog( szFileName, m_LapLog );
	if( iRet ){
		m_VsdLog->RotateMap( m_piParamT[ TRACK_MapAngle ] * ( -ToRAD / 10 ));
	}else{
		delete m_VsdLog;
		m_VsdLog = NULL;
	}
	
	return iRet;
}

int CVsdFilter::ReadGPSLog( const char *szFileName ){
	if( m_GPSLog ) delete m_GPSLog;
	m_GPSLog = new CVsdLog( this );
	int iRet = m_GPSLog->ReadGPSLog( szFileName );
	if( iRet ){
		m_GPSLog->RotateMap( m_piParamT[ TRACK_MapAngle ] * ( -ToRAD / 10 ));
	}else{
		delete m_GPSLog;
		m_GPSLog = NULL;
	}
	
	return iRet;
}

/***************************************************************************/

BOOL CVsdFilter::ParseMarkStr( const char *szMark ){
	
	do{
		while( *szMark && !isdigit( *szMark )) ++szMark;	// 数字までスキップ
		if( !*szMark ) break;
		
		SetFrameMark( atoi( szMark ));
		if( szMark = strchr( szMark, ',' )) ++szMark;	// 次のパラメータ
	}while( szMark );
	
	m_bCalcLapTimeReq = TRUE;
	return TRUE;
}


/*** ラップ番号 → ログ番号 変換 ********************************************/

double CVsdFilter::LapNum2LogNum( CVsdLog* Log, int iLapNum ){
	
	double dFrame;
	
	// iLapNum がおかしいときは 0 を返しとく
	if( iLapNum < 0 ) return 0;
	
	if( m_LapLog->m_iLapMode == LAPMODE_MAGNET ){
		// fLogNum は VSD ログ番号
		if( Log == m_VsdLog ) return m_LapLog->m_Lap[ iLapNum ].fLogNum;
		if( VsdSt == VsdEd )  return 0;
		
		// 一旦ビデオフレームに変換
		dFrame =
			( Log->Time( m_LapLog->m_Lap[ iLapNum ].fLogNum ) * SLIDER_TIME - VsdSt ) /
			( VsdEd - VsdSt ) * ( VideoEd - VideoSt ) + VideoSt;
		
		return Log->GetIndex( dFrame, VideoSt, VideoEd, GPSSt, GPSEd, -1 );
		
	}else if( m_LapLog->m_iLapMode != LAPMODE_HAND_VIDEO ){
		// fLogNum は GPS ログ番号
		if( Log == m_GPSLog ) return m_LapLog->m_Lap[ iLapNum ].fLogNum;
		if( GPSSt == GPSEd )  return 0;
		
		// 一旦ビデオフレームに変換
		dFrame =
			( Log->Time( m_LapLog->m_Lap[ iLapNum ].fLogNum ) * SLIDER_TIME - VsdSt ) /
			( VsdEd - VsdSt ) * ( VideoEd - VideoSt ) + VideoSt;
		
		return Log->GetIndex( dFrame, VideoSt, VideoEd, VsdSt, VsdEd, -1 );
	}
	
	// fLogNum はビデオフレーム番号
	if( VideoSt == VideoEd ) return 0;
	dFrame = m_LapLog->m_Lap[ iLapNum ].fLogNum;
	
	return Log == m_VsdLog ?
		Log->GetIndex( dFrame, VideoSt, VideoEd, VsdSt, VsdEd, -1 ) :
		Log->GetIndex( dFrame, VideoSt, VideoEd, GPSSt, GPSEd, -1 );
}

/*** ラップタイム再生成 (手動) **********************************************/

CLapLog *CVsdFilter::CreateLapTime( int iLapMode ){
	
	int		iTime, iPrevTime;
	int		iFrame = 0;
	double	dLogNum;
	LAP_t	LapTime;
	
	CLapLog *pLapLog = new CLapLog();
	pLapLog->m_iLapMode = iLapMode;
	
	while(( iFrame = GetFrameMark( iFrame )) >= 0 ){
		
		if( pLapLog->m_iLapMode == LAPMODE_HAND_GPS ){
			dLogNum	= GetLogIndex( iFrame, GPS, -1 );
			iTime	= ( int )( m_GPSLog->Time( dLogNum ) * 1000 );
			LapTime.fLogNum	= ( float )dLogNum;
		}else{
			// LAPMODE_HAND_VIDEO
			iTime	= ( int )( iFrame * 1000.0 / GetFPS());
			LapTime.fLogNum	= ( float )iFrame;
		}
		
		if( m_piParamT[ TRACK_SLineWidth ] < 0 ){
			// ジムカーナモード
			LapTime.uLap	= ( pLapLog->m_iLapNum / 2 ) + 1;
			LapTime.iTime	= ( pLapLog->m_iLapNum & 1 ) ? iTime - iPrevTime : 0;
		}else{
			LapTime.uLap	= pLapLog->m_iLapNum;
			LapTime.iTime	= pLapLog->m_iLapNum ? iTime - iPrevTime : 0;
		}
		
		if(
			pLapLog->m_iLapNum &&
			LapTime.iTime &&
			( pLapLog->m_iBestTime == TIME_NONE || pLapLog->m_iBestTime > LapTime.iTime )
		){
			pLapLog->m_iBestTime	= LapTime.iTime;
			pLapLog->m_iBestLap		= pLapLog->m_iLapNum - 1;
		}
		
		iPrevTime = iTime;
		++pLapLog->m_iLapNum;
		++iFrame;
		
		pLapLog->m_Lap.push_back( LapTime );
	}
	
	if( pLapLog->m_Lap.size() == 0 ){
		delete pLapLog;
		return NULL;
	}
	LapTime.fLogNum	= FLT_MAX;	// 番犬
	LapTime.iTime	= 0;		// 番犬
	pLapLog->m_Lap.push_back( LapTime );
	
	return pLapLog;
}

/*** ラップタイム再生成 (GPS auto) ******************************************/

CLapLog *CVsdFilter::CreateLapTimeAuto( void ){
	
	int iFrame;
	LAP_t	LapTime;
	
	if(( iFrame = GetFrameMark( 0 )) < 0 ) return NULL;
	
	/*** スタートラインの位置を取得 ***/
	// iFrame に対応する GPS ログ番号取得
	double dLogNum = GetLogIndex( iFrame, GPS, -1 );
	
	int iLogNum = ( int )dLogNum;
	
	// iLogNum ～ iLogNum + 1 の方位を算出
	
	double dAngle = atan2(
		( m_GPSLog->Y0( iLogNum + 1 ) - m_GPSLog->Y0( iLogNum )),
		( m_GPSLog->X0( iLogNum + 1 ) - m_GPSLog->X0( iLogNum ))
	);
	
	#define x1 m_dStartLineX1
	#define y1 m_dStartLineY1
	#define x2 m_dStartLineX2
	#define y2 m_dStartLineY2
	#define x3 m_GPSLog->X0( i )
	#define y3 m_GPSLog->Y0( i )
	#define x4 m_GPSLog->X0( i + 1 )
	#define y4 m_GPSLog->Y0( i + 1 )
	
	// 仮想光電管の位置を求める
	x2 = m_GPSLog->X0( dLogNum );	// スタート地点
	y2 = m_GPSLog->Y0( dLogNum );
	
	x1 = x2 + cos( dAngle + 90 * ToRAD ) * SLineWidth / 2;
	y1 = y2 + sin( dAngle + 90 * ToRAD ) * SLineWidth / 2;
	x2 = x2 + cos( dAngle - 90 * ToRAD ) * SLineWidth / 2;
	y2 = y2 + sin( dAngle - 90 * ToRAD ) * SLineWidth / 2;
	
	/*****/
	
	CLapLog *pLapLog = new CLapLog();
	pLapLog->m_iLapMode = LAPMODE_GPS;
	
	int iTime, iPrevTime;
	int	iLapNum = 0;
	
	for( int i = 1; i < m_GPSLog->GetCnt() - 1; ++i ){
		
		/*** 交差判定，交点判定 ***/
		double s1, s2, a;
		
		// 交点がスタートライン線分上かの判定
		s1 = ( x4 - x3 ) * ( y1 - y3 ) - ( x1 - x3 ) * ( y4 - y3 );
		s2 = ( x4 - x3 ) * ( y3 - y2 ) - ( x3 - x2 ) * ( y4 - y3 );
		a = ( s1 + s2 == 0 ) ? -1 : s1 / ( s1 + s2 );
		if( !( 0 <= a && a <= 1 )) continue;
		
		// 交点が iLogNum ～ +1 線分上かの判定
		s1 = ( x2 - x1 ) * ( y3 - y1 ) - ( y2 - y1 ) * ( x3 - x1 );
		s2 = ( x2 - x1 ) * ( y1 - y4 ) - ( y2 - y1 ) * ( x1 - x4 );
		a = ( s1 + s2 == 0 ) ? -1 : s1 / ( s1 + s2 );
		if( !( 0 <= a && a < 1 )) continue;
		
		// 進行方向の判定，dAngle ±45度
		double dAngle2 = dAngle - atan2(
			( m_GPSLog->Y0( i + 1 ) - m_GPSLog->Y0( i )),
			( m_GPSLog->X0( i + 1 ) - m_GPSLog->X0( i ))
		);
		if     ( dAngle2 < -180 * ToRAD ) dAngle2 += 360 * ToRAD;
		else if( dAngle2 >  180 * ToRAD ) dAngle2 -= 360 * ToRAD;
		if( dAngle2 < -45 * ToRAD || dAngle2 > 45 * ToRAD ) continue;
		
		#undef x1
		#undef y1
		#undef x2
		#undef y2
		#undef x3
		#undef y3
		#undef x4
		#undef y4
		
		// 半端な LogNum
		dLogNum = i + a;
		iTime = ( int )( m_GPSLog->Time( dLogNum ) * 1000 );
		
		if( m_piParamS[ SHADOW_LAP_START ] - 1 <= iLapNum && iLapNum <= m_piParamS[ SHADOW_LAP_END ] ){
			LapTime.uLap	= pLapLog->m_iLapNum;
			LapTime.fLogNum	= ( float )dLogNum;
			LapTime.iTime	= pLapLog->m_iLapNum ? iTime - iPrevTime : 0;
			
			if(
				pLapLog->m_iLapNum &&
				( pLapLog->m_iBestTime == TIME_NONE || pLapLog->m_iBestTime > LapTime.iTime )
			){
				pLapLog->m_iBestTime	= LapTime.iTime;
				pLapLog->m_iBestLap		= pLapLog->m_iLapNum - 1;
			}
			
			iPrevTime = iTime;
			++pLapLog->m_iLapNum;
			
			pLapLog->m_Lap.push_back( LapTime );
		}
		++iLapNum;
	}
	
	if( pLapLog->m_Lap.size() == 0 ){
		delete pLapLog;
		return NULL;
	}

	LapTime.fLogNum	= FLT_MAX;	// 番犬
	LapTime.iTime	= 0;		// 番犬
	pLapLog->m_Lap.push_back( LapTime );
	
	return pLapLog;
}

/*** ファイルリスト取得 *****************************************************/

// szPath は [ MAX_PATH + 1 ] の配列で，work に使用される
BOOL ListTree( LPTSTR szPath, LPCTSTR szFile, BOOL ( *CallbackFunc )( LPCTSTR, LPCTSTR, void * ), void *pParam ){
	
	HANDLE				hFindFile;		/* find handle						*/
	WIN32_FIND_DATA		FindData;		/* find data struc					*/
	
	BOOL	bSuccess = TRUE;			/* success flag						*/
	
	// szPath が \ で終わってない時の対処
	int iFileIdx = _tcslen( szPath );	// \0
	
	if( iFileIdx != 0 && szPath[ iFileIdx - 1 ] != '\\' ){
		_tcscat_s( szPath, MAX_PATH, _T( "\\" ));
		++iFileIdx;
	}
	
	_tcscat_s( szPath, MAX_PATH, szFile );
	
	if(( hFindFile = FindFirstFile( szPath, &FindData )) != INVALID_HANDLE_VALUE ){
		do{
			if( _tcscmp( FindData.cFileName, _T( "." ))&&
				_tcscmp( FindData.cFileName, _T( ".." ))){
				
				if( FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY ){
					if( _tcscmp( FindData.cFileName, LOG_READER_DIR ) != 0 ){
						_tcscpy_s( &szPath[ iFileIdx ], MAX_PATH - iFileIdx, FindData.cFileName );
						ListTree( szPath, szFile, CallbackFunc, pParam );
					}
				}else{
					szPath[ iFileIdx ] = _T( '\0' );
					if( !CallbackFunc( szPath, FindData.cFileName, pParam )){
						bSuccess = FALSE;
						break;
					}
				}
			}
		}while( FindNextFile( hFindFile, &FindData ));
		
		FindClose( hFindFile );
	}
	return( bSuccess );
}

/****************************************************************************/

BOOL WINAPI DllMain(
	HINSTANCE	hinstDLL,	// handle to DLL module
	DWORD		fdwReason,	// reason for calling function
	LPVOID		lpvReserved	// reserved
){
	if( fdwReason == DLL_PROCESS_ATTACH ){
		CVsdFilter::m_hInst = hinstDLL;
	}
	return TRUE;
}
