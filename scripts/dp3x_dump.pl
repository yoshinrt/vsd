#!/bin/perl -w

use Time::Local;

open( fpIn, $ARGV[ 0 ] );

read( fpIn, $_, 0x78, 0 );

( $l, $h ) = unpack( 'II', substr( $_, 0x48, 8 ));
$_ = $h * ( 0xFFFFFFFF + 1 ) + $l;
( $Sec, $Min, $Hour, $Day, $Mon, $Year ) =
	localtime( $_ / 1000 );

printf( "%X\n", $_ );
printf(
	"%4d/%02d/%02d %02d:%02d:%02d\n",
	$Year + 1900, $Mon + 1, $Day, $Hour, $Min, $Sec
);
exit 0;

while( read( fpIn, $_, 18, 0 )){
	
	#( $a, $b, $c, $d, $e, $f, $g, $h, $i ) = unpack( 'ssscccccc' );
	print join( "\t", unpack( 'ssscccccc' )) . "\n";
}
