#ifndef DEF_TRACKBAR_N
	#define DEF_TRACKBAR_N DEF_TRACKBAR
#endif

//			enum name
//				|				トラックバーの初期値
//				|					|	トラックバーの下限値
//				|					|		|	トラックバーの上限値
//				|					|		|		|	トラックバーの名前
//				|					|		|		|		|				config 名
//				|					|		|		|		|				|
DEF_TRACKBAR_N(	TRACK_LogOffset,	0,		-9999,	9999,	"GPS調整",		NULL	)
#ifndef GPS_ONLY
DEF_TRACKBAR(	PARAM_VSt,			0,		0,		9999,	"Vid先頭",		"video_start"	)
DEF_TRACKBAR(	PARAM_VEd,			0,		0,		9999,	"Vid最後",		"video_end"		)
DEF_TRACKBAR(	PARAM_LSt,			0,		0,		9999,	"log先頭",		"log_start"		)
DEF_TRACKBAR(	PARAM_LEd,			0,		0,		9999,	"log最後",		"log_end"		)
DEF_TRACKBAR(	PARAM_GSt,			0,		0,		9999,	"GPS先頭",		"gps_start"		)
DEF_TRACKBAR(	PARAM_GEd,			0,		0,		9999,	"GPS最後",		"gps_end"		)
#endif
DEF_TRACKBAR(	TRACK_SPEED,		180,	50,		400,	"スピード",		"speed_meter"	)
DEF_TRACKBAR(	TRACK_LineTrace,	240,	0,		1000,	"Map秒",		"map_length"	)
DEF_TRACKBAR(	TRACK_MapSize,		400,	0,		1000,	"Mapサイズ",	"map_size"		)
DEF_TRACKBAR(	TRACK_MapAngle,		0,		0,		3600,	"Map回転",		"map_angle"		)
DEF_TRACKBAR(	TRACK_G_Len,		30,		-1,		300,	"G長さ",		"g_length"		)
DEF_TRACKBAR(	TRACK_SLineWidth,	200,	0,		1000,	"Start幅",		"start_width"	)
#ifdef GPS_ONLY
DEF_TRACKBAR(	TRACK_Aspect,		1000,	500,	2000,	"Aspect比",		"aspect_ratio"	)
#endif

#undef DEF_TRACKBAR
#undef DEF_TRACKBAR_N
