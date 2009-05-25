#!/usr/bin/perl -w

# 歯抜けデータを保管する

@Line = ();
$PrevMileage = 0;

while( <> ){
	push( @Line, $_ );
	
	/^\S+\s+\S+\s+(\S+)\s+/;
	push( @MileageDiff, $1 - $PrevMileage );
	$PrevMileage = $1;
}

print $Line[ 0 ];

for( $i = 1; $i < $#Line; ++$i ){
	$tmp = 0;
	if(
		$MileageDiff[ $i ] > 0 &&
		( $MileageDiff[ $i - 1 ] + $MileageDiff[ $i + 1 ] ) > 0
	){
		$tmp = $MileageDiff[ $i ] / ( $MileageDiff[ $i - 1 ] + $MileageDiff[ $i + 1 ] );
		if( 0.8 <= $tmp ){
			@a = split( /\t/, $Line[ $i - 1 ] );
			@b = split( /\t/, $Line[ $i ] );
			
			printf( "%d\t%.2f\t%.2f\t%.3f\t%.3f\r\n",
				( $a[ 0 ] + $b[ 0 ] ) / 2,
				( $a[ 1 ] + $b[ 1 ] ) / 2,
				( $a[ 2 ] + $b[ 2 ] ) / 2,
				( $a[ 3 ] + $b[ 3 ] ) / 2,
				( $a[ 4 ] + $b[ 4 ] ) / 2
			);
		}
	}
	$_ = $Line[ $i ];
#	s/[\x0D\x0A]//g;
#	print "$_\t$MileageDiff[$i]\t$tmp\r\n";
	print $Line[ $i ];
}

print $Line[ $i ];
