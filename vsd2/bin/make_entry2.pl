#!/usr/bin/perl -w

# $Id$

use strict 'vars';
use strict 'refs';

$_ = ();

# SYM list parser
open( fpIn,  "< $ARGV[ 0 ]" );

my( $VarName );
my( %Vars );

while( <fpIn> ){
	last if( /^Entry/ );
}
$_ = <fpIn>; # 読み捨て

while( <fpIn> ){
	s/[\x0D\x0A]//g;
	last if( $_ eq '' );
	
	$_ .= <fpIn> if( !/\]$/ );
	
	if( /^([\w_]+)\s+([\w]+)/ ){
		$VarName = $1;
		$Vars{ $VarName } = $2;
		
		printf( "%s=%s\n", $VarName, $Vars{ $VarName } );
	}
}

close( fpIn );
shift;

# .c parser
while( $ARGV[0] ){
	open( fpIn, "cpp -DHOGE $ARGV[0] |" );
	$_ .= join( '', <fpIn> );
	close( fpIn );
	shift( @ARGV );
}

s/#.*$//gm;
1 while( s/{[^{}]*}/#/g );

s/\b(?:typedef|struct|union|enum)\b[^;]*;/ /gs;
s/=[^;]*;/;/gs;
s/\n/ /g;
s/\s*[;#]\s*/\n/g;
s/\b__inline\s+//g;
s/\bINLINE\s+//g;
# ↓ROMENTRY にある関数で削りたいやつ
s/^.*\b(?:main)\b.*$//gm;
s/\n+/\n/g;
s/^\s+//g;
s/\s+$//g;

my( @Defs ) = split( /\n/, $_ );

#open( fpOut, "> hoge.h" );
open( fpOut, "> rom_entry.h" );

while( $_ = shift( @Defs )){
	s/([_\w\d\$]+)/&Replace($1)/ge;
	
	my( $Ptr ) = ( /\(/ ) ? '' : '*';
	
	if( ! /\$ALREADY_USED\$/ ){
		if( s/#(.*)#(.*)#/(*)/g ){
			print( fpOut "#define $1\t($Ptr($_)0x$2)\n" );
		}else{
			print( fpOut "// warning: not found: $_\n" );
		}
	}
}

close( fpOut );

sub Replace {
	local( $_ ) = @_;
	
	if( defined( $Vars{ $_ } )){
		
		my( $Addr ) = $Vars{ $_ };
		
		#undef( $Vars{ $_ } );
		$Vars{ $_ } = '$ALREADY_USED$';
		
		$_ .= '_ROM' if( $_ eq '_INITSCT' );
		
		return( "#$_#$Addr#" );
	}
	return( $_ );
}
