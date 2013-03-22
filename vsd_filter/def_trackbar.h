#ifndef DEF_TRACKBAR_N
	#define DEF_TRACKBAR_N DEF_TRACKBAR
#endif

//			enum name
//				|				トラックバーの初期値
//				|					|	トラックバーの下限値
//				|					|		|	トラックバーの上限値
//				|					|		|		|	トラックバーの名前
//				|					|		|		|		|						config 名
//				|					|		|		|		|						|
DEF_TRACKBAR_N(	TRACK_LogOffset,	0,		-9999,	9999,	"ログ位置調整",		NULL	)
#ifndef PUBLIC_MODE
DEF_TRACKBAR(	PARAM_VSt,			0,		0,		9999,	"Video先頭",		"video_start"	)
DEF_TRACKBAR(	PARAM_VEd,			0,		0,		9999,	"Video最後",		"video_end"		)
DEF_TRACKBAR(	PARAM_LSt,			0,		0,		9999,	"ログ先頭",			"log_start"		)
DEF_TRACKBAR(	PARAM_LEd,			0,		0,		9999,	"ログ最後",			"log_end"		)
DEF_TRACKBAR(	PARAM_GSt,			0,		0,		9999,	"GPS先頭",			"gps_start"		)
DEF_TRACKBAR(	PARAM_GEd,			0,		0,		9999,	"GPS最後",			"gps_end"		)
#endif
DEF_TRACKBAR(	TRACK_LineTrace,	240,	0,		1000,	"走行軌跡長さ",		"map_length"	)
DEF_TRACKBAR(	TRACK_MapAngle,		0,		0,		3600,	"走行軌跡回転",		"map_angle"		)
DEF_TRACKBAR(	TRACK_SLineWidth,	200,	-1,		1000,	"計測地点幅",		"start_width"	)

#undef DEF_TRACKBAR
#undef DEF_TRACKBAR_N
