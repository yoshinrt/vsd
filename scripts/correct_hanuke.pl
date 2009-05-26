#!/usr/bin/perl -w

# ªı»¥§±•«°º•ø§Ú ‰¥∞§π§Î

# read
while( <> ){
	s/[\x0D\x0A]//g;
	push( @Log, [ split( /\t/, $_ ) ] );
}

PrintLog( 0 );

for( $i = 1; $i < $#Log; ++$i ){
	if(
		MileDiff( $i ) > 0 &&
		( MileDiff( $i - 1 ) + MileDiff( $i + 1 )) > 0
	){
		if( 0.8 <= MileDiff( $i ) / ( MileDiff( $i - 1 ) + MileDiff( $i + 1 ))){
			printf( "%d\t%.2f\t%.2f\t%.3f\t%.3f\r\n",
				( $Log[ $i - 1 ][ 0 ] + $Log[ $i ][ 0 ] ) / 2,
				( $Log[ $i - 1 ][ 1 ] + $Log[ $i ][ 1 ] ) / 2,
				( $Log[ $i - 1 ][ 2 ] + $Log[ $i ][ 2 ] ) / 2,
				( $Log[ $i - 1 ][ 3 ] + $Log[ $i ][ 3 ] ) / 2,
				( $Log[ $i - 1 ][ 4 ] + $Log[ $i ][ 4 ] ) / 2
			);
		}
	}
	PrintLog( $i );
}

PrintLog( $i );

sub MileDiff {
	local( $_ ) = @_;
	return $Log[ $_ ][ 2 ] - $Log[ $_ - 1 ][ 2 ];
}

sub PrintLog {
	local( $_ ) = @_;
	print join( "\t", @{ $Log[ $_ ] } ) . "\r\n";
}
