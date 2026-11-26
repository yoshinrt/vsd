//			enum name
//				|			トラックバーの初期値
//				|				|	トラックバーの下限値
//				|				|		|	トラックバーの上限値
//				|				|		|		|	トラックバーの名前
//				|				|		|		|		|
//				|				|		|		|		|
DEF_TRACKBAR(	vsd_log_offset,	0,		-9999,	9999,	"車両ログ位置調整"		)
DEF_TRACKBAR(	gps_log_offset,	0,		-9999,	9999,	"GPSログ位置調整"		)

#define DEF_PARAM_TRK(name, type, var, init, caption, min, max)	DEF_TRACKBAR(name, init, min, max, caption)
#include "def_param.h"

#undef DEF_TRACKBAR
