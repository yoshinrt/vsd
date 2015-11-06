#pragma comment( lib, "wininet.lib" )

#include <windows.h>
#include <wininet.h>
#include <stdio.h>

typedef struct {
	UINT	uCode;
	char	*szMsg;
} MsgTbl_t;

char *g_pBuf;

const MsgTbl_t MsgTbl[] = {
	{ 12001, "No more handles could be generated at this time." },
	{ 12002, "The request has timed out." },
	{ 12003, "An extended error was returned from the server. This is typically a string or buffer containing a verbose error message. Call InternetGetLastResponseInfo to retrieve the error text." },
	{ 12004, "An internal error has occurred." },
	{ 12005, "The URL is invalid." },
	{ 12006, "The URL scheme could not be recognized, or is not supported." },
	{ 12007, "The server name could not be resolved." },
	{ 12008, "The requested protocol could not be located." },
	{ 12009, "A request to InternetQueryOption or InternetSetOption specified an invalid option value." },
	{ 12010, "The length of an option supplied to InternetQueryOption or InternetSetOption is incorrect for the type of option specified." },
	{ 12011, "The requested option cannot be set, only queried." },
	{ 12012, "WinINet support is being shut down or unloaded." },
	{ 12013, "The request to connect and log on to an FTP server could not be completed because the supplied user name is incorrect." },
	{ 12014, "The request to connect and log on to an FTP server could not be completed because the supplied password is incorrect." },
	{ 12015, "The request to connect and log on to an FTP server failed." },
	{ 12016, "The requested operation is invalid." },
	{ 12017, "The operation was canceled, usually because the handle on which the request was operating was closed before the operation completed." },
	{ 12018, "The type of handle supplied is incorrect for this operation." },
	{ 12019, "The requested operation cannot be carried out because the handle supplied is not in the correct state." },
	{ 12020, "The request cannot be made via a proxy." },
	{ 12021, "A required registry value could not be located." },
	{ 12022, "A required registry value was located but is an incorrect type or has an invalid value." },
	{ 12023, "Direct network access cannot be made at this time." },
	{ 12024, "An asynchronous request could not be made because a zero context value was supplied." },
	{ 12025, "An asynchronous request could not be made because a callback function has not been set." },
	{ 12026, "The required operation could not be completed because one or more requests are pending." },
	{ 12027, "The format of the request is invalid." },
	{ 12028, "The requested item could not be located." },
	{ 12029, "The attempt to connect to the server failed." },
	{ 12030, "The connection with the server has been terminated." },
	{ 12031, "The connection with the server has been reset." },
	{ 12032, "The function needs to redo the request." },
	{ 12033, "The request to the proxy was invalid." },
	{ 12034, "A user interface or other blocking operation has been requested." },
	{ 12036, "The request failed because the handle already exists." },
	{ 12037, "SSL certificate date that was received from the server is bad. The certificate is expired." },
	{ 12038, "SSL certificate common name (host name field) is incorrect?for example, if you entered www.server.com and the common name on the certificate says www.different.com." },
	{ 12039, "The application is moving from a non-SSL to an SSL connection because of a redirect." },
	{ 12040, "The application is moving from an SSL to an non-SSL connection because of a redirect." },
	{ 12041, "The content is not entirely secure. Some of the content being viewed may have come from unsecured servers." },
	{ 12042, "The application is posting and attempting to change multiple lines of text on a server that is not secure." },
	{ 12043, "The application is posting data to a server that is not secure." },
	{ 12044, "The server is requesting client authentication." },
	{ 12045, "The function is unfamiliar with the Certificate Authority that generated the server's certificate." },
	{ 12046, "Client authorization is not set up on this computer." },
	{ 12047, "The application could not start an asynchronous thread." },
	{ 12048, "The function could not handle the redirection, because the scheme changed (for example, HTTP to FTP)." },
	{ 12049, "Another thread has a password dialog box in progress." },
	{ 12050, "The dialog box should be retried." },
	{ 12052, "The data being submitted to an SSL connection is being redirected to a non-SSL connection." },
	{ 12053, "The request requires a CD-ROM to be inserted in the CD-ROM drive to locate the resource requested." },
	{ 12054, "The requested resource requires Fortezza authentication." },
	{ 12055, "The SSL certificate contains errors." },
	{ 12056, "The SSL certificate was not revoked." },
	{ 12057, "Revocation of the SSL certificate failed." },
	{ 12110, "The requested operation cannot be made on the FTP session handle because an operation is already in progress." },
	{ 12111, "The FTP operation was not completed because the session was aborted." },
	{ 12112, "Passive mode is not available on the server." },
	{ 12130, "An error was detected while parsing data returned from the Gopher server." },
	{ 12131, "The request must be made for a file locator." },
	{ 12132, "An error was detected while receiving data from the Gopher server." },
	{ 12133, "The end of the data has been reached." },
	{ 12134, "The supplied locator is not valid." },
	{ 12135, "The type of the locator is not correct for this operation." },
	{ 12136, "The requested operation can be made only against a Gopher+ server, or with a locator that specifies a Gopher+ operation." },
	{ 12137, "The requested attribute could not be located." },
	{ 12138, "The locator type is unknown." },
	{ 12150, "The requested header could not be located." },
	{ 12151, "The server did not return any headers." },
	{ 12152, "The server response could not be parsed." },
	{ 12153, "The supplied header is invalid." },
	{ 12154, "The request made to HttpQueryInfo is invalid." },
	{ 12155, "The header could not be added because it already exists." },
	{ 12156, "The redirection failed because either the scheme changed (for example, HTTP to FTP) or all attempts made to redirect failed (default is five attempts)." },
	{ 12157, "The application experienced an internal error loading the SSL libraries." },
	{ 12158, "The function was unable to cache the file." },
	{ 12159, "The required protocol stack is not loaded and the application cannot start WinSock." },
	{ 12160, "The HTTP request was not redirected." },
	{ 12161, "The HTTP cookie requires confirmation." },
	{ 12162, "The HTTP cookie was declined by the server." },
	{ 12163, "The Internet connection has been lost." },
	{ 12164, "The website or server indicated is unreachable." },
	{ 12165, "The designated proxy server cannot be reached." },
	{ 12166, "There was an error in the automatic proxy configuration script." },
	{ 12167, "The automatic proxy configuration script could not be downloaded. The INTERNET_FLAG_MUST_CACHE_REQUEST flag was set." },
	{ 12168, "The redirection requires user confirmation." },
	{ 12169, "The SSL certificate is invalid." },
	{ 12170, "The SSL certificate was revoked." },
	{ 12171, "The function failed due to a security check." },
	{ 12172, "Initialization of the WinINet API has not occurred. Indicates that a higher-level function, such as InternetOpen, has not been called yet." },
	{ 12173, "Not currently implemented." },
	{ 12174, "The MS-Logoff digest header has been returned from the website. This header specifically instructs the digest package to purge credentials for the associated realm. This error will only be returned if INTERNET_ERROR_MASK_LOGIN_FAILURE_DISPLAY_ENTITY_BODY option has been set; otherwise, ERROR_INTERNET_LOGIN_FAILURE is returned." },
	{ 12175, "WinINet failed to perform content decoding on the response. For more information, see the Content Encoding topic." },
};

void Error( char *szMsg ){
	DWORD	dwError = GetLastError();
	char *pMsg = g_pBuf;
	
	FormatMessage(
		FORMAT_MESSAGE_FROM_SYSTEM,
		NULL, dwError, 0, g_pBuf, 1024, NULL
	);
	
	if( !*g_pBuf ){
		for( UINT u = 0; u < sizeof( MsgTbl ) / sizeof( MsgTbl_t ); ++u ){
			if( MsgTbl[ u ].uCode == dwError ){
				pMsg = MsgTbl[ u ].szMsg;
				break;
			}
		}
	}
	
	puts( szMsg );
	printf( "エラー 0x%X: %s\n", dwError, pMsg );
}

UINT HttpGet( char *szUrl, char *szFile ){
	
	//-------------------------------------------------------------
	// 画像の読み込み
	//-------------------------------------------------------------
	UINT	result = 1;
	
	#define BUF_SIZE ( 1024 * 1024 )
	g_pBuf	= new char[ BUF_SIZE ];
	
	// URL で開く
	HINTERNET	hInternet	= NULL;
	HINTERNET	hFile		= NULL;
	
	FILE	*fp = NULL;
	
	DWORD	dwReadSize;
	BOOL	bResult;
	
	#ifdef DEBUG
		UINT uStartTime = GetTickCount();
	#endif
	
	do{
		if(
			/* WININET初期化 */
			!( hInternet = InternetOpen(
				//"WININET Sample Program",
				"Mozilla/5.0 (compatible; MSIE 10.0; Windows NT 6.1; Trident/6.0)",
				INTERNET_OPEN_TYPE_PRECONFIG,
				NULL, NULL, 0
			))
		){
			Error( "InternetOpen() 失敗" );
			break;
		}
		
		if(
			/* URLのオープン */
			!( hFile = InternetOpenUrl(
				hInternet, szUrl, NULL, 0,
				INTERNET_FLAG_IGNORE_CERT_CN_INVALID | INTERNET_FLAG_IGNORE_CERT_DATE_INVALID, 0
			))
		){
			Error( "InternetOpenUrl 失敗" );
			
			DWORD	dwBufferLength = BUF_SIZE;
			DWORD	dwError;
			
			InternetGetLastResponseInfo(
				&dwError,
				g_pBuf,
				&dwBufferLength
			);
			
			printf( "HTTP レスポンス: 0x%X:%s\n", dwError, g_pBuf );
			break;
		}
		
		sprintf( g_pBuf, "%s.download", szFile );
		
		fp = fopen( g_pBuf, "wb" );
		if( fp == NULL ) break;
		
		/* オープンしたURLからデータを(BUF_SIZEバイトずつ)読み込む */
		UINT	uTotalSize = 0;
		
		while( 1 ){
			bResult = InternetReadFile( hFile, ( char *)g_pBuf, BUF_SIZE, &dwReadSize );
			if( bResult && ( dwReadSize == 0 )) break;
			
			fwrite( g_pBuf, 1, dwReadSize, fp );
			uTotalSize += dwReadSize;
			putchar( '.' );
		}
		
		fclose( fp );
		
		remove( szFile );
		sprintf( g_pBuf, "%s.download", szFile );
		rename( g_pBuf, szFile );
		
		printf( "\n%dbytes ダウンロード完了\n", uTotalSize );
		result = 0;
		
	}while( 0 );
	
	/* 後処理 */
	if( hFile )		InternetCloseHandle( hFile );
	if( hInternet )	InternetCloseHandle( hInternet );
	
	delete g_pBuf;
	return result;
}

int main( int argc, char **argv ){
	if( argc >= 3 ) return HttpGet( argv[ 1 ], argv[ 2 ]);
	return 1;
}
