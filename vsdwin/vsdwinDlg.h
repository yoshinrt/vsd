// vsdwinDlg.h : ヘッダー ファイル
//

#if !defined(AFX_VSDWINDLG_H__17659031_2E2A_4972_AEC1_8D4972FBD73A__INCLUDED_)
#define AFX_VSDWINDLG_H__17659031_2E2A_4972_AEC1_8D4972FBD73A__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

typedef struct {
	USHORT	uHz, uDuration;
} NOTE;

/////////////////////////////////////////////////////////////////////////////
// CVsdwinDlg ダイアログ

class CVsdwinDlg : public CDialog
{
// 構築
public:
	void CommThread( void );
	CVsdwinDlg(CWnd* pParent = NULL);	// 標準のコンストラクタ

// ダイアログ データ
	//{{AFX_DATA(CVsdwinDlg)
	enum { IDD = IDD_VSDWIN_DIALOG };
	CProgressCtrl	m_ProgressFuel;
	CStatic	m_StaticFuel;
	CSliderCtrl	m_SliderPlayPos;
	CStatic	m_StaticGear;
	CStatic	m_StaticWTemp;
	CProgressCtrl	m_ProgressWTemp;
	CProgressCtrl	m_ProgressSpeed;
	CStatic	m_StaticSpeed;
	CStatic	m_StaticTaco;
	CProgressCtrl	m_ProgressTaco;
	CEdit	m_EditDiff;
	CEdit	m_EditClock;
	CEdit	m_EditBest;
	CListCtrl	m_ListLapTime;
	//}}AFX_DATA

	// ClassWizard は仮想関数のオーバーライドを生成します。
	//{{AFX_VIRTUAL(CVsdwinDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV のサポート
	//}}AFX_VIRTUAL

// インプリメンテーション
protected:
	CMenu m_Menu;
	CFont font;
	HICON m_hIcon;

	// 生成されたメッセージ マップ関数
	//{{AFX_MSG(CVsdwinDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnMenuitemExit();
	afx_msg void OnMenuLaptime();
	afx_msg void OnTimer(UINT nIDEvent);
	afx_msg void OnMenuSetting();
	afx_msg void OnMenuGymkhana();
	afx_msg void OnMenuZeroFour();
	afx_msg void OnMenuZeroOne();
	afx_msg void OnClose();
	afx_msg void OnMenuSpeed();
	afx_msg void OnMenuTaco();
	afx_msg void OnMenuLedReverse();
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void OnDropFiles(HDROP hDropInfo);
	afx_msg void OnMenuGearMode();
	afx_msg void OnMenuBeep();
	afx_msg void OnMenuWTemp();
	afx_msg void OnMenuReplay();
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnMenuOpenFirm();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
	LOGFONT logfont;
private:
#ifdef USE_GIVEIO
	BOOL m_bIsOS_NT;
	BOOL m_bLoadGiveIO;
#endif
	static const NOTE m_BeepNoteBestLap[];
	static const NOTE m_BeepNoteNewLap[];
	NOTE* m_pPlayNote;
	UINT m_uNextClock;
	UINT m_uStartGThrethold;
	BOOL m_bReloadFirmware;
	CString m_strFirmwareName;
	UINT m_uSizeRecordLog;
	CStdioFile m_fpRecordLog;
	BOOL m_bReplay;
	BOOL m_bBeep;
	char m_szLEDMsg[ 256 ];
	char *m_pLEDMsg;
	static const UCHAR m_cLEDFont[];
	BOOL AddLapTime( char *szBuf, UINT uLapTimeDiff, BOOL bLog );
	CString m_strBestLapSound;
	CString m_strNewLapSound;
	UINT m_uGymkhanaStartMargin;
	int m_iDiffSign;
	UINT m_uLEDMode;
	BOOL m_bCommLog;
	BOOL m_bLEDReverse;
	BOOL m_bShiftWarning;
	int m_iCommDebug;
	volatile BOOL m_bExit;
	CStdioFile m_File;
	int m_iComm;
	UINT m_uMode;
	UINT m_uLapTimePrev;
	UINT m_uLapTimeHigh;
	LONG OnDoComm( WPARAM wParam, LPARAM lParam );
	UINT m_uLapCnt;
	UINT m_uBestLap;
	UINT m_uClock;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ は前行の直前に追加の宣言を挿入します。

#endif // !defined(AFX_VSDWINDLG_H__17659031_2E2A_4972_AEC1_8D4972FBD73A__INCLUDED_)
