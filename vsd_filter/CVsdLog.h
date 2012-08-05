/*****************************************************************************
	
	VSD -- vehicle data logger system  Copyright(C) by DDS
	
	CVsdLog.h - CVsdLog class header
	
*****************************************************************************/

#ifndef _CVsdLog_h_
#define _CVsdLog_h_

#define MAX_VSD_LOG		(( int )( LOG_FREQ * 3600 * 24 ))

/*** macros *****************************************************************/

#define DeleteIfZero( v ) if(( v ) && !( v )->m_iCnt ){ delete v; v = NULL; }

#ifdef GPS_ONLY
	#define INVERT_G	(-1)
#else
	#define INVERT_G	1
#endif

/*** new type ***************************************************************/

static const float fNaN = sqrt( -1.0f );

class VSD_LOG_t {
  public:
	VSD_LOG_t(){
		fX = fY = fNaN;
	}
	~VSD_LOG_t(){}
	
	double Speed()		{ return fSpeed; }		void SetSpeed( double d )		{ fSpeed	= ( float )d; }
	double Tacho()		{ return fTacho; }		void SetTacho( double d )		{ fTacho	= ( float )d; }
	double Mileage()	{ return fMileage; }	void SetMileage( double d )		{ fMileage	= ( float )d; }
	double Gx()			{ return fGx; }			void SetGx( double d )			{ fGx		= ( float )d; }
	double Gy()			{ return fGy; }			void SetGy( double d )			{ fGy		= ( float )d; }
	double X()			{ return fX; }			void SetX( double d )			{ fX		= ( float )d; }
	double Y()			{ return fY; }			void SetY( double d )			{ fY		= ( float )d; }
	double X0()			{ return fX0; }			void SetX0( double d )			{ fX0		= ( float )d; }
	double Y0()			{ return fY0; }			void SetY0( double d )			{ fY0		= ( float )d; }

  private:
	float	fSpeed;
	float	fTacho;
	float	fMileage;
	float	fGx, fGy;
	float	fX, fX0;
	float	fY, fY0;
};

class GPS_LOG_t {
  public:
	GPS_LOG_t(){
		fSpeed = FLT_MAX;
	}
	~GPS_LOG_t(){}
	
	double X()			{ return fX; }			void SetX( double d )			{ fX		= ( float )d; }
	double Y()			{ return fY; }			void SetY( double d )			{ fY		= ( float )d; }
	double Speed()		{ return fSpeed; }		void SetSpeed( double d )		{ fSpeed	= ( float )d; }
	double Bearing()	{ return fBearing; }	void SetBearing( double d )		{ fBearing	= ( float )d; }
	double Gx()			{ return fGx; }			void SetGx( double d )			{ fGx		= ( float )d; }
	double Gy()			{ return fGy; }			void SetGy( double d )			{ fGy		= ( float )d; }
	double Time()		{ return fTime; }		void SetTime( double d )		{ fTime		= ( float )d; }
	
	BOOL IsValidSpeed(){ return fSpeed != FLT_MAX; }
	
  private:
	float	fX;
	float	fY;
	float	fSpeed;
	float	fBearing;
	float	fGx, fGy;
	float	fTime;
};

class CVsdLog {
	
  public:
	std::vector<VSD_LOG_t>	m_Log;
	int			m_iCnt;
	
	int			m_iLogNum;
	int			m_iMaxSpeed;
	
	double		m_dLogNum;
	
	double		m_dMapSize;
	double		m_dMapOffsX;
	double		m_dMapOffsY;
	double		m_dFreq;
	
	double		m_dMaxG;	// â¡ë¨ G
	double		m_dMinG;	// å∏ë¨ G
	
	double		m_dLogStartTime;	// ÉçÉOäJénéûä‘
	
	CVsdLog();
	~CVsdLog();
	UINT GPSLogUpConvert( std::vector<GPS_LOG_t>& GPSLog, UINT uCnt, BOOL bAllParam = FALSE );
	void RotateMap( double dAngle );
	
	BOOL IsDataExist( void ){
		return 0 <= m_iLogNum && m_iLogNum < m_iCnt - 1;
	}
	
	#define VsdLogGetData( p, n ) (\
		( 0 <= ( n ) && ( n ) < m_iCnt ) ? \
			( float )( \
				m_Log[ ( UINT )( n )     ].p() * ( 1 - (( n ) - ( UINT )( n ))) + \
				m_Log[ ( UINT )( n ) + 1 ].p() * (      ( n ) - ( UINT )( n )) \
			) : 0 \
		)
	
	double Speed	( void ){ return VsdLogGetData( Speed,		m_dLogNum ); }
	double Tacho	( void ){ return VsdLogGetData( Tacho,		m_dLogNum ); }
	double Mileage	( void ){ return VsdLogGetData( Mileage,	m_dLogNum ); }
	double Gx		( void ){ return VsdLogGetData( Gx,			m_dLogNum ); }
	double Gy		( void ){ return VsdLogGetData( Gy,			m_dLogNum ); }
	double X		( void ){ return VsdLogGetData( X,			m_dLogNum ); }
	double X0		( void ){ return VsdLogGetData( X0,			m_dLogNum ); }
	double Y		( void ){ return VsdLogGetData( Y,			m_dLogNum ); }
	double Y0		( void ){ return VsdLogGetData( Y0,			m_dLogNum ); }
	
	double Speed	( int iIndex ){ return m_Log[ iIndex ].Speed(); }
	double Tacho	( int iIndex ){ return m_Log[ iIndex ].Tacho(); }
	double Mileage	( int iIndex ){ return m_Log[ iIndex ].Mileage(); }
	double Gx		( int iIndex ){ return m_Log[ iIndex ].Gx(); }
	double Gy		( int iIndex ){ return m_Log[ iIndex ].Gy(); }
	double X		( int iIndex ){ return m_Log[ iIndex ].X(); }
	double X0		( int iIndex ){ return m_Log[ iIndex ].X0(); }
	double Y		( int iIndex ){ return m_Log[ iIndex ].Y(); }
	double Y0		( int iIndex ){ return m_Log[ iIndex ].Y0(); }
	
	double Speed	( double dIndex ){ return VsdLogGetData( Speed,		dIndex ); }
	double Tacho	( double dIndex ){ return VsdLogGetData( Tacho,		dIndex ); }
	double Mileage	( double dIndex ){ return VsdLogGetData( Mileage,	dIndex ); }
	double Gx		( double dIndex ){ return VsdLogGetData( Gx,		dIndex ); }
	double Gy		( double dIndex ){ return VsdLogGetData( Gy,		dIndex ); }
	double X		( double dIndex ){ return VsdLogGetData( X,			dIndex ); }
	double X0		( double dIndex ){ return VsdLogGetData( X0,		dIndex ); }
	double Y		( double dIndex ){ return VsdLogGetData( Y,			dIndex ); }
	double Y0		( double dIndex ){ return VsdLogGetData( Y0,		dIndex ); }
};
#endif
