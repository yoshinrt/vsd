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
		
		#printf( "%s=%s\n", $VarName, $Vars{ $VarName } );
	}
}

close( fpIn );
shift;

# .c parser
while( $ARGV[0] ){
	#open( fpIn, "cpp -DHOGE $ARGV[0] |" );
	open( fpIn, "< $ARGV[0]" );
	$_ .= join( '', <fpIn> );
	close( fpIn );
	shift( @ARGV );
}

# lib から読まれる関数を追加定義
$_ .= <<'EOF';
int printf( const char *fmt, ... );
int sprintf( const char *fmt, ... );
EOF

s|/\*.*?\*/| |gs; s|//.*||gm;						# コメント削除
s/#.*$//gm;											# cpp ディレクティブ削除
1 while( s/{[^{}]*}/#/g );							# { ... } を # に置換
s/\b(?:typedef|struct|union|enum)\b[^;]*;/ /gs;		# struct 宣言等を削除
s/=[^;]*;/;/gs;										# 変数宣言の = 以降を削除
s/[\s\n]+/ /g; s/\s*[;#]\s*/\n/g;					# 改行しなおし
s/^\s+//g; s/\s+$//g;								# 先頭・最後のスペース削除
s/\b__inline\s+//g; s/\bINLINE\s+//g;				# インライン関数削除

# ↓ROMENTRY にある関数で削りたいやつ
s/^.*\b(?:main)\b.*$//gm;
s/\n+/\n/g;
s/^\s+//g;
s/\s+$//g;

my( @Defs ) = split( /\n/, $_ );

#open( fpOut, "> hoge.h" );
open( fpOut, "> rom_entry.h" );

#my( $Ptr );

while( $_ = shift( @Defs )){
	s/([_\w\d\$]+)/&Replace($1)/ge;
	
	#$Ptr = ( /\(/ ) ? '' : '*';
	
	if( ! /\$ALREADY_USED\$/ ){
		if( s/#(.*)#(.*)#/(*)/g ){
			print( fpOut "#define $1\t(*($_)$2)\n" );
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
		
		#$_ .= '_ROM' if( $_ eq '_INITSCT' );
		
		return( "#$_#$Addr#" );
	}
	return( $_ );
}
