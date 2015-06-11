#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/select.h>
#include <signal.h>

#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <strings.h>

#define BUF_SIZE	1024
char szBuf[ BUF_SIZE ];

//#define DEBUG

#ifdef DEBUG
	#define DebugMsg( fmt, ... )	printf( fmt, ##__VA_ARGS__ )
#else
	#define DebugMsg( fmt, ... )
#endif

static inline max( int a, int b ){
	return a > b ? a : b;
}

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
	
	// dump
	#ifdef DEBUG
		int i;
		for( i = 0; i < iReadSize; ++i ){
			if( ' ' <= szBuf[ i ] && szBuf[ i ] <= 0x7E ){
				printf( "%c", szBuf[ i ] );
			}else{
				printf( "\\x%02X ", szBuf[ i ] );
			}
		}
	#endif
	
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

int main( int argc, char **argv ){
	int fdSockListen, fdSock, fdSeri;
	struct sockaddr_in addr, client;
	int iLen;
	int i;
	fd_set rfds, rfds_tmp;
	struct termios ioOld, ioNew;
	
	int	iBaud = B9600;
	if( argc >= 2 ){
		iBaud = atoi( argv[ 1 ] );
		DebugMsg( "baud late = %d\n", iBaud );
		iBaud =
			iBaud == 0 ? B0 :
			iBaud == 50 ? B50 :
			iBaud == 75 ? B75 :
			iBaud == 110 ? B110 :
			iBaud == 134 ? B134 :
			iBaud == 150 ? B150 :
			iBaud == 200 ? B200 :
			iBaud == 300 ? B300 :
			iBaud == 600 ? B600 :
			iBaud == 1200 ? B1200 :
			iBaud == 1800 ? B1800 :
			iBaud == 2400 ? B2400 :
			iBaud == 4800 ? B4800 :
			iBaud == 9600 ? B9600 :
			iBaud == 19200 ? B19200 :
			iBaud == 38400 ? B38400 :
			iBaud == 57600 ? B57600 :
			iBaud == 115200 ? B115200 :
			iBaud == 230400 ? B230400 :
			B9600;
	}
	
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
	
	if( listen( fdSockListen, 3 ) != 0 ){
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
		
		DebugMsg( "start session\n" );
		
		// シリアルポート open・設定
		fdSeri = open( "/dev/ttyS0", O_RDWR | O_NOCTTY );
		if( fdSock < 0 ){
			perror( "tty" );
			return 1;
		}
		
		//tcgetattr( fdSeri, &ioOld ); /* 現在のポート設定を待避 */
		
		bzero( &ioNew, sizeof( ioNew ));
		ioNew.c_cflag = iBaud | CS8 | CLOCAL | CREAD;
		ioNew.c_iflag = IGNPAR;
		ioNew.c_oflag = 0;
		
		/* set input mode (non-canonical, no echo,...) */
		ioNew.c_lflag = 0;
		ioNew.c_cc[ VTIME ]	= 0;	/* キャラクタ間タイマは未使用 */
		ioNew.c_cc[ VMIN  ]	= 0;	/* n文字受け取るまでブロックする */
		
		tcflush( fdSeri, TCIFLUSH );
		i = tcsetattr( fdSeri, TCSANOW, &ioNew );
		DebugMsg( "tcsetattr:%d\n", i );
		
		// データが尽きるまでループ
		int fdMax = max( max( fdSock, fdSeri ), fdSockListen ) + 1;
		FD_ZERO( &rfds );
		FD_SET( fdSock, &rfds );
		FD_SET( fdSeri, &rfds );
		FD_SET( fdSockListen, &rfds );
		
		while( 1 ){
			// リード fdSeri コレクション
			rfds_tmp = rfds;
			i = select( fdMax, &rfds_tmp, NULL, NULL, NULL );
			
			if( i < 0 ){
				perror( "select" );
				break;
			}
			DebugMsg( "sel_exit%d\n", i );
			
			// リッスンソケットに接続があったら，現接続は切断
			if( FD_ISSET( fdSockListen, &rfds_tmp )){
				DebugMsg( "new connection\n" );
				break;
			}
			
			// データをリピートする
			if( FD_ISSET( fdSock, &rfds_tmp ) && Repeater( fdSeri, fdSock ) <= 0 ) break;
			if( FD_ISSET( fdSeri, &rfds_tmp ) && Repeater( fdSock, fdSeri ) <= 0 ) break;
		}
		
		DebugMsg( "end session\n" );
		//tcsetattr( fdSeri, TCSANOW, &ioOld );
		close( fdSeri );
		close( fdSock );
	}
	
	close( fdSockListen );
	
	return 0;
}
