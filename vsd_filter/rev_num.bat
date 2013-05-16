@echo off
set perlscr=%0 %*
set perlscr=%perlscr:\=/%
C:\cygwin\bin\bash --login -i -c 'cd "%CD%";CYGWIN=nodosfilewarning perl -x %perlscr%'
goto :EOF

##############################################################################
#!/usr/bin/perl -w
# .tab=4

$ENV{ 'LANG' } = 'C';
$_ = `svn info -r HEAD`;
/^Revision: (\d+)/m;

$NewNum = $1;

if( -e "rev_num.h" ){
	$_ = `cat rev_num.h`;
	/(\d+)/;
	$OldNum = $1;
}else{
	$OldNum = -1;
}

if( $OldNum != $NewNum ){
	open( FP, "> rev_num.h" );
	print( FP << "EOF" );
#define PROG_REVISION		$NewNum
#define PROG_REVISION_STR	"r$NewNum"
EOF
}

if( $#ARGV >= 0 ){
	system( join( " ", @ARGV ));
}
