// $Id

#ifndef _DDS_H
#define _DDS_H

/*** macros *****************************************************************/

#define Case	break; case
#define Default	break; default

#ifdef MONITOR_ROM
	#define INLINE	__inline
#else
	#define INLINE	static __inline
#endif

#define FALSE	0
#define TRUE	1

/*** new type ***************************************************************/

typedef unsigned char	UCHAR, BOOL;
typedef unsigned		UINT;
typedef unsigned long	ULONG;

#endif
