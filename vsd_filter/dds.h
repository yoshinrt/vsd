/*****************************************************************************

		dds.h -- Deyu Deyu Software's standard include file
		Copyright(C) 1997 - 2001 by Deyu Deyu Software

*****************************************************************************/

#ifndef _DDS_H
#define _DDS_H

//#define PUBLIC_MODE

#define	Case		break; case
#define Default		break; default
#define until( x )	while( !( x ))
#define INLINE		static __inline

/*** new type ***************************************************************/

typedef unsigned		UINT;
typedef unsigned char	UCHAR;
typedef int				BOOL;
typedef long long		INT64;

/*** debug macros ***********************************************************/

#if defined _DEBUG && !defined DEBUG
# define DEBUG
#endif

#ifdef DEBUG
# define IfDebug			if( 1 )
# define IfNoDebug			if( 0 )
# define DebugCmd( x )		x
# define NoDebugCmd( x )
# define DebugParam( x, y )	( x )

/*** output debug message ***************************************************/

#include <stdio.h>
#include <stdarg.h>

#define DEBUG_SPRINTF_BUFSIZE	1024
#define DEBUG_MESSAGE_HEADER	"DEBUG:"
#define DEBUG_MESSAGE_HEADER_W	L"DEBUG:"
#ifdef _MSC_VER
# define C_DECL __cdecl
#else
# define C_DECL
#endif

#ifdef __cplusplus
	template <typename T>
	static int C_DECL DebugMsg( const T *szFormat, ... ){
		va_list	arg;
		
		va_start( arg, szFormat );
		if( sizeof( T ) == 1 ){
			fputs( DEBUG_MESSAGE_HEADER, stdout );
			vprintf(( const char *)szFormat, arg );
		}else{
			fputws( DEBUG_MESSAGE_HEADER_W, stdout );
			vwprintf(( const wchar_t *)szFormat, arg );
		}
		va_end( arg );
		
		return( 0 );
	}
	
	template <typename T>
	static T *C_DECL DebugSPrintf( const T *szFormat, va_list arg ){
		static T	szMsg[ DEBUG_SPRINTF_BUFSIZE ];
		
		if( sizeof( T ) == 1 ){
			_vsnprintf( reinterpret_cast<char *>( szMsg ), DEBUG_SPRINTF_BUFSIZE, reinterpret_cast<const char *>( szFormat ), arg );
		}else{
			_vsnwprintf( reinterpret_cast<wchar_t *>( szMsg ), DEBUG_SPRINTF_BUFSIZE, reinterpret_cast<const wchar_t *>( szFormat ), arg );
		}
		szMsg[ DEBUG_SPRINTF_BUFSIZE - 1 ] = '\0';
		return( szMsg );
	}
	
	template <typename T>
	static int C_DECL DebugMsgD( const T *szFormat, ... ){
		
		va_list	arg;
		
		va_start( arg, szFormat );
		if( sizeof( T ) == 1 ){
			OutputDebugStringA( DEBUG_MESSAGE_HEADER );
			OutputDebugStringA( DebugSPrintf(( const char *)szFormat, arg ));
		}else{
			OutputDebugStringW( DEBUG_MESSAGE_HEADER_W );
			OutputDebugStringW( DebugSPrintf(( const wchar_t *)szFormat, arg ));
		}
		va_end( arg );
		
		return( 0 );
	}
	
	template <typename T>
	static int C_DECL DebugMsgW( const T *szFormat, ... ){
		
		va_list	arg;
		
		va_start( arg, szFormat );	
		if( sizeof( T ) == 1 ){
			MessageBoxA( NULL, DebugSPrintf( szFormat, arg ), DEBUG_MESSAGE_HEADER, MB_OK );
		}else{
			MessageBoxW( NULL, DebugSPrintf( szFormat, arg ), DEBUG_MESSAGE_HEADER_W, MB_OK );
		}
		va_end( arg );
		
		return( 0 );
	}
	
	template <typename T>
	static int C_DECL DebugPrintfD( const T *szFormat, ... ){
		
		va_list	arg;
		
		va_start( arg, szFormat );
		if( sizeof( T ) == 1 ){
			OutputDebugStringA( DebugSPrintf( szFormat, arg ));
		}else{
			OutputDebugStringW( DebugSPrintf( szFormat, arg ));
		}
		va_end( arg );
		
		return( 0 );
	}

#else /*__cplusplus*/
	#ifdef _MSC_VER
		#include <tchar.h>
		#include <wchar.h>
	#endif
	
	static TCHAR * C_DECL DebugSPrintf( TCHAR *szFormat, va_list arg ){
		
		static TCHAR	szMsg[ DEBUG_SPRINTF_BUFSIZE ];
		
		#ifdef _MSC_VER
			_vsntprintf_s( szMsg, DEBUG_SPRINTF_BUFSIZE, _TRUNCATE, szFormat, arg );
		#else /* _MSC_VER */
			vsprintf( szMsg, szFormat, arg );
		#endif /* _MSC_VER */
		
		szMsg[ DEBUG_SPRINTF_BUFSIZE - 1 ] = '\0';
		
		return( szMsg );
	}
	
	static int C_DECL DebugMsg( TCHAR *szFormat, ... ){
		
		va_list	arg;
		
		va_start( arg, szFormat );
		_fputts( DEBUG_MESSAGE_HEADER, stdout );
		_vtprintf( szFormat, arg );
		va_end( arg );
		
		return( 0 );
	}
	
	static int C_DECL DebugMsgD( TCHAR *szFormat, ... ){
		
		va_list	arg;
		
		va_start( arg, szFormat );
		OutputDebugString( DEBUG_MESSAGE_HEADER );
		OutputDebugString( DebugSPrintf( szFormat, arg ));
		va_end( arg );
		
		return( 0 );
	}
	
	static int C_DECL DebugMsgW( TCHAR *szFormat, ... ){
		
		va_list	arg;
		
		va_start( arg, szFormat );	
		MessageBox( NULL, DebugSPrintf( szFormat, arg ), _T( "DEBUG message" ), MB_OK );
		va_end( arg );
		
		return( 0 );
	}
	
	static int C_DECL DebugPrintfD( TCHAR *szFormat, ... ){
		
		va_list	arg;
		
		va_start( arg, szFormat );
		OutputDebugString( DebugSPrintf( szFormat, arg ));
		va_end( arg );
		
		return( 0 );
	}
#endif /* __cpulsplus */

#undef DEBUG_MESSAGE_HEADER
#ifdef C_DECL
# undef C_DECL
#endif

#undef DEBUG_SPRINTF_BUFSIZE

/*** no debug macros ********************************************************/

#else	/* _DEBUG */
# ifndef _MSC_VER
#  define __noop			0&&
# endif
# define IfDebug			if( 0 )
# define IfNoDebug			if( 1 )
# define DebugCmd( x )
# define NoDebugCmd( x )	x
# define DebugParam( x, y )	( y )
# define DebugMsg			__noop
# define DebugMsgW			__noop
# define DebugMsgD			__noop
# define DebugPrintf		__noop
# define DebugPrintfD		__noop
#endif	/* _DEBUG */

/*** constants definition ***************************************************/

#define REGISTORY_KEY_DDS	"Software\\DeyuDeyu\\"

#endif	/* !def _DDS_H */
