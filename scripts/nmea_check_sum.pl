#!/bin/perl -w

Sum( '$PMTK300,100,0,0,0,0' );
Sum( '$PMTK314,0,1,0,1,5,5,0,0,0,0,0,0,0,0,0,0,0,0,0' );
Sum( '$PMTK182,1,2,0004007D' );

sub Sum {
	local( $_ ) = @_;
	
	$Sum = 0;
	for( $i = 1; $i < length( $_ ); ++$i ){
		$Sum ^= ord( substr( $_, $i, 1 ));
	}
	
	printf( "$_*%02X\n", $Sum );
}
