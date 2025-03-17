/*****************************************************************************
	
	VSD -- vehicle data logger system  Copyright(C) by DDS
	
	CVsdLog.h - CVsdLog class header
	
*****************************************************************************/

#pragma once
#include "CV8If.h"

/*** macros *****************************************************************/

#define TIME_NONE		(( int )INT_MIN )
#define WATCHDOG_TIME	( 0x70000000 )

#define WATCHDOG_REC_NUM	2		// ログ先頭の番犬分のレコード数
#define TIME_STOP			3000	// 停車とみなす Log 時間間隔
#define TIME_STOP_MARGIN	300		// 停車とみなす Log に付加する Log の時間差分
#define KPH_STOP			4		// TIME_STOP を満たした時，停車とみなす速度

/*** Lap Time ***************************************************************/

enum {
	LAPMODE_HAND,		// 手動計測モード
	LAPMODE_GPS,		// GPS 自動計測モード
	LAPMODE_MAGNET,		// 磁気センサー自動計測モード
	LAPMODE_CHART,		// ラップチャート
};
enum {
	LAPSRC_VIDEO,		// 時計は Video フレーム
	LAPSRC_VSD,			// 時計は VSD ログ時計
	LAPSRC_GPS,			// 時計は GPS ログ時計
};

typedef struct {
	UINT	uLap;
	float	fLogNum;
	int		iTime;
} LAP_t;

class CVsdLog;
class CVsdFilter;
class CLapLog {
  public:
	CLapLog(){
		// 初期化
		m_iLapNum		= 0;
		m_iBestTime		= TIME_NONE;
		m_iBestLap		= 0;
		m_iLapMode		= LAPMODE_HAND;
		m_iLapSrc		= LAPSRC_VIDEO;
		m_iLapIdx		= -1;
		m_iBestLogNumRunning	= 0;
	}
	
	void PushLap( LAP_t &Lap ){
		m_Lap.push_back( Lap );
		
		if(
			m_iLapNum && Lap.iTime && (
				m_iBestTime == TIME_NONE || Lap.iTime < m_iBestTime
			)
		){
			m_iBestTime	= Lap.iTime;
			m_iBestLap	= m_iLapNum - 1;
		}
		
		++m_iLapNum;
	}
	
	std::vector<LAP_t>	m_Lap;
	int	m_iLapMode;
	int	m_iLapSrc;
	int	m_iLapNum;
	int	m_iBestTime;
	int	m_iBestLap;
	int	m_iLapIdx;
	int	m_iCurTime;		// 現在ラップ経過時間
	int	m_iDiffTime;
	int m_iBestLogNumRunning;
};

class CLapLogAll : public CLapLog {
  public:
	CLapLogAll(){
		m_iStartFrame	= 0;
		m_iEndFrame		= INT_MAX;
		m_iCamCarIdx	= 0;
		m_iPrevFrame	= INT_MAX;
		m_iSearchStartIdx	= -1;
	}
	
	std::vector<std::wstring>		m_strName;
	std::vector<std::vector<int> >	m_LapTable;
	std::vector<int>				m_iAllLapIdx;
	std::vector<int>				m_iAllGapInfo;
	std::vector<int>				m_iPositionInfo;
	std::vector<int>				m_iLapLeader;
	
	int GetLapTime( int iCar, int iLap ){
		if( iLap >= ( int )m_LapTable[ iCar ].size()) --iLap;
		return ( iLap < 1 ) ? TIME_NONE : ( m_LapTable[ iCar ][ iLap ] - m_LapTable[ iCar ][ iLap - 1 ] );
	}
	int GetLapTime( int iLap ){
		return GetLapTime( m_iCamCarIdx, iLap );
	}
	
	int GetLapFrame( int iCar, int iLap ){
		return m_iStartFrame +
			( int )(
				( double )( m_iEndFrame - m_iStartFrame ) *
				( m_LapTable[ iCar ][ iLap ]            - CamCarLap()[ 0 ] ) /
				( CamCarLap()[ CamCarLap().size() - 1 ] - CamCarLap()[ 0 ] )
			);
	}
	int GetLapFrame( int iLap ){
		return GetLapFrame( m_iCamCarIdx, iLap );
	}
	
	int GetGap( int iCar ){
		int iLap = m_iAllLapIdx[ iCar ];
		return m_LapTable[ iCar ][ iLap ] - m_LapTable[ m_iLapLeader[ iLap ]][ iLap ];
	}
	
	std::vector<int>& CamCarLap( void ){ return m_LapTable[ m_iCamCarIdx ]; }
	int LapChartRead( const char *szFileName, CVsdFilter *pVsd );
	void MakeCamLapData( int iStartFrame, int iEndFrame );
	void CalcLapInfo( int iFrameCnt, double dFPS );
	
	int	m_iStartFrame;
	int	m_iEndFrame;
	int	m_iCamCarIdx;
	int	m_iPrevFrame;
	int m_iSearchStartIdx;
};

/*** ログ 1項目 *************************************************************/

class CLog {
	
  public:
	// 値取得
	virtual double GetRaw( int    iIndex ) = 0;
	virtual double Get( double dIndex ) = 0;
	
	virtual double GetMin() = 0;
	virtual double GetMax() = 0;
	virtual void SetMaxMin( double dMaxVal, double dMinVal ) = 0;
	
	// 値設定
	virtual void Set( int iIndex, double dVal ) = 0;
	virtual void SetRaw( int iIndex, double dVal ) = 0;
	virtual void InitMinMax( void ) = 0;
	virtual int GetCnt( void ) = 0;
	virtual void Resize( int iCnt, double dVal ) = 0;
};

template <class T, int Scale>
class CLogVariant : public CLog {
	
  public:
	CLogVariant(){
		SetMax( m_Min );
		SetMin( m_Max );
	}
	
	// 値取得
	double GetRaw( int    iIndex ){ return UnScaled( m_Log[ iIndex ] ); }
	double Get( double dIndex ){
		double alfa = dIndex - ( UINT )dIndex;
		return
			GetRaw(( int )dIndex ) +
			( GetRaw(( int )dIndex + 1 ) - GetRaw(( int )dIndex )) * alfa;
	}
	
	double GetMin(){ return UnScaled( m_Min ); }
	double GetMax(){ return UnScaled( m_Max ); }
	virtual void SetMinRaw( double d ){ m_Min = Scaled( d ); }
	virtual void SetMaxRaw( double d ){ m_Max = Scaled( d ); }
	
	void SetMaxMin( double dMaxVal, double dMinVal ){
		if( GetMin() > dMinVal ) SetMinRaw( dMinVal );
		if( GetMax() < dMaxVal ) SetMaxRaw( dMaxVal );
	}
	
	// 値設定
	void Set( int iIndex, double dVal ){
		SetMaxMin( dVal, dVal );
		SetRaw( iIndex, dVal );
	}
	
	void SetRaw( int iIndex, double dVal ){
		// ★無い値を線形補間する必要あり
		if( GetCnt() > iIndex ) m_Log[ iIndex ] = Scaled( dVal );
		else while( GetCnt() <= iIndex ) m_Log.push_back( Scaled( dVal ));
	}
	
	int GetCnt( void ){ return m_Log.size(); }
	
	void InitMinMax( void ){
		SetMax( m_Min );
		SetMin( m_Max );
	}
	
	void Resize( int iCnt, double dVal ){
		m_Log.resize( iCnt, Scaled( dVal ));
	}
	
  protected:
	std::vector<T>	m_Log;
	T	m_Min;
	T	m_Max;
	
	// 最大・最小設定ヘルパ
	void SetMax( UCHAR	&v ){ v = UCHAR_MAX; }	void SetMin( UCHAR	&v ){ v = 0; }
	void SetMax( USHORT	&v ){ v = USHRT_MAX; }	void SetMin( USHORT	&v ){ v = 0; }
	void SetMax( short	&v ){ v = SHRT_MAX; }	void SetMin( short	&v ){ v = SHRT_MIN; }
	void SetMax( UINT	&v ){ v = UINT_MAX; }	void SetMin( UINT	&v ){ v = 0; }
	void SetMax( int	&v ){ v = INT_MAX; }	void SetMin( int	&v ){ v = INT_MIN; }
	void SetMax( float	&v ){ v = FLT_MAX; }	void SetMin( float	&v ){ v = -FLT_MAX; }
	void SetMax( double &v ){ v = DBL_MAX; }	void SetMin( double &v ){ v = -DBL_MAX; }

	// 内部形式変換
	T Scaled( double d ){ return ( T )( d * Scale ); }
	double UnScaled( T v ){ return ( double )v / Scale; }
};

typedef CLogVariant<double, 1>		CLogDouble;
typedef CLogVariant<float,	1>		CLogFloat;
typedef CLogVariant<short,	4096>	CLogShort4096;
typedef CLogVariant<USHORT,	64>		CLogUShort64;
typedef CLogVariant<USHORT,	128>	CLogUShort128;
typedef CLogVariant<USHORT,	1>		CLogUShort;
typedef CLogVariant<UINT,	1024>	CLogUInt1024;

class CLogInt : public CLogVariant<int, 1> {
  public:
	int GetIntRaw( int iIndex ){ return m_Log[ iIndex ]; }
	int GetInt( double dIndex ){
		double alfa = dIndex - ( UINT )dIndex;
		return
			GetIntRaw(( int )dIndex ) +
			( int )(( GetIntRaw(( int )dIndex + 1 ) - GetIntRaw(( int )dIndex )) * alfa );
	}
};

class CLogDoubleOffset : public CLogDouble {
  public:
	CLogDoubleOffset(){
		m_dBaseVal	= 0;
	}
	
	// 値取得
	double GetRaw( int    iIndex ){ return CLogDouble::GetRaw( iIndex ) + m_dBaseVal; }
	
	double GetDiff( int    iIndex ){ return CLogDouble::GetRaw( iIndex ); }
	double GetDiff( double dIndex ){
		double alfa = dIndex - ( UINT )dIndex;
		return
			GetDiff(( int )dIndex ) +
			( GetDiff(( int )dIndex + 1 ) - GetDiff(( int )dIndex )) * alfa;
	}
	
	double GetMin(){ return CLogDouble::GetMin() + m_dBaseVal; }
	double GetMax(){ return CLogDouble::GetMax() + m_dBaseVal; }
	void SetMinRaw( double d ){ CLogDouble::SetMinRaw( d - m_dBaseVal ); }
	void SetMaxRaw( double d ){ CLogDouble::SetMaxRaw( d - m_dBaseVal ); }
	
	void SetRaw( int iIndex, double dVal ){
		CLogDouble::SetRaw( iIndex, dVal - m_dBaseVal );
	}
	
	void SetBaseVal( double dVal ){ m_dBaseVal = dVal; }
	
  protected:
	double	m_dBaseVal;
};

class CLogDirection : public CLogUShort128 {
  public:
	double Get( double dIndex ){
		double alfa = dIndex - ( int )dIndex;
		double a = GetRaw(( int )dIndex );
		double b = GetRaw(( int )dIndex + 1 );
		
		if     ( a - b >= 180 ) a -= 360;
		else if( a - b < -180 ) b -= 360;
		
		double dResult = a + ( b - a )* alfa;
		if( dResult < 0 ) dResult += 360;
		
		return dResult;
	}
};

/*** 1個のログセット ********************************************************/

class CVsdFilter;
class CVsdLog : public CV8If {
	
  public:
	int GetCnt( void ){ return m_iCnt; }
	
	/////////////////////////
	
	int		m_iLogNum;
	double	m_dLogNum;
	
	double	m_dFreq;
	INT64	m_iLogStartTime;	// ログ開始時間
	
	// VSD ログ位置自動認識用
	int		m_iCalibStart;
	int		m_iCalibStop;
	
	// ログの map
	std::map<std::string, CLog *> m_Logs;
	
	// コンストラクタ・デストラクタ
	CVsdLog( CVsdFilter *pVsd );
	
	UINT GPSLogRescan();
	void RotateMap( double dAngle );
	double GetIndex( double dFromVal, int *piFrom, int *piLog, int iPrevIdx = -1 );
	double GetIndex( int iTime, int iPrevIdx = -1 );
	
	#ifdef DEBUG
		void Dump( char *szFileName );
	#endif
	
	BOOL IsDataExist( int iLogNum ){
		return 0 <= iLogNum && iLogNum < GetCnt();
	}
	
	int ReadLog( const char *szFileName, const char *szReaderFunc, CLapLog *&pLapLog );
	
	static double GPSLogGetLength(
		double dLong0, double dLati0,
		double dLong1, double dLati1
	);
	
	// 番犬追加
	void AddWatchDog( void );
	void AddStopRecord( int iIndex, int iTime ){
		if( m_pLogSpeed ) SetSpeed( iIndex, 0 );
		if( m_pLogGx    ) SetGx(    iIndex, 0 );
		if( m_pLogGy    ) SetGy(    iIndex, 0 );
		SetTime( iIndex, iTime );
	}
	
	// key の存在確認
	
	CLog *GetElement( const char *szKey, BOOL bCreate = FALSE );
	
	// レコードコピー
	void CopyRecord( int iTo, int iFrom );
	
	// CLog 取得
	CLog *GetLog( const char *szKey ){
		std::string strKey( szKey );
		if( m_Logs.find( strKey ) == m_Logs.end()){
			return NULL;
		}
		return m_Logs[ strKey ];
	}
	
	// set / get 関数
	double Get( const char *szKey, double dIndex ){
		std::string strKey( szKey );
		if( m_Logs.find( strKey ) == m_Logs.end()){
			return NaN;	// 要素なし
		}
		return m_Logs[ strKey ]->Get( dIndex );
	}
	
	void Set( const char *szKey, int iIndex, double dVal );
	
	double GetMin( const char *szKey ){
		CLog	*pLog = GetElement( szKey );
		return pLog ? pLog->GetMin() : NaN;
	}
	
	double GetMax( const char *szKey ){
		CLog	*pLog = GetElement( szKey );
		return pLog ? pLog->GetMax() : NaN;
	}
	
	#define DEF_LOG( name )	DEF_LOG_T( name, CLogDouble )
	#define DEF_LOG_T( name, type )	type	*m_pLog##name;
	#include "def_log.h"
	
	#define DEF_LOG( name ) double name( void          ){ return m_pLog##name->Get( m_dLogNum ); }
	#include "def_log.h"
	#define DEF_LOG( name ) double name( int    iIndex ){ return m_pLog##name->GetRaw( iIndex ); }
	#include "def_log.h"
	#define DEF_LOG( name ) double name( double dIndex ){ return m_pLog##name->Get( dIndex ); }
	#include "def_log.h"
	
	#define DEF_LOG( name )	void Set##name( int iIndex, double dVal ){ \
		m_pLog##name->Set( iIndex, dVal ); \
		if( m_iCnt <= iIndex ) m_iCnt = iIndex + 1; \
	}
	#include "def_log.h"
	
	#define DEF_LOG( name )	void SetRaw##name( int iIndex, double dVal ){ \
		m_pLog##name->SetRaw( iIndex, dVal ); \
		if( m_iCnt <= iIndex ) m_iCnt = iIndex + 1; \
	}
	#include "def_log.h"
	
	#define DEF_LOG( name ) double Max##name( void ){ return m_pLog##name->GetMax(); }
	#include "def_log.h"
	#define DEF_LOG( name ) double Min##name( void ){ return m_pLog##name->GetMin(); }
	#include "def_log.h"
	#define DEF_LOG( name ) void SetMaxMin##name( double dMax, double dMin ){ m_pLog##name->SetMaxMin( dMax, dMin ); }
	#include "def_log.h"
	
	double DateTime( void ){
		return ( m_pLogTime->Get( m_dLogNum ) + m_iLogStartTime );
	}
	
	int GetTime( int    iIndex ){ return m_pLogTime->GetIntRaw( iIndex ); }
	int GetTime( double dIndex ){ return m_pLogTime->GetInt( dIndex ); }
	int GetTime( void          ){ return m_pLogTime->GetInt( m_dLogNum ); }
	
	double X0( int    iIndex ){ return m_pLogLongitude->GetDiff( iIndex ) * m_dLong2Meter; }
	double X0( double dIndex ){ return m_pLogLongitude->GetDiff( dIndex ) * m_dLong2Meter; }
	double Y0( int    iIndex ){ return m_pLogLatitude->GetDiff( iIndex ) * m_dLati2Meter; }
	double Y0( double dIndex ){ return m_pLogLatitude->GetDiff( dIndex ) * m_dLati2Meter; }
	
  private:
	double m_dLong2Meter;
	double m_dLati2Meter;
	
	int	m_iCnt;
	CVsdFilter	*m_pVsd;
};
