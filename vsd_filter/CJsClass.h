/*****************************************************************************
	
	VSD -- vehicle data logger system  Copyright(C) by DDS
	
	CJsClass.cpp - CJsClass class implementation
	
*****************************************************************************/

#pragma once

class CJsClass {
  public:
	Persistent<Object> *m_pHolder;
	
	CJsClass(){
		m_pHolder = NULL;
	}
	
	~CJsClass(){
		if( m_pHolder ) m_pHolder->Reset();
	}
};
