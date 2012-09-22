/*****************************************************************************
	
	VSD -- vehicle data logger system  Copyright(C) by DDS
	
	CVsdLog.cpp - CVsdLog class implementation
	
*****************************************************************************/

#include "StdAfx.h"

#include "dds.h"
#include "dds_lib/dds_lib.h"
#include "../vsd/main.h"
#include "CVsdLog.h"
#include "CScript.h"
#include "CVsdFont.h"
#include "pixel.h"
#include "CVsdImage.h"
#include "CVsdFilter.h"
#include "CVsdFile.h"
#include "error_code.h"

/*** macros *****************************************************************/

#define GRAVITY			9.80665

/*** コンストラクタ *********************************************************/

CVsdLog::CVsdLog(){
	m_dFreq	= LOG_FREQ;
	
	m_dLogStartTime	= -1;
	
	m_dLong0	=
	m_dLati0	= 0;
	m_iCnt		= 0;
	
	#define DEF_LOG( name )	m_pLog##name = NULL;
	#include "def_log.h"
}

/*** frame# に対応するログ index を得る *************************************/

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
	
	// Time( idx ) <= dTime < Time( idx + 1 )
	// となる idx を見つける
	if(
		iPrevIdx < 0 || iPrevIdx >= GetCnt() ||
		Time( iPrevIdx ) > dTime
	){
		// iPrevIdx がおかしいので，binary serch する
		int iSt = 0;
		int iEd = GetCnt() - 1;
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
		( dTime           - Time( idx )) /
		( Time( idx + 1 ) - Time( idx ));
}

/*** GPS ログのダンプ *******************************************************/

#ifdef DEBUG
void CVsdLog::Dump( char *szFileName ){
	FILE *fp = fopen( szFileName, "w" );
	
	BOOL bFirst = TRUE;
	std::map<std::string, VSD_LOG_t *>::iterator it;
	
	// ヘッダ出力
	for( it = m_Logs.begin(); it != m_Logs.end(); ++it ){
		if( !bFirst ) fputs( "\t", fp ); bFirst = FALSE;
		fputs( it->first.c_str(), fp );
	}
	fputs( "\n", fp );
	
	for( int i = 0; i < GetCnt(); ++i ){
		BOOL bFirst = TRUE;
		for( it = m_Logs.begin(); it != m_Logs.end(); ++it ){
			if( !bFirst ) fputs( "\t", fp ); bFirst = FALSE;
			fprintf( fp, "%f", it->second->Get( i ));
		}
		fputs( "\n", fp );
	}
	fclose( fp );
}
#endif

/*** ログの Set *************************************************************/

void CVsdLog::Set( const char *szKey, int iIndex, double dVal ){
	std::string strKey( szKey );
	if( m_Logs.find( strKey ) == m_Logs.end()){
		// 要素なし，生成される
		VSD_LOG_t *p;
		m_Logs[ strKey ] = p = new VSD_LOG_t();
		
		// Speed とか基本データの参照用ポインタに代入
		#define DEF_LOG( name )	if( strKey == #name ) m_pLog##name = p;
		#include "def_log.h"
	}
	
	m_Logs[ strKey ]->Set( iIndex, dVal );
	
	if( m_iCnt <= iIndex ) m_iCnt = iIndex + 1;
}

/*** 1レコードコピー ********************************************************/

void CVsdLog::CopyRecord( int iTo, int iFrom ){
	std::map<std::string, VSD_LOG_t *>::iterator it;
	
	for( it = m_Logs.begin(); it != m_Logs.end(); ++it ){
		VSD_LOG_t *pLog = it->second;
		pLog->Set( iTo, pLog->Get( iFrom ));
	}
	if( m_iCnt <= iTo ) m_iCnt = iTo + 1;
}

/*** 番犬追加 ***************************************************************/

void CVsdLog::AddWatchDog( void ){
	int iCnt = GetCnt() - 1;
	CopyRecord( 0, 2 );           AddStopRecord( 0, -WATCHDOG_TIME );
	CopyRecord( 1, 2 );           AddStopRecord( 1, Time( 2 ) - 0.5 );
	CopyRecord( iCnt + 1, iCnt ); AddStopRecord( iCnt + 1, Time( iCnt ) + 0.5 );
	CopyRecord( iCnt + 2, iCnt ); AddStopRecord( iCnt + 2, WATCHDOG_TIME );
}

/*** GPS ログの up-convert **************************************************/

UINT CVsdLog::GPSLogRescan( void ){
	
	/*** VSD_LOG_t の方を走査して，いろいろ補正 *****************************/
	
	m_dFreq = 0;
	int		iFreqCnt = 0;
	
	#ifdef _OPENMP
		#pragma omp parallel
	#endif
	{
		if( m_pLogX0 != 0 && m_pLogY0 != 0 ){
			// スピード生成
			if( !m_pLogSpeed ){
				#ifdef _OPENMP
					#pragma omp for
				#endif
				for( int i = 0; i < GetCnt() - 1; ++i ){
					if( Time( i + 1 ) - Time( i ) >= ( TIME_STOP - TIME_STOP_MARGIN * 2 )){
						// 時間が開いている停止ログ
						SetSpeed( i++, 0 );
						SetSpeed( i,   0 );
					}else{
						SetSpeed( i,
							( Distance( i + 1 ) - Distance( i ))
							* ( 3600.0 / 1000 ) /
							( Time( i + 1 ) - Time( i ))
						);
					}
				}
				// 番犬はすでに 0 → SetSpeed( GetCnt() - 1, 0 );
			}
			
			// G 計算
			if( !m_pLogGx ){
				double	dBearingPrev;
				
				#ifdef _OPENMP
					#pragma omp for
				#endif
				for( int i = 0; i < GetCnt() - 1; ++i ){
					// bearing 計算
					double dBearing = atan2( Y0( i + 1 ) - Y0( i ), X0( i + 1 ) - X0( i ));
					
					// 横 G 計算
					if( i >= 2 ){
						// Gx / Gy を作る
						SetGy( i,
							( Speed( i + 1 ) - Speed( i ))
							* ( 1 / 3.600 / GRAVITY )
							/ ( Time( i + 1 ) - Time( i ))
						);
						
						// 横G = vω
						double dBearingDelta = dBearing - dBearingPrev;
						if     ( dBearingDelta >  M_PI ) dBearingDelta -= M_PI * 2;
						else if( dBearingDelta < -M_PI ) dBearingDelta += M_PI * 2;
						
						SetGx( i,
							dBearingDelta / GRAVITY
							/ ( Time( i + 1 ) - Time( i ))
							* ( Speed( i ) / 3.600 )
						);
						
						// ±5G 以上は，削除
						if( Gx( i ) < -3 || Gx( i ) > 3 ){
							SetGx( i, Gx( i - 1 ));
						}
					}
					
					dBearingPrev = dBearing;
					
					// 5km/h 以上の時のみ，ログ Freq を計算する
					/* ★暫定
					if( Speed( i ) >= 5 ){
						m_dFreq += Time( i ) - Time( i - 1 );
						++iFreqCnt;
					}
					*/
				}
				SetGx( GetCnt() - 1, 0 );
				SetGy( GetCnt() - 1, 0 );
				
				// スムージング
				#define G_SMOOTH_NUM	3
				double	dGx0, dGx1 = 0;
				double	dGy0, dGy1 = 0;
				
				for( int i = ( G_SMOOTH_NUM - 1 ) / 2; i < GetCnt() - G_SMOOTH_NUM / 2; ++i ){
					if( i < 2 || i >= GetCnt() - 2 ) continue;
					
					SetGx( i, dGx0 = (
						( G_SMOOTH_NUM >= 7 ? Gx( i - 3 ) : 0 ) +
						( G_SMOOTH_NUM >= 6 ? Gx( i + 3 ) : 0 ) +
						( G_SMOOTH_NUM >= 5 ? Gx( i - 2 ) : 0 ) +
						( G_SMOOTH_NUM >= 4 ? Gx( i + 2 ) : 0 ) +
						( G_SMOOTH_NUM >= 3 ? Gx( i - 1 ) : 0 ) +
						( G_SMOOTH_NUM >= 2 ? Gx( i + 1 ) : 0 ) +
						Gx( i + 0 )
					) / G_SMOOTH_NUM );
					SetGy( i, dGy0 = (
						( G_SMOOTH_NUM >= 7 ? Gy( i - 3 ) : 0 ) +
						( G_SMOOTH_NUM >= 6 ? Gy( i + 3 ) : 0 ) +
						( G_SMOOTH_NUM >= 5 ? Gy( i - 2 ) : 0 ) +
						( G_SMOOTH_NUM >= 4 ? Gy( i + 2 ) : 0 ) +
						( G_SMOOTH_NUM >= 3 ? Gy( i - 1 ) : 0 ) +
						( G_SMOOTH_NUM >= 2 ? Gy( i + 1 ) : 0 ) +
						Gy( i + 0 )
					) / G_SMOOTH_NUM );
					
					dGx1 = dGx1 * 0.9 + dGx0 * 0.1;
					dGy1 = dGy1 * 0.9 + dGy0 * 0.1;
					/* ★暫定
					if( MaxGx() < fabs( dGx1 )) MaxGx() = fabs( dGx1 );
					if( MaxGy() < dGy1 ) MaxGy() = dGy1;
					if( MinGy() > dGy1 ) MinGy() = dGy1;
					*/
				}
			}
		}
  	}
	
	// ★暫定
	//m_dFreq = iFreqCnt / m_dFreq;
	m_dFreq = 10;
	
	return GetCnt();
}

/*** 距離算出 **************************************************************/

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

/*** ログリード by JavaScript **********************************************/

int CVsdLog::ReadGPSLog( const char *szFileName ){
	{
		// JavaScript オブジェクト初期化
		CScript Script;
		Script.Initialize();
		if( Script.RunFile( L"log_reader\\nmea.js" ) != ERR_OK ){
			return 0;	// エラー
		}
		
		// スクリプト実行
		LPWSTR pStr = NULL;
		Script.Run_s( L"ReadLog", StringNew( pStr, szFileName ));
		delete [] pStr;
		
		/*** JS の Log にアクセス *******************************************/
		
		{
			#ifdef AVS_PLUGIN
				v8::Isolate::Scope IsolateScope( Script.m_pIsolate );
			#endif
			v8::HandleScope handle_scope;
			v8::Context::Scope context_scope( Script.m_Context );
			
			// "Log" 取得
			v8::Local<v8::Array> hLog = v8::Local<v8::Array>::Cast(
				Script.m_Context->Global()->Get( v8::String::New(( uint16_t *)L"Log" ))
			);
			if( !hLog->IsArray()) return 0;
			
			// Log の key 取得
			v8::Local<v8::Array> Keys = hLog->GetPropertyNames();
			
			// JavaScript Array の vector
			std::vector<v8::Local<v8::Array> >	JSArrays;
			
			// VSD_LOG_t * の vector
			std::vector<VSD_LOG_t *>	CArrays;
			
			UINT	uIdxTime	= ~0;
			UINT	uIdxDistance= ~0;
			UINT	uIdxX0		= ~0;
			UINT	uIdxY0		= ~0;
			
			
			for( UINT u = 0; u < Keys->Length(); ++u ){
				v8::String::AsciiValue strKey( Keys->Get( u ));
				char *pKey = *strKey;
				
				v8::Local<v8::Array> ArrayTmp = v8::Local<v8::Array>::Cast( hLog->Get( Keys->Get( u )));
				if( ArrayTmp->IsArray()){
					if     ( !strcmp( *strKey, "Time"      )) uIdxTime		= u;
					else if( !strcmp( *strKey, "Distance"  )) uIdxDistance	= u;
					else if( !strcmp( *strKey, "Longitude" )){ uIdxX0		= u; pKey = "X0"; }
					else if( !strcmp( *strKey, "Latitude"  )){ uIdxY0		= u; pKey = "Y0"; }
					
					// strKey で vector 作成
					CArrays.push_back( GetElement( pKey, TRUE ));
					
					// JS の property 名解決後の Array の vector を作る
					JSArrays.push_back( ArrayTmp );
				}
			}
			
			// Distance が無いときは，作る
			BOOL bExistDistance = TRUE;
			if( uIdxDistance == ~0 ){
				uIdxDistance = CArrays.size();
				CArrays.push_back( GetElement( "Distance", TRUE ));
				bExistDistance = FALSE;
			}
			
			// Time 存在確認
			if( uIdxTime == ~0 ) return 0;
			m_dLogStartTime = JSArrays[ uIdxTime ]->Get( 0 )->NumberValue() / 1000.0;
			
			// 緯度経度→メートル 変換定数
			double dLong2Meter = 0;
			double dLati2Meter = 0;
			
			if( uIdxX0 != ~0 && uIdxY0 != ~0 ){
				m_dLong0 = JSArrays[ uIdxX0 ]->Get( 0 )->NumberValue();
				m_dLati0 = JSArrays[ uIdxY0 ]->Get( 0 )->NumberValue();
				
				dLong2Meter = GPSLogGetLength( m_dLong0, m_dLati0, m_dLong0 + 1.0 / 3600, m_dLati0 ) * 3600;
				dLati2Meter = GPSLogGetLength( m_dLong0, m_dLati0, m_dLong0, m_dLati0 + 1.0 / 3600 ) * 3600;
			}
			
			// vector に積む
			double	dDistance = 0;
			int		iSameCnt = 0;
			
			for( UINT uIdx = 0; uIdx < JSArrays[ uIdxTime ]->Length(); ++uIdx ){
				int iCnt = GetCnt();
				
				for( UINT uKey = 0; uKey < Keys->Length(); ++uKey ){
					
					double dVal = JSArrays[ uKey ]->Get( uIdx )->NumberValue();
					
					if( uKey == uIdxTime ){
						if( dVal < m_dLogStartTime ) dVal += 24 * 3600;
						SetTime( iCnt, dVal / 1000.0 - m_dLogStartTime );
					}else if( uKey == uIdxX0 ){
						SetX0( iCnt, ( dVal - m_dLong0 ) * dLong2Meter );
					}else if( uKey == uIdxY0 ){
						SetY0( iCnt, ( m_dLati0 - dVal ) * dLati2Meter );
					}else{
						CArrays[ uKey ]->Set( iCnt, dVal );
					}
				}
				
				if( iCnt == 0 ){
					// 番犬作成
					SetDistance( 0, 0 );
					CopyRecord( 1, 0 );
					CopyRecord( 2, 0 );
					AddStopRecord( 0, -WATCHDOG_TIME );
					AddStopRecord( 1, -TIME_STOP_MARGIN );
				}else{
					// 同じ時間が連続する場合の時刻調整
					if( Time( iCnt - 1 ) == Time( iCnt )){
						// 時刻が同じログが続くときそのカウントをする
						++iSameCnt;
					}else if( iSameCnt ){
						// 時刻が同じログが途切れたので，時間を補正する
						++iSameCnt;
						
						// -4 -3 -2 -1 +0
						//  A  A  A  A  B
						//u =  1  2   3
						
						for( int j = 1; j < iSameCnt; ++j ){
							SetTime(
								iCnt - iSameCnt + j,
								Time( iCnt - iSameCnt ) + 
								( Time( iCnt ) - Time( iCnt - iSameCnt )) / iSameCnt * j
							);
						}
						iSameCnt = 0;
					}
					
					// 走行距離を作る
					if( !bExistDistance ){
						double x = X0( iCnt - 1 ) - X0( iCnt ); x *= x;
						double y = Y0( iCnt - 1 ) - Y0( iCnt ); y *= y;
						
						dDistance += sqrt( x + y );
						SetDistance( iCnt, dDistance );
					}
					
					// 前のログから TIME_STOP 離れていてかつ 指定km/h 以下なら，停止とみなす
					double dDiffTime = Time( iCnt ) - Time( iCnt - 1 );
					if(
						dDiffTime >= TIME_STOP &&
						Distance( iCnt ) / dDiffTime < ( 5.0 /*[km/h]*/ * 1000 / 3600 )
					){
						// -1 -0
						// A  B
						//     ↓
						// -1 -0 +1 +2
						// A  A' B' B
						CopyRecord( iCnt + 1, iCnt     ); // B'
						CopyRecord( iCnt + 2, iCnt     ); // B
						CopyRecord( iCnt,     iCnt - 1 ); // A'
						AddStopRecord( iCnt,     Time( iCnt - 1 ) + TIME_STOP_MARGIN ); // A'
						AddStopRecord( iCnt + 1, Time( iCnt + 2 ) - TIME_STOP_MARGIN ); // B'
					}
				}
			}
		}
	}
	
	/************************************************************************/
	
	if( GetCnt()){
		int iCnt = GetCnt();
		
		// 終端側の番犬
		CopyRecord( iCnt,     iCnt - 1 );
		CopyRecord( iCnt + 1, iCnt - 1 );
		AddStopRecord( iCnt,     Time( iCnt - 1 ) + TIME_STOP_MARGIN );
		AddStopRecord( iCnt + 1, WATCHDOG_TIME );
		
		
		#define DUMP_LOG
		#if defined DEBUG && defined DUMP_LOG
			Dump( "D:\\DDS\\vsd\\vsd_filter\\z_gpslog_raw.txt" );
		#endif
		
		// ログ再スキャン
		GPSLogRescan();
		
		#if defined DEBUG && defined DUMP_LOG
			Dump( "D:\\DDS\\vsd\\vsd_filter\\z_gpslog_upcon.txt" );
		#endif
		
		m_dLogStartTime += 9 * 3600;
	}
	return GetCnt();
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
	
	int iCnt = WATCHDOG_REC_NUM;
	
	while( gzgets( fp, szBuf, BUF_SIZE ) != Z_NULL ){
		if(( p = strstr( szBuf, "LAP" )) != NULL ){ // ラップタイム記録を見つけた
			
			if( pLapLog == NULL ){
				pLapLog = new CLapLog();
				pLapLog->m_iLapMode = LAPMODE_MAGNET;
			}
			
			UINT	uLap, uMin, uSec, uMSec;
			
			uReadCnt = sscanf( p, "LAP%d%d:%d.%d", &uLap, &uMin, &uSec, &uMSec );
			
			int iTime = ( uMin * 60 + uSec ) * 1000 + uMSec;
			
			LapTime.uLap	= uLap;
			LapTime.fLogNum	= ( float )iCnt;
			LapTime.iTime	= ( uReadCnt == 4 ) ? iTime : 0;
			pLapLog->m_Lap.push_back( LapTime );
			
			if(
				uReadCnt == 4 &&
				( pLapLog->m_iBestTime == TIME_NONE || pLapLog->m_iBestTime > iTime )
			){
				pLapLog->m_iBestTime	= iTime;
				pLapLog->m_iBestLap	= pLapLog->m_iLapNum - 1;
				
				iLogFreqLog	 += iCnt - ( int )pLapLog->m_Lap[ pLapLog->m_iLapNum - 1 ].fLogNum;
				iLogFreqTime += iTime;
			}
			++pLapLog->m_iLapNum;
		}
		
		if(( p = strstr( szBuf, "GPS" )) != NULL ){ // GPS記録を見つけた
			double dLong, dLati;
			
			sscanf( p, "GPS%lg%lg", &dLati, &dLong );
			SetLongitude( iCnt, dLong );
			SetLatitude(  iCnt, dLati );
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
			
			SetTacho( iCnt, iTacho );
			SetSpeed( iCnt, dSpeed );
			SetDistance( iCnt, dDistance );
			
			if( uReadCnt >= 5 ){
				if( dGx >= 4 ){	
					// 単位を G に変換
					dGx = -dGx / ACC_1G_Y;
					dGy =  dGy / ACC_1G_Z;
				}
				
				if( iCnt == 0 ){
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
					iCnt &&
					Distance( iCnt - 1 ) == Distance( iCnt ) &&
					( Gy( iCnt - 1 ) - dGy ) >= -0.02 &&
					( Gy( iCnt - 1 ) - dGy ) <=  0.02
				){
					dGcx += dGx / 160;
					dGcy += dGy / 160;
				}
				
				SetGx( iCnt, dGx );
				SetGy( iCnt, dGy );
			}
			
			// ログ開始・終了認識
			if( dSpeed >= 300 ){
				// ★MaxSpeed が 300km/h になるの放置中
				if( !bCalibrating ){
					bCalibrating = TRUE;
					m_iLogStart  = m_iLogStop;
					m_iLogStop   = iCnt;
				}
			}else{
				bCalibrating = FALSE;
			}
			
			SetDateTime( iCnt, ( double )( iCnt - WATCHDOG_REC_NUM ) / LOG_FREQ );
			
			++iCnt;
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
	
	if( GetCnt()){
		AddWatchDog();
	}
	
	return GetCnt();
}

/*** MAP 回転処理 ***********************************************************/

void CVsdLog::RotateMap( double dAngle ){
	for( int i = 0; i < GetCnt(); ++i ){
		SetX( i,  cos( dAngle ) * X0( i ) + sin( dAngle ) * Y0( i ));
		SetY( i, -sin( dAngle ) * X0( i ) + cos( dAngle ) * Y0( i ));
	}
}
