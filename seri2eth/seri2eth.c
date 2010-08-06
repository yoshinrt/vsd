#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/select.h>

#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <strings.h>

#define BUF_SIZE	1024
char szBuf[ BUF_SIZE ];

#define max( a, b ) (( a ) > ( b ) ? ( a ) : ( b ))

#ifdef DEBUG
	#define DebugMsg( fmt, ... )	printf( fmt, ... )
#else
	#define DebugMsg( fmt, ... )
#endif

/*** xfer *******************************************************************/

int Repeater( int fdOut, int fdIn ){
	int iReadSize, iWriteSize;
	char *pBuf;
	
	DebugMsg( "repeat %d<-%d\n", fdOut, fdIn );
	
	// リード
	iReadSize = read( fdIn, szBuf, BUF_SIZE );
	DebugMsg( "r%d\n", iReadSize );
	if( iReadSize < 0 ){
		perror( "read" );
		return -1;
	}
	if( iReadSize == 0 ) return 0;
	
	// write
	iWriteSize	= 0;
	pBuf		= szBuf;
	while( iReadSize ){
		iWriteSize = write( fdOut, pBuf, iReadSize );
		DebugMsg( "w%d\n", iWriteSize );
		if( iWriteSize < 0 ){
			perror( "write" );
			return -1;
		}
		pBuf		+= iWriteSize;
		iReadSize	-= iWriteSize;
	}
	
	return 1;
}

/*** main *******************************************************************/

int main( void ){
	int fdSockListen, fdSock, fdSeri;
	struct sockaddr_in addr, client;
	int iLen;
	int i;
	fd_set rfds;
	struct termios ioOld, ioNew;
	
	signal( SIGPIPE, SIG_IGN );	/* シグナルを無視する */
	
	fdSockListen = socket( AF_INET, SOCK_STREAM, 0 );
	if( fdSockListen < 0 ){
		perror( "socket" );
		return 1;
	}
	
	addr.sin_family = AF_INET;
	addr.sin_port = htons( 12345 );
	addr.sin_addr.s_addr = INADDR_ANY;
	
	if( bind( fdSockListen, ( struct sockaddr *)&addr, sizeof( addr )) != 0 ){
		perror( "bind" );
		return 1;
	}
	
	if( listen( fdSockListen, 5 ) != 0 ){
		perror( "listen" );
		return 1;
	}
	
	// コネクション毎の処理
	while( 1 ){
		iLen = sizeof( client );
		fdSock = accept( fdSockListen, ( struct sockaddr *)&client, &iLen );
		if( fdSock < 0 ){
			perror( "accept" );
			break;
		}
		
		// シリアルポート open・設定
		fdSeri = open( "/dev/ttyS0", O_RDWR | O_NOCTTY );
		if( fdSock < 0 ){
			perror( "tty" );
			break;
		}
		
		DebugMsg( "start session\n" );
		
		tcgetattr( fdSeri, &ioOld ); /* 現在のポート設定を待避 */
		
		bzero( &ioNew, sizeof( ioNew ));
		ioNew.c_cflag = B38400 | CS8 | CLOCAL | CREAD;
		ioNew.c_iflag = IGNPAR;
		ioNew.c_oflag = 0;
		
		/* set input mode (non-canonical, no echo,...) */
		ioNew.c_lflag = 0;
		ioNew.c_cc[ VTIME ]	= 0;	/* キャラクタ間タイマは未使用 */
		ioNew.c_cc[ VMIN  ]	= 0;	/* 5文字受け取るまでブロックする */
		
		tcflush( fdSeri, TCIFLUSH );
		i = tcsetattr( fdSeri, TCSANOW, &ioNew );
		DebugMsg( "tcsetattr:%d\n", i );
		
		// データが尽きるまでループ
		while( 1 ){
			
			// リード fdSeri コレクション
			FD_ZERO( &rfds );
			FD_SET( fdSock, &rfds );
			FD_SET( fdSeri, &rfds );
			i = select( max( fdSock, fdSeri ) + 1, &rfds, NULL, NULL, NULL );
			
			if( i < 0 ){
				perror( "select" );
				goto EndSession;
			}
			DebugMsg( "sel_exit%d\n", i );
			
			if( FD_ISSET( fdSock, &rfds ) && Repeater( fdSeri, fdSock ) <= 0 ) break;
			if( FD_ISSET( fdSeri, &rfds ) && Repeater( fdSock, fdSeri ) <= 0 ) break;
		}
		
	  EndSession:
		DebugMsg( "end session\n" );
		close( fdSock );
		
		tcsetattr( fdSeri, TCSANOW, &ioOld );
		close( fdSeri );
	}
	
	close( fdSockListen );
	
	return 0;
}
