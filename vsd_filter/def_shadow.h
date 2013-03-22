//			enum name
//				|				初期値
//				|				|		config 名
//				|				|		|

#ifdef PUBLIC_MODE
DEF_SHADOW(	PARAM_VSt,			0,		"video_start"	)
DEF_SHADOW(	PARAM_VEd,			0,		"video_end"		)
DEF_SHADOW(	PARAM_GSt,			0,		"gps_start"		)
DEF_SHADOW(	PARAM_GEd,			0,		"gps_end"		)
#endif

DEF_SHADOW(	METER_ADJUST,		1000,	"meter_adjust"	)	// メーター補正係数
DEF_SHADOW(	SHADOW_LAP_START,	0,		"lap_start"		)	// ラップ開始
DEF_SHADOW(	SHADOW_LAP_END,		INT_MAX,"lap_end"		)	// ラップ終了

#undef DEF_SHADOW
