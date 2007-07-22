// vsdwin.h : VSDWIN アプリケーションのメイン ヘッダー ファイルです。
//

#if !defined(AFX_VSDWIN_H__AE6F6DC6_DF23_4B49_9D4C_D69FF5393BAA__INCLUDED_)
#define AFX_VSDWIN_H__AE6F6DC6_DF23_4B49_9D4C_D69FF5393BAA__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// メイン シンボル

/////////////////////////////////////////////////////////////////////////////
// CVsdwinApp:
// このクラスの動作の定義に関しては vsdwin.cpp ファイルを参照してください。
//

class CVsdwinApp : public CWinApp
{
public:
	CVsdwinApp();

// オーバーライド
	// ClassWizard は仮想関数のオーバーライドを生成します。
	//{{AFX_VIRTUAL(CVsdwinApp)
	public:
	virtual BOOL InitInstance();
	virtual BOOL ProcessMessageFilter(int code, LPMSG lpMsg);
	virtual int ExitInstance();
	//}}AFX_VIRTUAL

// インプリメンテーション

	//{{AFX_MSG(CVsdwinApp)
		// メモ - ClassWizard はこの位置にメンバ関数を追加または削除します。
		//        この位置に生成されるコードを編集しないでください。
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
protected:
	HACCEL m_hAccel;
	CString GetModulePathFileName(LPCTSTR pName);
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ は前行の直前に追加の宣言を挿入します。

#endif // !defined(AFX_VSDWIN_H__AE6F6DC6_DF23_4B49_9D4C_D69FF5393BAA__INCLUDED_)
