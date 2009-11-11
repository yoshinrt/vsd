#!/bin/perl -w

$TSC = -1;
$GPSFreq = 16030000 / 65536 / 16;

while( <> ){
	++$TSC;
	
	@_ = split;
	$Speed = $_[ 1 ];
	
	if( s/.*GPS// ){
		@_ = split;
		
		$_ = sprintf(
			'$GPRMC,%010.3f,A,' .	# time
			'%f,N,' .	# lat
			'%f,E,' .	# long
			'%f,' .		# knot/h
			'%f' .		# bearing
			",010109,,,A",
			
			# time
			int( $TSC / $GPSFreq / 3600 ) * 10000 +
			int( $TSC / $GPSFreq / 60 ) % 60 * 100 +
			fmod( $TSC / $GPSFreq, 60 ),
			
			# lat
			int( $_[ 0 ] ) * 100 + fmod( $_[ 0 ], 1 ) * 60,
			
			# long
			int( $_[ 1 ] ) * 100 + fmod( $_[ 1 ], 1 ) * 60,
			
			# knot/h
		#	$_[ 2 ] / 1.85200,	# GPS スピード
			$Speed / 1.85200,	# VSD スピード
			
			# bearing
			$_[ 3 ]
		);
		
		$Sum = 0;
		for( $i = 1; $i < length( $_ ); ++$i ){
			$Sum ^= ord( substr( $_, $i, 1 ));
		}
		
		printf( "$_*%02X\n", $Sum );
	}
}

sub fmod {
	my( $a, $b ) = @_;
	
	return $a - int( $a / $b ) * $b;
}
