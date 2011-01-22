#!/usr/bin/perl -w

$Mod = 50;	# 0.1…√√±∞Ã

while( <> ){
	print if( /([\d+]+.\d)/ && (( $1 * 10 ) % $Mod ) == 0 );
}
