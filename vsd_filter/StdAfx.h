#pragma once

#define _CRT_NO_VA_START_VALIDATION
#define _USE_MATH_DEFINES

#include <windows.h>

#include <algorithm>
#include <assert.h>
#include <condition_variable>
#include <crtdbg.h>
#include <ctype.h>
#include <direct.h>
#include <float.h>
#include <functional>
#include <gdiplus.h>
#include <limits.h>
#include <limits>
#include <map>
#include <math.h>
#include <mutex>
#include <numbers>
#include <stdarg.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <tchar.h>
#include <thread>
#include <time.h>
#include <vector>
#include <wincon.h>
#include <wininet.h>

#include <zconf.h>
#include <zlib.h>
#include <contrib/minizip/unzip.h>

#include "dds.h"

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

inline constexpr double	LOG_FREQ	= 16.0;
inline constexpr int	BUF_SIZE	= 1024;
inline constexpr double	ToRAD		= std::numbers::pi / 180.0;
inline constexpr double	SLIDER_TIME	= LOG_FREQ / 1000.0;
inline constexpr double	NaN			= std::numeric_limits<double>::quiet_NaN();
inline constexpr int	INVALID_INT	= INT_MIN;

#define WIDEN_(x) L##x
#define WIDEN(x)  WIDEN_(x)
