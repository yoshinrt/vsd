/*****************************************************************************
	
	VSD -- vehicle data logger system  Copyright(C) by DDS
	
	CVsdLog.h - CVsdLog class header
	$Id: CVsdFilter.h 218 2009-10-29 15:22:40Z  $
	
*****************************************************************************/

#ifndef _CVsdLog_h_
#define _CVsdLog_h_

#define MAX_VSD_LOG		(( int )( LOG_FREQ * 3600 * 2 ))

/*** macros *****************************************************************/

#define DeleteIfZero( v ) if(( v ) && !( v )->m_iCnt ){ delete v; v = NULL; }

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
	float	fVX;
	float	fVY;
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
	
	CVsdLog();
	~CVsdLog();
	UINT GPSLogUpConvert( GPS_LOG_t *GPSLog, UINT uCnt );
	void RotateMap( double dAngle );
	
	#define VsdLogGetData( p ) (( float )( \
		m_Log[ ( UINT )m_dLogNum     ].p * ( 1 - ( m_dLogNum - ( UINT )m_dLogNum )) + \
		m_Log[ ( UINT )m_dLogNum + 1 ].p * (       m_dLogNum - ( UINT )m_dLogNum )))
	
	float Speed		( void ){ return VsdLogGetData( fSpeed	); }
	float Tacho		( void ){ return VsdLogGetData( fTacho	); }
	float Mileage	( void ){ return VsdLogGetData( fMileage); }
	float Gx		( void ){ return VsdLogGetData( fGx		); }
	float Gy		( void ){ return VsdLogGetData( fGy		); }
	float X			( void ){ return VsdLogGetData( fX		); }
	float X0		( void ){ return VsdLogGetData( fX0		); }
	float Y			( void ){ return VsdLogGetData( fY		); }
	float Y0		( void ){ return VsdLogGetData( fY0		); }
};
#endif
