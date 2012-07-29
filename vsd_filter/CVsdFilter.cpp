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

#define MAX_POLY_HEIGHT	2000		// polygon 用ライン数
#define SLineWidth		( m_piParamT[ TRACK_SLineWidth ] / 10.0 )

/*** static member **********************************************************/

/*** tarckbar / checkbox conf_name 名 ***/

const char *CVsdFilter::m_szTrackbarName[] = {
	#define DEF_TRACKBAR( id, init, min, max, name, conf_name ) conf_name,
	#include "def_trackbar.h"
};

const char *CVsdFilter::m_szCheckboxName[] = {
	#define DEF_CHECKBOX( id, init, name, conf_name ) conf_name,
	#include "def_checkbox.h"
};

const char *CVsdFilter::m_szShadowParamName[] = {
	#define DEF_SHADOW( id, init, conf_name ) conf_name,
	#include "def_shadow.h"
};

/*** コンストラクタ *********************************************************/

CVsdFilter::CVsdFilter (){
	
	m_VsdLog 			= NULL;
	m_GPSLog 			= NULL;
	
	m_Lap	 			= new LAP_t[ MAX_LAP ];
	m_iLapNum			= 0;
	m_Lap[ 0 ].fLogNum	= FLT_MAX;	// 番犬
	m_Lap[ 0 ].iTime	= 0;		// 番犬
	
	m_iBestTime			= BESTLAP_NONE;
	m_iBestLap			= 0;
	m_iLapMode			= LAPMODE_HAND_VIDEO;
	
	m_iLapIdx			= -1;
	m_iBestLogNumRunning= 0;
	
	m_bCalcLapTimeReq	= FALSE;
	
	m_szLogFile			= new char[ MAX_PATH + 1 ];
	m_szGPSLogFile		= new char[ BUF_SIZE ];
	
	// DrawPolygon 用バッファ
	m_Polygon			= new PolygonData_t[ MAX_POLY_HEIGHT ];
	
	m_pFont				= NULL;
	
	m_logfont.lfHeight			= 0;							// 文字セルまたは文字の高さ
	m_logfont.lfWidth			= 0;							// 平均文字幅
	m_logfont.lfEscapement		= 0;							// 文字送りの方向とX軸との角度
	m_logfont.lfOrientation		= 0;							// ベースラインとX軸との角度
	m_logfont.lfWeight			= FW_REGULAR;					// フォントの太さ
	m_logfont.lfItalic			= FALSE;						// イタリック体指定
	m_logfont.lfUnderline		= FALSE;						// 下線付き指定
	m_logfont.lfStrikeOut		= FALSE;						// 打ち消し線付き指定
	m_logfont.lfCharSet			= DEFAULT_CHARSET;				// キャラクタセット
	m_logfont.lfOutPrecision	= OUT_DEFAULT_PRECIS;			// 出力精度
	m_logfont.lfClipPrecision	= CLIP_DEFAULT_PRECIS;			// クリッピングの精度
	m_logfont.lfQuality			= PROOF_QUALITY;				// 出力品質
	m_logfont.lfPitchAndFamily	= FIXED_PITCH | FF_DONTCARE;	// ピッチとファミリ
	strcpy( m_logfont.lfFaceName, "ＭＳ ゴシック" );			// フォント名
	
	// str param に初期値設定
	#define DEF_STR_PARAM( id, var, init, conf_name ) strcpy( var, init );
	#include "def_str_param.h"
	
	m_Script	= NULL;
}

/*** デストラクタ ***********************************************************/

CVsdFilter::~CVsdFilter (){
	delete m_VsdLog;
	delete m_GPSLog;
	delete [] m_Lap;
	delete [] m_szLogFile;
	delete [] m_szGPSLogFile;
	delete [] m_Polygon;
	delete m_pFont;
	delete m_Script;
}

/*** 設定ロード *************************************************************/

char *CVsdFilter::IsConfigParam( const char *szParamName, char *szBuf, int &iVal ){
	
	int	iLen;
	
	while( isspace( *szBuf )) ++szBuf;
	
	if(
		strncmp( szBuf, szParamName, iLen = strlen( szParamName )) == 0 &&
		szBuf[ iLen ] == '='
	){
		iVal = atoi( szBuf + iLen + 1 );
		return szBuf + iLen + 1;
	}
	
	return NULL;
}

char *CVsdFilter::IsConfigParamStr( const char *szParamName, char *szBuf, char *szDst ){
	
	int		iLen;
	char	*p;
	
	while( isspace( *szBuf )) ++szBuf;
	
	if(
		strncmp( szBuf, szParamName, iLen = strlen( szParamName )) == 0 &&
		szBuf[ iLen ] == '='
	){
		szBuf += iLen + 1;	// " を指しているはず
		
		// 文字列先頭
		if( p = strchr( szBuf, '"' )){
			szBuf = p + 1;
		}
		
		strcpy( szDst, szBuf );
		
		// 文字列終端
		if(( p = strchr( szDst, '"' )) || ( p = strchr( szDst, ',' ))){
			*p = '\0';
		}
		
		return szDst;
	}
	
	return NULL;
}

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

BOOL CVsdFilter::ConfigLoad( const char *szFileName ){
	
	int 	i, iVal;
	FILE	*fp;
	char	szBuf[ BUF_SIZE ];
	
	if(( fp = fopen( szFileName, "r" )) != NULL ){
		m_bCalcLapTimeReq = TRUE;
		
		while( fgets( szBuf, BUF_SIZE, fp )){
			if( char *p = IsConfigParam( "mark", szBuf, iVal )){
				// ラップタイムマーク
				ParseMarkStr( p + 1 );
			}
			
			// str param のリード
			#define DEF_STR_PARAM( id, var, init, conf_name ) else if( IsConfigParamStr( conf_name, szBuf, var ));
			#include "def_str_param.h"
			
			else{
				// Mark 以外のパラメータ
				for( i = 0; i < TRACK_N; ++i ){
					if(
						m_szTrackbarName[ i ] &&
						IsConfigParam( m_szTrackbarName[ i ], szBuf, iVal )
					){
						m_piParamT[ i ] = iVal;
						goto Next;
					}
				}
				
				for( i = 0; i < CHECK_N; ++i ){
					if(
						m_szCheckboxName[ i ] &&
						IsConfigParam( m_szCheckboxName[ i ], szBuf, iVal )
					){
						m_piParamC[ i ] = iVal;
						goto Next;
					}
				}
				
				for( i = 0; i < SHADOW_N; ++i ){
					if( IsConfigParam( m_szShadowParamName[ i ], szBuf, iVal )){
						m_piParamS[ i ] = iVal;
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

/*** GPS ログリード ********************************************************/

double CVsdFilter::GPSLogGetLength(
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

BOOL CVsdFilter::GPSLogLoad( const char *szFileName ){
	
	UINT	uGPSCnt = 0;
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
	
	if( m_GPSLog ){
		delete m_GPSLog;
		m_GPSLog = NULL;
	}
	
	GPS_LOG_t	*GPSLog = new GPS_LOG_t[ ( int )( MAX_VSD_LOG * GPS_FREQ / LOG_FREQ ) ];
	
	while( *szFileName ){
		
		// ファイル名を / で分解
		char const *p = szFileName;
		if( !( p = strchr( szFileName, '/' ))) p = strchr( szFileName, '\0' );
		strncpy( szBuf, szFileName, p - szFileName );
		*( szBuf + ( p - szFileName )) = '\0';
		szFileName = *p ? p + 1 : p;	// p == '/' ならスキップ
		
		if(( fp = gzopen(( char *)szBuf, "rb" )) == NULL ) return FALSE;
		
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
				
				// 単位を補正
				// 緯度・経度→メートル
				GPSLog[ uGPSCnt ].fX = ( float )(( dLong - dLong0 ) * dLong2Meter );
				GPSLog[ uGPSCnt ].fY = ( float )(( dLati0 - dLati ) * dLati2Meter );
				
				// 速度・向き→ベクトル座標
				GPSLog[ uGPSCnt ].fSpeed	= ( float )( BigEndianS( 12 ) / 10.0 );
				GPSLog[ uGPSCnt ].fBearing	= ( float )BigEndianS( 14 );
				GPSLog[ uGPSCnt ].fTime 	= ( float )dTime;
				
				if( uGPSCnt >=2 ){
					if( GPSLog[ uGPSCnt - 1 ].fTime == GPSLog[ uGPSCnt ].fTime ){
						// 時刻が同じログが続くときそのカウントをする
						++uSameCnt;
					}else if( uSameCnt ){
						// 時刻が同じログが途切れたので，時間を補正する
						++uSameCnt;
						
						for( u = 1; u < uSameCnt; ++ u ){
							GPSLog[ uGPSCnt - uSameCnt + u ].fTime +=
								( GPSLog[ uGPSCnt ].fTime - GPSLog[ uGPSCnt - uSameCnt ].fTime )
								/ uSameCnt * u;
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
				
				// 単位を補正
				// 緯度・経度→メートル
				GPSLog[ uGPSCnt ].fX = ( float )(( dLong - dLong0 ) * dLong2Meter );
				GPSLog[ uGPSCnt ].fY = ( float )(( dLati0 - dLati ) * dLati2Meter );
				
				// 速度・向き→ベクトル座標
				GPSLog[ uGPSCnt ].fSpeed	= ( float )( *( short int *)( szBuf + 0x4 ) / 10.0 );
				
				GPSLog[ uGPSCnt ].fBearing	= FLT_MAX;
				GPSLog[ uGPSCnt ].fTime 	= ( float )( dTime - dTime0 );
				
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
			
			// 海外対応w
			if( cNorthSouth == 'S' ) dLati = -dLati;
			if( cEastWest   == 'W' ) dLong = -dLong;
			
			// 値補正
			// 225446.00 → 22:54:46.00
			dTime =	( int )dTime / 10000 * 3600 +
					( int )dTime / 100 % 100 * 60 +
					fmod( dTime, 100 );
			
			// 4916.452653 → 49度16.45分
			dLati =	( int )dLati / 100 + fmod( dLati, 100 ) / 60;
			dLong =	( int )dLong / 100 + fmod( dLong, 100 ) / 60;
			
			if( uGPSCnt == 0 ){
				dLati0 = dLati;
				dLong0 = dLong;
				dTime0 = dTime;
				
				dLong2Meter = GPSLogGetLength( dLong, dLati, dLong + 1.0 / 3600, dLati ) * 3600;
				dLati2Meter = GPSLogGetLength( dLong, dLati, dLong, dLati + 1.0 / 3600 ) * 3600;
			}
			
			if( dTime < dTime0 ) dTime += 24 * 3600;
			dTime -= dTime0;
			
			// 単位を補正
			// 緯度・経度→メートル
			GPSLog[ uGPSCnt ].fX = ( float )(( dLong - dLong0 ) * dLong2Meter );
			GPSLog[ uGPSCnt ].fY = ( float )(( dLati0 - dLati ) * dLati2Meter );
			
			// 速度・向き→ベクトル座標
			GPSLog[ uGPSCnt ].fSpeed	= uParamCnt < 6 ? FLT_MAX : ( float )( dSpeed * 1.852 ); // knot/h → km/h
			GPSLog[ uGPSCnt ].fBearing	= uParamCnt < 7 ? FLT_MAX : ( float )dBearing;
			GPSLog[ uGPSCnt ].fTime		= ( float )dTime;
			
			if( uGPSCnt >=2 ){
				if( GPSLog[ uGPSCnt - 1 ].fTime == GPSLog[ uGPSCnt ].fTime ){
					// 時刻が同じログが続くときそのカウントをする
					++uSameCnt;
				}else if( uSameCnt ){
					// 時刻が同じログが途切れたので，時間を補正する
					++uSameCnt;
					
					for( u = 1; u < uSameCnt; ++ u ){
						GPSLog[ uGPSCnt - uSameCnt + u ].fTime +=
							( GPSLog[ uGPSCnt ].fTime - GPSLog[ uGPSCnt - uSameCnt ].fTime )
							/ uSameCnt * u;
					}
					uSameCnt = 0;
				}
			}
			
			// 20km/h 以下で 3秒以上 log の間隔が開くとき，0km/h の log を補完する
			if(
				uGPSCnt >= 2 &&
				GPSLog[ uGPSCnt ].fTime - GPSLog[ uGPSCnt - 1 ].fTime >= 3 &&
				GPSLog[ uGPSCnt ].fSpeed <= 20
			){
				// -1 +0 +1 +2
				// A  B
				//  ↓
				// A  A' B' B
				
				// データ B のコピー
				GPSLog[ uGPSCnt + 1 ] =
				GPSLog[ uGPSCnt + 2 ] = GPSLog[ uGPSCnt ];
				
				// データ A のコピー
				GPSLog[ uGPSCnt ] = GPSLog[ uGPSCnt - 1 ];
				
				// スピードを 0 に
				GPSLog[ uGPSCnt ].fSpeed = GPSLog[ uGPSCnt + 1 ].fSpeed = 0;
				
				// 時間調整
				float fDiff = GPSLog[ uGPSCnt - 1 ].fTime - GPSLog[ uGPSCnt - 2 ].fTime;
				GPSLog[ uGPSCnt     ].fTime += fDiff;
				GPSLog[ uGPSCnt + 1 ].fTime -= fDiff;
				
				uGPSCnt += 2;
			}
			
			uGPSCnt++;
		}
		
		gzclose( fp );
	}
	
	/************************************************************************/
	
	// アップコンバート用バッファ確保・初期化
	m_GPSLog = new CVsdLog;
	m_GPSLog->GPSLogUpConvert( GPSLog, uGPSCnt, TRUE );
	m_GPSLog->RotateMap( m_piParamT[ TRACK_MapAngle ] * ( -ToRAD / 10 ));
	
	DebugCmd( {
		FILE *fpp = fopen( "D:\\DDS\\vsd\\vsd_filter\\z_gps_raw.txt", "w" );
		for( u = 0; u < uGPSCnt; ++u ){
			fprintf( fpp, "%g\t%g\t%g\t%g\t%g\t%g\t%g\n",
				GPSLog[ u ].fX,
				GPSLog[ u ].fY,
				GPSLog[ u ].fSpeed,
				GPSLog[ u ].fBearing,
				GPSLog[ u ].fGx,
				GPSLog[ u ].fGy,
				GPSLog[ u ].fTime
			);
		}
		fclose( fpp );
	} )
	
	delete [] GPSLog;
	
	DebugCmd( {
		FILE *fpp = fopen( "D:\\DDS\\vsd\\vsd_filter\\z_upcon_gps.txt", "w" );
		for( u = 0; u < ( UINT )m_GPSLog->m_iCnt; ++u ){
			fprintf( fpp, "%g\t%g\t%g\t%g\t%g\t%g\t%g\t%g\t%g\n",
				m_GPSLog->m_Log[ u ].fSpeed,
				m_GPSLog->m_Log[ u ].fTacho,
				m_GPSLog->m_Log[ u ].fMileage,
				m_GPSLog->m_Log[ u ].fX,
				m_GPSLog->m_Log[ u ].fX0,
				m_GPSLog->m_Log[ u ].fY,
				m_GPSLog->m_Log[ u ].fY0,
				m_GPSLog->m_Log[ u ].fGx,
				m_GPSLog->m_Log[ u ].fGy
			);
		}
		fclose( fpp );
	} )
	
	DeleteIfZero( m_GPSLog );
	if( m_GPSLog ) m_GPSLog->m_dLogStartTime = dTime0 + 9 * 3600;
	
	return m_GPSLog != NULL;
}

/*** ログリード *************************************************************/

BOOL CVsdFilter::ReadLog( const char *szFileName ){
	
	TCHAR	szBuf[ BUF_SIZE ];
	gzFile	fp;
	BOOL	bCalibrating = FALSE;
	
	if( m_VsdLog ){
		delete m_VsdLog;
		m_VsdLog = NULL;
	}
	
	if(( fp = gzopen(( char *)szFileName, "rb" )) == NULL ) return FALSE;
	
	// GPS ログ用
	UINT		uGPSCnt = 0;
	GPS_LOG_t	*GPSLog = new GPS_LOG_t[ ( int )( MAX_VSD_LOG / LOG_FREQ ) ];
	
	// 初期化
	m_iLapNum		= 0;
	m_iBestTime		= BESTLAP_NONE;
	m_iBestLap		= 0;
	m_iLapIdx		= -1;
	m_iBestLogNumRunning	= 0;
	
	m_VsdLog = new CVsdLog;
	
	// ログリード
	
	UINT	uReadCnt, uLap, uMin, uSec, uMSec;
	UINT	uLogNum = 0;
	double	dGcx = 0;
	double	dGcy = 0;
	double	dGx, dGy;
	
	double	dLati;
	double	dLong;
	double	dLati0 = 0;
	double	dLong0 = 0;
	double	dLati2Meter, dLong2Meter;
	double	dSpeed;
	double	dBearing;
	
	m_iLogStart = m_iLogStop = 0;
	
	TCHAR	*p;
	
	int	iLogFreqLog		= 0;
	int	iLogFreqTime	= 0;
	
	while( gzgets( fp, szBuf, BUF_SIZE ) != Z_NULL ){
		if(( p = strstr( szBuf, "LAP" )) != NULL ){ // ラップタイム記録を見つけた
			m_iLapMode = LAPMODE_MAGNET;
			
			uReadCnt = sscanf( p, "LAP%d%d:%d.%d", &uLap, &uMin, &uSec, &uMSec );
			
			int iTime = ( uMin * 60 + uSec ) * 1000 + uMSec;
			
			m_Lap[ m_iLapNum ].uLap		= uLap;
			m_Lap[ m_iLapNum ].fLogNum	= ( float )uLogNum;
			m_Lap[ m_iLapNum ].iTime	= ( uReadCnt == 4 ) ? iTime : 0;
			
			if(
				uReadCnt == 4 &&
				( m_iBestTime == BESTLAP_NONE || m_iBestTime > iTime )
			){
				m_iBestTime	= iTime;
				m_iBestLap	= m_iLapNum - 1;
				
				iLogFreqLog	 += uLogNum - ( int )m_Lap[ m_iLapNum - 1 ].fLogNum;
				iLogFreqTime += iTime;
			}
			++m_iLapNum;
		}
		
		if(( p = strstr( szBuf, "GPS" )) != NULL ){ // GPS記録を見つけた
			sscanf( p, "GPS%lg%lg%lg%lg", &dLati, &dLong, &dSpeed, &dBearing );
			
			if( uGPSCnt == 0 ){
				dLati0 = dLati;
				dLong0 = dLong;
				
				dLong2Meter = GPSLogGetLength( dLong, dLati, dLong + 1.0 / 3600, dLati ) * 3600;
				dLati2Meter = GPSLogGetLength( dLong, dLati, dLong, dLati + 1.0 / 3600 ) * 3600;
			}
			
			// 単位を補正
			// 緯度・経度→メートル
			GPSLog[ uGPSCnt ].fX = ( float )(( dLong - dLong0 ) * dLong2Meter );
			GPSLog[ uGPSCnt ].fY = ( float )(( dLati0 - dLati ) * dLati2Meter );
			GPSLog[ uGPSCnt ].fSpeed	= ( float )dSpeed;
			GPSLog[ uGPSCnt ].fBearing	= ( float )dBearing;
			
			// LOG_FREQ は 15Hz だった時代のログでは間違いだが，等比分割だし実害なし?
			GPSLog[ uGPSCnt++ ].fTime = (( int )uLogNum - GPS_LOG_OFFS ) / ( float )LOG_FREQ;
		}
		
		// 普通の log
		if(( uReadCnt = sscanf( szBuf, "%g%g%g%lg%lg",
			&m_VsdLog->m_Log[ uLogNum ].fTacho,
			&m_VsdLog->m_Log[ uLogNum ].fSpeed,
			&m_VsdLog->m_Log[ uLogNum ].fMileage,
			&dGy,
			&dGx
		)) >= 3 ){
			if( uReadCnt < 5 && uLogNum ){
				// Gデータがないときは，speedから求める←廃止
				dGx = 0;
				dGy = 0;
				//dGy = ( m_VsdLog->m_Log[ uLogNum ].fSpeed - m_VsdLog->m_Log[ uLogNum - 1 ].fSpeed ) * ( 1000.0 / 3600 / 9.8 * LOG_FREQ );
			}else{
				if( dGx >= 4 ){	
					// 単位を G に変換
					dGx = -dGx / ACC_1G_Y;
					dGy =  dGy / ACC_1G_Z;
				}
				
				if( uLogNum == 0 ){
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
					uLogNum &&
					m_VsdLog->m_Log[ uLogNum - 1 ].fMileage == m_VsdLog->m_Log[ uLogNum ].fMileage &&
					( m_VsdLog->m_Log[ uLogNum - 1 ].fGy - dGy ) >= -0.02 &&
					( m_VsdLog->m_Log[ uLogNum - 1 ].fGy - dGy ) <=  0.02
				){
					dGcx += dGx / 160;
					dGcy += dGy / 160;
				}
			}
			
			m_VsdLog->m_Log[ uLogNum ].fGx = ( float )dGx;
			m_VsdLog->m_Log[ uLogNum ].fGy = ( float )dGy;
			
			// ログ開始・終了認識
			if( m_VsdLog->m_Log[ uLogNum ].fSpeed >= 300 ){
				if( !bCalibrating ){
					bCalibrating = TRUE;
					m_iLogStart  = m_iLogStop;
					m_iLogStop   = uLogNum;
				}
			}else{
				bCalibrating = FALSE;
				
				if( m_VsdLog->m_iMaxSpeed < m_VsdLog->m_Log[ uLogNum ].fSpeed )
					m_VsdLog->m_iMaxSpeed = ( int )ceil( m_VsdLog->m_Log[ uLogNum ].fSpeed / 10 ) * 10;
			}
			
			// メーター補正
			m_VsdLog->m_Log[ uLogNum ].fTacho = ( float )(
				m_VsdLog->m_Log[ uLogNum ].fTacho * m_piParamS[ METER_ADJUST ] / 1000.0
			);
			m_VsdLog->m_Log[ uLogNum ].fSpeed = ( float )(
				m_VsdLog->m_Log[ uLogNum ].fSpeed * m_piParamS[ METER_ADJUST ] / 1000.0
			);
			++uLogNum;
		}
	}
	m_VsdLog->m_iCnt = uLogNum;
	
	// 古い log は LOG_FREQ とは限らないので，計算で求める
	if( iLogFreqTime ){
		m_VsdLog->m_dFreq = iLogFreqLog / ( iLogFreqTime / 1000.0 );
	}
	
	/*** GPS ログから軌跡を求める *******************************************/
	
	if( uGPSCnt ){
		m_VsdLog->GPSLogUpConvert( GPSLog, uGPSCnt );
		m_VsdLog->RotateMap( m_piParamT[ TRACK_MapAngle ] * ( -ToRAD / 10 ));
	}
	
	delete [] GPSLog;
	
	/************************************************************************/
	
	gzclose( fp );
	
	m_Lap[ m_iLapNum ].fLogNum	= FLT_MAX;	// 番犬
	m_Lap[ m_iLapNum ].iTime	= 0;		// 番犬
	
	DeleteIfZero( m_VsdLog );
	
	return TRUE;
}

/*** ラップ番号 → ログ番号 変換 ********************************************/

double CVsdFilter::LapNum2LogNum( CVsdLog *Log, int iLapNum ){
	
	double a;
	
	// iLapNum がおかしいときは 0 を返しとく
	if( iLapNum < 0 ) return 0;
	
	if( m_iLapMode == LAPMODE_MAGNET || m_iLapMode == LAPMODE_HAND_MAGNET ){
		// iLogNum は VSD ログ番号
		if( Log == m_VsdLog ) return m_Lap[ iLapNum ].fLogNum;
		if( LogSt == LogEd )  return 0;
		a = ( m_Lap[ iLapNum ].fLogNum - LogSt ) / ( LogEd - LogSt );
		
	}else if( m_iLapMode == LAPMODE_GPS || m_iLapMode == LAPMODE_HAND_GPS ){
		// iLogNum は GPS ログ番号
		if( Log == m_GPSLog ) return m_Lap[ iLapNum ].fLogNum;
		if( GPSSt == GPSEd )  return 0;
		a = ( m_Lap[ iLapNum ].fLogNum - GPSSt ) / ( GPSEd - GPSSt );
		
	}else{
		// iLogNum はビデオフレーム番号
		if( VideoSt == VideoEd ) return 0;
		a = ( m_Lap[ iLapNum ].fLogNum - VideoSt ) / ( VideoEd - VideoSt );
	}
	
	return Log == m_VsdLog ?
		a * ( LogEd - LogSt ) + LogSt :
		a * ( GPSEd - GPSSt ) + GPSSt;
}

/*** ラップタイム再計算 (手動) **********************************************/

void CVsdFilter::CalcLapTime( void ){
	
	int		iTime, iPrevTime;
	int		iFrame = 0;
	double	dLogNum;
	
	m_iLapNum	= 0;
	m_iBestTime	= BESTLAP_NONE;
	
	if( m_VsdLog ){
		m_iLapMode = LAPMODE_HAND_MAGNET;
	}else if( m_GPSLog ){
		m_iLapMode = LAPMODE_HAND_GPS;
	}else{
		m_iLapMode = LAPMODE_HAND_VIDEO;
	}
	
	while(( iFrame = GetFrameMark( iFrame )) >= 0 ){
		
		if( m_iLapMode == LAPMODE_HAND_MAGNET ){
			dLogNum	= ConvParam( iFrame, Video, Log );
			iTime	= ( int )( dLogNum / m_VsdLog->m_dFreq * 1000 );
		}else if( m_iLapMode == LAPMODE_HAND_GPS ){
			dLogNum	= ConvParam( iFrame, Video, GPS );
			iTime	= ( int )( dLogNum / LOG_FREQ * 1000 );
		}else{
			iTime	= ( int )( iFrame * 1000.0 / GetFPS());
		}
		
		m_Lap[ m_iLapNum ].fLogNum	= m_iLapMode == LAPMODE_HAND_VIDEO ? iFrame : ( float )dLogNum;
		
		if( m_piParamT[ TRACK_SLineWidth ] < 0 ){
			// ジムカーナモード
			m_Lap[ m_iLapNum ].uLap		= ( m_iLapNum / 2 ) + 1;
			m_Lap[ m_iLapNum ].iTime	= ( m_iLapNum & 1 ) ? iTime - iPrevTime : 0;
		}else{
			m_Lap[ m_iLapNum ].uLap		= m_iLapNum;
			m_Lap[ m_iLapNum ].iTime	= m_iLapNum ? iTime - iPrevTime : 0;
		}
		
		if(
			m_iLapNum &&
			m_Lap[ m_iLapNum ].iTime &&
			( m_iBestTime == BESTLAP_NONE || m_iBestTime > m_Lap[ m_iLapNum ].iTime )
		){
			m_iBestTime	= m_Lap[ m_iLapNum ].iTime;
			m_iBestLap	= m_iLapNum - 1;
		}
		
		iPrevTime = iTime;
		++m_iLapNum;
		++iFrame;
	}
	m_Lap[ m_iLapNum ].fLogNum	= FLT_MAX;	// 番犬
	m_Lap[ m_iLapNum ].iTime	= 0;		// 番犬
}

/*** ラップタイム再計算 (GPS auto) ******************************************/

void CVsdFilter::CalcLapTimeAuto( void ){
	
	int iFrame;
	
	if(( iFrame = GetFrameMark( 0 )) < 0 ) return;
	
	m_iLapMode = LAPMODE_GPS;
	
	/*** スタートラインの位置を取得 ***/
	// iFrame に対応する GPS ログ番号取得
	double dLogNum = ConvParam( iFrame, Video, GPS );
	
	int iLogNum = ( int )dLogNum;
	
	// iLogNum ～ iLogNum + 1 の方位を算出
	
	double dAngle = atan2(
		( m_GPSLog->m_Log[ iLogNum + 1 ].fY0 - m_GPSLog->m_Log[ iLogNum ].fY0 ),
		( m_GPSLog->m_Log[ iLogNum + 1 ].fX0 - m_GPSLog->m_Log[ iLogNum ].fX0 )
	);
	
	#define x1 m_dStartLineX1
	#define y1 m_dStartLineY1
	#define x2 m_dStartLineX2
	#define y2 m_dStartLineY2
	#define x3 m_GPSLog->m_Log[ i ].fX0
	#define y3 m_GPSLog->m_Log[ i ].fY0
	#define x4 m_GPSLog->m_Log[ i + 1 ].fX0
	#define y4 m_GPSLog->m_Log[ i + 1 ].fY0
	
	// 仮想光電管の位置を求める
	x2 = m_GPSLog->X0( dLogNum );	// スタート地点
	y2 = m_GPSLog->Y0( dLogNum );
	
	x1 = x2 + cos( dAngle + 90 * ToRAD ) * SLineWidth / 2;
	y1 = y2 + sin( dAngle + 90 * ToRAD ) * SLineWidth / 2;
	x2 = x2 + cos( dAngle - 90 * ToRAD ) * SLineWidth / 2;
	y2 = y2 + sin( dAngle - 90 * ToRAD ) * SLineWidth / 2;
	
	/*****/
	
	m_iLapNum	= 0;
	m_iBestTime	= BESTLAP_NONE;
	
	int iTime, iPrevTime;
	int	iLapNum = 0;
	
	for( int i = 0; i < m_GPSLog->m_iCnt - 1; ++i ){
		
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
			( m_GPSLog->m_Log[ i + 1 ].fY0 - m_GPSLog->m_Log[ i ].fY0 ),
			( m_GPSLog->m_Log[ i + 1 ].fX0 - m_GPSLog->m_Log[ i ].fX0 )
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
		iTime = ( int )( dLogNum / LOG_FREQ * 1000 );
		iPrevTime;
		
		if( m_piParamS[ SHADOW_LAP_START ] - 1 <= iLapNum && iLapNum <= m_piParamS[ SHADOW_LAP_END ] ){
			m_Lap[ m_iLapNum ].uLap		= m_iLapNum;
			m_Lap[ m_iLapNum ].fLogNum	= ( float )dLogNum;
			m_Lap[ m_iLapNum ].iTime	= m_iLapNum ? iTime - iPrevTime : 0;
			
			if(
				m_iLapNum &&
				( m_iBestTime == BESTLAP_NONE || m_iBestTime > m_Lap[ m_iLapNum ].iTime )
			){
				m_iBestTime	= m_Lap[ m_iLapNum ].iTime;
				m_iBestLap	= m_iLapNum - 1;
			}
			
			iPrevTime = iTime;
			++m_iLapNum;
		}
		++iLapNum;
	}
	m_Lap[ m_iLapNum ].fLogNum	= FLT_MAX;	// 番犬
	m_Lap[ m_iLapNum ].iTime	= 0;		// 番犬
}
