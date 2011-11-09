#!/usr/bin/perl -w

$Diff = 5.0;
if( $ARGV[ 0 ] =~ /^-(\d.*)/ ){
	$Diff = $1;
	shift( @ARGV );
}

$PrevTime = -1000;

while( <> ){
	if( /(\d\d)(\d\d)(\d+\.?\d*)/ ){
		$Time = $1 * 3600 + $2 * 60 + $3;
		
		if(
			$PrevTime == $Time ||
			( $Time < $PrevTime ? ( $Time + 24 * 3600 ) : $Time ) - $PrevTime >= $Diff
		){
			print;
			$PrevTime = $Time;
		}
	}
}
