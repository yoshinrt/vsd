//			enum name
//				|					初期値
//				|					|			config 名
//				|					|			|

DEF_SHADOW(	SHADOW_LAP_CHART_St,	0,			"lap_chart_start"	)
DEF_SHADOW(	SHADOW_LAP_CHART_Ed,	0,			"lap_chart_end"	)

DEF_SHADOW(	SHADOW_LAP_START,		0,			"lap_start"		)	// ラップ開始
DEF_SHADOW(	SHADOW_LAP_END,			INT_MAX,	"lap_end"		)	// ラップ終了
DEF_SHADOW(	SHADOW_SAVING,			0,			"saving"		)	// avs 用 IsSaving 値

#undef DEF_SHADOW
