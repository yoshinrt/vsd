//			enum name
//				|				トラックバーの初期値
//				|					|	トラックバーの下限値
//				|					|		|	トラックバーの上限値
//				|					|		|		|	トラックバーの名前
//				|					|		|		|		|
DEF_TRACKBAR( TRACK_VSt,			0,		0,		10000,	"vid先頭"	)
DEF_TRACKBAR( TRACK_VSt2,			0,		-200,	200,	""			)
DEF_TRACKBAR( TRACK_VEd,			0,		0,		10000,	"vid最後"	)
DEF_TRACKBAR( TRACK_VEd2,			0,		-200,	200,	""			)
#ifndef CIRCUIT_TOMO
DEF_TRACKBAR( TRACK_LSt,			0,		0,		10000,	"log先頭"	)
DEF_TRACKBAR( TRACK_LSt2,			0,		-200,	200,	""			)
DEF_TRACKBAR( TRACK_LEd,			0,		0,		10000,	"log最後"	)
DEF_TRACKBAR( TRACK_LEd2,			0,		-200,	200,	""			)
DEF_TRACKBAR( TRACK_LineTrace,		0,		0,		1000,	"Map秒"	)
DEF_TRACKBAR( TRACK_MapSize,		400,	0,		1000,	"Mapサイズ"	)
//DEF_TRACKBAR( TRACK_MapRot,			0,		0,		360,	"Map回転"	)
#else
DEF_TRACKBAR( TRACK_LSt,			0,		0,		10000,	"log先頭"	)
DEF_TRACKBAR( TRACK_LSt2,			0,		0,		1000,	"   (ms)"	)
DEF_TRACKBAR( TRACK_LEd,			0,		0,		10000,	"log最後"	)
DEF_TRACKBAR( TRACK_LEd2,			0,		0,		1000,	"   (ms)"	)
DEF_TRACKBAR( TRACK_GEAR1,			1315,	1,		9999,	"1速"		)
DEF_TRACKBAR( TRACK_GEAR2,			2030,	1,		9999,	"2速"		)
DEF_TRACKBAR( TRACK_GEAR3,			2810,	1,		9999,	"3速"		)
DEF_TRACKBAR( TRACK_GEAR4,			3569,	1,		9999,	"4速"		)
DEF_TRACKBAR( TRACK_GEAR5,			4544,	1,		9999,	"5速"		)
DEF_TRACKBAR( TRACK_GEAR6,			5741,	1,		9999,	"6速"		)
DEF_TRACKBAR( TRACK_PULSE_TACHO,	2000,	1,		9999,	"パルスTA"	)
DEF_TRACKBAR( TRACK_PULSE_SPEED,	2548,	1,		9999,	"パルスSP"	)
DEF_TRACKBAR( TRACK_TACHO,			8,		1,		  20,	"メータTA"	)
DEF_TRACKBAR( TRACK_SPEED,			180,	50,		 400,	"メータSP"	)
#endif

#undef DEF_TRACKBAR
