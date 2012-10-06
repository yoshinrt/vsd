#!/usr/bin/perl -w

use Time::HiRes qw(sleep);

open( fpCom, "> /dev/ttyS3" ) || die( "Can't open COM\n" );

$PrevTime = 0;

while( <> ){
	s/[\x0D\x0A]+/\r\n/g;
	/^\$\w+,(\d\d)(\d\d)(\d\d)\.?(\d*)/;
	
	$Time = $1 * 3600 + $2 * 60 + $3 + "0.$4";
	$Time += 24 * 3600 if( $PrevTime > $Time );
	
	( $Sleep, $PrevTime ) = ( $Time - $PrevTime, $Time );
	$Sleep = 1 if( $Sleep > 1 );
	sleep( $Sleep / 3 );
	
	print( fpCom );
	print;
}

close( fpCom );
