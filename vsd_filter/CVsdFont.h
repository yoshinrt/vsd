/*****************************************************************************
	
	VSD -- vehicle data logger system  Copyright(C) by DDS
	
	CVsdFont.h - CVsdFont class header
	
*****************************************************************************/

#ifndef _CVsdFont_h_
#define _CVsdFont_h_

/*** new type ***************************************************************/

class CFontGlyph {
  public:
	BYTE	*pBuf;
	BYTE	*pBufOutline;
	int		iW, iH;
	int		iOrgY;
	int		iCellIncX;
	
	CFontGlyph(){
		pBuf = pBufOutline = NULL;
	}
	
	~CFontGlyph(){
		if( pBuf        ) delete [] pBuf;
		if( pBufOutline ) delete [] pBufOutline;
	}
};

class CVsdFont {
  public:
	CVsdFont( const char *szFontName, int iSize, UINT uAttr = 0 );
	CVsdFont( LPCWSTR szFontName, int iSize, UINT uAttr = 0 );
	~CVsdFont(){}
	
	void CreateFont( const char *szFontName, int iSize, UINT uAttr );
	void CreateFont( void );
	
	static BOOL ExistFont( WCHAR c ){ return FONT_CHAR_FIRST <= c && c <= FONT_CHAR_LAST; }
	BOOL IsOutline( void ){ return m_uAttr & ATTR_OUTLINE; }
	BOOL IsFixed( void ){ return m_uAttr & ATTR_FIXED; }
	BOOL IsNoAntialias( void ){ return m_uAttr & ATTR_NOANTIALIAS; }
	
	CFontGlyph& FontGlyph( WCHAR c ){
		return m_FontGlyph[ c - FONT_CHAR_FIRST ];
	}
	
	static const UINT ATTR_BOLD			= 1 << 0;
	static const UINT ATTR_ITALIC		= 1 << 1;
	static const UINT ATTR_OUTLINE		= 1 << 2;
	static const UINT ATTR_FIXED		= 1 << 3;
	static const UINT ATTR_NOANTIALIAS	= 1 << 4;
	
	int GetWidth( void ){ return m_iFontW; }
	int GetHeight( void ){ return m_iFontH; }	// !js_var:Height
	
	int GetW_Space( void ){ return m_iFontW_Space; }
	
	int GetTextWidth( LPCWSTR szMsg ){	// !js_func
		
		if( m_uAttr & ATTR_FIXED ){
			return wcslen( szMsg ) * GetWidth();
		}
		
		int iWidth = 0;
		for( int i = 0; szMsg[ i ]; ++i ){
			iWidth += ExistFont( szMsg[ i ] ) ? FontGlyph( szMsg[ i ] ).iCellIncX : m_iFontW_Space;
		}
		return iWidth;
	}
	
  private:
	static const int FONT_CHAR_FIRST = '!';
	static const int FONT_CHAR_LAST	 = '~';
	
	CFontGlyph m_FontGlyph[ FONT_CHAR_LAST - FONT_CHAR_FIRST + 1 ];
	LOGFONT	m_LogFont;
	
	int	m_iFontW, m_iFontH, m_iFontW_Space;
	
	UINT	m_uAttr;
};
#endif
