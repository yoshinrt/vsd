#ifndef DEF_LOG_T
	#define DEF_LOG_T( name, type )	DEF_LOG( name )
#endif

DEF_LOG_T( Time,		CLogInt )
DEF_LOG( Speed )
DEF_LOG( Tacho )
DEF_LOG( Distance )
DEF_LOG_T( Direction,	CLogDirection )
DEF_LOG_T( Longitude,	CLogFloatOffset )
DEF_LOG_T( Latitude,	CLogFloatOffset )
DEF_LOG( X )
DEF_LOG( Y )
DEF_LOG( Gx )
DEF_LOG( Gy )
#ifdef USE_TURN_R
DEF_LOG( TurnR )
#endif

#undef DEF_LOG
#undef DEF_LOG_T
