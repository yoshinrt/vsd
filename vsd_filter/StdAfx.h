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
#include <string>
#include <tchar.h>
#include <direct.h>
#include <v8.h>
#include <vector>
#include <zconf.h>
#include <zlib.h>
#include <vector>
#include <map>
#include <functional>
#include <limits.h>

#ifdef _OPENMP
	#include <omp.h>
#endif

#define BUF_SIZE	1024
#define ToRAD		( M_PI / 180 )
#define SLIDER_TIME	LOG_FREQ

#define	getcwd	_getcwd
#define	chdir	_chdir
