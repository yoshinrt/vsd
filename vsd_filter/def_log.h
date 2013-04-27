#ifndef DEF_LOG_T
	#define DEF_LOG_T( name, type )	DEF_LOG( name )
#endif

DEF_LOG_T( Time,		CLogFloat )
DEF_LOG_T( Speed,		CLogUShort128 )
DEF_LOG_T( Tacho,		CLogUShort )
DEF_LOG_T( Distance,	CLogUInt1024 )
DEF_LOG_T( Direction,	CLogDirection )
DEF_LOG_T( Longitude,	CLogFloatOffset )
DEF_LOG_T( Latitude,	CLogFloatOffset )
DEF_LOG( X )
DEF_LOG( Y )
DEF_LOG_T( Gx,			CLogShort4096 )
DEF_LOG_T( Gy,			CLogShort4096 )
DEF_LOG_T( Accel,		CLogAccel )
DEF_LOG_T( Brake,		CLogAccel )
#ifdef USE_TURN_R
DEF_LOG( TurnR )
#endif

#undef DEF_LOG
#undef DEF_LOG_T
