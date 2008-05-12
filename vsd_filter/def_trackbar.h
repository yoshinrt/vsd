//			enum name
//				|			トラックバーの初期値
//				|				|	トラックバーの下限値
//				|				|	|	トラックバーの上限値
//				|				|	|		|		トラックバーの名前
//				|				|	|		|		|
DEF_TRACKBAR( TRACK_VSt,		0,	0,		10000,	"v先頭"		)
DEF_TRACKBAR( TRACK_VSt2,		0,	-200,	200,	""			)
DEF_TRACKBAR( TRACK_VEd,		0,	0,		10000,	"v最後"		)
DEF_TRACKBAR( TRACK_VEd2,		0,	-200,	200,	""			)
DEF_TRACKBAR( TRACK_LSt,		0,	0,		10000,	"log先頭"	)
DEF_TRACKBAR( TRACK_LSt2,		0,	-200,	200,	""			)
DEF_TRACKBAR( TRACK_LEd,		0,	0,		10000,	"log最後"	)
DEF_TRACKBAR( TRACK_LEd2,		0,	-200,	200,	""			)
DEF_TRACKBAR( TRACK_LineTrace,	0,	0,		1000,	"走行Line"	)
DEF_TRACKBAR( TRACK_Resv,		0,	0,		100,	""			)

#undef DEF_TRACKBAR
