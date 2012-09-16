/*****************************************************************************
	
	VSD -- vehicle data logger system  Copyright(C) by DDS
	
	CVsdLog.h - CVsdLog class header
	
*****************************************************************************/

#pragma once

/*** macros *****************************************************************/

#define TIME_NONE	(( int )0x80000000 )
#define WATCHDOG_TIME	1E+12

#define WATCHDOG_REC_NUM	2	// ログ先頭の番犬分のレコード数

/*** Lap Time ***************************************************************/

enum {
	LAPMODE_HAND_VIDEO,		// 手動計測モード，時計は Video フレーム
	LAPMODE_HAND_GPS,		// 手動計測モード，時計は GPS ログ時計
	LAPMODE_GPS,			// GPS 自動計測モード
	LAPMODE_MAGNET,			// 磁気センサー自動計測モード
};

typedef struct {
	UINT	uLap;
	float	fLogNum;
	int		iTime;
} LAP_t;

class CVsdLog;
class CLapLog {
  public:
	CLapLog(){
		// 初期化
		m_iLapNum		= 0;
		m_iBestTime		= TIME_NONE;
		m_iBestLap		= 0;
		m_iLapMode		= LAPMODE_HAND_VIDEO;
		m_iLapIdx		= -1;
		m_iBestLogNumRunning	= 0;
	}
	
	~CLapLog(){}
	
	std::vector<LAP_t>	m_Lap;
	int	m_iLapMode;
	int	m_iLapNum;
	int	m_iBestTime;
	int	m_iBestLap;
	int	m_iLapIdx;
	int	m_iCurTime;		// 現在ラップ経過時間
	int	m_iDiffTime;
	int m_iBestLogNumRunning;
};

/*** new type ***************************************************************/

class VSD_LOG_t {
	
  public:
	VSD_LOG_t(){
		m_fMin =  FLT_MAX;
		m_fMax = -FLT_MAX;
	}
	
	~VSD_LOG_t(){}
	
	// 値取得
	double Get( int    iIndex ){ return m_Log[ iIndex ]; }
	double Get( double dIndex ){
		double alfa = dIndex - ( UINT )dIndex;
		return
			m_Log[ ( UINT )dIndex     ] * ( 1 - alfa ) +
			m_Log[ ( UINT )dIndex + 1 ] * (     alfa );
	}
	
	double GetMin(){ return m_fMin; }
	double GetMax(){ return m_fMax; }
	
	// 値設定
	void Set( int iIndex, double dVal ){
		if(      m_fMin > dVal ) m_fMin = ( float )dVal;
		else if( m_fMax < dVal ) m_fMax = ( float )dVal;
		
		// ★無い値を線形補間する必要あり
		if( GetCnt() > iIndex ) m_Log[ iIndex ] = ( float )dVal;
		else while( GetCnt() <= iIndex ) Push( dVal );
	}
	
	int GetCnt( void ){
		return m_Log.size();
	}
	
	void InitMinMax( void ){
		m_fMin =  FLT_MAX;
		m_fMax = -FLT_MAX;
	}
	
  private:
	float	m_fMin;
	float	m_fMax;
	std::vector<float>	m_Log;
};

class CVsdLog {
	
  public:
	int GetCnt( void ){ return m_iCnt; }
	
	/////////////////////////
	
	int		m_iLogNum;
	double	m_dLogNum;
	
	double	m_dFreq;
	double	m_dLogStartTime;	// ログ開始時間
	
	// 緯度経度→メートル 変換用
	double	m_dLong0;
	double	m_dLati0;
	
	// VSD ログ位置自動認識用
	int		m_iLogStart;
	int		m_iLogStop;
	
	UINT	m_uSameCnt;
	
	// ログの map
	std::map<std::string, VSD_LOG_t *> m_Logs;
	
	// コンストラクタ・デストラクタ
	CVsdLog();
	~CVsdLog(){}
	
	UINT GPSLogUpConvert();
	void RotateMap( double dAngle );
	double GetIndex( double dFrame, int iVidSt, int iVidEd, int iLogSt, int iLogEd, int iPrevIdx );
	double GetIndex( double dTime, int iPrevIdx );
	
	#ifdef DEBUG
		void Dump( char *szFileName );
	#endif
	
	BOOL IsDataExist( int iLogNum ){
		return 0 <= iLogNum && iLogNum < GetCnt();
	}
	
	void PushRecord( void );
	int ReadGPSLog( const char *szFileName );
	int ReadLog( const char *szFileName, CLapLog *&pLapLog );
	
	double GPSLogGetLength(
		double dLong0, double dLati0,
		double dLong1, double dLati1
	);
	
	// 終端側の番犬追加
	void AddWatchDog( void ){
		/* ★保留
		m_Log.push_back( m_Log[ GetCnt() - 1 ] );
		m_Log.push_back( m_Log[ GetCnt() - 1 ] );
		m_Log[ GetCnt() - 2 ].StopLog( Time( GetCnt() - 2 ) + 0.5 );
		m_Log[ GetCnt() - 1 ].StopLog( WATCHDOG_TIME );
		*/
	}
	
	// key の存在確認
	
	VSD_LOG_t *GetEmement( const char *szKey ){
		std::string strKey( szKey );
		return GetElement( strKey );
	}
	VSD_LOG_t *GetElement( const std::string &strKey ){
		std::map<std::string, VSD_LOG_t *>::iterator itr;
		if(( itr = m_Logs.find( strKey )) != m_Logs.end()){
			return itr->second;
		}
		return NULL;
	}
	
	// レコードコピー
	void CopyRecord( int iTo, int iFrom );
	
	// set / get 関数
	
	double Get( const char *szKey, int iIndex ){
		std::string strKey( szKey );
		return Get( strKey, iIndex );
	}
	
	double Get( const std::string &strKey, int iIndex ){
		if( m_Logs.find( strKey ) == m_Logs.end()){
			return 0;	// 要素なし
		}
		return m_Logs[ strKey ]->Get( iIndex );
	}
	
	double Get( const char *szKey, double dIndex ){
		std::string strKey( szKey );
		return Get( strKey, dIndex );
	}
	
	double Get( const std::string &strKey, double dIndex ){
		if( m_Logs.find( strKey ) == m_Logs.end()){
			return 0;	// 要素なし
		}
		return m_Logs[ strKey ]->Get( dIndex );
	}
	
	void Set( const char *szKey, int iIndex, double dVal );
	void Set( const std::string &strKey, int iIndex, double dVal );
	
	double GetMin( const char *szKey ){
		std::string strKey( szKey );
		return GetMin( strKey );
	}
	
	double GetMin( const std::string &strKey ){
		VSD_LOG_t	*pLog = GetElement( strKey );
		return pLog ? pLog->GetMin() : 0;
	}
	
	double GetMax( const char *szKey ){
		std::string strKey( szKey );
		return GetMax( strKey );
	}
	
	double GetMax( const std::string &strKey ){
		VSD_LOG_t	*pLog = GetElement( strKey );
		return pLog ? pLog->GetMax() : 0;
	}
	
	#define DEF_LOG( name ) static const std::string m_str##name;
	#include "def_log.h"
	#define DEF_LOG( name ) double name( void          ){ return Get( m_str##name, m_dLogNum ); }
	#include "def_log.h"
	#define DEF_LOG( name ) double name( int    iIndex ){ return Get( m_str##name, iIndex ); }
	#include "def_log.h"
	#define DEF_LOG( name ) double name( double dIndex ){ return Get( m_str##name, dIndex ); }
	#include "def_log.h"
	#define DEF_LOG( name ) void Set##name( int iIndex, double dVal ){ Set( m_str##name, iIndex, dVal ); }
	#include "def_log.h"
	#define DEF_LOG( name ) double Max##name( void ){ return GetMax( m_str##name ); }
	#include "def_log.h"
	#define DEF_LOG( name ) double Min##name( void ){ return GetMin( m_str##name ); }
	#include "def_log.h"
	
	// 緯度経度セット
	void SetLongitude( int iIndex, double dVal ){
		if( m_dLong0 == 0 ) m_dLong0 = dVal;
		SetX0( iIndex, dVal - m_dLong0 );
	}
	
	void SetLatitude( int iIndex, double dVal ){
		if( m_dLati0 == 0 ) m_dLati0 = dVal;
		SetY0( iIndex, m_dLati0 - dVal );
	}
	
	void SetDateTime( int iIndex, double dVal ){
		if( m_dLogStartTime == 0 ) m_dLogStartTime = dVal;
		else if( dVal < m_dLogStartTime ) dVal += 24 * 3600;
		SetTime( iIndex, dVal - m_dLogStartTime );
	}
	
  private:
	int	m_iCnt;
};
