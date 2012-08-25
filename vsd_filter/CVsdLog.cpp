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
	
	m_uSameCnt		= 0;
	
	m_dLong0 =
	m_dLati0 =
	m_dLong2Meter =
	m_dLati2Meter = 0;
}

/*** fraem# に対応するログ index を得る *************************************/

double CVsdLog::GetIndex(
	double	dFrame,
	int iVidSt, int iVidEd,
	int iLogSt, int iLogEd,
	int iPrevIdx
){
	if( iVidSt == iVidEd ) return 0;
	return GetIndex(
		( iLogSt + ( iLogEd - iLogSt ) * ( dFrame - iVidSt ) / ( double )( iVidEd - iVidSt )) / SLIDER_TIME,
		iPrevIdx
	);
}

double CVsdLog::GetIndex( double dTime, int iPrevIdx ){
	int idx;
	
	if( dTime < 0 ) return -1;
	if( dTime >= Time( m_iCnt - 1 )) return m_iCnt;
	
	// Time( idx ) <= dTime < Time( idx + 1 )
	// となる idx を見つける
	if(
		iPrevIdx < 0 || iPrevIdx >= m_iCnt ||
		Time( iPrevIdx ) > dTime
	){
		// iPrevIdx がおかしいので，binary serch する
		int iSt = 0;
		int iEd = m_iCnt - 1;
		while( 1 ){
			idx = ( iSt + iEd ) / 2;
			if( iSt == iEd ) break;
			
			if( Time( idx ) > dTime ){
				iEd = idx - 1;
			}else if( Time( idx + 1 ) <= dTime ){
				iSt = idx + 1;
			}else{
				// ヒット
				break;
			}
		}
	}else{
		// iPrevIdx は正常なので，これを起点に単純サーチする
		idx = iPrevIdx;
		while( Time( idx + 1 ) <= dTime ) ++idx;
	}
	
	// index の端数を求める
	return idx +
		( dTime                   - Time( idx )) /
		( Time( idx + 1 ) - Time( idx ));
}

/*** GPS ログのダンプ *******************************************************/

#ifdef DEBUG
void CVsdLog::Dump( char *szFileName ){
	FILE *fp = fopen( szFileName, "w" );
	fputs( "Time\tSpeed\tTacho\tDistance\tX\tY\tX0\tY0\tGx\tGy\tBearing\n", fp );
	
	for( int i = 0; i < m_iCnt; ++i ){
		fprintf( fp, "%g\t%g\t%g\t%g\t%g\t%g\t%g\t%g\t%g\t%g\t%g\n",
			Time( i ),
			Speed( i ),
			Tacho( i ),
			Distance( i ),
			X( i ),
			Y( i ),
			X0( i ),
			Y0( i ),
			Gx( i ),
			Gy( i ),
			Bearing( i )
		);
	}
	fclose( fp );
}
#endif

/*** GPS ログの up-convert **************************************************/

UINT CVsdLog::GPSLogUpConvert( void ){
	
	int	i;
	
	if( m_iCnt < 2 ) return 0;			// 2個データがなければ終了
	
	VSD_LOG_t	VsdLogTmp;
	VsdLogTmp.SetTime( FLT_MAX );
	m_Log.push_back( VsdLogTmp );		// 番犬
	
	double	dDistance = 0;
	double	dBearing;
	
	/*** VSD_LOG_t の方を走査して，いろいろ補正 *****************************/
	
	m_dFreq = 0;
	int iFreqCnt = 0;
	
	for( i = 1; i < m_iCnt - 1; ++i ){
		// speed がない場合の補正
		if( 0 /* ★★★暫定★★★ */ ){
			m_Log[ i ].SetSpeed(
				sqrt(
					pow( X0( i + 1 ) - X0( i - 1 ), 2 ) +
					pow( Y0( i + 1 ) - Y0( i - 1 ), 2 )
				) / ( Time( i + 1 ) - Time( i - 1 ))
				* ( 3600.0 / 1000 )
			);
		}
		
		// bearing がない場合の補正
		m_Log[ i ].SetBearing(
			fmod(
				atan2(
					Y0( i + 1 ) - Y0( i - 1 ),
					X0( i + 1 ) - X0( i - 1 )
				) / ToRAD + ( 360 * 2 - 90 ),
				360
			)
		);
		
		// 5km/h 以上の時のみ，ログ Freq を計算する
		if( Speed( i ) >= 5 ){
			m_dFreq += Time( i ) - Time( i - 1 );
			++iFreqCnt;
		}
	}
	
	m_dFreq = iFreqCnt / m_dFreq;
	
	m_Log[ 0          ].SetSpeed  ( Speed( 1 ));
	m_Log[ 0          ].SetBearing( Bearing( 1 ));
	m_Log[ m_iCnt - 1 ].SetSpeed  ( Speed( m_iCnt - 2 ));
	m_Log[ m_iCnt - 1 ].SetBearing( Bearing( m_iCnt - 2 ));
	
	for( i = 1; i < m_iCnt - 1; ++i ){
		// Gx / Gy を作る
		m_Log[ i ].SetGy(
			( Speed( i + 1 ) - Speed( i - 1 ))
			* ( 1 / 3.600 / GRAVITY )
			/ ( Time( i + 1 ) - Time( i - 1 ))
		);
		if( Gy( i ) > 10 ){
			int a= 0;
		}
		// 横G = vω
		dBearing = Bearing( i + 1 ) - Bearing( i - 1 );
		if     ( dBearing >  180 ) dBearing -= 360;
		else if( dBearing < -180 ) dBearing += 360;
		
		m_Log[ i ].SetGx(
			dBearing * ( ToRAD / GRAVITY )
			/ ( Time( i + 1 ) - Time( i - 1 ))
			* ( Speed( i ) / 3.600 )
		);
		
		// ±5G 以上は，削除
		if( Gx( i ) < -3 || Gx( i ) > 3 ){
			m_Log[ i ].SetGx( Gx( i - 1 ));
		}
	}
	
	m_Log[ 0 ].SetGx(			Gx( 1 ));
	m_Log[ 0 ].SetGy(			Gy( 1 ));
	m_Log[ m_iCnt - 1 ].SetGx(	Gx( m_iCnt - 2 ));
	m_Log[ m_iCnt - 1 ].SetGy(	Gy( m_iCnt - 2 ));
	
	/************************************************************************/
	
	for( i = 0; i < m_iCnt; ++i ){
		if( m_iMaxSpeed < Speed( i ))
			m_iMaxSpeed = ( int )ceil( Speed( i ));
		
		if( i ) dDistance += sqrt(
			pow( X0( i - 1 ) - X0( i ), 2 ) +
			pow( Y0( i - 1 ) - Y0( i ), 2 )
		);
		
		m_Log[ i ].SetDistance( dDistance );
		m_Log[ i ].SetTacho( 0 );
	}
	
	// スムージング
	#define G_SMOOTH_NUM	2
	double	dGx0, dGx1 = 0;
	double	dGy0, dGy1 = 0;
	
	for( i = ( G_SMOOTH_NUM - 1 ) / 2; i < m_iCnt - G_SMOOTH_NUM / 2; ++i ){
		m_Log[ i ].SetGx( dGx0 = (
			( G_SMOOTH_NUM >= 5 ? Gx( i - 2 ) : 0 ) +
			( G_SMOOTH_NUM >= 4 ? Gx( i + 2 ) : 0 ) +
			( G_SMOOTH_NUM >= 3 ? Gx( i - 1 ) : 0 ) +
			( G_SMOOTH_NUM >= 2 ? Gx( i + 1 ) : 0 ) +
			Gx( i + 0 )
		) / G_SMOOTH_NUM );
		m_Log[ i ].SetGy( dGy0 = (
			( G_SMOOTH_NUM >= 5 ? Gy( i - 2 ) : 0 ) +
			( G_SMOOTH_NUM >= 4 ? Gy( i + 2 ) : 0 ) +
			( G_SMOOTH_NUM >= 3 ? Gy( i - 1 ) : 0 ) +
			( G_SMOOTH_NUM >= 2 ? Gy( i + 1 ) : 0 ) +
			Gy( i + 0 )
		) / G_SMOOTH_NUM );
		
		dGx1 = dGx1 * 0.9 + dGx0 * 0.1;
		dGy1 = dGy1 * 0.9 + dGy0 * 0.1;
		if( m_dMaxGx < fabs( dGx1 )) m_dMaxGx = fabs( dGx1 );
		if( m_dMaxGy < dGy1 ) m_dMaxGy = dGy1;
		if( m_dMinGy > dGy1 ) m_dMinGy = dGy1;
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

void CVsdLog::PushRecord( VSD_LOG_t& VsdLogTmp, double dLong, double dLati ){
	
	if( m_iCnt == 0 ){
		m_dLogStartTime = VsdLogTmp.Time();
		
		// 小さい方の番犬
		m_Log.push_back( VsdLogTmp );
		m_Log[ 0 ].SetTime( FLT_MIN );
		++m_iCnt;
	}
	
	if( dLong == 0 ){
		// GPS データがないので，直前の x, y データコピー
		VsdLogTmp.SetX0( X0( m_iCnt - 1 ));
		VsdLogTmp.SetY0( Y0( m_iCnt - 1 ));
	}else{
		// 一発目の GPS データ，それを原点にする
		if( m_dLong0 == 0 ){
			m_dLong0 = dLong;
			m_dLati0 = dLati;
			m_dLong2Meter = GPSLogGetLength( dLong, dLati, dLong + 1.0 / 3600, dLati ) * 3600;
			m_dLati2Meter = GPSLogGetLength( dLong, dLati, dLong, dLati + 1.0 / 3600 ) * 3600;
		}
		
		// 単位を補正
		// 緯度・経度→メートル
		VsdLogTmp.SetX0(( dLong - m_dLong0 ) * m_dLong2Meter );
		VsdLogTmp.SetY0(( m_dLati0 - dLati ) * m_dLati2Meter );
		
		if( VsdLogTmp.Time() < m_dLogStartTime ) VsdLogTmp.SetTime( VsdLogTmp.Time() + 24 * 3600 );
		VsdLogTmp.SetTime( VsdLogTmp.Time() - m_dLogStartTime );
	}
	
	DebugCmd(
		VsdLogTmp.SetX( dLong - m_dLong0 );
		VsdLogTmp.SetY( dLati - m_dLati0 );
	)
	
	m_Log.push_back( VsdLogTmp );
	
	if( m_iCnt >= 2 ){
		if( Time( m_iCnt - 1 ) == VsdLogTmp.Time()){
			// 時刻が同じログが続くときそのカウントをする
			++m_uSameCnt;
		}else if( m_uSameCnt ){
			// 時刻が同じログが途切れたので，時間を補正する
			++m_uSameCnt;
			
			for( UINT u = 1; u < m_uSameCnt; ++ u ){
				m_Log[ m_iCnt - m_uSameCnt + u ].SetTime(
					Time(( int )( m_iCnt - m_uSameCnt + u )) +
					( VsdLogTmp.Time() - Time(( int )( m_iCnt - m_uSameCnt )))
					/ m_uSameCnt * u
				);
			}
			m_uSameCnt = 0;
		}
		
		// 20km/h 以下で 3秒以上 log の間隔が開くとき，0km/h の log を補完する
		if(
			VsdLogTmp.Time() - Time( m_iCnt - 1 ) >= 3 &&
			VsdLogTmp.Speed() <= 20
		){
			// -1 +0 +1 +2
			// A  B
			//  ↓
			// A  A' B' B
			
			// [+0] にデータ A->A' のコピー
			m_Log[ m_iCnt ] = m_Log[ m_iCnt - 1 ];
			
			// データ B のコピー
			m_Log.push_back( VsdLogTmp );	// [+1]
			m_Log.push_back( VsdLogTmp );	// [+2]
			
			// スピードを 0 に
			m_Log[ m_iCnt ].SetSpeed( 0 );
			m_Log[ m_iCnt ].SetGx( 0 );
			m_Log[ m_iCnt ].SetGy( 0 );
			m_Log[ m_iCnt + 1 ].SetSpeed( 0 );
			m_Log[ m_iCnt + 1 ].SetGx( 0 );
			m_Log[ m_iCnt + 1 ].SetGy( 0 );
			
			// 時間調整
			m_Log[ m_iCnt     ].SetTime( Time( m_iCnt )     + 0.5 ); // 適当に 0.5秒
			m_Log[ m_iCnt + 1 ].SetTime( Time( m_iCnt + 1 ) - 0.5 );
			
			m_iCnt += 2;
		}
	}
	
	m_iCnt++;
}

int CVsdLog::ReadGPSLog( const char *szFileName ){
	TCHAR	szCurDir[ MAX_PATH ];
	TCHAR	szBuf[ BUF_SIZE ];
	
	double	dLati;
	double	dLong;
	double	dSpeed;
	double	dTime;
	gzFile	fp;
	
	UINT	u;
	
	getcwd( szCurDir, MAX_PATH );	// カレント dir
	
	// マルチファイルの場合，1個目は dir 名なのでそこに cd
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
				
				VSD_LOG_t	VsdLogTmp;
				
				// 値補正
				// 2254460 → 22:54:46.0
				u = BigEndianI( 0 ) & 0x3FFFFF;
				VsdLogTmp.SetTime(
					u / 100000 * 3600 +
					u / 1000 % 100 * 60 +
					( u % 1000 ) / 10.0
				);
				
				// 速度
				VsdLogTmp.SetSpeed( BigEndianS( 12 ) / 10.0 );
				
				PushRecord(
					VsdLogTmp,
					BigEndianI( 4 ) / 460800.0,
					BigEndianI( 8 ) / 460800.0
				);
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
				
				VSD_LOG_t	VsdLogTmp;
				
				VsdLogTmp.SetTime( dTime + m_iCnt / 5 );	// 5Hz 固定らしい
				
				// 速度
				VsdLogTmp.SetSpeed( *( short int *)( szBuf + 0x4 ) / 10.0 );
				
				PushRecord(
					VsdLogTmp,
					*( short int *)( szBuf + 0x0 ) / 460800.0 + dLongBase,
					*( short int *)( szBuf + 0x2 ) / 460800.0 + dLatiBase
				);
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
				"%lg,",		// speed
				// 1	2		3				4		5			6
				&dTime, &dLati, &cNorthSouth, &dLong, &cEastWest, &dSpeed
			);
			
			// $GPRMC センテンス以外はスキップ
			if( uParamCnt < 5 ) continue;
			
			// 値補正
			// 225446.00 → 22:54:46.00
			dTime =	( int )dTime / 10000 * 3600 +
					( int )dTime / 100 % 100 * 60 +
					fmod( dTime, 100 );
			
			// 緯度経度の変換: 4916.452653 → 49度16.45分
			dLong = ( int )dLong / 100 + fmod( dLong, 100 ) / 60;
			dLati = ( int )dLati / 100 + fmod( dLati, 100 ) / 60;
			
			// 海外対応w
			if( cEastWest   == 'W' ) dLong = -dLong;
			if( cNorthSouth == 'S' ) dLati = -dLati;
			
			VSD_LOG_t	VsdLogTmp;
			
			VsdLogTmp.SetTime( dTime );
			
			// 速度
			if( uParamCnt >= 6 ) VsdLogTmp.SetSpeed( dSpeed * 1.852 ); // knot/h → km/h
			
			PushRecord( VsdLogTmp, dLong, dLati );
		}
		
		gzclose( fp );
	}
	
	chdir( szCurDir );	// pwd を元に戻す
	
	/************************************************************************/
	
	//#define DUMP_LOG
	#if defined DEBUG && defined DUMP_LOG
		Dump( "D:\\DDS\\vsd\\vsd_filter\\z_gpslog_raw.txt" );
	#endif
	
	// アップコンバート用バッファ確保・初期化
	GPSLogUpConvert();
	
	#if defined DEBUG && defined DUMP_LOG
		Dump( "D:\\DDS\\vsd\\vsd_filter\\z_gpslog_upcon.txt" );
	#endif
	
	m_dLogStartTime += 9 * 3600;
	
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
		
		double	dLati = 0;
		double	dLong = 0;
		if(( p = strstr( szBuf, "GPS" )) != NULL ){ // GPS記録を見つけた
			sscanf( p, "GPS%lg%lg", &dLati, &dLong );
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
					Distance( m_iCnt - 1 ) == VsdLogTmp.Distance() &&
					( Gy( m_iCnt - 1 ) - dGy ) >= -0.02 &&
					( Gy( m_iCnt - 1 ) - dGy ) <=  0.02
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
			PushRecord( VsdLogTmp, dLong, dLati );
		}
	}
	
	// 古い log は LOG_FREQ とは限らないので，計算で求める
	if( iLogFreqTime ){
		m_dFreq = iLogFreqLog / ( iLogFreqTime / 1000.0 );
	}
	
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
		if( _isnan( X0( i ))){
			m_Log[ i ].SetX( X0( i ));
			m_Log[ i ].SetY( Y0( i ));
		}else{
			m_Log[ i ].SetX(  cos( dAngle ) * X0( i ) + sin( dAngle ) * Y0( i ));
			m_Log[ i ].SetY( -sin( dAngle ) * X0( i ) + cos( dAngle ) * Y0( i ));
			
			if     ( dMaxX < X( i )) dMaxX = X( i );
			else if( dMinX > X( i )) dMinX = X( i );
			if     ( dMaxY < Y( i )) dMaxY = Y( i );
			else if( dMinY > Y( i )) dMinY = Y( i );
		}
	}
	
	m_dMapSizeX	= dMaxX - dMinX;
	m_dMapSizeY	= dMaxY - dMinY;
	m_dMapOffsX	= dMinX;
	m_dMapOffsY	= dMinY;
}
