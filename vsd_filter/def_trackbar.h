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
DEF_TRACKBAR(	TRACK_VSt,			0,		0,		9999,	"vid先頭",	"video_start"	)
DEF_TRACKBAR_N(	TRACK_VSt2,			0,		-200,	200,	"",			NULL			)
DEF_TRACKBAR(	TRACK_VEd,			0,		0,		9999,	"vid最後",	"video_end"		)
DEF_TRACKBAR_N(	TRACK_VEd2,			0,		-200,	200,	"",			NULL			)
DEF_TRACKBAR(	TRACK_LSt,			0,		0,		9999,	"log先頭",	"log_start"		)
DEF_TRACKBAR_N(	TRACK_LSt2,			0,		-200,	200,	"",			NULL			)
DEF_TRACKBAR(	TRACK_LEd,			0,		0,		9999,	"log最後",	"log_end"		)
DEF_TRACKBAR_N(	TRACK_LEd2,			0,		-200,	200,	"",			NULL			)
DEF_TRACKBAR(	TRACK_LineTrace,	0,		0,		1000,	"Map秒",	"map_length"	)
DEF_TRACKBAR(	TRACK_MapSize,		400,	0,		1000,	"Mapサイズ","map_size"		)
DEF_TRACKBAR(	TRACK_MapAngle,		0,		0,		3600,	"Map回転",	"map_angle"		)
DEF_TRACKBAR(	TRACK_G_Len,		45,		-1,		200,	"G長さ",	"g_length"		)

#undef DEF_TRACKBAR
#undef DEF_TRACKBAR_N
