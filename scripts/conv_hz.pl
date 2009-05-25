#!/usr/bin/perl -w

# ログのサンプリング周波数を変更するスクリプト

$InHz  = 16030000 / 65536 / 16;
$OutHz = 16;

# read
while( <> ){
	s/[\x0D\x0A]//g;
	push( @Log, [ split( /\t/, $_ ) ] );
}

$OutLogCnt = $#Log / $InHz * $OutHz;

for( $i = 0; $i < $OutLogCnt; ++$i ){
	# ログ位置計算
	$LogPos = $i / $OutHz * $InHz;
	$LogPosInt = int( $LogPos );
	$LogPos    -= $LogPosInt;
	
	# 中間値計算
	printf( "%d\t%.2f\t%.2f\t%.3f\t%.3f",
		$Log[ $LogPosInt ][ 0 ] * ( 1 - $LogPos ) + $Log[ $LogPosInt + 1 ][ 0 ] * $LogPos,
		$Log[ $LogPosInt ][ 1 ] * ( 1 - $LogPos ) + $Log[ $LogPosInt + 1 ][ 1 ] * $LogPos,
		$Log[ $LogPosInt ][ 2 ] * ( 1 - $LogPos ) + $Log[ $LogPosInt + 1 ][ 2 ] * $LogPos,
		$Log[ $LogPosInt ][ 3 ] * ( 1 - $LogPos ) + $Log[ $LogPosInt + 1 ][ 3 ] * $LogPos,
		$Log[ $LogPosInt ][ 4 ] * ( 1 - $LogPos ) + $Log[ $LogPosInt + 1 ][ 4 ] * $LogPos
	);
	
	if( defined( $Log[ $LogPosInt ][ 6 ] )){
		@_ = @{ $Log[ $LogPosInt ] };
		splice( @_, 0, 6 );
		splice( @{ $Log[ $LogPosInt ] }, 6 );
		print( "\t" . join( "\t", @_ ));
	}
	
	print( "\r\n" );
}
