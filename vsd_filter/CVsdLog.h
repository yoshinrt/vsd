/*****************************************************************************
	
	VSD -- vehicle data logger system  Copyright(C) by DDS
	
	CVsdLog.h - CVsdLog class header
	$Id$
	
*****************************************************************************/

#ifndef _CVsdLog_h_
#define _CVsdLog_h_

#define MAX_VSD_LOG		(( int )( LOG_FREQ * 3600 * 2 ))

/*** macros *****************************************************************/

#define DeleteIfZero( v ) if(( v ) && !( v )->m_iCnt ){ delete v; v = NULL; }

#define ToRAD		( M_PI / 180 )

#ifdef GPS_ONLY
	#define INVERT_G	(-1)
#else
	#define INVERT_G	1
#endif

/*** new type ***************************************************************/

typedef struct {
	float	fSpeed;
	float	fTacho;
	float	fMileage;
	float	fGx, fGy;
	float	fX, fX0;
	float	fY, fY0;
} VSD_LOG_t;

typedef struct {
	float	fX;
	float	fY;
	float	fSpeed;
	float	fBearing;
	float	fGx, fGy;
	float	fTime;
} GPS_LOG_t;

class CVsdLog {
	
  public:
	VSD_LOG_t	*m_Log;
	int			m_iCnt;
	
	int			m_iLogNum;
	double		m_dLogNum;
	
	double		m_dMapSize;
	double		m_dMapOffsX;
	double		m_dMapOffsY;
	double		m_dFreq;
	
	double		m_dMaxG;	// ‰Á‘¬ G
	double		m_dMinG;	// Œ¸‘¬ G
	
	int			m_iMaxSpeed;
	
	CVsdLog();
	~CVsdLog();
	UINT GPSLogUpConvert( GPS_LOG_t *GPSLog, UINT uCnt, BOOL bAllParam = FALSE );
	void RotateMap( double dAngle );
	
	BOOL IsDataExist( void ){
		return 0 <= m_iLogNum && m_iLogNum < m_iCnt - 1;
	}
	
	#define VsdLogGetData( p, n ) (\
		( 0 <= ( n ) && ( n ) < m_iCnt ) ? \
			( float )( \
				m_Log[ ( UINT )( n )     ].p * ( 1 - (( n ) - ( UINT )( n ))) + \
				m_Log[ ( UINT )( n ) + 1 ].p * (      ( n ) - ( UINT )( n )) \
			) : 0 \
		)
	
	float Speed		( void ){ return VsdLogGetData( fSpeed,		m_dLogNum ); }
	float Tacho		( void ){ return VsdLogGetData( fTacho,		m_dLogNum ); }
	float Mileage	( void ){ return VsdLogGetData( fMileage,	m_dLogNum ); }
	float Gx		( void ){ return VsdLogGetData( fGx,		m_dLogNum ); }
	float Gy		( void ){ return VsdLogGetData( fGy,		m_dLogNum ); }
	float X			( void ){ return VsdLogGetData( fX,			m_dLogNum ); }
	float X0		( void ){ return VsdLogGetData( fX0,		m_dLogNum ); }
	float Y			( void ){ return VsdLogGetData( fY,			m_dLogNum ); }
	float Y0		( void ){ return VsdLogGetData( fY0,		m_dLogNum ); }
	
	float Speed		( double dIndex ){ return VsdLogGetData( fSpeed,	dIndex ); }
	float Tacho		( double dIndex ){ return VsdLogGetData( fTacho,	dIndex ); }
	float Mileage	( double dIndex ){ return VsdLogGetData( fMileage,	dIndex ); }
	float Gx		( double dIndex ){ return VsdLogGetData( fGx,		dIndex ); }
	float Gy		( double dIndex ){ return VsdLogGetData( fGy,		dIndex ); }
	float X			( double dIndex ){ return VsdLogGetData( fX,		dIndex ); }
	float X0		( double dIndex ){ return VsdLogGetData( fX0,		dIndex ); }
	float Y			( double dIndex ){ return VsdLogGetData( fY,		dIndex ); }
	float Y0		( double dIndex ){ return VsdLogGetData( fY0,		dIndex ); }
};
#endif
