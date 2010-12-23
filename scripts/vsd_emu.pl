#!/usr/bin/perl -w

use Socket;
use Time::HiRes qw(sleep);

### サーバ

# 1. 受付用ソケットの作成
my $SockListen;
socket( $SockListen, PF_INET, SOCK_STREAM, getprotobyname( 'tcp' ))
	or die "Cannot create socket: $!";

setsockopt( $SockListen, SOL_SOCKET, SO_REUSEADDR, 1 );

# 2. 受付用ソケット情報の作成
my $pack_addr = sockaddr_in( 12345, INADDR_ANY );

# 3. 受付用ソケットと受付用ソケット情報を結びつける
bind( $SockListen, $pack_addr ) or die "Cannot bind: $!";

# 4. 接続を受け付ける準備をする。
listen( $SockListen, SOMAXCONN ) or die "Cannot listen: $!";

# 5. 接続を受け付けて応答する。
my $SockClient; # クライアントとの通信用のソケット

# 接続まち
accept( $SockClient, $SockListen );
print "Connected\n";

# unbuffered
select( $SockClient );
$| = 1;
select( STDOUT );

$Buf = '';

WaitCmd( 'z' ); SendData( ':' );
WaitCmd( 'Ss1a' ); SendData( ':' );	# l
# GetData();

$PULSE_PER_1KM	= 15473.76689;	# ELISE(CE28N)

#$ACC_1G_X	= 6762.594337;
$ACC_1G_Y	= 6667.738702;
$ACC_1G_Z	= 6842.591839;

$iCnt = 0;
$PrevTime = 0;

while( <> ){
	
	s/[\x0D\x0A]//g;
	@_ = split( /\s+/, $_ );
	
	$_ = pack( 'S6',
		$_[ 0 ],
		int( $_[ 1 ] * 100 ),
		int( $_[ 2 ] / 1000 * $PULSE_PER_1KM ),
		$iCnt++,
		int( -$_[ 4 ] * $ACC_1G_Y + 32000 ),
		int(  $_[ 3 ] * $ACC_1G_Z + 32000 )
	);
	
	# ラップタイム
	if( defined( $_[ 6 ] ) && $_[ 6 ] =~ /^LAP/ ){
		if( $_[ 7 ] eq 'start' ){
			# LAP start なので，とりあえず RTC をクリア
			$PrevTime = 1;
			$_ .= pack( 'I', $PrevTime );
		}else{
			# ラップタイム記録発見
			$_[ 7 ] =~ /(.+):(.+)/;
			$PrevTime += int(( $1 * 60 + $2 ) * 256 );
			$_ .= pack( 'I', $PrevTime );
		}
	}
	
	# 0xFE, FF 処理
	s/\xFE/\xFE\x00/g;
	s/\xFF/\xFE\x01/g;
	
	last if( !defined( send( $SockClient, $_ . "\xFF", 0 )));
	
	sleep( 1 / 16 );
#	sleep( 1 / 10 );
	
	#GetData( MSG_DONTWAIT );
}


sub GetData {
	my( $param ) = @_;
	local( $_ );
	my( $tmp );
	
	recv( $SockClient, $_, 1024, defined( $param ) ? $param : 0 );
	$tmp = $_;
	
	s/([\x00-\x1F\[\x7E\-\xFF])/sprintf( '[%02X]', ord( $1 ))/ge;
	print "Recv:$_\n";
	
	$tmp;
}

sub SendData {
	local( $_ ) = @_;
	
	send( $SockClient, $_, 0 );
	#print <$SockClient>;
	
	s/([\x00-\x1F\[\x7E\-\xFF])/sprintf( '[%02X]', ord( $1 ))/ge;
	print "Send:$_\n";
}

sub WaitCmd {
	local( $_ ) = @_;
	
	print( "Waiting $_\n" );
	
	while( $Buf !~ /$_/ ){
		$Buf .= GetData();
	}
	print( "OK\n" );
	
	$Buf = defined( $' ) ? $' : '';
}
