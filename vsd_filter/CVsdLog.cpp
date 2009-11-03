/*****************************************************************************
	
	VSD -- vehicle data logger system  Copyright(C) by DDS
	
	CVsdLog.cpp - CVsdLog class implementation
	$Id$
	
*****************************************************************************/

#include <windows.h>
#include <stdio.h>
#include <math.h>

#include "dds.h"
#include "../vsd/main.h"

#include "CVsdLog.h"

/*** macros *****************************************************************/

#define GRAVITY			9.80665

#ifdef GPS_ONLY
	#define INVERT_GPS_ONLY	(-1)
#else
	#define INVERT_GPS_ONLY	1
#endif

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

UINT CVsdLog::GPSLogUpConvert( GPS_LOG_t *GPSLog, UINT uCnt, BOOL bAllParam ){
	
	UINT	u		= 0;
	
	m_iCnt = 0;
	
	if( uCnt < 2 ) return 0;						// 2個データがなければ終了
	GPSLog[ uCnt ].fTime = ( float )999999999.9;	// 番犬
	
	double	t;
	double	dMileage = 0;
	double	dBearing;
	double	dBearingPrev = GPSLog[ 0 ].fBearing;
	
	m_Log[ 0 ].fSpeed	=
	m_Log[ 0 ].fTacho	=
	m_Log[ 0 ].fGx		=
	m_Log[ 0 ].fGy		=
	m_Log[ 0 ].fMileage	= 0;
	
	for( ;m_iCnt < MAX_VSD_LOG; ++m_iCnt ){
		
		// GPSLog[ u ].fTime <= m_iCnt / LOG_FREQ < GPSLog[ u + 1 ].fTime
		// の範囲になるよう調整
		
		// m_iCnt が下限を下回っているのでインクリ
		if(( float )m_iCnt / LOG_FREQ < GPSLog[ u ].fTime ){
			for( ; ( float )m_iCnt / LOG_FREQ < GPSLog[ u ].fTime; ++m_iCnt );
		}
		
		// m_iCnt が上限を上回っているので，u をインクリ
		if(( float )m_iCnt / LOG_FREQ >= GPSLog[ u + 1 ].fTime ){
			for( ; ( float )m_iCnt / LOG_FREQ >= GPSLog[ u + 1 ].fTime; ++u );
			
			// GPS ログ範囲を超えたので return
			if( u > uCnt - 2 ) break;
			
			// bearing の 360度 wrap around 対策
			if( GPSLog[ u + 1 ].fBearing - GPSLog[ u ].fBearing > 180 )
				GPSLog[ u + 1 ].fBearing -= 360;
			else if( GPSLog[ u + 1 ].fBearing - GPSLog[ u ].fBearing < -180 )
				GPSLog[ u + 1 ].fBearing += 360;
		}
		
		// 5秒以上 GPS ログがあいていれば，補完情報の計算をしない
		//if( GPSLog[ u + 1 ].fTime - GPSLog[ u ].fTime > 5 ) continue;
		
		t = (( double )m_iCnt / LOG_FREQ - GPSLog[ u ].fTime )
			/ ( GPSLog[ u + 1 ].fTime - GPSLog[ u ].fTime );
		
		#define GetLogIntermediateVal( p )\
			( GPSLog[ u ].p * ( 1 - t ) + GPSLog[ u + 1 ].p * t )
		
		m_Log[ m_iCnt ].fX0 = ( float )GetLogIntermediateVal( fX );
		m_Log[ m_iCnt ].fY0 = ( float )GetLogIntermediateVal( fY );
		
		if( bAllParam ){
			// 1秒以上あいていて，5km/h 以下なら 0km/h ログが削られていると判断
			m_Log[ m_iCnt ].fSpeed = ( float )GetLogIntermediateVal( fSpeed );
			if(
				GPSLog[ u + 1 ].fTime - GPSLog[ u ].fTime >= 1 &&
				m_Log[ m_iCnt ].fSpeed < 5
			) m_Log[ m_iCnt ].fSpeed = 0;
			
			if( m_iCnt ){
				dMileage += sqrt(
					pow( m_Log[ m_iCnt - 1 ].fX0 - m_Log[ m_iCnt ].fX0, 2 ) +
					pow( m_Log[ m_iCnt - 1 ].fY0 - m_Log[ m_iCnt ].fY0, 2 )
				);
				
				m_Log[ m_iCnt ].fGy = INVERT_GPS_ONLY * ( float )(
					( m_Log[ m_iCnt ].fSpeed - m_Log[ m_iCnt - 1 ].fSpeed )
					* ( LOG_FREQ / 3.600 / GRAVITY )
				);
				
				// 横G = vω
				// 360度の wrap around
				dBearing = GetLogIntermediateVal( fBearing );
				
				dBearingPrev = dBearing - dBearingPrev;
				if     ( dBearingPrev >  180 ) dBearingPrev -= 360;
				else if( dBearingPrev < -180 ) dBearingPrev += 360;
				
				m_Log[ m_iCnt ].fGx = INVERT_GPS_ONLY * ( float )(
					dBearingPrev * ( ToRAD * LOG_FREQ / GRAVITY )
					* ( m_Log[ m_iCnt ].fSpeed / 3.600 )
				);
				
				dBearingPrev = dBearing;
			}
			m_Log[ m_iCnt ].fMileage = ( float )dMileage;
			m_Log[ m_iCnt ].fTacho = 0;
		}
	}
	
	// スムージング
	if( bAllParam ){
		UINT	v = 3;
		while( v-- ) for( u = 2; u < ( UINT )m_iCnt - 2; ++u ){
			m_Log[ u - 2 ].fGx = (
				m_Log[ u - 2 ].fGx +
				m_Log[ u - 1 ].fGx +
				m_Log[ u + 0 ].fGx +
				m_Log[ u + 1 ].fGx +
				m_Log[ u + 2 ].fGx
			) / 5;
			m_Log[ u - 2 ].fGy = (
				m_Log[ u - 2 ].fGy +
				m_Log[ u - 1 ].fGy +
				m_Log[ u + 0 ].fGy +
				m_Log[ u + 1 ].fGy +
				m_Log[ u + 2 ].fGy
			) / 5;
		}
	}
	return m_iCnt;
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
