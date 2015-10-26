/*****************************************************************************
	
	VSD -- vehicle data logger system  Copyright(C) by DDS
	
	CVsdFilterLog.h - JavaScript Log object I/F
	
*****************************************************************************/

#pragma once

class CVsdFilterLog : public CVsdFilter {
	
  public:
	static CVsdFilterLog *Cast( CVsdFilter *obj ){
		return static_cast<CVsdFilterLog *>( obj );
	}
	
	Handle<Value> ValueOfIndex(	// !js_func
		const char *szPropName, double dIdx
	){
		return AccessLog( szPropName, dIdx );
	}
};
