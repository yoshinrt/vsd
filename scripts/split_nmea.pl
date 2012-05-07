#!/usr/bin/perl -w

use Time::Local;

$SpeedTh = 10;
$TimeTh  = 60 * 5;

$Line = '';

$LastMoved = 0;
$InFileName = $ARGV[ 0 ];

while( <> ){
	if( /^\$GPRMC/ ){
		
		@_ = split( /,/, $_ );
		
		$_[ 1 ] =~ /^(..)(..)(..)/;
		( $Hour, $Min, $Sec ) = ( $1, $2, $3 );
		
		$_[ 9 ] =~ /^(..)(..)(.*)/;
		( $Day, $Mon, $Year ) = ( $1, $2, $3 );
		
		$Time = timelocal(
			$Sec, $Min, $Hour,
			$Day, $Mon - 1, $Year + 2000
		);
		
		$Speed = $_[ 7 ] * 1.85200;
		
		if( $Speed >= $SpeedTh ){
			if( $Time - $LastMoved >= $TimeTh ){
				# 最後に動いてから TimeTh 以上空いた場合，
				# そこが split 開始点になる．
				
				# 前回の終了点
				push( @SplitTbl, $LastMoved ) if( $LastMoved );
				# 開始点
				push( @SplitTbl, $Time );
			}
			
			# 最後に動いた時間を記録
			$LastMoved = $Time;
		}
		
		push( @Data, [ $Time, $Line . $_ ] );
		$Line = '';
	}else{
		$Line .= $_;
	}
}

exit( 0 ) if( !$LastMoved );
push( @SplitTbl, $LastMoved );

$End = 0;
$bOutput = 0;

for( $i = 0; $i <= $#Data; ++$i ){
	
	$_ = $Data[ $i ];
	
	if( ${ $_ }[ 0 ] > $End ){
		last if( $#SplitTbl < 0 );
		
		# 分割した nmea の最後に TimeTh 分のダミーデータをつける
		AddDummyRMC( ${ $Data[ $i - 1 ] }[ 1 ], $End ) if( $End != 0 );
		
		$Start = shift( @SplitTbl );
		$End   = shift( @SplitTbl );
		
		printf( "%s - %s\n",
			GetDate( $Start + 9 * 3600 ),
			GetDate( $End   + 9 * 3600 )
		);
		
		# ファイル名を作る
		if( $InFileName =~ /\.[^\.\/\\]+$/ ){
			$Base = $`;
			$Ext  = $&;
		}else{
			$Base = $ARGV[ 0 ];
			$Ext  = '';
		}
		
		( $Sec, $Min, $Hour, $Day, $Mon, $Year ) =
			localtime( $Start + 9 * 3600 );
		
		$Year += 1900;
		$Mon  += 1;
		
		$FileName = sprintf(
		#	"${Base}_%4d%02d%02d_%02d%02d%02d$Ext",
			"%4d%02d%02d_%02d%02d%02d$Ext",
			$Year, $Mon, $Day, $Hour, $Min, $Sec
		);
		
		open( fpOut, "| gzip > $FileName.gz" );
		#open( fpOut, "> $FileName" );
		$bOutput = 0;
	}
	
	$Start -= $TimeTh;
	$End   += $TimeTh;
	
	if( $Start <= ${ $_ }[ 0 ] && ${ $_ }[ 0 ] <= $End ){
		# 分割した nmea の先頭に TimeTh 分のダミーデータをつける
		if( $bOutput == 0 ){
			AddDummyRMC( ${ $_ }[ 1 ], $Start );
			$bOutput = 1;
		}
		
		print( fpOut ${ $_ }[ 1 ] );
	}
}

# 最後に分割した nmea の最後に TimeTh 分のダミーデータをつける
AddDummyRMC( ${ $Data[ $i - 1 ] }[ 1 ], $End );

sub GetDate {
	my( $Sec, $Min, $Hour, $Day, $Mon, $Year ) = localtime( $_[ 0 ] );
	
	return sprintf(
		"%4d/%02d/%02d %02d:%02d:%02d",
		$Year + 1900, $Mon + 1, $Day, $Hour, $Min, $Sec
	);
}

sub AddDummyRMC {
	my( $Data, $Time ) = @_;
	my( $Sec, $Min, $Hour, $Day, $Mon, $Year );
	my( $i, $Sum );
	
	local( $_ );
	
	$_ = $Data;
	/(\$GPRMC.*)/; $_ = $1;
	
	@_ = split( /,/, $_ );
	
	( $Sec, $Min, $Hour, $Day, $Mon, $Year ) = localtime( $Time );
	
	$_[ 1 ] = sprintf( '%02d%02d%02d', $Hour, $Min, $Sec );
	$_[ 9 ] = sprintf( '%02d%02d%02d', $Day, $Mon + 1, $Year % 100 );
	
	$_[ 7 ] = 0;
	
	$_ = join( ',', @_ );
	s/\*.*//s;
	
	$Sum = 0;
	for( $i = 1; $i < length( $_ ); ++$i ){
		$Sum ^= ord( substr( $_, $i, 1 ));
	}
	
	printf( fpOut "$_*%02X\n", $Sum );
}
