/*****************************************************************************
	
	VSD -- vehicle data logger system  Copyright(C) by DDS
	
	CVsdParam.cpp - CVsdFilter parameter
	
*****************************************************************************/

class CVsdParam {
public:
	#define DEF_PARAM_STR(name, type, var, init, ...)	type var;
	#define DEF_PARAM_TRK	DEF_PARAM_STR
	#define DEF_PARAM_CHK	DEF_PARAM_STR
	#define DEF_PARAM_SDW	DEF_PARAM_STR
	#include "def_param.h"
	
	std::vector<int>	m_Mark;
	
	CVsdParam() { Initialize(); }

	void Initialize();
	bool Load(const char *szCfgFile);
	bool Save(const char* szCfgFile, const char* szMovieFile, int fpsDivident, int fpsDivisor) const;
};
