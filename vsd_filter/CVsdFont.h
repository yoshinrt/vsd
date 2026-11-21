/*****************************************************************************
	
	VSD -- vehicle data logger system  Copyright(C) by DDS
	
	CVsdFont.h - CVsdFont class header
	
*****************************************************************************/

#pragma once
#include "CJsClass.h"

/*** new type ***************************************************************/

class CFontGlyph {
  public:
	std::vector<BYTE>	buf;
	short	iW, iH;
	short	iOrgY;
	short	iCellIncX;
	
	CFontGlyph(){
		iCellIncX	= -1;
	}
};

class CVsdFont : public CJsClass {
  public:
	CVsdFont(const char *szFontName, int iSize, UINT uAttr = 0);
	CVsdFont(LPCWSTR szFontName, int iSize, UINT uAttr = 0);
	
	CVsdFont(std::string FontName, int iSize, UINT uAttr = 0){
		CVsdFont(Utf8ToAnsi(FontName).c_str(), iSize, uAttr);
	}
	/* !js_constructor
		.constructor<std::string, int, UINT>()
	*/
	
	~CVsdFont();
	
	static BOOL ExistFont(WCHAR c){
		return FONT_CHAR_FIRST <= c;
	}
	BOOL IsOutline(void){return m_uAttr & ATTR_OUTLINE;}
	BOOL IsFixed(void){return m_uAttr & ATTR_FIXED;}
	BOOL IsNoAntialias(void){return m_uAttr & ATTR_NOANTIALIAS;}
	
	CFontGlyph& FontGlyph(WCHAR c){
		auto itr = m_FontGlyph.find(c);
		if(itr != m_FontGlyph.end()){
			return *(itr->second);
		}
		
		// 見つからなかったので，作成
		return CreateFontGlyph(c);
	}
	
	static const UINT ATTR_BOLD			= 1 << 0;
	static const UINT ATTR_ITALIC		= 1 << 1;
	static const UINT ATTR_OUTLINE		= 1 << 2;
	static const UINT ATTR_FIXED		= 1 << 3;
	static const UINT ATTR_NOANTIALIAS	= 1 << 4;
	
	int GetWidth(void){return m_iFontW;}
	int GetHeight(void){return m_iFontH;}	// !js_var:Height
	
	int GetW_Space(void){return m_iFontW_Space;}
	
	int GetTextWidth(LPCWSTR szMsg){	// !js_func
		if(!szMsg) return 0;
		
		if(m_uAttr & ATTR_FIXED){
			return wcslen(szMsg) * GetWidth();
		}
		
		int iWidth = 0;
		for(int i = 0; szMsg[i]; ++i){
			iWidth += ExistFont(szMsg[i]) ? FontGlyph(szMsg[i]).iCellIncX : m_iFontW_Space;
		}
		return iWidth;
	}
	
  private:
	void CreateFont(const char *szFontName, int iSize, UINT uAttr);
	void CreateFont(void);
	
	CFontGlyph& CreateFontGlyph(WCHAR c);
	
	static const int FONT_CHAR_FIRST = '!';
	
	void DestroyFont(void);
	
	std::map<WCHAR, std::unique_ptr<CFontGlyph>> m_FontGlyph;
	LOGFONT	m_LogFont;
	
	HDC		m_hdc = nullptr;
	HFONT	m_hFont = nullptr;
	HFONT	m_hFontOld = nullptr;
	LONG	m_tmAscent = 0;
	
	int	m_iFontW, m_iFontH, m_iFontW_Space;
	
	UINT	m_uAttr;
	
	static const MAT2	mat;
	
#ifdef DEBUG
	static int m_iObjCnt;
#endif
};
