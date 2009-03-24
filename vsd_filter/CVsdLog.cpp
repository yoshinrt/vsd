/*****************************************************************************
	
	VSD -- vehicle data logger system  Copyright(C) by DDS
	
	CVsdLog.cpp - CVsdLog class implementation
	$Id$
	
*****************************************************************************/

#include <windows.h>
#include <stdio.h>
#include <math.h>
#include <stddef.h>

#include "vsd_filter.h"
#include "dds.h"
#include "../vsd/main.h"
#include "dds_lib/dds_lib.h"

#ifndef AVS_PLUGIN
	#include "filter.h"
#endif
#include "CVsdLog.h"

/*** macros *****************************************************************/

#define LAT_M_DEG	110863.95	// 緯度1度の距離
#define LNG_M_DEG	111195.10	// 経度1度の距離 @ 0N

/*** static member **********************************************************/

/*** tarckbar / checkbox id 名 ***/

const char *CVsdLog::m_szTrackbarName[] = {
	#define DEF_TRACKBAR( id, init, min, max, name ) #id,
	#include "def_trackbar.h"
};

const char *CVsdLog::m_szCheckboxName[] = {
	#define DEF_CHECKBOX( id, init, name ) #id,
	#include "def_checkbox.h"
};


/*** コンストラクタ *********************************************************/

CVsdLog::CVsdLog () {
	
	m_VsdLog 		= new VSD_LOG_t[ MAX_VSD_LOG ];;
	m_iVsdLogNum	= 0;
	
	m_Lap	 			= new LAP_t[ MAX_LAP ];
	m_iLapNum			= 0;
	m_Lap[ 0 ].iLogNum	= 0x7FFFFFFF;	// 番犬
	m_Lap[ 0 ].fTime	= 0;			// 番犬
	
	m_fBestTime		= -1;
	m_iBestLapLogNum= 0;
	
	m_dMapSize		= 0;
	m_dMapOffsX		= 0;
	m_dMapOffsY		= 0;
	
	m_dVideoFPS		= 30.0;
	
	m_bCalcLapTimeReq	= FALSE;
}

/*** デストラクタ ***********************************************************/

CVsdLog::~CVsdLog () {
	delete m_VsdLog;
	m_VsdLog	= NULL;
	m_iVsdLogNum= 0;
	
	delete m_Lap;
	m_Lap		= NULL;
	m_iLapNum	= 0;
}

/*** 設定ロード・セーブ *****************************************************/

BOOL CVsdLog::ConfigLoad( char *szFileName ){
	
	int 	i, iLen;
	FILE	*fp;
	char	szBuf[ BUF_SIZE ];
	BOOL	bMark = FALSE;
	
#ifndef AVS_PLUGIN
	FRAME_STATUS	fsp;
#endif
	
	if(( fp = fopen( szFileName, "r" )) != NULL ){
		m_bCalcLapTimeReq = TRUE;
		
		while( fgets( szBuf, BUF_SIZE, fp )){
			if( bMark ){
				// マークセット
				i = atoi( szBuf );
			#ifndef AVS_PLUGIN
				filter->exfunc->get_frame_status( editp, i, &fsp );
				fsp.edit_flag |= EDIT_FRAME_EDIT_FLAG_MARKFRAME;
				filter->exfunc->set_frame_status( editp, i, &fsp );
			#endif
				
			}else if( strncmp( szBuf, "MARK:", 5 ) == 0 ){
				// マークモードに入る
				bMark = TRUE;
				
			}else{
				// Mark 以外のパラメータ
				for( i = 0; i < TRACK_N; ++i ){
					if(
						strncmp( szBuf, m_szTrackbarName[ i ], iLen = strlen( m_szTrackbarName[ i ] )) == 0 &&
						szBuf[ iLen ] == '='
					){
						m_piParamT[ i ] = atoi( szBuf + iLen + 1 );
						goto Next;
					}
				}
				
				for( i = 0; i < CHECK_N; ++i ){
					if(
						strncmp( szBuf, m_szCheckboxName[ i ], iLen = strlen( m_szCheckboxName[ i ] )) == 0 &&
						szBuf[ iLen ] == '='
					){
						m_piParamC[ i ] = atoi( szBuf + iLen + 1 );
						goto Next;
					}
				}
			}
		  Next: ;
		}
		fclose( fp );
	}
	return TRUE;
}

#ifndef AVS_PLUGIN
BOOL CVsdLog::ConfigSave( char *szFileName ){
	FILE	*fp;
	int		i;
	
	if(( fp = fopen( szFileName, "w" )) == NULL ) return FALSE;
	
	for( i = 0; i < TRACK_N; ++i ){
		fprintf( fp, "%s=%u\n", m_szTrackbarName[ i ], m_piParamT[ i ] );
	}
	
	for( i = 0; i < CHECK_N; ++i ){
		if(
			i == CHECK_FRAME
			#ifndef CIRCUIT_TOMO
			|| i == CHECK_LOGPOS
			#endif
		) continue;
		
		fprintf( fp, "%s=%u\n", m_szCheckboxName[ i ], m_piParamC[ i ] );
	}
	
	// 手動ラップ計測マーク出力
	if( IsHandLaptime() && m_iLapNum ){
		fputs( "MARK:\n", fp );
		
		for( i = 0; i < m_iLapNum; ++i ){
			fprintf( fp, "%u\n", m_Lap[ i ].iLogNum );
		}
	}
	
	fclose( fp );
	return TRUE;
}
#endif

/*** ログリード *************************************************************/

#ifdef CIRCUIT_TOMO
UINT CVsdLog::ReadPTD( FILE *fp, UINT uOffs ){
	
	UINT	uCnt		= 0;
	UINT	uPulseCnt	= 0;
	UINT	uOutCnt		= 0;
	UINT	uPrevTime	= 0;
	UINT	u;
	
	fscanf( fp, "%u", &uCnt );
	
	while( uCnt ){
		do{
			fscanf( fp, "%u", &u );
			
			--uCnt;
			++uPulseCnt;
		}while(( double )u / PTD_LOG_FREQ < ( double )uOutCnt / LOG_FREQ && uCnt );
		
		while(( double )uOutCnt / LOG_FREQ < ( double )u / PTD_LOG_FREQ ){
			*( float *)(( char *)( &m_VsdLog[ uOutCnt ] ) + uOffs ) =
				( float )(( double )uPulseCnt / ( u - uPrevTime ) * PTD_LOG_FREQ );
			++uOutCnt;
		}
		uPrevTime = u;
		uPulseCnt = 0;
	}
	return( uOutCnt );
}
#endif

BOOL CVsdLog::ReadLog( char *szFileName ){
	TCHAR	szBuf[ BUF_SIZE ];
	FILE	*fp;
	BOOL	bCalibrating = FALSE;
	
	float		NaN = 0;
	NaN /= *( volatile float *)&NaN;
	
	// config ロード
	ConfigLoad( ChangeExt( szBuf, szFileName, "cfg" ));
	
	/******************/
	
	if( IsExt( szFileName, "cfg" ) || ( fp = fopen( szFileName, "r" )) == NULL ) return FALSE;
	
#ifdef CIRCUIT_TOMO
	int i;
	for( i = 0; i < MAX_VSD_LOG; ++i ){
		m_VsdLog[ i ].fSpeed = m_VsdLog[ i ].fTacho = 0;
	}
	
	m_iVsdLogNum	= ReadPTD( fp, offsetof( VSD_LOG_t, fTacho ));
	i				= ReadPTD( fp, offsetof( VSD_LOG_t, fSpeed ));
	
	if( i > m_iVsdLogNum ) m_iVsdLogNum = i;
	
#else // CIRCUIT_TOMO
	
	// GPS ログ用
	UINT		uGPSCnt = 0;
	GPS_LOG_t	*GPSLog = new GPS_LOG_t[ ( int )( MAX_VSD_LOG / LOG_FREQ ) ];
	
	// 20070814 以降のログは，横 G が反転している
	BOOL bReverseGy	= ( strcmp( StrTokFile( NULL, szBuf, STF_NAME ), "vsd20070814" ) >= 0 );
	
	// ログリード
	
	UINT	uCnt, uLap, uMin, uSec, uMSec;
	double	dGcx = 0;
	double	dGcy = 0;
	double	dGx, dGy;
	
	float	fLati;
	float	fLong;
	double	dLati0 = 0;
	double	dLong0 = 0;
	float	fSpeed;
	float	fBearing;
	
	m_iLogStart = m_iLogStop = 0;
	
	TCHAR	*p;
	
	while( fgets( szBuf, BUF_SIZE, fp ) != NULL ){
		if(( p = strstr( szBuf, "LAP" )) != NULL ){ // ラップタイム記録を見つけた
			uCnt = sscanf( p, "LAP%d%d:%d.%d", &uLap, &uMin, &uSec, &uMSec );
			
			double dTime = uMin * 60 + uSec + ( double )uMSec / 1000;
			
			m_Lap[ m_iLapNum ].uLap		= uLap;
			m_Lap[ m_iLapNum ].iLogNum	= m_iVsdLogNum;
			m_Lap[ m_iLapNum ].fTime	=
				( uCnt == 4 ) ? ( float )dTime : 0;
			
			if(
				uCnt == 4 &&
				( m_fBestTime == -1 || m_fBestTime > dTime )
			){
				m_fBestTime			= ( float )dTime;
				m_iBestLapLogNum	= m_Lap[ m_iLapNum - 1 ].iLogNum;
			}
			++m_iLapNum;
		}
		
		if(( p = strstr( szBuf, "GPS" )) != NULL ){ // GPS記録を見つけた
			sscanf( p, "GPS%g%g%g%g", &fLati, &fLong, &fSpeed, &fBearing );
			
			if( dLati0 == 0 ){
				dLati0 = fLati;
				dLong0 = fLong;
			}
			
			// 単位を補正
			// 緯度・経度→メートル
			GPSLog[ uGPSCnt ].fX = ( float )(( fLong - dLong0 ) * LAT_M_DEG * cos( fLati * ToRAD ));
			GPSLog[ uGPSCnt ].fY = ( float )(( dLati0 - fLati ) * LNG_M_DEG );
			
			// 速度・向き→ベクトル座標
			double dBearing	= fBearing * ToRAD;
			double dSpeed	= fSpeed * ( 1000.0 / 3600 );
			GPSLog[ uGPSCnt ].fVX	= ( float )(  sin( dBearing ) * dSpeed );
			GPSLog[ uGPSCnt ].fVY	= ( float )( -cos( dBearing ) * dSpeed );
			
			GPSLog[ uGPSCnt++ ].iLogNum =
				( m_iVsdLogNum - GPS_LOG_OFFS ) >= 0 ?
					( m_iVsdLogNum - GPS_LOG_OFFS ) : 0;
		}
		
		// 普通の log
		if(( uCnt = sscanf( szBuf, "%g%g%g%lg%lg",
			&m_VsdLog[ m_iVsdLogNum ].fTacho,
			&m_VsdLog[ m_iVsdLogNum ].fSpeed,
			&m_VsdLog[ m_iVsdLogNum ].fMileage,
			&dGy,
			&dGx
		)) >= 3 ){
			if( uCnt < 5 && m_iVsdLogNum ){
				// Gデータがないときは，speedから求める←廃止
				dGx = 0;
				dGy = 0;
				//dGy = ( m_VsdLog[ m_iVsdLogNum ].fSpeed - m_VsdLog[ m_iVsdLogNum - 1 ].fSpeed ) * ( 1000.0 / 3600 / 9.8 * LOG_FREQ );
			}else if( dGx >= 4 ){	// 4 以下なら，すでに G が計算済み
				if( bReverseGy ) dGx = -dGx;
				
				if( m_iVsdLogNum < G_CX_CNT ){
					// G センター検出
					dGcx += dGx;
					dGcy += dGy;
					dGx = dGy = 0;
					
					if( m_iVsdLogNum == G_CX_CNT - 1 ){
						dGcx /= G_CX_CNT;
						dGcy /= G_CX_CNT;
					}
				}else{
					// 単位を G に変換
					dGx = ( dGx - dGcx ) / ACC_1G_Y;
					dGy = ( dGy - dGcy ) / ( bReverseGy ? ACC_1G_Z : ACC_1G_X );
				}
			}
			m_VsdLog[ m_iVsdLogNum ].fGx = ( float )dGx;
			m_VsdLog[ m_iVsdLogNum ].fGy = ( float )dGy;
			
			m_VsdLog[ m_iVsdLogNum ].fX0 = m_VsdLog[ m_iVsdLogNum ].fY0 = INVALID_POS_D;
			
			// ログ開始・終了認識
			if( m_VsdLog[ m_iVsdLogNum ].fSpeed >= 300 ){
				if( !bCalibrating ){
					bCalibrating = TRUE;
					m_iLogStart  = m_iLogStop;
					m_iLogStop   = m_iVsdLogNum;
				}
			}else{
				bCalibrating = FALSE;
			}
			
			++m_iVsdLogNum;
		}
	}
	
	/*** GPS ログから軌跡を求める ***************************************/
	
	if( uGPSCnt ){
		UINT	u;
		
		// 補完点を計算
		for( u = 0; u < uGPSCnt - 1; ++u ){
			// 2秒以上 GPS ログがあいていれば，補完情報の計算をしない
			if( GPSLog[ u + 1 ].iLogNum - GPSLog[ u ].iLogNum > ( int )( LOG_FREQ * 5 )) continue;
			
			double dX3, dX2, dX1, dX0;
			double dY3, dY2, dY1, dY0;
			
			#define X0	GPSLog[ u ].fX
			#define Y0	GPSLog[ u ].fY
			#define VX0	GPSLog[ u ].fVX
			#define VY0	GPSLog[ u ].fVY
			#define X1	GPSLog[ u + 1 ].fX
			#define Y1	GPSLog[ u + 1 ].fY
			#define VX1	GPSLog[ u + 1 ].fVX
			#define VY1	GPSLog[ u + 1 ].fVY
			
			dX3 = 2 * ( X0 - X1 ) + VX0 + VX1;
			dX2 = 3 * ( -X0 + X1 ) - 2 * VX0 - VX1;
			dX1 = VX0;
			dX0 = X0;
			dY3 = 2 * ( Y0 - Y1 ) + VY0 + VY1;
			dY2 = 3 * ( -Y0 + Y1 ) - 2 * VY0 - VY1;
			dY1 = VY0;
			dY0 = Y0;
			
			#undef X0
			#undef Y0
			#undef VX0
			#undef VY0
			#undef X1
			#undef Y1
			#undef VX1
			#undef VY1
			
			for( int i = GPSLog[ u ].iLogNum; i <= GPSLog[ u + 1 ].iLogNum; ++i ){
				double t =
					( double )( i - GPSLog[ u ].iLogNum ) /
					( GPSLog[ u + 1 ].iLogNum - GPSLog[ u ].iLogNum );
				
				m_VsdLog[ i ].fX0 = ( float )((( dX3 * t + dX2 ) * t + dX1 ) * t + dX0 );
				m_VsdLog[ i ].fY0 = ( float )((( dY3 * t + dY2 ) * t + dY1 ) * t + dY0 );
			}
		}
		
		// Map 回転
		RotateMap();
		
		/*
		FILE *fp = fopen( "c:\\dds\\hoge.log", "w" );
		for( i = 0; i < m_iVsdLogNum; ++i )
			fprintf( fp, "%g\t%g\n", m_VsdLog[ i ].fX, m_VsdLog[ i ].fY );
		fclose( fp );
		*/
	}
	
	delete [] GPSLog;
#endif // CIRCUIT_TOMO
	
	/********************************************************************/
	
	fclose( fp );
	
	m_Lap[ m_iLapNum ].iLogNum	= 0x7FFFFFFF;	// 番犬
	m_Lap[ m_iLapNum ].fTime	= 0;			// 番犬
	
	return TRUE;
}

/*** MAP 回転処理 ***********************************************************/

#ifndef CIRCUIT_TOMO
void CVsdLog::RotateMap( void ){
	
	int i;
	double dMaxX, dMinX, dMaxY, dMinY;
	double dMapAngle = ( m_piParamT[ TRACK_MapAngle ] * ( -ToRAD / 10 ));
	
	dMaxX = dMinX = dMaxY = dMinY = 0;
	
	for( i = 0; i < m_iVsdLogNum; ++i ){
		m_VsdLog[ i ].fX = ( float )(  cos( dMapAngle ) * m_VsdLog[ i ].fX0 + sin( dMapAngle ) * m_VsdLog[ i ].fY0 );
		m_VsdLog[ i ].fY = ( float )( -sin( dMapAngle ) * m_VsdLog[ i ].fX0 + cos( dMapAngle ) * m_VsdLog[ i ].fY0 );
		
		if     ( dMaxX < m_VsdLog[ i ].fX ) dMaxX = m_VsdLog[ i ].fX;
		else if( dMinX > m_VsdLog[ i ].fX ) dMinX = m_VsdLog[ i ].fX;
		if     ( dMaxY < m_VsdLog[ i ].fY ) dMaxY = m_VsdLog[ i ].fY;
		else if( dMinY > m_VsdLog[ i ].fY ) dMinY = m_VsdLog[ i ].fY;
	}
	
	dMaxX -= dMinX;
	dMaxY -= dMinY;
	
	m_dMapSize	= dMaxX > dMaxY ? dMaxX : dMaxY;
	m_dMapOffsX		= dMinX;
	m_dMapOffsY		= dMinY;
}
#endif
