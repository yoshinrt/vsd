#!/usr/bin/perl
# VSD ログフォーマットをヘッダ・タイムスタンプ付きに更新するスクリプト

use Time::Local;

$name = $ARGV[ 0 ];
$name =~ s/\.gz$//;

open( fpIn, "gunzip -c $ARGV[ 0 ] |" );
open( fpOut, "> $name" );

$bGPS = 0;
$bLap = 0;
$bSector = 0;

$cnt = 0;

# base time
$ARGV[ 0 ] =~ /(\d\d\d\d)(\d\d)(\d\d)_(\d\d)(\d\d)(\d\d)/;
$time = timelocal( $6, $5, $4, $3, $2 - 1, $1 - 1900 );

while( <fpIn> ){
	
	s/[\x0D\x0A]//g;
	
	if( /GPS\s+(\S+)\s+(\S+)/ ){
		$bGPS = 1;
		$GPS[ $cnt ] = [ $1, $2 ];
	}
	
	if( /LAP\d+\s+(\S+)/ ){
		$bLap = 1;
		$Lap[ $cnt ] = $1 eq 'start' ? "0:00.000" : $1;
	}
	
	if( /Sector\d+\s+(\S+)/ ){
		$bSector = 1;
		$Sector[ $cnt ] = $1;
	}
	
	s/(?:LAP|GPS|Sector).*//;
	s/\s+$//;
	
	$Data[ $cnt ] = [ split( /\t/, $_ ) ];
	++$cnt;
}

print fpOut "Date/Time\tTacho\tSpeed\tDistance";
print fpOut "\tGy\tGx" if( $#{ $Data[ 0 ] } >= 4 );
print fpOut "\tAuxInfo" if( $#{ $Data[ 0 ] } >= 5 );
print fpOut "\tLapTime" if( $bLap );
print fpOut "\tSectorTime" if( $bSector );
print fpOut "\tLatitude\tLongitude" if( $bGPS );
print fpOut "\n";

$cnt = 0;
foreach $_ ( @Data ){
	( $sec, $min, $hour, $day, $mon, $year ) = gmtime( $time );
	
	printf(
		fpOut "%04d-%02d-%02dT%02d:%02d:%02d.%03dZ\t",
		$year + 1900, $mon + 1, $day, $hour, $min, $sec,
		int(( $cnt % 16 ) * 1000 / 16 )
	);
	print fpOut join( "\t", @{ $_ } );
	print fpOut "\t$Lap[ $cnt ]" if( $bLap );
	print fpOut "\t$Sector[ $cnt ]" if( $bSector );
	print fpOut "\t$GPS[ $cnt ][ 0 ]\t$GPS[ $cnt ][ 1 ]" if( $bGPS );
	print fpOut "\n";
	
	++$cnt;
	++$time if( $cnt % 16 == 0 );
}

system << "EOF";
rm $name.gz
gzip -9 $name
EOF
