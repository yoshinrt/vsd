#!/usr/bin/perl -w

$a = 6378137.0;
$b = 6356752.314140;
$e2 = ( $a ** 2 - $b ** 2 ) / $a ** 2;
$Mnum = $a * ( 1 - $e2 );

sub deg2rad {
	$_[ 0 ] / 180 * 3.14159265358979;
}

sub AccurateDistance{
	my( $x1, $y1, $x2, $y2 ) = @_;	# lon0, lat0, lon1, lat1
	$dy = deg2rad( $y1 - $y2 );
	$dx = deg2rad( $x1 - $x2 );
	$My = deg2rad(( $y1 + $y2 ) / 2 );
	$W = sqrt( 1 - $e2 * sin( $My ) ** 2 );
	$M = $Mnum / $W ** 3;
	$N = $a / $W;
	sqrt(( $dy * $M ) ** 2 + ( $dx * $N * cos( $My )) ** 2 );
};

sub AddChksum {
	local( $_ ) = @_;
	my( $Sum ) = 0;
	my( $idx );
	
	for( $idx = 1; $idx < length( $_ ); ++$idx ){
		$Sum ^= ord( substr( $_, $idx, 1 ));
	}
	
	return sprintf( "$_*%02X\n", $Sum );
}

open( $fpIn, "zcat $ARGV[ 0 ] |" );

while( <$fpIn> ){
	if( /^GPS/ ){
		
		@_ = split( /\t/, $_ );
		
		# 0   1                        2            3           4      5
		# 0   Date/Time                lon          lat         alt    spd
		# GPS 2021-07-23T02:08:23.100Z 136.50534833 34.80690667 41.800 6.130
		
		$_[ 1 ] =~ /\d\d(\d+)-(\d+)-(\d+)T(\d+):(\d+):([\d\.]+)/;
		
		$Date	= "$3$2$1";
		$Time	= "$4$5$6";
		$Lat	= $_[ 3 ];
		$Lng	= $_[ 2 ];
		$_[ 5 ]	/= 1.85200;	# knot
		
		if( !defined( $Lng2M )){
			$Lng0 = $Lng;
			$Lat0 = $Lat;
			
			$Lng2M = AccurateDistance( $Lng, $Lat, $Lng + 1 / 3600, $Lat ) * 3600;
			$Lat2M = AccurateDistance( $Lng, $Lat, $Lng, $Lat + 1 / 3600 ) * 3600;
		}
		
		$LatStr = int( $Lat ) * 100 + fmod( $Lat, 1 ) * 60;
		$LngStr = int( $Lng ) * 100 + fmod( $Lng, 1 ) * 60;
		
		# 方位
		$Bearing =
			atan2(( $Lng - $Lng0 ) * $Lng2M, ( $Lat - $Lat0 ) * $Lat2M )
			* ( 180 / 3.14159265358979 ) + 360;
		$Bearing -= 360 if( $Bearing >= 360 );
		
		print(
			AddChksum( "\$GPGGA,$Time,$LatStr,$LngStr,1,12,0.5,$_[4],M,,,," ) .
			AddChksum( sprintf(
				"\$GPRMC,$Time,A,$LatStr,N,$LngStr,E,%.3f,%.3f,$Date,,,A",
				$_[ 5 ], $Bearing
			))
		);
		
		( $Lng0, $Lat0 ) = ( $Lng, $Lat );
	}
}

sub fmod {
	my( $a, $b ) = @_;
	
	return $a - int( $a / $b ) * $b;
}
