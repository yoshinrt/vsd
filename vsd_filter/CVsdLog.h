/*****************************************************************************
	
	VSD -- vehicle data logger system  Copyright(C) by DDS
	
	CVsdLog.h - CVsdLog class header
	$Id$
	
*****************************************************************************/

#ifndef _CVsdLog_h_
#define _CVsdLog_h_

typedef struct {
	float	fSpeed;
	float	fTacho;
#ifndef CIRCUIT_TOMO
	float	fMileage;
	float	fGx, fGy;
	float	fX, fX0;
	float	fY, fY0;
#endif
} VSD_LOG_t;

typedef struct {
	USHORT	uLap;
	int		iLogNum;
	float	fTime;
} LAP_t;

typedef struct {
	float	fX;
	float	fY;
	float	fVX;
	float	fVY;
	int		iLogNum;
} GPS_LOG_t;

class CVsdLog {
	
  public:
	CVsdLog();
	~CVsdLog();
	
	BOOL ConfigLoad( char *szFileName );
#ifndef AVS_PLUGIN
	BOOL ConfigSave( char *szFileName );
#endif
	BOOL ReadLog( char *szFileName );
#ifdef CIRCUIT_TOMO
	UINT ReadPTD( FILE *fp, UINT uOffs );
#else
	void RotateMap( void );
#endif
	
	// 手動ラップタイム計測モードかどうか
	BOOL IsHandLaptime( void ){
		#ifdef CIRCUIT_TOMO
			return TRUE;
		#else
			return m_iVsdLogNum == 0;
		#endif
	}
	
	VSD_LOG_t	*m_VsdLog;
	int			m_iVsdLogNum;
	LAP_t		*m_Lap;
	int			m_iLapNum;
	float		m_fBestTime;
	int			m_iBestLapLogNum;
	
	double		m_dMapSize;
	double		m_dMapOffsX;
	double		m_dMapOffsY;
	
	int			m_iLogStart;
	int			m_iLogStop;
	
	double		m_dVideoFPS;
	
	int			*m_piParamT;
	int			*m_piParamC;
	
	BOOL		m_bCalcLapTimeReq;
	
	static const char *m_szTrackbarName[];
	static const char *m_szCheckboxName[];
	
#ifndef AVS_PLUGIN
	void	*editp;
	FILTER	*filter;
#endif
	
  private:
};
#endif
