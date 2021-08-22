// vsdwin.cpp : アプリケーション用クラスの定義を行います。
//

#include "stdafx.h"
#include "vsdwin.h"
#include "vsdwinDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CVsdwinApp

BEGIN_MESSAGE_MAP(CVsdwinApp, CWinApp)
	//{{AFX_MSG_MAP(CVsdwinApp)
		// メモ - ClassWizard はこの位置にマッピング用のマクロを追加または削除します。
		//        この位置に生成されるコードを編集しないでください。
	//}}AFX_MSG
	ON_COMMAND(ID_HELP, CWinApp::OnHelp)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CVsdwinApp クラスの構築

CVsdwinApp::CVsdwinApp()
{
	// TODO: この位置に構築用のコードを追加してください。
	// ここに InitInstance 中の重要な初期化処理をすべて記述してください。
}

/////////////////////////////////////////////////////////////////////////////
// 唯一の CVsdwinApp オブジェクト

CVsdwinApp theApp;

/////////////////////////////////////////////////////////////////////////////
// CVsdwinApp クラスの初期化

//引数：ファイル名
//戻り値：当アプリケーションのexeファイルと同じディレクトリになるフルパス
CString CVsdwinApp::GetModulePathFileName(LPCTSTR pName)
{
	//exeファイルのフルパスを取得
	TCHAR path[_MAX_PATH];
	::GetModuleFileName(NULL,path,sizeof path);

	//exeファイルのパスを分解
	TCHAR drive[_MAX_DRIVE];
	TCHAR dir[_MAX_DIR];
	TCHAR fname[_MAX_FNAME];
	TCHAR ext[_MAX_EXT];
	_splitpath(path,drive,dir,fname,ext);

	//引数のファイル名をexeファイルのパスでフルパス化
	_makepath(path,drive,dir,pName,"");

	CString str(path);
	return str;
}

BOOL CVsdwinApp::InitInstance()
{
	// 標準的な初期化処理
	// もしこれらの機能を使用せず、実行ファイルのサイズを小さくしたけ
	//  れば以下の特定の初期化ルーチンの中から不必要なものを削除して
	//  ください。

#ifdef _AFXDLL
	Enable3dControls();			// 共有 DLL 内で MFC を使う場合はここをコールしてください。
#else
	Enable3dControlsStatic();	// MFC と静的にリンクする場合はここをコールしてください。
#endif
	
	m_hAccel = ::LoadAccelerators(AfxGetInstanceHandle(), MAKEINTRESOURCE(IDR_ACCELERATOR1));
	
	// INIファイルのパス名を変更する
	if( m_pszProfileName ) {
		delete ((void*)m_pszProfileName);
		m_pszProfileName = new char[MAX_PATH];
		if( !m_pszProfileName ) {
			AfxMessageBox("メモリ不足エラーです。");
			return FALSE;
		}
		CString str = GetModulePathFileName( "vsdwin.ini" );
		strcpy((LPTSTR)m_pszProfileName, str );
	}
	
	CVsdwinDlg dlg;
	m_pMainWnd = &dlg;
	int nResponse = dlg.DoModal();
	if (nResponse == IDOK)
	{
		// TODO: ダイアログが <OK> で消された時のコードを
		//       記述してください。
	}
	else if (nResponse == IDCANCEL)
	{
		// TODO: ダイアログが <ｷｬﾝｾﾙ> で消された時のコードを
		//       記述してください。
	}

	// ダイアログが閉じられてからアプリケーションのメッセージ ポンプを開始するよりは、
	// アプリケーションを終了するために FALSE を返してください。
	return FALSE;
}

BOOL CVsdwinApp::ProcessMessageFilter(int code, LPMSG lpMsg) 
{
	if(m_hAccel!= NULL){
		if(::TranslateAccelerator(m_pMainWnd -> m_hWnd, m_hAccel, lpMsg)){	// WM_KEYDOWN&WM_SYSKEYDOWNをWM_COMMAND&WM_SYSCOMMANDに変換。
			return TRUE;													// 変換元メッセージを処理したくないときはTRUEを返す。
//			return CWinApp::ProcessMessageFilter(code, lpMsg);				// 既定の処理も行いたい場合はこっち。
		}
	}
	return CWinApp::ProcessMessageFilter(code, lpMsg);
}

int CVsdwinApp::ExitInstance() 
{
	// TODO: この位置に固有の処理を追加するか、または基本クラスを呼び出してください
	if( m_pszProfileName ) {
		delete ((void*)m_pszProfileName);
		m_pszProfileName = NULL;  // これを忘れずに！！
	}
	return CWinApp::ExitInstance();
}
