#ifndef DEF_CHECKBOX_N
	#define DEF_CHECKBOX_N DEF_CHECKBOX
#endif

//			enum name
//				|				チェックボックスの初期値 (値は0か1)
//				|				|	チェックボックスの名前
//				|				|	|					config 名
//				|				|	|					|
DEF_CHECKBOX(	CHECK_LAP,		1,	"ラップタイム",		"lap_time"	)
#ifndef CIRCUIT_TOMO
DEF_CHECKBOX(	CHECK_SNAKE,	1,	"G軌跡",			"g_snake"	)
DEF_CHECKBOX_N(	CHECK_LOGPOS,	0,	"log位置自動認識",	NULL		)
#endif
DEF_CHECKBOX_N(	CHECK_FRAME,	0,	"フレーム表示",		NULL		)

#undef DEF_CHECKBOX
#undef DEF_CHECKBOX_N
