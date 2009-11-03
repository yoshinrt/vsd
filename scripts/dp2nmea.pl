#!/bin/perl -w

open( fpIn, $ARGV[ 0 ] );

seek( fpIn, 0x100, 0 );

while( read( fpIn, $_, 16, 0 )){
	$_ = reverse( $_ );
	
	( $Bearing, $Speed, $Lat, $Long, $Time ) = unpack( 'SSIII' );
	
	$Lat  /= 460800;
	$Long /= 460800;
	
	$_ = sprintf(
		'$GPRMC,%010.3f,A,' .	# time
		'%f,N,' .	# lat
		'%f,E,' .	# long
		'%f,' .		# knot/h
		'%f' .		# bearing
		",010109,,,A",
		
		# time
		$Time / 10,
		
		# lat
		int( $Lat ) * 100 + fmod( $Lat, 1 ) * 60,
		
		# long
		int( $Long ) * 100 + fmod( $Long, 1 ) * 60,
		
		# knot/h
		$Speed / 10 / 1.85200,
		
		# bearing
		$Bearing
	);
	
	$Sum = 0;
	for( $i = 1; $i < length( $_ ); ++$i ){
		$Sum ^= ord( substr( $_, $i, 1 ));
	}
	
	printf( "$_*%02X\n", $Sum );
}

sub fmod {
	my( $a, $b ) = @_;
	
	return $a - int( $a / $b ) * $b;
}
