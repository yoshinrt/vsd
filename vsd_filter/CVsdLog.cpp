/*****************************************************************************
	
	VSD -- vehicle data logger system  Copyright(C) by DDS
	
	CVsdLog.cpp - CVsdLog class implementation
	
*****************************************************************************/

#include "StdAfx.h"
#include "CVsdFilter.h"

/*** macros *****************************************************************/

#define GRAVITY				9.80665
#define CALIB_MARK_SPEED	300
#define MAX_TURN_R			200

/*** コンストラクタ *********************************************************/

CVsdLog::CVsdLog( CVsdFilter *pVsd ){
	m_dFreq	= LOG_FREQ;
	
	m_iLogStartTime	= -1;
	
	m_dLong2Meter = 0;
	m_dLati2Meter = 0;
	m_iCnt		= 0;
	m_pVsd		= pVsd;
	
	#define DEF_LOG( name )	m_pLog##name = NULL;
	#include "def_log.h"
}

/*** frame# に対応するログ index を得る *************************************/

double CVsdLog::GetIndex( double dFromVal, int *piFrom, int *piLog, int iPrevIdx ){
	if( piFrom[ 0 ] == piFrom[ 1 ] ) return 2;	// 番犬を除いた Time=0 のログ
	return GetIndex(
		(
			piLog[ 0 ] +
			( piLog[ 1 ] - piLog[ 0 ] ) * ( dFromVal - piFrom[ 0 ] ) /
			( double )( piFrom[ 1 ] - piFrom[ 0 ] )
		) / SLIDER_TIME,
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
	std::map<std::string, CLog *>::iterator it;
	
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
			fprintf( fp, "%f", it->second->GetRaw( i ));
		}
		fputs( "\n", fp );
	}
	fclose( fp );
}
#endif

/*** key の存在確認 *********************************************************/

CLog *CVsdLog::GetElement( const char *szKey, BOOL bCreate ){
	std::string strKey( szKey );
	std::map<std::string, CLog *>::iterator itr;
	
	if(( itr = m_Logs.find( strKey )) != m_Logs.end()){
		return itr->second;
	}
	if( bCreate ){
		CLog *p;
		
		// Speed とか基本データの参照用ポインタに代入
		if( 0 );
		#define DEF_LOG( name )	DEF_LOG_T( name, CLogFloat )
		#define DEF_LOG_T( name, type )	\
			else if( strKey == #name ) p = m_pLog##name = new type();
		#include "def_log.h"
		else{
			p = new CLogFloat();
		}
		
		m_Logs[ strKey ] = p;
		return p;
	}
	return NULL;
}

/*** ログの Set *************************************************************/

void CVsdLog::Set( const char *szKey, int iIndex, double dVal ){
	GetElement( szKey, TRUE )->Set( iIndex, dVal );
	if( m_iCnt <= iIndex ) m_iCnt = iIndex + 1;
}

/*** 1レコードコピー ********************************************************/

void CVsdLog::CopyRecord( int iTo, int iFrom ){
	std::map<std::string, CLog *>::iterator it;
	
	for( it = m_Logs.begin(); it != m_Logs.end(); ++it ){
		CLog *pLog = it->second;
		pLog->Set( iTo, pLog->GetRaw( iFrom ));
	}
	if( m_iCnt <= iTo ) m_iCnt = iTo + 1;
}

/*** 番犬追加 ***************************************************************/

void CVsdLog::AddWatchDog( void ){
	int iCnt = GetCnt() - 1;
	CopyRecord( 0, 2 );           AddStopRecord( 0, -WATCHDOG_TIME );
	CopyRecord( 1, 2 );           AddStopRecord( 1, Time( 2 ) - 500 );
	CopyRecord( iCnt + 1, iCnt ); AddStopRecord( iCnt + 1, Time( iCnt ) + 500 );
	CopyRecord( iCnt + 2, iCnt ); AddStopRecord( iCnt + 2, WATCHDOG_TIME );
}

/*** GPS ログの up-convert **************************************************/

UINT CVsdLog::GPSLogRescan( void ){
	
	/*** CLog の方を走査して，いろいろ補正 *****************************/
	
	BOOL	bCreateSpeed	= FALSE;
	BOOL	bCreateG		= FALSE;
	BOOL	bCreateDir		= FALSE;
#ifdef USE_TURN_R
	BOOL	bCreateTurnR	= FALSE;
#endif
	
	if( m_pLogLongitude != NULL && m_pLogLatitude != NULL ){
		if( !m_pLogSpeed ){
			bCreateSpeed = TRUE;
			GetElement( "Speed", TRUE );
			m_pLogSpeed->Resize( GetCnt(), 0 );
		}
		
		if( !m_pLogGx ){
			bCreateG = TRUE;
			GetElement( "Gx", TRUE );
			m_pLogGx->Resize( GetCnt(), 0 );
			GetElement( "Gy", TRUE );
			m_pLogGy->Resize( GetCnt(), 0 );
		}
		
		if( !m_pLogDirection ){
			bCreateDir = TRUE;
			GetElement( "Direction", TRUE );
			m_pLogDirection->Resize( GetCnt(), 0 );
		}
		
		#ifdef USE_TURN_R
			if( !m_pLogTurnR ){
				bCreateTurnR = TRUE;
				GetElement( "TurnR", TRUE );
				m_pLogTurnR->Resize( GetCnt(), MAX_TURN_R );
			}
		#endif
	}
	
	#pragma omp parallel
	{
		double dMaxSpeed, dMinSpeed;
		double dMaxGx, dMinGx;
		double dMaxGy, dMinGy;
		
		dMaxSpeed = -FLT_MAX;
		dMinSpeed =  FLT_MAX;
		
		// スピード生成
		if( bCreateSpeed ){
			#pragma omp for
			for( int i = 1; i < GetCnt(); ++i ){
				if( Time( i ) - Time( i - 1 ) >= ( TIME_STOP - TIME_STOP_MARGIN * 2 )){
					// 時間が開いている停止ログ
					SetRawSpeed( i, 0 );
					if( dMaxSpeed < 0 ) dMaxSpeed = 0;
					if( dMinSpeed > 0 ) dMinSpeed = 0;
				}else{
					double d;
					SetRawSpeed( i,
						d = ( Distance( i ) - Distance( i - 1 ))
						* ( 3600.0 / 1000 * 1000 ) /
						( Time( i ) - Time( i - 1 ))
					);
					if( dMaxSpeed < d ) dMaxSpeed = d;
					if( dMinSpeed > d ) dMinSpeed = d;
				}
			}
			// 番犬はすでに 0 → SetRawSpeed( GetCnt() - 1, 0 );
			
			#pragma omp critical
			{
				SetMaxMinSpeed( dMaxSpeed, dMinSpeed );
			}
		}
		
		// G 計算
		if( bCreateG ){
			double	dBearingPrev = 100;
			
			#pragma omp for
			for( int i = 2; i < GetCnt() - 1; ++i ){
				if( dBearingPrev = 100 ){
					dBearingPrev = atan2( Y0( i ) - Y0( i - 2 ), X0( i ) - X0( i - 2 ));
				}
				
				// bearing 計算
				double dBearing = atan2( Y0( i + 1 ) - Y0( i - 1 ), X0( i + 1 ) - X0( i - 1 ));
				
				if( bCreateDir ){
					double dDir = dBearing / ToRAD + 90;
					if( dDir < 0 ) dDir += 360;
					SetDirection( i, dDir );
				}
				
				// 横 G 計算
				// Gx / Gy を作る
				SetRawGy( i,
					( Speed( i ) - Speed( i - 1 ))
					* ( 1000 / 3.600 / GRAVITY )
					/ ( Time( i ) - Time( i - 1 ))
				);
				
				// 横G = vω
				double dBearingDelta = dBearing - dBearingPrev;
				if     ( dBearingDelta >  M_PI ) dBearingDelta -= M_PI * 2;
				else if( dBearingDelta < -M_PI ) dBearingDelta += M_PI * 2;
				
				SetRawGx( i,
					dBearingDelta * ( 1000 / GRAVITY )
					/ ( Time( i ) - Time( i - 1 ))
					* ( Speed( i ) / 3.600 )
				);
				
				// ±5G 以上は，削除
				if( Gx( i ) < -3 || Gx( i ) > 3 ){
					SetRawGx( i, Gx( i - 1 ));
				}
				
				dBearingPrev = dBearing;
			}
			
			// スムージング
			#define G_SMOOTH_NUM	3	// 平均化する要素数
			#define G_SMOOTH_CNT	( m_dFreq > 5 ? 2 : 1 )	// スムージング回数
			#define G_SMOOTH_RATIO	0.3	// min/max 計算用係数
			
			#pragma omp single
			{
				// 番犬と，最初 / 最後の G 初期化
				SetRawGx( 0, 0 ); SetRawGy( 0, 0 );
				SetRawGx( 1, 0 ); SetRawGy( 1, 0 );
				SetRawGx( 2, 0 ); SetRawGy( 2, 0 );
				SetRawGx( GetCnt() - 3, 0 ); SetRawGy( GetCnt() - 3, 0 );
				SetRawGx( GetCnt() - 2, 0 ); SetRawGy( GetCnt() - 2, 0 );
				SetRawGx( GetCnt() - 1, 0 ); SetRawGy( GetCnt() - 1, 0 );
			}
			
			for( UINT v = G_SMOOTH_CNT; v; --v ){
				dMaxGx = dMaxGy = -FLT_MAX;
				dMinGx = dMinGy =  FLT_MAX;
				double dx0, dy0;
				dx0 = dy0 = 0;
				
				#pragma omp for
				for( int i = ( G_SMOOTH_NUM - 1 ) / 2; i < GetCnt() - G_SMOOTH_NUM / 2; ++i ){
					
					// 番犬分は処理スキップ
					if( i < 2 || i >= GetCnt() - 2 ) continue;
					
					if( Speed( i ) == 0 ){
						SetRawGx( i, 0 );
						SetRawGy( i, 0 );
						
						if( dMaxGx < 0 ) dMaxGx = 0;
						if( dMinGx > 0 ) dMinGx = 0;
						if( dMaxGy < 0 ) dMaxGy = 0;
						if( dMinGy > 0 ) dMinGy = 0;
					}else{
						double dx, dy;
						
						SetRawGx( i, dx = (
							( G_SMOOTH_NUM >= 7 ? Gx( i - 3 ) : 0 ) +
							( G_SMOOTH_NUM >= 6 ? Gx( i + 3 ) : 0 ) +
							( G_SMOOTH_NUM >= 5 ? Gx( i - 2 ) : 0 ) +
							( G_SMOOTH_NUM >= 4 ? Gx( i + 2 ) : 0 ) +
							( G_SMOOTH_NUM >= 3 ? Gx( i - 1 ) : 0 ) +
							( G_SMOOTH_NUM >= 2 ? Gx( i + 1 ) : 0 ) +
							Gx( i + 0 )
						) / G_SMOOTH_NUM );
						SetRawGy( i, dy = (
							( G_SMOOTH_NUM >= 7 ? Gy( i - 3 ) : 0 ) +
							( G_SMOOTH_NUM >= 6 ? Gy( i + 3 ) : 0 ) +
							( G_SMOOTH_NUM >= 5 ? Gy( i - 2 ) : 0 ) +
							( G_SMOOTH_NUM >= 4 ? Gy( i + 2 ) : 0 ) +
							( G_SMOOTH_NUM >= 3 ? Gy( i - 1 ) : 0 ) +
							( G_SMOOTH_NUM >= 2 ? Gy( i + 1 ) : 0 ) +
							Gy( i + 0 )
						) / G_SMOOTH_NUM );
						
						if( v == 1 ){
							dx0 = dx * G_SMOOTH_RATIO + dx0 * ( 1 - G_SMOOTH_RATIO );
							dy0 = dy * G_SMOOTH_RATIO + dy0 * ( 1 - G_SMOOTH_RATIO );
							
							if( dMaxGx < dx0 ) dMaxGx = dx0;
							if( dMinGx > dx0 ) dMinGx = dx0;
							if( dMaxGy < dy0 ) dMaxGy = dy0;
							if( dMinGy > dy0 ) dMinGy = dy0;
						}
					}
				}
			}
			
			#pragma omp critical
			{
				SetMaxMinGx( dMaxGx, dMinGx );
				SetMaxMinGy( dMaxGy, dMinGy );
			}
			
			#ifdef USE_TURN_R
				if( bCreateTurnR ){
					#pragma omp for
					for( int i = 2; i < GetCnt(); ++i ){
						// r = v * v / G
						double dTurnR = Gx( i ) == 0 || Speed( i ) < 1 ?
							MAX_TURN_R :
							pow( Speed( i ) * ( 1000.0 / 3600 ), 2 ) / ( fabs( Gx( i )) * GRAVITY );
						
						if( dTurnR > MAX_TURN_R ) dTurnR = MAX_TURN_R;
						SetTurnR( i, dTurnR );
					}
				}
			#endif
		}
  	}
	
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

/*** ログリード *************************************************************/

int CVsdLog::ReadLog( const char *szFileName, const char *szReaderFunc, CLapLog *&pLapLog ){
	{
		// JavaScript オブジェクト初期化
		CScript Script( m_pVsd );
		if( Script.InitLogReader() != ERR_OK ){
			return 0;
		}
		
		// スクリプト実行
		LPWSTR pStr = NULL;
		LPWSTR pReader = NULL;
		Script.Run_ss( L"ReadLog",
			StringNew( pStr, szFileName ),
			StringNew( pReader, szReaderFunc )
		);
		delete [] pStr;
		delete [] pReader;
		
		if( Script.m_uError != ERR_OK ){
			m_pVsd->DispErrorMessage( Script.GetErrorMessage());
			return 0;
		}
		
		/*** JS の Log にアクセス *******************************************/
		
		{
			v8::Isolate::Scope IsolateScope( Script.m_pIsolate );
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
			
			// CLog * の vector
			std::vector<CLog *>	CArrays;
			
			UINT	uIdxTime		= ~0;
			UINT	uIdxLapTime		= ~0;
			UINT	uIdxDistance	= ~0;
			UINT	uIdxSpeed		= ~0;
			UINT	uIdxLong		= ~0;
			UINT	uIdxLati		= ~0;
			
			UINT	uIdx = 0;
			for( UINT u = 0; u < Keys->Length(); ++u ){
				v8::String::AsciiValue strKey( Keys->Get( u ));
				char *pKey = *strKey;
				
				v8::Local<v8::Array> ArrayTmp = v8::Local<v8::Array>::Cast( hLog->Get( Keys->Get( u )));
				if( ArrayTmp->IsArray()){
					if     ( !strcmp( *strKey, "Time"      )) uIdxTime		= uIdx;
					else if( !strcmp( *strKey, "Distance"  )) uIdxDistance	= uIdx;
					else if( !strcmp( *strKey, "Speed"     )) uIdxSpeed		= uIdx;
					else if( !strcmp( *strKey, "Longitude" )) uIdxLong		= uIdx;
					else if( !strcmp( *strKey, "Latitude"  )) uIdxLati		= uIdx;
					else if( !strcmp( *strKey, "LapTime"   )){
						uIdxLapTime	= uIdx;
						CArrays.push_back( NULL );
						goto Skip;	// LapTime は CVsdLog には積まない
					}
					
					// strKey で vector 作成
					CArrays.push_back( GetElement( pKey, TRUE ));
					
				  Skip:
					// JS の property 名解決後の Array の vector を作る
					JSArrays.push_back( ArrayTmp );
					
					++uIdx;
				}
			}
			
			// Time 存在確認
			if( uIdxTime == ~0 ) return 0;
			m_iLogStartTime = ( time_t )JSArrays[ uIdxTime ]->Get( 0 )->NumberValue();
			
			BOOL bCreateDistance = FALSE;
			
			if( uIdxLong != ~0 && uIdxLati != ~0 ){
				// Distance が無いときは，作る
				if( uIdxDistance == ~0 ){
					uIdxDistance = uIdx++;
					CArrays.push_back( GetElement( "Distance", TRUE ));
					bCreateDistance = TRUE;
				}
				
				// 緯度経度→メートル 変換定数
				double	dLong0;
				double	dLati0;
				
				dLong0 = JSArrays[ uIdxLong ]->Get( 0 )->NumberValue();
				m_pLogLongitude->SetBaseVal( dLong0 );
				dLati0 = JSArrays[ uIdxLati ]->Get( 0 )->NumberValue();
				m_pLogLatitude->SetBaseVal( dLati0 );
				
				m_dLong2Meter =  GPSLogGetLength( dLong0, dLati0, dLong0 + 1.0 / 3600, dLati0 ) * 3600;
				m_dLati2Meter = -GPSLogGetLength( dLong0, dLati0, dLong0, dLati0 + 1.0 / 3600 ) * 3600;
			}
			
			// vector に積む
			double	dDistance = 0;
			int		iSameCnt = 0;
			
			double	dLogHzTime	= 0;
			double	iLogHzCnt	= 0;
			
			UINT	uLapCnt	= 1;
			
			UINT	uCalibrating = 0;
			
			for( UINT uIdx = 0; uIdx < JSArrays[ uIdxTime ]->Length(); ++uIdx ){
				int iCnt = GetCnt();
				int	iLapTime = -1;
				
				for( UINT uKey = 0; uKey < Keys->Length(); ++uKey ){
					
					double dVal = JSArrays[ uKey ]->Get( uIdx )->NumberValue();
					
					if( uKey == uIdxTime ){
						time_t t = ( time_t )dVal;
						if( t < m_iLogStartTime ) t += 24 * 3600 * 1000;
						SetTime( iCnt, ( double )( t - m_iLogStartTime ));
					}else if( uKey == uIdxSpeed ){
						// キャリブレーション中は，一旦 0km/h にする
						if( dVal >= CALIB_MARK_SPEED ){
							++uCalibrating;
							dVal = 0;
						}else{
							uCalibrating = 0;
						}
						CArrays[ uKey ]->Set( iCnt, dVal );
					}else if( uKey == uIdxLapTime ){
						if( !_isnan( dVal )) iLapTime = ( int )dVal;
					}else{
						CArrays[ uKey ]->Set( iCnt, dVal );
					}
				}
				
				if( iCnt == 0 ){
					// 番犬作成
					if( uIdxDistance != ~0 ) SetDistance( 0, 0 );
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
					if( bCreateDistance ){
						double x = X0( iCnt - 1 ) - X0( iCnt ); x *= x;
						double y = Y0( iCnt - 1 ) - Y0( iCnt ); y *= y;
						
						dDistance += sqrt( x + y );
						SetDistance( iCnt, dDistance );
					}
					
					// 前のログから TIME_STOP 離れていてかつ 指定km/h 以下なら，停止とみなす
					double dDiffTime = Time( iCnt ) - Time( iCnt - 1 );
					if(
						uIdxDistance != ~0 &&
						dDiffTime >= TIME_STOP &&
						( Distance( iCnt ) - Distance( iCnt - 1 )) / dDiffTime < ( 5.0 /*[km/h]*/ * 1000 / 3600 )
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
					}else{
						// 停止期間でなければ，ログ Hz を計算する
						dLogHzTime += dDiffTime;
						++iLogHzCnt;
					}
				}
				
				// ラップタイムを積む
				if( iLapTime >= 0 ){
					if( pLapLog == NULL ){
						pLapLog = new CLapLog();
						pLapLog->m_iLapMode = LAPMODE_MAGNET;
					}
					LAP_t	LapTime;
					
					LapTime.uLap	= uLapCnt;
					LapTime.fLogNum	= ( float )( GetCnt() - 1 );
					LapTime.iTime	= iLapTime;
					pLapLog->m_Lap.push_back( LapTime );
					
					if( iLapTime ) ++uLapCnt;
					
					if(
						iLapTime > 0 &&
						( pLapLog->m_iBestTime == TIME_NONE || pLapLog->m_iBestTime > iLapTime )
					){
						pLapLog->m_iBestTime	= iLapTime;
						pLapLog->m_iBestLap	= pLapLog->m_iLapNum - 1;
					}
					++pLapLog->m_iLapNum;
				}
				
				// キャリブレーション時の時間を記録
				if( uCalibrating ){
					if( uCalibrating == 1 ){
						m_dCalibStart = m_dCalibStop;
						m_dCalibStop  = Time( GetCnt() - 1 );
					}
					// 300km/h に戻す
					SetRawSpeed( GetCnt() - 1, CALIB_MARK_SPEED );
				}
			}
			// ログ Hz 最終集計
			m_dFreq = iLogHzCnt / dLogHzTime * 1000;
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
		
		// Time の Max Min 設定
		m_pLogTime->InitMinMax();
		m_pLogTime->SetMaxMin( Time( GetCnt() - 2 ), 0 );
	}
	
	// Lap log の番犬
	if( pLapLog ){
		LAP_t	LapTime;
		LapTime.fLogNum	= FLT_MAX;	// 番犬
		LapTime.iTime	= 0;		// 番犬
		pLapLog->m_Lap.push_back( LapTime );
	}
	
	return GetCnt();
}

/*** MAP 回転処理 ***********************************************************/

void CVsdLog::RotateMap( double dAngle ){
	
	if( !m_pLogLongitude ) return;
	
	if( m_pLogX ){
		m_pLogX->InitMinMax();
		m_pLogY->InitMinMax();
	}
	
	for( int i = 0; i < GetCnt(); ++i ){
		SetX( i,  cos( dAngle ) * X0( i ) + sin( dAngle ) * Y0( i ));
		SetY( i, -sin( dAngle ) * X0( i ) + cos( dAngle ) * Y0( i ));
	}
}
