/*****************************************************************************
	
	VSD -- vehicle data logger system  Copyright(C) by DDS
	
	CVsdFont.cpp - CVsdFont class implementation
	
*****************************************************************************/

#include "StdAfx.h"
#include "CVsdFont.h"
#include "CScript.h"

/*** macros *****************************************************************/

/*** static member **********************************************************/

#ifdef DEBUG
int CVsdFont::m_iObjCnt = 0;
#endif

/*** コンストラクタ *********************************************************/

CVsdFont::CVsdFont(const char *szFontName, int iSize, UINT uAttr){
	//DebugMsgD("new CFont %d:%X\n", ++m_iObjCnt, this);
	CreateFont(szFontName, iSize, uAttr);
}

CVsdFont::CVsdFont(LPCWSTR szFontName, int iSize, UINT uAttr){
	CreateFont(WideToAnsi(szFontName).c_str(), iSize, uAttr);
}

CVsdFont::~CVsdFont(){
	//DebugMsgD("delete CFont %d:%X\n", m_iObjCnt--, this);
	DestroyFont();
}

/*** フォント作成 ***********************************************************/

void CVsdFont::CreateFont(const char *szFontName, int iSize, UINT uAttr){
	if(iSize <= 0){
		JsRangeError("Font size must be > 0");
	}
	
	m_uAttr = uAttr;
	
	m_LogFont.lfHeight			= iSize;						// 文字セルまたは文字の高さ
	m_LogFont.lfWidth			= 0;							// 平均文字幅
	m_LogFont.lfEscapement		= 0;							// 文字送りの方向とX軸との角度
	m_LogFont.lfOrientation		= 0;							// ベースラインとX軸との角度
	m_LogFont.lfWeight			= uAttr & ATTR_BOLD ? FW_BOLD : FW_REGULAR;	// フォントの太さ
	m_LogFont.lfItalic			= uAttr & ATTR_ITALIC ? TRUE : FALSE;		// イタリック体指定
	m_LogFont.lfUnderline		= FALSE;						// 下線付き指定
	m_LogFont.lfStrikeOut		= FALSE;						// 打ち消し線付き指定
	m_LogFont.lfCharSet			= DEFAULT_CHARSET;				// キャラクタセット
	m_LogFont.lfOutPrecision	= OUT_DEFAULT_PRECIS;			// 出力精度
	m_LogFont.lfClipPrecision	= CLIP_DEFAULT_PRECIS;			// クリッピングの精度
	m_LogFont.lfQuality			= PROOF_QUALITY;				// 出力品質
	m_LogFont.lfPitchAndFamily	= FIXED_PITCH | FF_DONTCARE;	// ピッチとファミリ
	strcpy(m_LogFont.lfFaceName, szFontName);					// フォント名
	
	CreateFont();
}

const MAT2 CVsdFont::mat = {{0, 1}, {0, 0}, {0, 0}, {0, 1}};

void CVsdFont::DestroyFont(void){
	if(m_hdc){
		if(m_hFontOld){
			SelectObject(m_hdc, m_hFontOld);
			m_hFontOld = nullptr;
		}
		if(m_hFont){
			DeleteObject(m_hFont);
			m_hFont = nullptr;
		}
		ReleaseDC(nullptr, m_hdc);			/* ウインドウのHDC解放 */
		m_hdc = nullptr;
	}
}

void CVsdFont::CreateFont(void){
	DestroyFont();
	
	// DC, FONT ハンドル取得
	m_hdc		= GetDC(nullptr);
	m_hFont		= CreateFontIndirect(&m_LogFont);
	m_hFontOld	= (HFONT)SelectObject(m_hdc, m_hFont);
	
	// tmAscent 取得
	TEXTMETRIC tm;
	GetTextMetrics(m_hdc, &tm);
	m_tmAscent = tm.tmAscent;
	GLYPHMETRICS	gm;
	
	int	iBitmapDepth = IsNoAntialias() ? GGO_BITMAP : GGO_GRAY8_BITMAP;
	
	// プロポーショナルの Space 幅取得
	GetGlyphOutlineW(m_hdc, ' ', iBitmapDepth, &gm, 0, nullptr, &mat);
	m_iFontW_Space = gm.gmCellIncX;
	
	GetGlyphOutlineW(m_hdc, 'B', iBitmapDepth, &gm, 0, nullptr, &mat);
	m_iFontW = gm.gmCellIncX;	// 参考: 'B' の文字送り幅
	m_iFontH = tm.tmHeight;
}

// 漢字 glyph 作成
CFontGlyph& CVsdFont::CreateFontGlyph(WCHAR c){
	GLYPHMETRICS	gm;
	
	int	iBitmapDepth = IsNoAntialias() ? GGO_BITMAP : GGO_GRAY8_BITMAP;
	
	// 必要配列サイズ取得
	int iSize = GetGlyphOutlineW(m_hdc, c, iBitmapDepth, &gm, 0, nullptr, &mat);
	
	std::unique_ptr<CFontGlyph> upGlyph(new CFontGlyph);
	CFontGlyph *pGlyph = upGlyph.get();
	
	if(iSize > 0){
		pGlyph->buf.resize(iSize);
		
		// フォントデータ取得
		GetGlyphOutlineW(m_hdc, c, iBitmapDepth, &gm, iSize, pGlyph->buf.data(), &mat);
		pGlyph->iW			= gm.gmBlackBoxX;
		pGlyph->iH			= gm.gmBlackBoxY;
		pGlyph->iOrgY		= (short)(m_tmAscent - gm.gmptGlyphOrigin.y);
		pGlyph->iCellIncX	= (short)gm.gmCellIncX;
	}else{
		pGlyph->buf.clear();
		pGlyph->iW			=
		pGlyph->iH			=
		pGlyph->iOrgY		= 0;
		pGlyph->iCellIncX	= m_iFontW_Space;
	}
	
	CFontGlyph &ref = *pGlyph;
	m_FontGlyph[c] = std::move(upGlyph);
	return ref;
}
