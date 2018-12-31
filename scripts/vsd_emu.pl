#!/usr/bin/perl

use Socket;
use Time::HiRes qw(sleep);

$TIMER_HZ	= 200000;
$ConnMode	= 1;	# 0:ether client  1:ether server 2:com

if( $ARGV[ 0 ] =~ /\.gz$/ ){
	open( fpIn, "gunzip -c $ARGV[ 0 ] |" );
}else{
	open( fpIn, "< $ARGV[ 0 ]" );
}

# ヘッダ解析

$_ = <fpIn>;
s/[\x0D\x0A]//g;
@_ = split( /\t/, $_ );

$IdxDate		= 0x7FFFFFFF;
$IdxTacho		= 0x7FFFFFFF;
$IdxSpeed		= 0x7FFFFFFF;
$IdxDistance	= 0x7FFFFFFF;
$IdxGx			= 0x7FFFFFFF;
$IdxGy			= 0x7FFFFFFF;
$IdxThrottle	= 0x7FFFFFFF;
$IdxAuxInfo		= 0x7FFFFFFF;
$IdxLapTime		= 0x7FFFFFFF;
$IdxSectorTime	= 0x7FFFFFFF;

for( $i = 0; $i <= $#_; ++$i ){
	if(     $_[ $i ] eq "Date/Time"		){ $IdxDate			= $i;
	}elsif( $_[ $i ] eq "Tacho"			){ $IdxTacho		= $i;
	}elsif( $_[ $i ] eq "Speed"			){ $IdxSpeed		= $i;
	}elsif( $_[ $i ] eq "Distance"		){ $IdxDistance		= $i;
	}elsif( $_[ $i ] eq "Gx"			){ $IdxGx			= $i;
	}elsif( $_[ $i ] eq "Gy"			){ $IdxGy			= $i;
	}elsif( $_[ $i ] eq "Throttle(raw)"	){ $IdxThrottle		= $i;
	}elsif( $_[ $i ] eq "AuxInfo"		){ $IdxAuxInfo		= $i;
	}elsif( $_[ $i ] eq "LapTime"		){ $IdxLapTime		= $i;
	}elsif( $_[ $i ] eq "SectorTime"	){ $IdxSectorTime	= $i;
	}
}

if( 0 ){
	print( "IdxDate = $IdxDate\n" );
	print( "IdxTacho = $IdxTacho\n" );
	print( "IdxSpeed = $IdxSpeed\n" );
	print( "IdxDistance = $IdxDistance\n" );
	print( "IdxGx = $IdxGx\n" );
	print( "IdxGy = $IdxGy\n" );
	print( "IdxThrottle = $IdxThrottle\n" );
	print( "IdxAuxInfo = $IdxAuxInfo\n" );
	print( "IdxLapTime = $IdxLapTime\n" );
	print( "IdxSectorTime = $IdxSectorTime\n" );
}

my $Handle;

if( $ConnMode == 2 ){
	# BT COM の着信を有効，COMn の n - 1 をttySm に設定
	open( $Handle, "> /dev/ttyS4" ) || die( "Can't open COM\n" );
}else{
	if( $ConnMode == 1 ){
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
		# 接続まち
		accept( $Handle, $SockListen );
		print "Connected\n";
	}else{
		# クライアント
		# 1. ソケットの作成
		socket( $Handle, PF_INET, SOCK_STREAM, getprotobyname( 'tcp' ))
			or die "Cannot create socket: $!";
		
		# 2. ソケット情報の作成
		
		# 接続先のホスト名
		my $remote_host = '192.168.0.132';
		my $packed_remote_host = inet_aton( $remote_host )
			or die "Cannot pack $remote_host: $!";
		
		# 接続先のポート番号
		my $remote_port = 12345;
		
		# ホスト名とポート番号をパック
		my $sock_addr = sockaddr_in( $remote_port, $packed_remote_host )
			or die "Cannot pack $remote_host:$remote_port: $!";
		
		# 3. ソケットを使って接続
		connect( $Handle, $sock_addr ) 
			or die "Cannot connect $remote_host:$remote_port: $!";
	}
}

# unbuffered
select( $Handle );
$| = 1;
select( STDOUT );

$Buf = '';

WaitCmd( 'z' ); SendData( ':' );
WaitCmd( 'S7' ); SendData( ':' );	# l
# GetData();

$PULSE_PER_1KM	= 15473.76689;	# ELISE(CE28N)

$iCnt = 0;
$PrevLapTime = 1;
$PrevTime = -1;

while( <fpIn> ){
	
	s/[\x0D\x0A]//g;
	@_ = split( /\t/, $_ );
	
	$_[ $IdxDate ] =~ /(\d+):(\d+):([\d\.]+)/;
	$Time = $1 * 3600 + $2 * 60 + $3;
	
	$_ = pack( 'S7',
		$_[ $IdxTacho ],
		int( $_[ $IdxSpeed ] * 100 ),
		int( $_[ $IdxDistance ] / 1000 * $PULSE_PER_1KM ),
		(( $Time * $TIMER_HZ ) >> 5 ) & 0xFFFF,	# TSC
		int( -$_[ $IdxGy ] * 4096 + 0x8000 ),
		int(  $_[ $IdxGx ] * 4096 + 0x8000 ),
		$_[ $IdxThrottle ] > 0 ? $_[ $IdxThrottle ] : 0x8000
	);
	
	# ラップタイム
	if( defined( $_[ $IdxLapTime ] ) && $_[ $IdxLapTime ] =~ /(.+):(.+)/){
		# ラップタイム記録発見
		$PrevLapTime += int(( $1 * 60 + $2 ) * $TIMER_HZ );
		$_ .= pack( 'I', $PrevLapTime );
	}
	
	# 0xFE, FF 処理
	s/\xFE/\xFE\x00/g;
	s/\xFF/\xFE\x01/g;
	
	last if( !SendData( $_ . "\xFF" ));
	
	sleep( $Time - $PrevTime ) if( $PrevTime >= 0 );
	$PrevTime = $Time;
	
	#GetData( MSG_DONTWAIT );
}

sub GetData {
	my( $param ) = @_;
	local( $_ );
	my( $tmp );
	
	if( $ConnMode == 2 ){
		sysread( $Handle, $_, 1 );
	}else{
		recv( $Handle, $_, 1024, defined( $param ) ? $param : 0 );
	}
	
	return $_ if( !$_ );
	$tmp = $_;
	
	s/([\x00-\x1F\[\x7E-\xFF])/sprintf( '[%02X]', ord( $1 ))/ge;
	print "Recv:$_\n";
	
	$tmp;
}

sub SendData {
	local( $_ ) = @_;
	my $ret;
	
	if( $ConnMode == 2 ){
		$ret = print $Handle $_;
	}else{
		$ret = send( $Handle, $_, 0 ) ? 1 : 0;
	}
	
	s/([\x00-\x1F\[\x7E-\xFF])/sprintf( '[%02X]', ord( $1 ))/ge;
	print "Send:$_\n";
}

sub WaitCmd {
	local( $_ ) = @_;
	
	print( "Waiting $_\n" );
	
	while( $Buf !~ /$_/ ){
		print $Buf;
		$Buf .= GetData();
	}
	print( "OK\n" );
	
	$Buf = defined( $' ) ? $' : '';
}
