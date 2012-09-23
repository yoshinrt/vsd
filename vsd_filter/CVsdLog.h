/*****************************************************************************
	
	VSD -- vehicle data logger system  Copyright(C) by DDS
	
	CVsdLog.h - CVsdLog class header
	
*****************************************************************************/

#pragma once

/*** macros *****************************************************************/

#define TIME_NONE	(( int )0x80000000 )
#define WATCHDOG_TIME	1E+12

#define WATCHDOG_REC_NUM	2	// ログ先頭の番犬分のレコード数
#define TIME_STOP			3	// 停車とみなす Log 時間間隔
#define TIME_STOP_MARGIN	0.5	// 停車とみなす Log に付加する Log の時間差分

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
	
	void Push( double dVal ){
		m_Log.push_back(( float )dVal );
	}
	
	void InitMinMax( void ){
		m_fMin =  FLT_MAX;
		m_fMax = -FLT_MAX;
	}
	
	void Resize( int iCnt, double dVal ){ m_Log.resize( iCnt, ( float )dVal ); }
	
  private:
	std::vector<float>	m_Log;
	float	m_fMin;
	float	m_fMax;
};

class CVsdFilter;
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
	
	// ログの map
	std::map<std::string, VSD_LOG_t *> m_Logs;
	
	// コンストラクタ・デストラクタ
	CVsdLog( CVsdFilter *pVsd );
	~CVsdLog(){}
	
	UINT GPSLogRescan();
	void RotateMap( double dAngle );
	double GetIndex( double dFrame, int iVidSt, int iVidEd, int iLogSt, int iLogEd, int iPrevIdx );
	double GetIndex( double dTime, int iPrevIdx );
	
	#ifdef DEBUG
		void Dump( char *szFileName );
	#endif
	
	BOOL IsDataExist( int iLogNum ){
		return 0 <= iLogNum && iLogNum < GetCnt();
	}
	
	int ReadLog( const char *szFileName, CLapLog *&pLapLog );
	
	double GPSLogGetLength(
		double dLong0, double dLati0,
		double dLong1, double dLati1
	);
	
	// 番犬追加
	void AddWatchDog( void );
	void AddStopRecord( int iIndex, double dTime ){
		if( m_pLogSpeed ) SetSpeed( iIndex, 0 );
		if( m_pLogGx    ) SetGx(    iIndex, 0 );
		if( m_pLogGy    ) SetGy(    iIndex, 0 );
		SetTime( iIndex, dTime );
	}
	
	// key の存在確認
	
	VSD_LOG_t *GetElement( const char *szKey, BOOL bCreate = FALSE );
	
	// レコードコピー
	void CopyRecord( int iTo, int iFrom );
	
	// set / get 関数
	template<typename T>
	double Get( const char *szKey, T Index ){
		std::string strKey( szKey );
		if( m_Logs.find( strKey ) == m_Logs.end()){
			return 0;	// 要素なし
		}
		return m_Logs[ strKey ]->Get( Index );
	}
	
	void Set( const char *szKey, int iIndex, double dVal );
	
	double GetMin( const char *szKey ){
		VSD_LOG_t	*pLog = GetElement( szKey );
		return pLog ? pLog->GetMin() : 0;
	}
	
	double GetMax( const char *szKey ){
		VSD_LOG_t	*pLog = GetElement( szKey );
		return pLog ? pLog->GetMax() : 0;
	}
	
	#define DEF_LOG( name ) VSD_LOG_t	*m_pLog##name;
	#include "def_log.h"
	#define DEF_LOG( name ) double name( void          ){ return m_pLog##name->Get( m_dLogNum ); }
	#include "def_log.h"
	#define DEF_LOG( name ) double name( int    iIndex ){ return m_pLog##name->Get( iIndex ); }
	#include "def_log.h"
	#define DEF_LOG( name ) double name( double dIndex ){ return m_pLog##name->Get( dIndex ); }
	#include "def_log.h"
	#define DEF_LOG( name ) void Set##name( int iIndex, double dVal ){ \
		if( !m_pLog##name ) m_pLog##name = GetElement( #name, TRUE ); \
		m_pLog##name->Set( iIndex, dVal ); \
		if( m_iCnt <= iIndex ) m_iCnt = iIndex + 1; \
	}
	#include "def_log.h"
	#define DEF_LOG( name ) double Max##name( void ){ return m_pLog##name->GetMax(); }
	#include "def_log.h"
	#define DEF_LOG( name ) double Min##name( void ){ return m_pLog##name->GetMin(); }
	#include "def_log.h"
	
  private:
	int	m_iCnt;
	CVsdFilter	*m_pVsd;
};
