#ifndef DEF_LOG_T
	#define DEF_LOG_T( name, type )	DEF_LOG( name )
#endif

DEF_LOG_T( Time,		CLogInt )
DEF_LOG( Speed )
DEF_LOG_T( Tacho,		CLogUShort )
DEF_LOG_T( Distance,	CLogUInt1024 )
DEF_LOG_T( Direction,	CLogDirection )
DEF_LOG_T( Longitude,	CLogDoubleOffset )
DEF_LOG_T( Latitude,	CLogDoubleOffset)
DEF_LOG( X )
DEF_LOG( Y )
DEF_LOG_T( Gx,			CLogShort4096 )
DEF_LOG_T( Gy,			CLogShort4096 )
#ifdef USE_TURN_R
DEF_LOG( TurnR )
#endif

#undef DEF_LOG
#undef DEF_LOG_T
