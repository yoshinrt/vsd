/*****************************************************************************
	
	VSD -- vehicle data logger system  Copyright(C) by DDS
	
	CV8If.cpp - CV8If class implementation
	
*****************************************************************************/

#pragma once

class CV8If {
  public:
	Persistent<Object> *m_pHolder;
	
	CV8If(){
		m_pHolder = NULL;
	}
	
	~CV8If(){
		if( m_pHolder ) m_pHolder->Reset();
	}
};
