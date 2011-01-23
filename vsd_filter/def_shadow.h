//			enum name
//				|				初期値
//				|				|		config 名
//				|				|		|

#ifdef GPS_ONLY
DEF_SHADOW(	PARAM_VSt,			0,		"video_start"	)
DEF_SHADOW(	PARAM_VEd,			0,		"video_end"		)
DEF_SHADOW(	PARAM_GSt,			0,		"gps_start"		)
DEF_SHADOW(	PARAM_GEd,			0,		"gps_end"		)
#endif

DEF_SHADOW(	SHADOW_METER_CX,	-1,		"meter_cx"		)	// メーター中心座標 X (-1:default)
DEF_SHADOW(	SHADOW_METER_CY,	-1,		"meter_cy"		)	// メーター中心座標 Y
DEF_SHADOW(	SHADOW_METER_R,		-1,		"meter_r"		)	// メーター半径

DEF_SHADOW(	SHADOW_G_SCALE,		1500,	"g_scale"		)	// G スケール * 1000
DEF_SHADOW(	METER_ADJUST,		1000,	"meter_adjust"	)	// メーター補正係数
DEF_SHADOW(	SHADOW_FONT_SIZE,	-1,		"font_size"		)	// フォントサイズ
DEF_SHADOW(	SHADOW_FONT_ATTR,	0,		"font_attr"		)	// +1: 太字 +2:斜体
DEF_SHADOW(	SHADOW_LAP_START,	0,		"lap_start"		)	// ラップ開始
DEF_SHADOW(	SHADOW_LAP_END,		INT_MAX,"lap_end"		)	// ラップ終了


#undef DEF_SHADOW
