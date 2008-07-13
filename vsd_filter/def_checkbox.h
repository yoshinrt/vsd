//			enum name
//				|			チェックボックスの初期値 (値は0か1)
//				|			|	チェックボックスの名前
//				|			|	|
#ifndef CIRCUIT_TOMO
DEF_CHECKBOX( CHECK_LAP,	1,	"ラップタイム" )
#endif
DEF_CHECKBOX( CHECK_FRAME,	0,	"フレーム表示" )
#ifndef CIRCUIT_TOMO
DEF_CHECKBOX( CHECK_SNAKE,	0,	"G軌跡" )
DEF_CHECKBOX( CHECK_LOGPOS,	1,	"log位置自動認識" )
#endif

#undef DEF_CHECKBOX
