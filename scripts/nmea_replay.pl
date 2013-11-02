#!/usr/bin/perl -w

use Time::HiRes qw(sleep);

$Scale	= 1;	# ÇÜÂ®ÀßÄê
$Dist	= 10;	# ºÇÄã°ÜÆ°µ÷Î¥

open( fpCom, "> /dev/ttyS2" ) || die( "Can't open COM\n" );
#open( fpCom, "| cat" ) || die( "Can't open COM\n" );

$PrevTime = 0;
$PrevLon = 0;
$PrevLat = 0;

undef( $Alt );

$a = 6378137.0;
$b = 6356752.314140;
$e2 = ( $a ** 2 - $b ** 2 ) / $a ** 2;
$Mnum = $a * ( 1 - $e2 );

sub deg2rad {
	$_[ 0 ] / 180 * 3.14159265358979;
}

sub Distance{
	my( $x1, $y1, $x2, $y2 ) = @_;
	$dy = deg2rad( $y1 - $y2 );
	$dx = deg2rad( $x1 - $x2 );
	$My = deg2rad(( $y1 + $y2 ) / 2 );
	$W = sqrt( 1 - $e2 * sin( $My ) ** 2 );
	$M = $Mnum / $W ** 3;
	$N = $a / $W;
	sqrt(( $dy * $M ) ** 2 + ( $dx * $N * cos( $My )) ** 2 );
};

while( <> ){
	s/[\x0D\x0A]+//g;
	s/\*..$//;
	
	if( /^\$GPGGA/ ){
		# 0		 1			2			3 4			   5 6      9
		# $GPGGA,235117.000,3426.318331,N,13515.098949,E,1,12,0,39.948,M,,,,*13
		
		@_ = split( /,/, $_ );
		$Alt = +$_[ 9 ];
	}elsif( /^\$GPRMC/ ){
		# 0		 1			2 3			  4 5			 6 7	 8	  9
		# $GPRMC,235117.000,A,3426.318331,N,13515.098949,E,3.372,2.08,270410,,,A*5C
		
		@_ = split( /,/, $_ );
		
		# °ÜÆ°µ÷Î¥·×»»
		$_[ 3 ] =~ /^(\d+)(\d\d(\.\d+)?)$/;
		$Lat = $1 + $2 / 60 * ( $_[ 4 ] eq 'N' ? 1 : -1 );
		$_[ 5 ] =~ /^(\d+)(\d\d(\.\d+)?)$/;
		$Lon = $1 + $2 / 60 * ( $_[ 6 ] eq 'E' ? 1 : -1 );
		
		next if( Distance( $Lon, $Lat, $PrevLon, $PrevLat ) < $Dist );
		
		# wait »þ´Ö·×»»
		$_[ 1 ] =~ /(\d\d)(\d\d)(\d\d)\.?(\d*)/;
		
		$Time = $1 * 3600 + $2 * 60 + $3 + "0.$4";
		$Time += 24 * 3600 if( $PrevTime > $Time );
		
		( $Sleep, $PrevTime ) = ( $Time - $PrevTime, $Time );
		$Sleep = 1 if( $Sleep > 1 );
		
		# NMEA ¤Î»þ¹ï¤ò¸½ºß»þ¤Ë½¤Àµ
		( $sec, $min, $hour, $mday, $mon, $year ) = gmtime( time );
		$_[ 1 ] = sprintf( '%02d%02d%02d', $hour, $min, $sec );
		$_[ 9 ] = sprintf( '%02d%02d%02d', $mday, $mon + 1, $year % 100 );
		
		$_ = '';
		
		if( defined( $Alt )){
			$_ = AddChksum( "\$GPGGA,$_[ 1 ],$_[ 3 ],$_[ 4 ],$_[ 5 ],$_[ 6 ],2,12,0.5,$Alt,M,,,," );
		}
		
		$_ .= AddChksum( join( ',', @_ ));
		
		sleep( $Sleep / $Scale );
		
		print( fpCom );
		print;
		
		$PrevLon = $Lon;
		$PrevLat = $Lat;
		
		undef $Alt;
	}
}

close( fpCom );

sub AddChksum {
	local( $_ ) = @_;
	my( $Sum ) = 0;
	my( $i );
	
	for( $i = 1; $i < length( $_ ); ++$i ){
		$Sum ^= ord( substr( $_, $i, 1 ));
	}
	
	return sprintf( "$_*%02X\n", $Sum );
}

