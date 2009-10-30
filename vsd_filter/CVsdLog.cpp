/*****************************************************************************
	
	VSD -- vehicle data logger system  Copyright(C) by DDS
	
	CVsdLog.cpp - CVsdLog class implementation
	$Id: CVsdLog.cpp 218 2009-10-29 15:22:40Z  $
	
*****************************************************************************/

#include <windows.h>
#include <stdio.h>
#include <math.h>

#include "dds.h"
#include "../vsd/main.h"

#include "CVsdLog.h"

/*** macros *****************************************************************/

/*** コンストラクタ *********************************************************/

CVsdLog::CVsdLog(){
	m_iCnt	= 0;
	m_Log	= new VSD_LOG_t[ MAX_VSD_LOG ];
	
	float	NaN = 0;
	NaN /= *( volatile float *)&NaN;
	
	// 初期化
	for( UINT u = 0; u < MAX_VSD_LOG; ++u ){
		m_Log[ u ].fX0 = m_Log[ u ].fY0 = NaN;
	}
}

/*** デストラクタ ***********************************************************/

CVsdLog::~CVsdLog(){
	delete [] m_Log;
	m_Log	= NULL;
	m_iCnt	= 0;
}

/*** GPS ログの up-convert **************************************************/

UINT CVsdLog::GPSLogUpConvert( GPS_LOG_t *GPSLog, UINT uCnt ){
	
	UINT	u		= 0;
	BOOL	bReload	= TRUE;
	
	double	dX3, dX2, dX1, dX0;
	double	dY3, dY2, dY1, dY0;
	double	dDiff;
	double	t;
	
	m_iCnt = 0;
	
	if( uCnt < 2 ) return 0;						// 2個データがなければ終了
	GPSLog[ uCnt ].fTime = ( float )999999999.9;	// 番犬
	
	for( ;; ++m_iCnt ){
		
		// GPSLog[ u ].fTime <= m_iCnt / LOG_FREQ < GPSLog[ u + 1 ].fTime
		// の範囲になるよう調整
		
		// m_iCnt が下限を下回っているのでインクリ
		if(( float )m_iCnt / LOG_FREQ < GPSLog[ u ].fTime ){
			bReload = TRUE;
			for( ; ( float )m_iCnt / LOG_FREQ < GPSLog[ u ].fTime; ++m_iCnt );
		}
		
		// m_iCnt が上限を上回っているので，u をインクリ
		if(( float )m_iCnt / LOG_FREQ >= GPSLog[ u + 1 ].fTime ){
			bReload = TRUE;
			for( ; ( float )m_iCnt / LOG_FREQ >= GPSLog[ u + 1 ].fTime; ++u );
			
			// GPS ログ範囲を超えたので return
			if( u > uCnt - 2 ) return m_iCnt;
		}
		
		if( bReload ){
			bReload = FALSE;
			
			dDiff = ( GPSLog[ u + 1 ].fTime - GPSLog[ u ].fTime );
			
			// パラメータをリロード
			#define X0	GPSLog[ u ].fX
			#define Y0	GPSLog[ u ].fY
			#define VX0	( GPSLog[ u ].fVX * dDiff )
			#define VY0	( GPSLog[ u ].fVY * dDiff )
			#define X1	GPSLog[ u + 1 ].fX
			#define Y1	GPSLog[ u + 1 ].fY
			#define VX1	( GPSLog[ u + 1 ].fVX * dDiff )
			#define VY1	( GPSLog[ u + 1 ].fVY * dDiff )
			
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
		}
		
		// 5秒以上 GPS ログがあいていれば，補完情報の計算をしない
		//if( GPSLog[ u + 1 ].fTime - GPSLog[ u ].fTime > 5 ) continue;
		
		t = (( double )m_iCnt / LOG_FREQ - GPSLog[ u ].fTime ) / dDiff;
		
		m_Log[ m_iCnt ].fX0 = ( float )((( dX3 * t + dX2 ) * t + dX1 ) * t + dX0 );
		m_Log[ m_iCnt ].fY0 = ( float )((( dY3 * t + dY2 ) * t + dY1 ) * t + dY0 );
	}
}

/*** MAP 回転処理 ***********************************************************/

void CVsdLog::RotateMap( double dAngle ){
	
	int	i;
	double dMaxX, dMinX, dMaxY, dMinY;
	
	dMaxX = dMinX = dMaxY = dMinY = 0;
	
	for( i = 0; i < m_iCnt; ++i ){
		m_Log[ i ].fX = ( float )(  cos( dAngle ) * m_Log[ i ].fX0 + sin( dAngle ) * m_Log[ i ].fY0 );
		m_Log[ i ].fY = ( float )( -sin( dAngle ) * m_Log[ i ].fX0 + cos( dAngle ) * m_Log[ i ].fY0 );
		
		if     ( dMaxX < m_Log[ i ].fX ) dMaxX = m_Log[ i ].fX;
		else if( dMinX > m_Log[ i ].fX ) dMinX = m_Log[ i ].fX;
		if     ( dMaxY < m_Log[ i ].fY ) dMaxY = m_Log[ i ].fY;
		else if( dMinY > m_Log[ i ].fY ) dMinY = m_Log[ i ].fY;
	}
	
	dMaxX -= dMinX;
	dMaxY -= dMinY;
	
	m_dMapSize	= dMaxX > dMaxY ? dMaxX : dMaxY;
	m_dMapOffsX	= dMinX;
	m_dMapOffsY	= dMinY;
}
