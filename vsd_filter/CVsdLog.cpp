/*****************************************************************************
	
	VSD -- vehicle data logger system  Copyright(C) by DDS
	
	CVsdLog.cpp - CVsdLog class implementation
	
*****************************************************************************/

#include "StdAfx.h"

#include "dds.h"
#include "../vsd/main.h"
#include "CVsdLog.h"

/*** macros *****************************************************************/

#define GRAVITY			9.80665

/*** コンストラクタ *********************************************************/

CVsdLog::CVsdLog(){
	m_iCnt	= 0;
	m_Log	= new VSD_LOG_t[ MAX_VSD_LOG ];
	m_dFreq	= LOG_FREQ;
	
	double	NaN = sqrt( -1.0f );
	
	// 初期化
	for( UINT u = 0; u < MAX_VSD_LOG; ++u ){
		m_Log[ u ].SetX0( NaN );
		m_Log[ u ].SetY0( NaN );
	}
	
	m_dMaxG = 0;
	m_dMinG = 0;
	
	m_iMaxSpeed		= 0;
	m_dLogStartTime	= -1;
}

/*** デストラクタ ***********************************************************/

CVsdLog::~CVsdLog(){
	delete [] m_Log;
	m_Log	= NULL;
	m_iCnt	= 0;
}

/*** GPS ログの up-convert **************************************************/

UINT CVsdLog::GPSLogUpConvert( GPS_LOG_t *GPSLog, UINT uCnt, BOOL bAllParam ){
	
	UINT	u;
	
	m_iCnt = 0;
	
	if( uCnt < 2 ) return 0;			// 2個データがなければ終了
	GPSLog[ uCnt ].SetTime( FLT_MAX );	// 番犬
	
	double	t;
	double	dMileage = 0;
	double	dBearing;
	
	m_Log[ 0 ].SetSpeed( 0 );
	m_Log[ 0 ].SetTacho( 0 );
	m_Log[ 0 ].SetGx( 0 );
	m_Log[ 0 ].SetGy( 0 );
	m_Log[ 0 ].SetMileage( 0 );
	
	/*** GPS_LOG_t の方を走査して，いろいろ補正 *****************************/
	
	for( u = 1; u < uCnt - 1; ++u ){
		// speed がない場合の補正
		if( GPSLog[ u ].Speed() == FLT_MAX ){
			GPSLog[ u ].SetSpeed(
				sqrt(
					pow( GPSLog[ u + 1 ].X() - GPSLog[ u - 1 ].X(), 2 ) +
					pow( GPSLog[ u + 1 ].Y() - GPSLog[ u - 1 ].Y(), 2 )
				) / ( GPSLog[ u + 1 ].Time() - GPSLog[ u - 1 ].Time() )
				* ( 3600.0 / 1000 )
			);
		}
		
		// bearing がない場合の補正
		//if( GPSLog[ u ].Bearing() == FLT_MAX ){
		if( 1 ){	// GPS データの bearing は無視
			GPSLog[ u ].SetBearing(
				fmod(
					atan2(
						GPSLog[ u + 1 ].Y() - GPSLog[ u - 1 ].Y(),
						GPSLog[ u + 1 ].X() - GPSLog[ u - 1 ].X()
					) / ToRAD + 360 * 2 + 90,
					360
				)
			);
		}
	}
	
	GPSLog[ 0 ].SetSpeed(			GPSLog[ 1 ].Speed());
	GPSLog[ 0 ].SetBearing(			GPSLog[ 1 ].Bearing());
	GPSLog[ uCnt - 1 ].SetSpeed(	GPSLog[ uCnt - 2 ].Speed());
	GPSLog[ uCnt - 1 ].SetBearing(	GPSLog[ uCnt - 2 ].Bearing());
	
	for( u = 1; u < uCnt - 1; ++u ){
		// Gx / Gy を作る
		GPSLog[ u ].SetGy(
			( GPSLog[ u + 1 ].Speed() - GPSLog[ u - 1 ].Speed() )
			* ( 1 / 3.600 / GRAVITY )
			/ ( GPSLog[ u + 1 ].Time() - GPSLog[ u - 1 ].Time() )
		);
		
		// 横G = vω
		dBearing = GPSLog[ u + 1 ].Bearing() - GPSLog[ u - 1 ].Bearing();
		if     ( dBearing >  180 ) dBearing -= 360;
		else if( dBearing < -180 ) dBearing += 360;
		
		GPSLog[ u ].SetGx(
			dBearing * ( ToRAD / GRAVITY )
			/ ( GPSLog[ u + 1 ].Time() - GPSLog[ u - 1 ].Time())
			* ( GPSLog[ u ].Speed() / 3.600 )
		);
		
		// ±5G 以上は，削除
		if( GPSLog[ u ].Gx() < -3 || GPSLog[ u ].Gx() > 3 ){
			GPSLog[ u ].SetGx( GPSLog[ u - 1 ].Gx());
		}
	}
	
	GPSLog[ 0 ].SetGx(			GPSLog[ 1 ].Gx());
	GPSLog[ 0 ].SetGy(			GPSLog[ 1 ].Gy());
	GPSLog[ uCnt - 1 ].SetGx(	GPSLog[ uCnt - 2 ].Gx());
	GPSLog[ uCnt - 1 ].SetGy(	GPSLog[ uCnt - 2 ].Gy());
	
	/************************************************************************/
	
	for( m_iCnt = 0, u = 0;m_iCnt < MAX_VSD_LOG; ++m_iCnt ){
		
		// GPSLog[ u ].Time() <= m_iCnt / LOG_FREQ < GPSLog[ u + 1 ].Time()
		// の範囲になるよう調整
		
		// m_iCnt が下限を下回っているのでインクリ
		if(( float )m_iCnt / LOG_FREQ < GPSLog[ u ].Time() ){
			for( ; ( float )m_iCnt / LOG_FREQ < GPSLog[ u ].Time(); ++m_iCnt );
		}
		
		// m_iCnt が上限を上回っているので，u をインクリ
		if(( float )m_iCnt / LOG_FREQ >= GPSLog[ u + 1 ].Time() ){
			for( ; ( float )m_iCnt / LOG_FREQ >= GPSLog[ u + 1 ].Time(); ++u );
			
			// GPS ログ範囲を超えたので return
			if( u > uCnt - 2 ) break;
		}
		
		// 5秒以上 GPS ログがあいていれば，補完情報の計算をしない
		//if( GPSLog[ u + 1 ].Time() - GPSLog[ u ].Time() > 5 ) continue;
		
		t = (( double )m_iCnt / LOG_FREQ - GPSLog[ u ].Time() )
			/ ( GPSLog[ u + 1 ].Time() - GPSLog[ u ].Time() );
		
		#define GetLogIntermediateVal( p )\
			( GPSLog[ u ].p() * ( 1 - t ) + GPSLog[ u + 1 ].p() * t )
		
		m_Log[ m_iCnt ].SetX0( GetLogIntermediateVal( X ));
		m_Log[ m_iCnt ].SetY0( GetLogIntermediateVal( Y ));
		
		if( bAllParam ){
			m_Log[ m_iCnt ].SetSpeed( GetLogIntermediateVal( Speed ));
			
			if( m_iMaxSpeed < m_Log[ m_iCnt ].Speed() )
				m_iMaxSpeed = ( int )ceil( m_Log[ m_iCnt ].Speed() / 10 ) * 10;
			
			if( m_iCnt ){
				dMileage += sqrt(
					pow( m_Log[ m_iCnt - 1 ].X0() - m_Log[ m_iCnt ].X0(), 2 ) +
					pow( m_Log[ m_iCnt - 1 ].Y0() - m_Log[ m_iCnt ].Y0(), 2 )
				);
			}
			
			m_Log[ m_iCnt ].SetMileage( dMileage );
			m_Log[ m_iCnt ].SetTacho( 0 );
			
			m_Log[ m_iCnt ].SetGx( GetLogIntermediateVal( Gx ));
			m_Log[ m_iCnt ].SetGy( GetLogIntermediateVal( Gy ));
		}else{
			// PSP GPS log のときは，G の MAX 値のみをチェック
			if( m_dMaxG < m_Log[ m_iCnt ].Gy() ) m_dMaxG = m_Log[ m_iCnt ].Gy();
			if( m_dMinG > m_Log[ m_iCnt ].Gy() ) m_dMinG = m_Log[ m_iCnt ].Gy();
		}
	}
	
	// スムージング
	if( bAllParam ){
		UINT	v = 2;
		double	d;
		double	d2 = 0;
		
		while( v-- ) for( u = 2; u < ( UINT )m_iCnt - 2; ++u ){
			m_Log[ u ].SetGx((
				m_Log[ u - 2 ].Gx() +
				m_Log[ u - 1 ].Gx() +
				m_Log[ u + 0 ].Gx() +
				m_Log[ u + 1 ].Gx() +
				m_Log[ u + 2 ].Gx()
			) / 5 );
			m_Log[ u ].SetGy( d = (
				m_Log[ u - 2 ].Gy() +
				m_Log[ u - 1 ].Gy() +
				m_Log[ u + 0 ].Gy() +
				m_Log[ u + 1 ].Gy() +
				m_Log[ u + 2 ].Gy()
			) / 5 );
			
			d2 = d2 * 0.9 + d * 0.1;
			if( m_dMaxG < d2 ) m_dMaxG = d2;
			if( m_dMinG > d2 ) m_dMinG = d2;
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
		if( _isnan( m_Log[ i ].X0() )){
			m_Log[ i ].SetX( m_Log[ i ].X0());
			m_Log[ i ].SetY( m_Log[ i ].Y0());
		}else{
			m_Log[ i ].SetX(  cos( dAngle ) * m_Log[ i ].X0() + sin( dAngle ) * m_Log[ i ].Y0());
			m_Log[ i ].SetY( -sin( dAngle ) * m_Log[ i ].X0() + cos( dAngle ) * m_Log[ i ].Y0());
			
			if     ( dMaxX < m_Log[ i ].X() ) dMaxX = m_Log[ i ].X();
			else if( dMinX > m_Log[ i ].X() ) dMinX = m_Log[ i ].X();
			if     ( dMaxY < m_Log[ i ].Y() ) dMaxY = m_Log[ i ].Y();
			else if( dMinY > m_Log[ i ].Y() ) dMinY = m_Log[ i ].Y();
		}
	}
	
	dMaxX -= dMinX;
	dMaxY -= dMinY;
	
	m_dMapSize	= dMaxX > dMaxY ? dMaxX : dMaxY;
	m_dMapOffsX	= dMinX;
	m_dMapOffsY	= dMinY;
}
