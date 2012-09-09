/*****************************************************************************
	
	VSD -- vehicle data logger system  Copyright(C) by DDS
	
	CVsdLog.h - CVsdLog class header
	
*****************************************************************************/

#pragma once

/*** macros *****************************************************************/

#define TIME_NONE	(( int )0x80000000 )
#define WATCHDOG_TIME	1E+12

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
		memset( this, 0, sizeof( VSD_LOG_t ));
	}
	~VSD_LOG_t(){}
	
	void StopLog( double dTime ){
		SetSpeed( 0 );
		SetGx( 0 );
		SetGy( 0 );
		SetTime( dTime );
	}
	
	double Speed(){ return uSpeed / 100.0; }void SetSpeed	( double d ){ uSpeed	= ( USHORT )( d * 100 ); }
	double Tacho()	{ return uTacho; }		void SetTacho	( double d ){ uTacho	= ( USHORT )d; }
	double Distance(){ return fDistance; }	void SetDistance( double d ){ fDistance	= ( float )d; }
	double X()		{ return fX; }			void SetX		( double d ){ fX		= ( float )d; }
	double Y()		{ return fY; }			void SetY		( double d ){ fY		= ( float )d; }
	double X0()		{ return fX0; }			void SetX0		( double d ){ fX0		= ( float )d; }
	double Y0()		{ return fY0; }			void SetY0		( double d ){ fY0		= ( float )d; }
	double Gx()		{ return iGx / 100.0; }	void SetGx		( double d ){ iGx		= ( int )( d * 100 ); }
	double Gy()		{ return iGy / 100.0; }	void SetGy		( double d ){ iGy		= ( int )( d * 100 ); }
	double Time()	{ return fTime; }		void SetTime	( double d ){ fTime		= ( float )d; }
	
  private:
	float	fTime;
	float	fX, fX0;
	float	fY, fY0;
	float	fDistance;
	USHORT	uSpeed;
	USHORT	uTacho;
	short	iGx, iGy;
};

class CVsdLog {
	
  public:
	std::vector<VSD_LOG_t>	m_Log;
	int GetCnt( void ){ return m_Log.size(); }
	
	int			m_iLogNum;
	int			m_iMaxSpeed;
	int			m_iMaxTacho;
	
	double		m_dLogNum;
	
	double		m_dMapSizeX;
	double		m_dMapSizeY;
	double		m_dMapOffsX;
	double		m_dMapOffsY;
	double		m_dFreq;
	
	double		m_dMaxGx;	// 左右 G
	double		m_dMaxGy;	// 加速 G
	double		m_dMinGy;	// 減速 G
	
	double		m_dLogStartTime;	// ログ開始時間
	
	// 緯度経度→メートル 変換用
	double m_dLong0;
	double m_dLati0;
	double m_dLong2Meter;
	double m_dLati2Meter;
	
	// VSD ログ位置自動認識用
	int			m_iLogStart;
	int			m_iLogStop;
	
	UINT		m_uSameCnt;
	
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
	
	void PushRecord( VSD_LOG_t& VsdLogTmp, double dLong, double dLati );
	int ReadGPSLog( const char *szFileName );
	int ReadLog( const char *szFileName, CLapLog *&pLapLog );
	double GPSLogGetLength(
		double dLong0, double dLati0,
		double dLong1, double dLati1
	);
	
	// 終端側の番犬追加
	void AddWatchDog( void ){
		m_Log.push_back( m_Log[ GetCnt() - 1 ] );
		m_Log.push_back( m_Log[ GetCnt() - 1 ] );
		m_Log[ GetCnt() - 2 ].StopLog( Time( GetCnt() - 2 ) + 0.5 );
		m_Log[ GetCnt() - 1 ].StopLog( WATCHDOG_TIME );
	}
	
	#define VsdLogGetData( p, n ) \
		( float )( \
			m_Log[ ( UINT )( n )     ].p() * ( 1 - (( n ) - ( UINT )( n ))) + \
			m_Log[ ( UINT )( n ) + 1 ].p() * (      ( n ) - ( UINT )( n )) \
		) \
	
	double Speed	( void ){ return VsdLogGetData( Speed,		m_dLogNum ); }
	double Tacho	( void ){ return VsdLogGetData( Tacho,		m_dLogNum ); }
	double Distance	( void ){ return VsdLogGetData( Distance,	m_dLogNum ); }
	double Gx		( void ){ return VsdLogGetData( Gx,			m_dLogNum ); }
	double Gy		( void ){ return VsdLogGetData( Gy,			m_dLogNum ); }
	double X		( void ){ return VsdLogGetData( X,			m_dLogNum ); }
	double X0		( void ){ return VsdLogGetData( X0,			m_dLogNum ); }
	double Y		( void ){ return VsdLogGetData( Y,			m_dLogNum ); }
	double Y0		( void ){ return VsdLogGetData( Y0,			m_dLogNum ); }
	double Time		( void ){ return VsdLogGetData( Time,		m_dLogNum ); }
	
	double Speed	( int iIndex ){ return m_Log[ iIndex ].Speed(); }
	double Tacho	( int iIndex ){ return m_Log[ iIndex ].Tacho(); }
	double Distance	( int iIndex ){ return m_Log[ iIndex ].Distance(); }
	double Gx		( int iIndex ){ return m_Log[ iIndex ].Gx(); }
	double Gy		( int iIndex ){ return m_Log[ iIndex ].Gy(); }
	double X		( int iIndex ){ return m_Log[ iIndex ].X(); }
	double X0		( int iIndex ){ return m_Log[ iIndex ].X0(); }
	double Y		( int iIndex ){ return m_Log[ iIndex ].Y(); }
	double Y0		( int iIndex ){ return m_Log[ iIndex ].Y0(); }
	double Time		( int iIndex ){ return m_Log[ iIndex ].Time(); }
	
	double Speed	( double dIndex ){ return VsdLogGetData( Speed,		dIndex ); }
	double Tacho	( double dIndex ){ return VsdLogGetData( Tacho,		dIndex ); }
	double Distance	( double dIndex ){ return VsdLogGetData( Distance,	dIndex ); }
	double Gx		( double dIndex ){ return VsdLogGetData( Gx,		dIndex ); }
	double Gy		( double dIndex ){ return VsdLogGetData( Gy,		dIndex ); }
	double X		( double dIndex ){ return VsdLogGetData( X,			dIndex ); }
	double X0		( double dIndex ){ return VsdLogGetData( X0,		dIndex ); }
	double Y		( double dIndex ){ return VsdLogGetData( Y,			dIndex ); }
	double Y0		( double dIndex ){ return VsdLogGetData( Y0,		dIndex ); }
	double Time		( double dIndex ){ return VsdLogGetData( Time,		dIndex ); }
};
