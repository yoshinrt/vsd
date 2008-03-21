#!/usr/bin/perl

open( fpIn, "od -t x1 -v $ARGV[0] |" );
open( fpOut, ">$ARGV[1]" );
while( <fpIn> ){
	s/[\x0D\x0A]//g;
	s/^[^ ]+ *//g;
	next if( $_ eq '' );
	s/ /,0x/g;
	s/$/,/g;
	s/^/0x/;
	print fpOut "$_\n";
}
