@echo off
set perlscr=%0 %*
set perlscr=%perlscr:\=\\\\%
C:\cygwin\bin\bash --login -i -c 'cd "%CD%";perl -x %perlscr%'
goto :EOF

#.tab=4
##############################################################################
#!/usr/bin/perl -w

# $Id$

use strict 'vars';
use strict 'refs';

$_ = ();

@ARGV = ( '300thew3/monitor/Release/MONITOR.MAP', 'main2.c', 'sci.c' );

# SYM list parser
open( fpIn,  "< $ARGV[ 0 ]" );

while( <fpIn> ){
	last if( /^  SYMBOL        / );
}

my( $VarName );
my( %Vars );

while( <fpIn> ){
	if( /^  _?([\w\d_\$]+)[\n\r]*$/ ){
		$VarName = $1;
		$_ = <fpIn>;
		/^\s+(\S+)/;
		$Vars{ $VarName } = $1;
		
		#printf( "%s=%s\n", $VarName, $Vars{ $VarName } );
	}
}

close( fpIn );
shift;

# .c parser
while( $ARGV[0] ){
	open( fpIn, "cpp -DMONITOR_ROM $ARGV[0] |" );
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
# «ROMENTRY ‚É‚ ‚éŠÖ”‚Åí‚è‚½‚¢‚â‚Â
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
