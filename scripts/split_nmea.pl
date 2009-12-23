#!/usr/bin/perl -w

use Time::Local;

$SpeedTh = 10;
$TimeTh  = 60;

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
			if( $Time - $LastMoved >= $TimeTh * 2 ){
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

foreach ( @Data ){
	
	if( ${ $_ }[ 0 ] > $End ){
		
		last if( $#SplitTbl < 0 );
		
		$Start = shift( @SplitTbl ) - $TimeTh;
		$End   = shift( @SplitTbl ) + $TimeTh;
		
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
			localtime( ${ $_ }[ 0 ] + 9 * 3600 );
		
		$Year += 1900;
		$Mon  += 1;
		
		$FileName = sprintf(
			"${Base}_%4d%02d%02d_%02d%02d%02d$Ext",
			$Year, $Mon, $Day, $Hour, $Min, $Sec
		);
		
		open( fpOut, ">$FileName" );
	}
	
	if( $Start <= ${ $_ }[ 0 ] && ${ $_ }[ 0 ] <= $End ){
		print( fpOut ${ $_ }[ 1 ] );
	}
}

sub GetDate {
	my( $Sec, $Min, $Hour, $Day, $Mon, $Year ) = localtime( $_[ 0 ] );
	
	return sprintf(
		"%4d/%02d/%02d %02d:%02d:%02d",
		$Year + 1900, $Mon + 1, $Day, $Hour, $Min, $Sec
	);
}
