#!/usr/bin/perl -w
# SCRIPTER_OPT=-S6
# -*- tab-width: 4 -*-

$File = $ARGV[ 0 ];

# git リビジョンを得る
$Rev = split( /\n/, `git log --oneline` );
print "Commit:$Rev ";

# git 更新されているかを得る
$ModCnt = split( /\n/, `git diff --stat` );
if( $ModCnt > 0 ){
	++$Rev;
	
	print "Modified:$Rev ";
}

$PrevRev = 0;

# file 全リード
if( -e $File ){
	open( $fp, "< $File" );
	$_ = join( '', <$fp> );
	close( $fp );
	
	# バージョンコードを得る
	$PrevRev = +$1 if( /(\d+)/ );
}

	print "Prev:$PrevRev\n";

if( $PrevRev != $Rev ){	# rev 番号に差異がある
	
	# git diff がないが，何故か rev 番号がずれていた場合，
	# この修正により rev が一つ進む
	#++$Rev if( $ModCnt == 0 );
	
	print( "r$PrevRev -> $Rev updated\n" );
	
	open( $fpOut, "> ${File}" );
	print( {$fpOut} << "EOF" );
#define PROG_REVISION	$Rev
#define PROG_REVISION_STR	"r$Rev"
EOF
	close();
}
