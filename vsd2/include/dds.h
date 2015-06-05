/*****************************************************************************

		dds.h -- Deyu Deyu Software's standard include file
		Copyright(C) 1997 - 2001 by Deyu Deyu Software

*****************************************************************************/

#ifndef _DDS_H
#define _DDS_H

#define	Case		break; case
#define Default		break; default
#define until( x )	while( !( x ))

#ifdef EXEC_SRAM
	#define INLINE		_Pragma( "inline" ) static
#else
	#define INLINE		_Pragma( "inline" )
#endif
#define BOTTOM		_Pragma( "location=\".BottomOfSram\"" )

/*** new type ***************************************************************/

//typedef unsigned		UINT;
//typedef unsigned char	UCHAR;
//typedef int				BOOL;
typedef unsigned long long	ULLONG;

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
# define DbgMsg( x )		printf x

/*** no debug macros ********************************************************/

#else	/* _DEBUG */
# define IfDebug			if( 0 )
# define IfNoDebug			if( 1 )
# define DebugCmd( x )
# define NoDebugCmd( x )	x
# define DebugParam( x, y )	( y )
# define DbgMsg( x )
#endif	/* _DEBUG */

/*** constants definition ***************************************************/

#define REGISTORY_KEY_DDS	"Software\\DeyuDeyu\\"

#endif	/* !def _DDS_H */
