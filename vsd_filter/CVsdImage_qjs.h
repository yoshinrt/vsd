/*****************************************************************************
	
	VSD -- vehicle data logger system  Copyright(C) by DDS
	
	CVsdImage_qjs.cpp - Image file operation
	
*****************************************************************************/

#pragma once
#include "CPixel.h"
#include "CJsClass.h"
#include <mutex>
#include <condition_variable>

#define RABY_TRANSPARENT	0x80FF8000
#define IMG_INET_ASYNC		0x1

class CVsdImage_qjs : public CJsClass {
  public:
	CVsdImage_qjs();
	
	CVsdImage_qjs(qjsb::Value file, qjsb::Value flag);
	/* !js_constructor
		.constructor<qjsb::Value, qjsb::Value>()
	*/
	~CVsdImage_qjs();
	
	void DeleteAsync(void);
	
	UINT Load(LPCWSTR szFileName, UINT uFlag = 0);
	
	void Clone(const CVsdImage_qjs *pOrg);	// !js_func
	
	inline UINT GetPixel0(int x, int y){
		return m_ImgBuf[(x - m_iOffsX) + (y - m_iOffsY) * m_iRawWidth].ycbcr;
	}
	
	inline CPixelImg GetPixelRaw(int x, int y){
		return m_ImgBuf[x + y * m_iRawWidth];
	}
	
	inline UINT GetPixel(int x, int y){
		int x0 = x - m_iOffsX;
		int y0 = y - m_iOffsY;
		
		if(0 <= x0 && x0 < m_iRawWidth && 0 <= y0 && y0 < m_iRawHeight){
			return GetPixel0(x, y);
		}
		return RABY_TRANSPARENT;
	}
	
	template<typename Tx, typename Ty>
	UINT Resampling(Tx x, Ty y);
	UINT Resampling(double x0, double x1, int y);
	UINT Resampling(int x, double y0, double y1);
	UINT Resize(int iWidth, int iHeight);			// !js_func
	UINT Rotate(int cx, int cy, double dAngle);	// !js_func
	UINT Clip(int x1, int y1, int x2, int y2);
	
	UINT WaitAsyncLoadComplete(int iMsec);	// !js_func
	
	int m_iWidth;	// !js_var:Width
	int m_iHeight;	// !js_var:Height
	
	volatile int	m_iStatus;	// !js_var:Status
	enum {
		IMG_STATUS_LOAD_COMPLETE,
		IMG_STATUS_LOAD_INCOMPLETE,
		IMG_STATUS_LOAD_FAILED,
		IMG_STATUS_DESTROYED,
	};
	
	int m_iRawWidth;
	int m_iRawHeight;
	int m_iOffsX;
	int m_iOffsY;
	
	std::mutex				m_Mutex;
	std::condition_variable	m_CondVar;
	bool					m_bLoading = false;
	
  private:
	std::wstring			m_FileName;
	std::vector<CPixelImg>	m_ImgBuf;
};
