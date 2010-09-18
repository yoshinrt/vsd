#!/bin/perl -w

@_ = split( /\xFF/, join( '', <> ));

foreach $_ ( @_ ){
	s/\xFE\x01/\xFF/g;
	s/\xFE\x00/\xFE/g;
	
	( $Tacho, $Speed, $Mileage, $Tsc, $Gx, $Gy, $Time ) =
		unpack( "S6I", $_ );
	
	printf( "%u\t%u\t%u\t%u\t%u\t%u\n", $Tacho, $Speed, $Mileage, $Tsc, $Gx, $Gy );
}
