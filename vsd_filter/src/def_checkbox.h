#define DEF_PARAM_CHK(name, type, var, init, caption)	DEF_CHECKBOX(name, init, caption)
#include "def_param.h"

//			enum name
//				|				チェックボックスの初期値 (値は0か1)
//				|				|	チェックボックスの名前
//				|				|	|						config 名
//				|				|	|
DEF_CHECKBOX(	auto_sync,		1,	"半自動同期"	)

#undef DEF_CHECKBOX
