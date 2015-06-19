#!/bin/perl -w

@_ = split( /\xFF/, join( '', <> ));

foreach $_ ( @_ ){
	s/\xFE\x01/\xFF/g;
	s/\xFE\x00/\xFE/g;
	
	( $Tacho, $Speed, $Mileage, $Tsc, $Gx, $Gy, $Throttle, $Time ) =
		unpack( "S7I", $_ );
	
	printf( "%u\t%u\t%u\t%u\t%u\t%u\t%u\n", $Tacho, $Speed, $Mileage, $Gx, $Gy, $Tsc, $Throttle );
}
