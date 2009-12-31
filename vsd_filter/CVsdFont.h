/*****************************************************************************
	
	VSD -- vehicle data logger system  Copyright(C) by DDS
	
	CVsdFont.h - CVsdFont class header
	$Id$
	
*****************************************************************************/

#ifndef _CVsdFont_h_
#define _CVsdFont_h_

/*** new type ***************************************************************/

class CVsdFont {
  public:
	CVsdFont( LOGFONT &logfont );
	~CVsdFont();
	
	int GetW( void ){ return m_iFontW; }
	int GetH( void ){ return m_iFontH; }
	
	int	m_iFontW, m_iFontH, m_iBMP_W;
	
	LPDWORD m_pFontData;
	HDC		m_hdcBMP;
	HBITMAP	m_hbmpBMP, m_hbmpOld;
	
	// ƒtƒHƒ“ƒg
	DWORD GetPix( UCHAR c, int x, int y ){
		c -= ' ';
		
		return m_pFontData[
			( y + c / 16 * m_iFontH ) * m_iBMP_W +
			( x + c % 16 * m_iFontW * 2 + m_iFontW / 2 )
		];
	}
};
#endif
