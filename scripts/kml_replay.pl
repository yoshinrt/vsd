#!/usr/bin/perl -w

use Time::HiRes qw(sleep);
use Term::ReadKey;

ReadMode 'cbreak';

$Speed	= 64;	# [km/h]
$Stop	= 0;

# BT COM の着信を有効，COMn の n - 1 をttySm に設定
open( $fp, "> /dev/ttyS2" ) || die( "Can't open COM\n" );
#open( $fp, "| cat" ) || die( "Can't open COM\n" );

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

sub Distance{
	my( $x1, $y1, $x2, $y2 ) = @_;	# lon0, lat0, lon1, lat1
	sqrt(
		(( $x2 - $x1 ) * $Lng2M ) ** 2 +
		(( $y2 - $y1 ) * $Lat2M ) ** 2
	);
};

# read kml
$_ = join( '', <> );

while( s/.*?<LineString>//s ){
	s#.*?<coordinates>(.*?)</coordinates>##s;
	
	@_ = split( /\s+/, $1 );
	
	foreach $Data ( @_ ){
		$Data =~ /^([^,]+),([^,]+)/;
		push( @Lng, 0 + $1 );
		push( @Lat, 0 + $2 );
	}
}

$Lng = $Lng0 = $Lng[ 0 ];
$Lat = $Lat0 = $Lat[ 0 ];

$Lng2M = AccurateDistance( $Lng, $Lat, $Lng + 1 / 3600, $Lat ) * 3600;
$Lat2M = AccurateDistance( $Lng, $Lat, $Lng, $Lat + 1 / 3600 ) * 3600;

$CmdNum = 0;

for( $idx = 1; $idx <= $#Lng; ){
	
	if( !$Stop ){
		$Dist = $Speed / 3.6;
		
		# P=(Lng,Lat) 地点から Dist [M] 進んだ地点の点 Q を求める．
		# P <= Q < [$idx] となる範囲で．
		for( ;
			$idx <= $#Lng &&
			( $tmp = Distance( $Lng, $Lat, $Lng[ $idx ], $Lat[ $idx ] )) <= $Dist;
			++$idx
		){
			$Dist -= $tmp;
			( $Lng, $Lat ) = ( $Lng[ $idx ], $Lat[ $idx ] );
		}
		
		last if( $idx > $#Lng );
		
		# P <= Q < [$idx] の範囲で，残り $Dist 進んだ座標を求める
		$alfa = $Dist / Distance( $Lng, $Lat, $Lng[ $idx ], $Lat[ $idx ] );
		$Lng = ( $Lng[ $idx ] - $Lng ) * $alfa + $Lng;
		$Lat = ( $Lat[ $idx ] - $Lat ) * $alfa + $Lat;
		
		# 検算
		#printf( "$idx: %.8f\n", AccurateDistance( $Lng0, $Lat0, $Lng, $Lat ));
		
		# 方位
		$Bearing =
			atan2(( $Lng - $Lng0 ) * $Lng2M, ( $Lat - $Lat0 ) * $Lat2M )
			* ( 180 / 3.14159265358979 ) + 360;
		$Bearing -= 360 if( $Bearing >= 360 );
	}
	
	# NMEA の時刻を現在時に修正
	( $sec, $min, $hour, $mday, $mon, $year ) = gmtime( time );
	$Time = sprintf( '%02d%02d%02d', $hour, $min, $sec );
	$Date = sprintf( '%02d%02d%02d', $mday, $mon + 1, $year % 100 );
	
	$Knot = $Speed / 1.852;
	$LngStr = NmeaDeg( $Lng ) . ( $Lng >= 0 ? ",E" : ",W" );
	$LatStr = NmeaDeg( $Lat ) . ( $Lat >= 0 ? ",N" : ",S" );
	
	$_ =
		AddChksum( "\$GPGGA,$Time,$LatStr,$LngStr,1,12,0.5,10.0,M,,,," ) .
		AddChksum( "\$GPRMC,$Time,A,$LatStr,$LngStr,$Knot,$Bearing,$Date,,,A" );
	
	print( $fp $_ ) if( defined $fp );
	print "$idx:$_" if( $bDump );
	
	# キー処理
	while( 1 ){
		$Key = ReadKey( -1 );
		last if( !defined( $Key ));
		
		if( $Key eq 'd' ){
			$bDump = !$bDump;
			$CmdNum = 0;
		}elsif( $Key eq 's' ){
			$Speed = $CmdNum;
			print( "Spd = $Speed\n" );
			$CmdNum = 0;
		}elsif( $Key eq 'p' ){	Goto( $idx - 100 );
		}elsif( $Key eq 'n' ){	Goto( $idx + 100 );
		}elsif( $Key eq 'P' ){	Goto( $idx - 500 );
		}elsif( $Key eq 'N' ){	Goto( $idx + 500 );
		}elsif( $Key eq 'g' ){	Goto( $CmdNum );
		}elsif( $Key eq ' ' ){	$Stop = !$Stop; print( $Stop ? "stop\n" : "start\n" );
		}elsif( $Key eq '.' ){	print( ".=$idx\n" );
		}elsif( $Key =~ /\d/ ){ $CmdNum = $CmdNum * 10 + $Key;
		}elsif( $Key eq '?' ){	print( << 'EOF' );
Help
----
Idx ctrl: P:-500 p:-100 n:+100 N:+500
<num>g	: goto <num>
.	: current index
<num>s	: set speed to <num>
SPACE	: stop/start
d	: dump on/off
0-9	: input num
EOF
		}
	}
	
	sleep( 1 );
	
	( $Lng0, $Lat0 ) = ( $Lng, $Lat );
}

sub Goto {
	( $idx ) = @_;
	
	$idx = 0 if( $idx < 0 );
	$idx = $#Lng if( $idx > $#Lng );
	( $Lng, $Lat ) = ( $Lng[ $idx ], $Lat[ $idx ] );
	print( "goto $idx\n" );
	$CmdNum = 0;
}

sub AddChksum {
	local( $_ ) = @_;
	my( $Sum ) = 0;
	my( $idx );
	
	for( $idx = 1; $idx < length( $_ ); ++$idx ){
		$Sum ^= ord( substr( $_, $idx, 1 ));
	}
	
	return sprintf( "$_*%02X\n", $Sum );
}

sub NmeaDeg {
	local( $_ ) = @_;
	$_ = abs( $_ );
	return int( $_ ) * 100 + (( $_ - int( $_ )) * 60 );
}
