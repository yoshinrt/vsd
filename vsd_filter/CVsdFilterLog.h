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
	
	void ValueOfIndex(	// !js_func
		ReturnValue<Value> Ret,
		const char *szPropName,
		double dIdx
	){
		AccessLog( Ret, szPropName, dIdx );
	}
};
