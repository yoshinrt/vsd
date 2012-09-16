#pragma once

#include <windows.h>
#include <crtdbg.h>
#include <assert.h>
#include <ctype.h>
#include <float.h>
#include <gdiplus.h>
#include <map>
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
#include <v8.h>
#include <zconf.h>
#include <zlib.h>

#include <string>
#include <vector>
#include <map>
#include <functional>

#ifdef _OPENMP
	#include <omp.h>
#endif

#define BUF_SIZE	1024
#define ToRAD		( M_PI / 180 )
#define SLIDER_TIME	LOG_FREQ

#define	getcwd	_getcwd
#define	chdir	_chdir
