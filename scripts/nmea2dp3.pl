#!/bin/perl -w

# ヘッダ作成
$Buf = "\x00" x 256;

substr( $Buf, 0x1 , 1 )		= "\x01";
substr( $Buf, 0x10, 0x10 )	= '0' x 16;
substr( $Buf, 0x81, 1 )		= "\x01";
substr( $Buf, 0x88, 6 )		= "\xFF\xDE\x32\x01\x00\x01";

$FileName = $ARGV[ 0 ];

if( $FileName =~ /\.gz$/ ){
	open( fpIn, "gunzip -c $FileName |" );
}else{
	open( fpIn, "< $FileName" );
}

# データ変換
while( <fpIn> ){
	if( /GPRMC/ ){
		# $GPRMC,042246.600,A, 3604.729800,N,13631.146238,E,0.763,  153.25,   031111,,,A*56
		( undef, $Time, undef, $Lati, undef, $Long, undef,  $Speed, $Bearing ) = split( /,/, $_ );
		
		if(( $Time += 90000 ) >= 240000 ){
			$Time -= 240000;
		}
		
		$_ = pack(
			'SSIII',
			int( $Bearing ),
			int( $Speed * 10 * 1.85200 ),
			int(( int( $Lati / 100 ) + fmod( $Lati, 100 ) / 60 ) * 460800 ),
			int(( int( $Long / 100 ) + fmod( $Long, 100 ) / 60 ) * 460800 ),
			int( $Time * 10 )
		);
		
		$Buf .= reverse( $_ );
	}
}
# ファイル出力

$_ = pack( 'I', length( $Buf ) + 0x10 );
substr( $Buf, 0x0C, 4 ) = $_;
substr( $Buf, 0x84, 4 ) = $_;

open( fpOut, "> $FileName.dp3" );
print( fpOut $Buf );
close( fpOut );

sub fmod {
	my( $a, $b ) = @_;
	
	return $a - int( $a / $b ) * $b;
}
