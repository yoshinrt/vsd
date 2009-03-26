#ifndef DEF_TRACKBAR_N
	#define DEF_TRACKBAR_N DEF_TRACKBAR
#endif

//			enum name
//				|				トラックバーの初期値
//				|					|	トラックバーの下限値
//				|					|		|	トラックバーの上限値
//				|					|		|		|	トラックバーの名前
//				|					|		|		|		|			config 名
//				|					|		|		|		|			|
DEF_TRACKBAR(	TRACK_VSt,			0,		0,		10000,	"vid先頭",	"video_start"	)
DEF_TRACKBAR_N(	TRACK_VSt2,			0,		-200,	200,	"",			NULL			)
DEF_TRACKBAR(	TRACK_VEd,			0,		0,		10000,	"vid最後",	"video_end"		)
DEF_TRACKBAR_N(	TRACK_VEd2,			0,		-200,	200,	"",			NULL			)
#ifndef CIRCUIT_TOMO
DEF_TRACKBAR(	TRACK_LSt,			0,		0,		10000,	"log先頭",	"log_start"		)
DEF_TRACKBAR_N(	TRACK_LSt2,			0,		-200,	200,	"",			NULL			)
DEF_TRACKBAR(	TRACK_LEd,			0,		0,		10000,	"log最後",	"log_end"		)
DEF_TRACKBAR_N(	TRACK_LEd2,			0,		-200,	200,	"",			NULL			)
DEF_TRACKBAR(	TRACK_LineTrace,	0,		0,		1000,	"Map秒",	"map_length"	)
DEF_TRACKBAR(	TRACK_MapSize,		400,	0,		1000,	"Mapサイズ","map_size"		)
DEF_TRACKBAR(	TRACK_MapAngle,		0,		0,		3600,	"Map回転",	"map_angle"		)
#else
DEF_TRACKBAR(	TRACK_LSt,			0,		0,		10000,	"log先頭",	"log_start"		)
DEF_TRACKBAR_N(	TRACK_LSt2,			0,		0,		1000,	"   (ms)",	NULL			)
DEF_TRACKBAR(	TRACK_LEd,			0,		0,		10000,	"log最後",	"log_end"		)
DEF_TRACKBAR_N(	TRACK_LEd2,			0,		0,		1000,	"   (ms)",	NULL			)
DEF_TRACKBAR(	TRACK_GEAR1,		1315,	1,		9999,	"1速",		"gear1"			)
DEF_TRACKBAR(	TRACK_GEAR2,		2030,	1,		9999,	"2速",		"gear2"			)
DEF_TRACKBAR(	TRACK_GEAR3,		2810,	1,		9999,	"3速",		"gear3"			)
DEF_TRACKBAR(	TRACK_GEAR4,		3569,	1,		9999,	"4速",		"gear4"			)
DEF_TRACKBAR(	TRACK_GEAR5,		4544,	1,		9999,	"5速",		"gear5"			)
DEF_TRACKBAR(	TRACK_GEAR6,		5741,	1,		9999,	"6速",		"gear6"			)
DEF_TRACKBAR(	TRACK_PULSE_TACHO,	2000,	1,		9999,	"パルスTA",	"tacho_pulse"	)
DEF_TRACKBAR(	TRACK_PULSE_SPEED,	2548,	1,		9999,	"パルスSP",	"speed_pulse"	)
DEF_TRACKBAR(	TRACK_TACHO,		8,		1,		  20,	"メータTA",	"tacho_meter"	)
DEF_TRACKBAR(	TRACK_SPEED,		180,	50,		 400,	"メータSP",	"speed_meter"	)
#endif

#undef DEF_TRACKBAR
#undef DEF_TRACKBAR_N
