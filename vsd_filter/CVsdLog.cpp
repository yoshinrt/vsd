/*****************************************************************************
	
	VSD -- vehicle data logger system  Copyright(C) by DDS
	
	CVsdLog.cpp - CVsdLog class implementation
	
*****************************************************************************/

#include "StdAfx.h"

#include "dds.h"
#include "dds_lib/dds_lib.h"
#include "../vsd/main.h"
#include "CVsdLog.h"

/*** macros *****************************************************************/

#define GRAVITY			9.80665
#define GPS_LOG_OFFS	15

/*** コンストラクタ *********************************************************/

CVsdLog::CVsdLog(){
	m_iCnt	= 0;
	m_dFreq	= LOG_FREQ;
	
	m_dMaxGx =
	m_dMaxGy =
	m_dMinGy = 0;
	
	m_iMaxSpeed		= 0;
	m_iMaxTacho		= 0;
	m_dLogStartTime	= -1;
}

/*** fraem# に対応するログ index を得る *************************************/

double CVsdLog::GetIndex(
	double	dFrame,
	int iVidSt, int iVidEd,
	int iLogSt, int iLogEd,
	int iPrevIdx
){
	if( iVidSt == iVidEd ) return -1;
	
	double dTime = ( iLogSt + ( iLogEd - iLogSt ) * ( dFrame - iVidSt ) / ( double )( iVidEd - iVidSt )) / SLIDER_TIME;
	
	int idx;
	
	if( dTime < 0 ) return -1;
	if( dTime >= m_Log[ m_iCnt - 1 ].Time()) return m_iCnt;
	
	// m_Log[ idx ].Time() <= dTime < m_Log[ idx + 1 ].Time()
	// となる idx を見つける
	if(
		iPrevIdx < 0 || iPrevIdx >= m_iCnt ||
		m_Log[ iPrevIdx ].Time() > dTime
	){
		// iPrevIdx がおかしいので，binary serch する
		int iSt = 0;
		int iEd = m_iCnt - 1;
		while( iSt != iEd ){
			idx = ( iSt + iEd ) / 2;
			if( m_Log[ idx ].Time() > dTime ){
				iEd = idx - 1;
			}else if( m_Log[ idx + 1 ].Time() <= dTime ){
				iSt = idx + 1;
			}else{
				// ヒット
				break;
			}
		}
	}else{
		// iPrevIdx は正常なので，これを起点に単純サーチする
		idx = iPrevIdx;
		while( m_Log[ idx + 1 ].Time() <= dTime ) ++idx;
	}
	
	// index の端数を求める
	return idx +
		( dTime                   - m_Log[ idx ].Time()) /
		( m_Log[ idx + 1 ].Time() - m_Log[ idx ].Time());
}

/*** GPS ログの up-convert **************************************************/

UINT CVsdLog::GPSLogUpConvert( std::vector<GPS_LOG_t>& GPSLog, BOOL bAllParam ){
	
	UINT	u;
	
	m_iCnt = 0;
	
	UINT uCnt = GPSLog.size();
	if( uCnt < 2 ) return 0;			// 2個データがなければ終了
	
	GPS_LOG_t	VsdLogTmp;
	VsdLogTmp.SetTime( FLT_MAX );
	GPSLog.push_back( VsdLogTmp );		// 番犬
	
	double	dDistance = 0;
	double	dBearing;
	
	/*
	VSD_LOG_t	LogTmp;
	
	LogTmp.SetSpeed( 0 );
	LogTmp.SetTacho( 0 );
	LogTmp.SetGx( 0 );
	LogTmp.SetGy( 0 );
	LogTmp.SetDistance( 0 );
	
	m_Log.push_back( LogTmp );
	*/
	
	/*** GPS_LOG_t の方を走査して，いろいろ補正 *****************************/
	
	for( u = 1; u < uCnt - 1; ++u ){
		// speed がない場合の補正
		if( !GPSLog[ u ].IsValidSpeed()){
			GPSLog[ u ].SetSpeed(
				sqrt(
					pow( GPSLog[ u + 1 ].X() - GPSLog[ u - 1 ].X(), 2 ) +
					pow( GPSLog[ u + 1 ].Y() - GPSLog[ u - 1 ].Y(), 2 )
				) / ( GPSLog[ u + 1 ].Time() - GPSLog[ u - 1 ].Time())
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
			( GPSLog[ u + 1 ].Speed() - GPSLog[ u - 1 ].Speed())
			* ( 1 / 3.600 / GRAVITY )
			/ ( GPSLog[ u + 1 ].Time() - GPSLog[ u - 1 ].Time())
		);
		if( GPSLog[ u ].Gy() > 10 ){
			int a= 0;
		}
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
	
	for( m_iCnt = 0; m_iCnt < ( int )uCnt - 2; ++m_iCnt ){
		// 5秒以上 GPS ログがあいていれば，補完情報の計算をしない
		//if( GPSLog[ m_iCnt + 1 ].Time() - GPSLog[ m_iCnt ].Time() > 5 ) continue;
		
		#define GetLogIntermediateVal( p ) GPSLog[ m_iCnt ].p()
		
		if( bAllParam ){
			VSD_LOG_t	LogTmp;
			m_Log.push_back( LogTmp );
		}
		
		m_Log[ m_iCnt ].SetTime( GetLogIntermediateVal( Time ));
		m_Log[ m_iCnt ].SetX0( GetLogIntermediateVal( X ));
		m_Log[ m_iCnt ].SetY0( GetLogIntermediateVal( Y ));
		
		if( bAllParam ){
			m_Log[ m_iCnt ].SetSpeed( GetLogIntermediateVal( Speed ));
			
			if( m_iMaxSpeed < m_Log[ m_iCnt ].Speed())
				m_iMaxSpeed = ( int )ceil( m_Log[ m_iCnt ].Speed());
			
			if( m_iCnt ){
				dDistance += sqrt(
					pow( m_Log[ m_iCnt - 1 ].X0() - m_Log[ m_iCnt ].X0(), 2 ) +
					pow( m_Log[ m_iCnt - 1 ].Y0() - m_Log[ m_iCnt ].Y0(), 2 )
				);
			}
			
			m_Log[ m_iCnt ].SetDistance( dDistance );
			m_Log[ m_iCnt ].SetTacho( 0 );
			
			m_Log[ m_iCnt ].SetGx( GetLogIntermediateVal( Gx ));
			m_Log[ m_iCnt ].SetGy( GetLogIntermediateVal( Gy ));
		}else{
			// PSP GPS log のときは，G の MAX 値のみをチェック
			if( m_dMaxGy < m_Log[ m_iCnt ].Gy()) m_dMaxGy = m_Log[ m_iCnt ].Gy();
			if( m_dMinGy > m_Log[ m_iCnt ].Gy()) m_dMinGy = m_Log[ m_iCnt ].Gy();
		}
	}
	
	// スムージング
	if( bAllParam ){
		UINT	v = 2;
		double	dGx0, dGx1 = 0;
		double	dGy0, dGy1 = 0;
		
		while( v-- ) for( u = 2; u < ( UINT )m_iCnt - 2; ++u ){
			m_Log[ u ].SetGx( dGx0 = (
				m_Log[ u - 2 ].Gx() +
				m_Log[ u - 1 ].Gx() +
				m_Log[ u + 0 ].Gx() +
				m_Log[ u + 1 ].Gx() +
				m_Log[ u + 2 ].Gx()
			) / 5 );
			m_Log[ u ].SetGy( dGy0 = (
				m_Log[ u - 2 ].Gy() +
				m_Log[ u - 1 ].Gy() +
				m_Log[ u + 0 ].Gy() +
				m_Log[ u + 1 ].Gy() +
				m_Log[ u + 2 ].Gy()
			) / 5 );
			
			dGx1 = dGx1 * 0.9 + dGx0 * 0.1;
			dGy1 = dGy1 * 0.9 + dGy0 * 0.1;
			if( m_dMaxGx < fabs( dGx1 )) m_dMaxGx = fabs( dGx1 );
			if( m_dMaxGy < dGy1 ) m_dMaxGy = dGy1;
			if( m_dMinGy > dGy1 ) m_dMinGy = dGy1;
		}
	}
	return m_iCnt;
}

/*** GPS ログリード ********************************************************/

double CVsdLog::GPSLogGetLength(
	double dLong0, double dLati0,
	double dLong1, double dLati1
){
	// ヒュベニの公式 http://yamadarake.jp/trdi/report000001.html
	const double a	= 6378137.000;
	const double b	= 6356752.314245;
	const double e2	= ( a * a - b * b ) / ( a * a );
	
	double dx	= ( dLong1 - dLong0 ) * ToRAD;
	double dy	= ( dLati1 - dLati0 ) * ToRAD;
	double uy	= ( dLati0 + dLati1 ) / 2 * ToRAD;
	double W	= sqrt( 1 - e2 * sin( uy ) * sin( uy ));
	double M	= a * ( 1 - e2 ) / pow( W, 3 );
	double N	= a / W;
	
	return	sqrt( dy * dy * M * M + pow( dx * N * cos( uy ), 2 ));
}

#define	getcwd	_getcwd
#define	chdir	_chdir

int CVsdLog::ReadGPSLog( const char *szFileName ){
	UINT	uGPSCnt = 0;
	TCHAR	szCurDir[ MAX_PATH ];
	TCHAR	szBuf[ BUF_SIZE ];
	
	double	dLati, dLati0 = 0;
	double	dLong, dLong0 = 0;
	double	dLati2Meter, dLong2Meter;
	double	dSpeed;
	double	dBearing;
	double	dTime, dTime0;
	gzFile	fp;
	
	UINT	u;
	UINT	uSameCnt = 0;
	
	std::vector<GPS_LOG_t> GPSLog;
	
	getcwd( szCurDir, MAX_PATH );	// カレント dir
	
	// マルチファイルの場合，1個目は dir めいなのでそこに cd
	char const *p;
	if( p = strchr( szFileName, '/' )){
		strncpy( szBuf, szFileName, p - szFileName );
		*( szBuf + ( p - szFileName )) = '\0';
		chdir( szBuf );
		
		szFileName = p + 1;
	}
	
	while( *szFileName ){
		
		// ファイル名を / で分解
		p = szFileName;
		if( !( p = strchr( szFileName, '/' ))) p = strchr( szFileName, '\0' );
		strncpy( szBuf, szFileName, p - szFileName );
		*( szBuf + ( p - szFileName )) = '\0';
		szFileName = *p ? p + 1 : p;	// p == '/' ならスキップ
		
		if(( fp = gzopen(( char *)szBuf, "rb" )) == NULL ) return 0;
		
		/*** dp3 ****************************************************************/
		
		if( IsExt(( char *)szBuf, "dp3" )){
			
			gzseek( fp, 0x100, SEEK_CUR );
			
			#define BigEndianI( p )	( \
				( *(( UCHAR *)szBuf + p + 0 ) << 24 ) | \
				( *(( UCHAR *)szBuf + p + 1 ) << 16 ) | \
				( *(( UCHAR *)szBuf + p + 2 ) <<  8 ) | \
				( *(( UCHAR *)szBuf + p + 3 )       ))
			
			#define BigEndianS( p )	( \
				( *(( UCHAR *)szBuf + p + 0 ) <<  8 ) | \
				( *(( UCHAR *)szBuf + p + 1 )       ))
			
			while( gzread( fp, szBuf, 16 )){
				
				u = BigEndianI( 0 ) & 0x3FFFFF;
				// 値補正
				// 2254460 → 22:54:46.0
				dTime =	u / 100000 * 3600 +
						u / 1000 % 100 * 60 +
						( u % 1000 ) / 10.0;
				
				dLati = BigEndianI( 8 ) / 460800.0;
				dLong = BigEndianI( 4 ) / 460800.0;
				
				if( uGPSCnt == 0 ){
					dLati0 = dLati;
					dLong0 = dLong;
					dTime0 = dTime;
					
					dLong2Meter = GPSLogGetLength( dLong, dLati, dLong + 1.0 / 3600, dLati ) * 3600;
					dLati2Meter = GPSLogGetLength( dLong, dLati, dLong, dLati + 1.0 / 3600 ) * 3600;
				}
				
				if( dTime < dTime0 ) dTime += 24 * 3600;
				dTime -= dTime0;
				
				GPS_LOG_t	VsdLogTmp;
				
				// 単位を補正
				// 緯度・経度→メートル
				VsdLogTmp.SetX(( dLong - dLong0 ) * dLong2Meter );
				VsdLogTmp.SetY(( dLati0 - dLati ) * dLati2Meter );
				
				// 速度・向き→ベクトル座標
				VsdLogTmp.SetSpeed( BigEndianS( 12 ) / 10.0 );
				//VsdLogTmp.SetBearing( BigEndianS( 14 ));
				VsdLogTmp.SetTime( dTime );
				
				GPSLog.push_back( VsdLogTmp );
				
				if( uGPSCnt >=2 ){
					if( GPSLog[ uGPSCnt - 1 ].Time() == VsdLogTmp.Time()){
						// 時刻が同じログが続くときそのカウントをする
						++uSameCnt;
					}else if( uSameCnt ){
						// 時刻が同じログが途切れたので，時間を補正する
						++uSameCnt;
						
						for( u = 1; u < uSameCnt; ++ u ){
							GPSLog[ uGPSCnt - uSameCnt + u ].SetTime(
								( VsdLogTmp.Time() - GPSLog[ uGPSCnt - uSameCnt ].Time())
								/ uSameCnt * u
							);
						}
						uSameCnt = 0;
					}
				}
				uGPSCnt++;
			}
		}
		
		/*** dp3x ***************************************************************/
		
		if( IsExt(( char *)szBuf, "dp3x" )){
			
			// 原点取得
			gzread( fp, szBuf, 0x78 );
			double dLatiBase = *( int *)( szBuf + 0x54 ) / 460800.0;
			double dLongBase = *( int *)( szBuf + 0x50 ) / 460800.0;
			
			// 時間取得 UTC * 1000
			dTime = fmod(( double )*( __int64 *)( szBuf + 0x48 ) / 1000, 3600 * 24 )
				+ 9 * 3600; // なぜか UTC-9 の時間なので，補正
			
			while( gzread( fp, szBuf, 18 )){
				
				dLati = *( short int *)( szBuf + 0x2 ) / 460800.0 + dLatiBase;
				dLong = *( short int *)( szBuf + 0x0 ) / 460800.0 + dLongBase;
				
				if( uGPSCnt == 0 ){
					dLati0 = dLati;
					dLong0 = dLong;
					dTime0 = dTime;
					
					dLong2Meter = GPSLogGetLength( dLong, dLati, dLong + 1.0 / 3600, dLati ) * 3600;
					dLati2Meter = GPSLogGetLength( dLong, dLati, dLong, dLati + 1.0 / 3600 ) * 3600;
				}
				
				GPS_LOG_t	VsdLogTmp;
				
				// 単位を補正
				// 緯度・経度→メートル
				VsdLogTmp.SetX(( dLong - dLong0 ) * dLong2Meter );
				VsdLogTmp.SetY(( dLati0 - dLati ) * dLati2Meter );
				
				// 速度・向き→ベクトル座標
				VsdLogTmp.SetSpeed( *( short int *)( szBuf + 0x4 ) / 10.0 );
				
				//VsdLogTmp.SetBearing( FLT_MAX );
				VsdLogTmp.SetTime( dTime - dTime0 );
				
				GPSLog.push_back( VsdLogTmp );
				
				uGPSCnt++;
				dTime += 0.2;
			}
		}
		
		/*** nmea ***************************************************************/
		
		else while( gzgets( fp, szBuf, BUF_SIZE ) != Z_NULL ){
			
			char	cNorthSouth;
			char	cEastWest;
			UINT	uParamCnt;
			
			uParamCnt = sscanf( szBuf,
				"$GPRMC,"
				"%lg,%*c,"	// time
				"%lg,%c,"	// lat
				"%lg,%c,"	// long
				"%lg,%lg,",	// speed, bearing
				// 1	2		3				4		5			6		7
				&dTime, &dLati, &cNorthSouth, &dLong, &cEastWest, &dSpeed, &dBearing
			);
			
			// $GPRMC センテンス以外はスキップ
			if( uParamCnt < 5 ) continue;
			
			// 値補正
			// 225446.00 → 22:54:46.00
			dTime =	( int )dTime / 10000 * 3600 +
					( int )dTime / 100 % 100 * 60 +
					fmod( dTime, 100 );
			
			// 4916.452653 → 49度16.45分
			dLati =	( int )dLati / 100 + fmod( dLati, 100 ) / 60;
			dLong =	( int )dLong / 100 + fmod( dLong, 100 ) / 60;
			
			// 海外対応w
			if( cNorthSouth == 'S' ) dLati = -dLati;
			if( cEastWest   == 'W' ) dLong = -dLong;
			
			if( uGPSCnt == 0 ){
				dLati0 = dLati;
				dLong0 = dLong;
				dTime0 = dTime;
				
				dLong2Meter = GPSLogGetLength( dLong, dLati, dLong + 1.0 / 3600, dLati ) * 3600;
				dLati2Meter = GPSLogGetLength( dLong, dLati, dLong, dLati + 1.0 / 3600 ) * 3600;
			}
			
			if( dTime < dTime0 ) dTime += 24 * 3600;
			dTime -= dTime0;
			
			
			GPS_LOG_t	VsdLogTmp;
			// 単位を補正
			// 緯度・経度→メートル
			VsdLogTmp.SetX(( dLong - dLong0 ) * dLong2Meter );
			VsdLogTmp.SetY(( dLati0 - dLati ) * dLati2Meter );
			
			// 速度・向き→ベクトル座標
			if( uParamCnt >= 6 ) VsdLogTmp.SetSpeed( dSpeed * 1.852 ); // knot/h → km/h
			//VsdLogTmp.SetBearing( uParamCnt < 7 ? FLT_MAX : dBearing );
			VsdLogTmp.SetTime( dTime );
			
			GPSLog.push_back( VsdLogTmp );
			
			if( uGPSCnt >=2 ){
				if( GPSLog[ uGPSCnt - 1 ].Time() == VsdLogTmp.Time()){
					// 時刻が同じログが続くときそのカウントをする
					++uSameCnt;
				}else if( uSameCnt ){
					// 時刻が同じログが途切れたので，時間を補正する
					++uSameCnt;
					
					for( u = 1; u < uSameCnt; ++ u ){
						GPSLog[ uGPSCnt - uSameCnt + u ].SetTime(
							GPSLog[ uGPSCnt - uSameCnt + u ].Time() +
							( VsdLogTmp.Time() - GPSLog[ uGPSCnt - uSameCnt ].Time())
							/ uSameCnt * u
						);
					}
					uSameCnt = 0;
				}
			}
			
			// 20km/h 以下で 3秒以上 log の間隔が開くとき，0km/h の log を補完する
			if(
				uGPSCnt >= 2 &&
				VsdLogTmp.Time() - GPSLog[ uGPSCnt - 1 ].Time() >= 3 &&
				VsdLogTmp.Speed() <= 20
			){
				// -1 +0 +1 +2
				// A  B
				//  ↓
				// A  A' B' B
				
				// データ B のコピー
				GPSLog.push_back( VsdLogTmp );	// +1
				GPSLog.push_back( VsdLogTmp );	// +2
				
				// データ A->A' のコピー
				GPSLog[ uGPSCnt ] = GPSLog[ uGPSCnt - 1 ];
				
				// スピードを 0 に
				GPSLog[ uGPSCnt ].SetSpeed( 0 );
				GPSLog[ uGPSCnt ].SetGx( 0 );
				GPSLog[ uGPSCnt ].SetGy( 0 );
				GPSLog[ uGPSCnt + 1 ].SetSpeed( 0 );
				GPSLog[ uGPSCnt + 1 ].SetGx( 0 );
				GPSLog[ uGPSCnt + 1 ].SetGy( 0 );
				
				// 時間調整
				GPSLog[ uGPSCnt     ].SetTime( GPSLog[ uGPSCnt     ].Time() + 0.5 );
				GPSLog[ uGPSCnt + 1 ].SetTime( GPSLog[ uGPSCnt + 1 ].Time() - 0.5 );
				
				uGPSCnt += 2;
			}
			
			uGPSCnt++;
		}
		
		gzclose( fp );
	}
	
	chdir( szCurDir );	// pwd を元に戻す
	
	/************************************************************************/
	
	// アップコンバート用バッファ確保・初期化
	GPSLogUpConvert( GPSLog, TRUE );
	
#if 1
	DebugCmd( {
		FILE *fpp = fopen( "D:\\DDS\\vsd\\vsd_filter\\z_gps_raw.txt", "w" );
		for( u = 0; u < uGPSCnt; ++u ){
			fprintf( fpp, "%g\t%g\t%g\t%g\t%g\t%g\n",
				GPSLog[ u ].X(),
				GPSLog[ u ].Y(),
				GPSLog[ u ].Speed(),
				//GPSLog[ u ].Bearing(),
				GPSLog[ u ].Gx(),
				GPSLog[ u ].Gy(),
				GPSLog[ u ].Time()
			);
		}
		fclose( fpp );
	} )
	
	DebugCmd( {
		FILE *fpp = fopen( "D:\\DDS\\vsd\\vsd_filter\\z_upcon_gps.txt", "w" );
		for( u = 0; u < ( UINT )m_iCnt; ++u ){
			fprintf( fpp, "%g\t%g\t%g\t%g\t%g\t%g\t%g\t%g\t%g\n",
				m_Log[ u ].Speed(),
				m_Log[ u ].Tacho(),
				m_Log[ u ].Distance(),
				m_Log[ u ].X(),
				m_Log[ u ].Y(),
				m_Log[ u ].X0(),
				m_Log[ u ].Y0(),
				m_Log[ u ].Gx(),
				m_Log[ u ].Gy()
			);
		}
		fclose( fpp );
	} )
#endif
	
	if( m_Log.size()) m_dLogStartTime = dTime0 + 9 * 3600;
	
	return m_iCnt;
}

/*** ログリード *************************************************************/

int CVsdLog::ReadLog( const char *szFileName, CLapLog *&pLapLog ){
	
	TCHAR	szBuf[ BUF_SIZE ];
	gzFile	fp;
	BOOL	bCalibrating = FALSE;
	
	if(( fp = gzopen(( char *)szFileName, "rb" )) == NULL ) return 0;
	
	pLapLog			= NULL;
	
	// ログリード
	
	UINT	uReadCnt;
	double	dGcx = 0;
	double	dGcy = 0;
	
	m_iLogStart = m_iLogStop = 0;
	
	TCHAR	*p;
	
	int	iLogFreqLog		= 0;
	int	iLogFreqTime	= 0;
	
	LAP_t	LapTime;
	
	while( gzgets( fp, szBuf, BUF_SIZE ) != Z_NULL ){
		VSD_LOG_t	VsdLogTmp;
		
		if(( p = strstr( szBuf, "LAP" )) != NULL ){ // ラップタイム記録を見つけた
			
			if( pLapLog == NULL ){
				pLapLog = new CLapLog();
				pLapLog->m_iLapMode = LAPMODE_MAGNET;
			}
			
			UINT	uLap, uMin, uSec, uMSec;
			
			uReadCnt = sscanf( p, "LAP%d%d:%d.%d", &uLap, &uMin, &uSec, &uMSec );
			
			int iTime = ( uMin * 60 + uSec ) * 1000 + uMSec;
			
			LapTime.uLap	= uLap;
			LapTime.fLogNum	= ( float )m_iCnt;
			LapTime.iTime	= ( uReadCnt == 4 ) ? iTime : 0;
			pLapLog->m_Lap.push_back( LapTime );
			
			if(
				uReadCnt == 4 &&
				( pLapLog->m_iBestTime == TIME_NONE || pLapLog->m_iBestTime > iTime )
			){
				pLapLog->m_iBestTime	= iTime;
				pLapLog->m_iBestLap	= pLapLog->m_iLapNum - 1;
				
				iLogFreqLog	 += m_iCnt - ( int )pLapLog->m_Lap[ pLapLog->m_iLapNum - 1 ].fLogNum;
				iLogFreqTime += iTime;
			}
			++pLapLog->m_iLapNum;
		}
		
		if(( p = strstr( szBuf, "GPS" )) != NULL ){ // GPS記録を見つけた
			double	dLati;
			double	dLong;
			
			sscanf( p, "GPS%lg%lg", &dLati, &dLong );
			VsdLogTmp.SetX( dLong );
			VsdLogTmp.SetY( dLati );
		}
		
		int		iTacho;
		double	dSpeed;
		double	dDistance;
		double	dGx, dGy;
		
		// 普通の log
		if(( uReadCnt = sscanf( szBuf, "%u%lg%lg%lg%lg",
			&iTacho,
			&dSpeed,
			&dDistance,
			&dGy,
			&dGx
		)) >= 3 ){
			
			VsdLogTmp.SetTacho( iTacho );
			VsdLogTmp.SetSpeed( dSpeed );
			VsdLogTmp.SetDistance( dDistance );
			
			if( m_iMaxTacho < iTacho ) m_iMaxTacho = iTacho;
			
			if( uReadCnt < 5 && m_iCnt ){
				// Gデータがないときは，speedから求める←廃止
				dGx = 0;
				dGy = 0;
				//dGy = ( VsdLogTmp.fSpeed - m_Log[ m_iCnt - 1 ].fSpeed ) * ( 1000.0 / 3600 / 9.8 * LOG_FREQ );
			}else{
				if( dGx >= 4 ){	
					// 単位を G に変換
					dGx = -dGx / ACC_1G_Y;
					dGy =  dGy / ACC_1G_Z;
				}
				
				if( m_iCnt == 0 ){
					// G センターの初期値
					dGcx = dGx;
					dGcy = dGy;
				}
				
				// G センターだし
				dGx -= dGcx;
				dGy -= dGcy;
				
				// 静止していると思しきときは，G センターを補正する
				// 走行距離 == 0 && ±0.02G
				if(
					m_iCnt &&
					m_Log[ m_iCnt - 1 ].Distance() == VsdLogTmp.Distance() &&
					( m_Log[ m_iCnt - 1 ].Gy() - dGy ) >= -0.02 &&
					( m_Log[ m_iCnt - 1 ].Gy() - dGy ) <=  0.02
				){
					dGcx += dGx / 160;
					dGcy += dGy / 160;
				}
			}
			
			VsdLogTmp.SetGx( dGx );
			VsdLogTmp.SetGy( dGy );
			
			if( m_dMaxGx < fabs( dGx )) m_dMaxGx = fabs( dGx );
			if( m_dMaxGy < -dGy       ) m_dMaxGy = -dGy;
			
			// ログ開始・終了認識
			if( VsdLogTmp.Speed() >= 300 ){
				if( !bCalibrating ){
					bCalibrating = TRUE;
					m_iLogStart  = m_iLogStop;
					m_iLogStop   = m_iCnt;
				}
			}else{
				bCalibrating = FALSE;
				
				if( m_iMaxSpeed < VsdLogTmp.Speed())
					m_iMaxSpeed = ( int )ceil( VsdLogTmp.Speed());
			}
			
			VsdLogTmp.SetTime(( double )m_iCnt / LOG_FREQ );
			
			m_Log.push_back( VsdLogTmp );
			++m_iCnt;
		}
	}
	
	// 古い log は LOG_FREQ とは限らないので，計算で求める
	if( iLogFreqTime ){
		m_dFreq = iLogFreqLog / ( iLogFreqTime / 1000.0 );
	}
	
	/*** GPS ログから軌跡を求める *******************************************/
	
	//if( uGPSCnt ) GPSLogUpConvert( GPSLog );
	
	/************************************************************************/
	
	gzclose( fp );
	
	// Lap log の番犬
	if( pLapLog ){
		LapTime.fLogNum	= FLT_MAX;	// 番犬
		LapTime.iTime	= 0;		// 番犬
		pLapLog->m_Lap.push_back( LapTime );
	}
	
	// Vsd log の番犬
	m_Log.push_back( m_Log[ m_iCnt - 1 ] );
	m_Log[ m_iCnt ].SetTime( FLT_MAX );
	
	return m_iCnt;
}

/*** MAP 回転処理 ***********************************************************/

void CVsdLog::RotateMap( double dAngle ){
	
	int	i;
	double dMaxX, dMinX, dMaxY, dMinY;
	
	dMaxX = dMinX = dMaxY = dMinY = 0;
	
	for( i = 0; i < m_iCnt; ++i ){
		if( _isnan( m_Log[ i ].X0())){
			m_Log[ i ].SetX( m_Log[ i ].X0());
			m_Log[ i ].SetY( m_Log[ i ].Y0());
		}else{
			m_Log[ i ].SetX(  cos( dAngle ) * m_Log[ i ].X0() + sin( dAngle ) * m_Log[ i ].Y0());
			m_Log[ i ].SetY( -sin( dAngle ) * m_Log[ i ].X0() + cos( dAngle ) * m_Log[ i ].Y0());
			
			if     ( dMaxX < m_Log[ i ].X()) dMaxX = m_Log[ i ].X();
			else if( dMinX > m_Log[ i ].X()) dMinX = m_Log[ i ].X();
			if     ( dMaxY < m_Log[ i ].Y()) dMaxY = m_Log[ i ].Y();
			else if( dMinY > m_Log[ i ].Y()) dMinY = m_Log[ i ].Y();
		}
	}
	
	m_dMapSizeX	= dMaxX - dMinX;
	m_dMapSizeY	= dMaxY - dMinY;
	m_dMapOffsX	= dMinX;
	m_dMapOffsY	= dMinY;
}
