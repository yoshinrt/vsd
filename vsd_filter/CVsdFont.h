/*****************************************************************************
	
	VSD -- vehicle data logger system  Copyright(C) by DDS
	
	CVsdFont.h - CVsdFont class header
	
*****************************************************************************/

#ifndef _CVsdFont_h_
#define _CVsdFont_h_

/*** new type ***************************************************************/

typedef struct {
	BYTE	*pBuf;
	int		iW, iH;
	int		iOrgY;
} tFontGlyph;

class CVsdFont {
  public:
	CVsdFont( LOGFONT &logfont );
	~CVsdFont();
	
	int GetW( void ){ return m_iFontW; }
	int GetH( void ){ return m_iFontH; }
	
	int	m_iFontW, m_iFontH;
	
	tFontGlyph *m_FontGlyph;
	
	// ƒtƒHƒ“ƒg
	DWORD GetPix( UCHAR c, int x, int y ){
		int	i = c - ' ';
		return 0;
	}
};
#endif
