/*****************************************************************************
	
	VSD -- vehicle data logger system  Copyright(C) by DDS
	
	error_code.h - error code
	
*****************************************************************************/

#pragma once

enum {
	#define DEF_ERROR( id, msg )	id,
	#include "def_error.h"
};
