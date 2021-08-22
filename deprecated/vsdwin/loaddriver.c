/*****************************************************************************
		Device driver easy load / unload
		Aug 20 1999 kashiwano masahiro
*****************************************************************************/

#include <windows.h>
#include <conio.h>
#include "dds_lib.h"

/*****************************************************************************

デバイスドライバを登録・開始する
戻り値
	TRUE	正常終了
	FALSE	ドライバー登録，開始失敗
			デバイスドライバを制御できる権限がないと失敗する

引数
	szFileName		ドライバーのファイル名
	szDriverName	ドライバーの名前．ドライバーを特定できる名前
					UnloadDriverの引数にも使う

*****************************************************************************/

BOOL LoadDriver( char *szFileName, char *szDriverName ){
	SC_HANDLE		hSCManager;
	SC_HANDLE		hService;
	SERVICE_STATUS	serviceStatus;
	BOOL			bRet = FALSE;
	
	// サービスコントロールマネージャを開く
	hSCManager = OpenSCManager( NULL, NULL, SC_MANAGER_ALL_ACCESS );
	if( !hSCManager ) return( FALSE );
	
	// 既にドライバーが存在するか確認するためにドライバーを開く
	hService = OpenService(
		hSCManager,
		szDriverName,
		SERVICE_ALL_ACCESS
	);
	
	if( hService ){
		// 既に動作している場合は停止させて削除する
		// 通常はドライバーが存在するときはLoadDriverを呼び出さないので普段はありえない
		// ドライバの異常が考えられる
		bRet = ControlService( hService, SERVICE_CONTROL_STOP, &serviceStatus );
		bRet = DeleteService( hService );
		CloseServiceHandle( hService );
	}
	
	// ドライバーを登録する
	hService = CreateService(
		hSCManager,
		szDriverName,
		szDriverName,
		SERVICE_ALL_ACCESS,
		SERVICE_KERNEL_DRIVER,		// カーネルドライバ
		SERVICE_DEMAND_START,		// 後でStartService()によって開始する
		SERVICE_ERROR_NORMAL,
		szFileName,					// ドライバーファイルのパス
		NULL, NULL, NULL, NULL, NULL
	);
	
	if( hService ) {
		
		// ドライバーを開始する
		bRet = StartService( hService, 0, NULL );
		
		// ハンドルを閉じる
		CloseServiceHandle( hService );
	}
	// サービスコントロールマネージャを閉じる
	CloseServiceHandle( hSCManager );
	
	return( bRet );
}

/*****************************************************************************

ドライバーを停止する
戻り値
	TRUE	正常終了
	FALSE	失敗

引数
	szDriverName	ドライバーの名前

*****************************************************************************/

BOOL UnloadDriver( char *szDriverName ){
	SC_HANDLE		hSCManager;
	SC_HANDLE		hService;
	SERVICE_STATUS  serviceStatus;
	BOOL			bRet = FALSE;
	
	// サービスコントロールマネージャを開く
	hSCManager = OpenSCManager( NULL, NULL, SC_MANAGER_ALL_ACCESS );
	if( !hSCManager ) return FALSE;
	
	// ドライバーのサービスを開く
	hService = OpenService( hSCManager, szDriverName, SERVICE_ALL_ACCESS );
	
	if( hService ) {
		// ドライバーを停止させる 
		bRet = ControlService( hService, SERVICE_CONTROL_STOP, &serviceStatus );
		
		// ドライバーの登録を消す
		if( bRet == TRUE ) bRet = DeleteService( hService );
		
		// ハンドルを閉じる
		CloseServiceHandle( hService );
	}
	// サービスコントロールマネージャを閉じる
	CloseServiceHandle( hSCManager );
	
	return( bRet );
}

/*** カレント dir の giveio.sys をロードする ********************************/
/*
	入力 : szDriverName は LoadDriver のそれと同じ
	出力 : TRUE = 成功
*/
BOOL LoadGiveIO( char *szDriverName ){
	
	HANDLE	hDriver;
	char	szModuleName[ MAX_PATH ];
	char	*p;
	
	GetModuleFileName( NULL, szModuleName, MAX_PATH );
	if( p = StrTokFile( NULL, szModuleName, STF_NODE )) strcpy( p, "GIVEIO.SYS" );
	
	if( !LoadDriver( szModuleName, szDriverName )) return( FALSE );
	
    hDriver = CreateFile(
    	"\\\\.\\giveio", GENERIC_READ, 0, NULL,
		OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL
	);
    
    if( hDriver == INVALID_HANDLE_VALUE ) return( FALSE );
	
	CloseHandle( hDriver );
	return( TRUE );
}

/*********************************************************************
  Set PC's speaker frequency in Hz.  The speaker is controlled by an
Intel 8253/8254 timer at I/O port addresses 0x40-0x43.
*********************************************************************/

void SetBeepFreq(int hz){
	hz = 1193180 / hz;						// clocked at 1.19MHz
	_outp(0x43, 0xb6);						// timer 2, square wave
	_outp(0x42, hz);
	_outp(0x42, hz >> 8);
}

/*********************************************************************
  Pass a note, in half steps relative to 400 Hz.  The 12 step scale
is an exponential thing.  The speaker control is at port 0x61.
Setting the lowest two bits enables timer 2 of the 8253/8254 timer
and turns on the speaker.
*********************************************************************/
/*
void PlayNote(NOTE note){
	_outp(0x61, _inp(0x61) | 0x03);			// start speaker going
	SetBeepFreq((int)(400 * pow(2, note.pitch / 12.0)));
	Sleep(note.duration);
	_outp(0x61, _inp(0x61) & ~0x03);		// stop that racket!
}
*/
/*********************************************************************
  Open and close the GIVEIO device.  This should give us direct I/O
access.  Then try it out by playin' our tune.
*********************************************************************/

BOOL LoadBeepDriver( char *drivername ){
	HANDLE h;
	
	char	szModuleName[ MAX_PATH ];
	char	*p;
	
	GetModuleFileName( NULL, szModuleName, MAX_PATH );
	if( p = StrTokFile( NULL, szModuleName, STF_NODE ))
		strcpy( p, "GIVEIO.SYS" );
	
	if( LoadDriver( szModuleName, drivername ) == FALSE ){
		return FALSE;
	}
	
    h = CreateFile("\\\\.\\giveio", GENERIC_READ, 0, NULL,
					OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if(h == INVALID_HANDLE_VALUE) {
        //printf("Couldn't access giveio device\n");
        return FALSE;
    }
	CloseHandle(h);
	return TRUE;
}
