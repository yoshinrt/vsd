#pragma once

#define _CRT_NO_VA_START_VALIDATION

#include <windows.h>
#include <crtdbg.h>
#include <assert.h>
#include <ctype.h>
#include <float.h>
#include <gdiplus.h>
#include <map>
#include <algorithm>
#define _USE_MATH_DEFINES
#include <math.h>
#include <objbase.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <direct.h>
#include <tchar.h>
#include <time.h>
#include <ole2.h>
#include <Wininet.h>
#include <limits.h>
#include <wincon.h>

#include <string>
#include <vector>
#include <map>
#include <functional>
#include <limits>
#include <thread>

#include <zconf.h>
#include <zlib.h>
#include <contrib/minizip/unzip.h>

#include "dds_lib/dds.h"
#include <assert.h>
#include "dds_lib/dds_lib.h"

#include "quickjs.h"
#include "quickjs-libc.h"

inline bool JS_IsArray(JSContext*, JSValueConst val) {
	return JS_IsArray(val);
}

#define QJSBRIDGE_QUICKJS_NG
#include "qjsbridge.hpp"

#ifdef _OPENMP
	#include <omp.h>
	#ifdef AVS_PLUGIN
//		#define _OPENMP_AVS
	#endif
#endif

#define LOG_FREQ	16.0
#define BUF_SIZE	1024
#define ToRAD		(M_PI / 180)
#define SLIDER_TIME	(LOG_FREQ / 1000.0)
#define NaN			std::numeric_limits<double>::quiet_NaN()
#define INVALID_INT		INT_MIN

#define WIDEN_(x) L##x
#define WIDEN(x)  WIDEN_(x)
