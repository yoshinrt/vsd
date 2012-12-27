@echo off
set perlscr=%0 %*
set perlscr=%perlscr:\=/%
C:\cygwin\bin\bash --login -i -c 'cd "%CD%";CYGWIN=nodosfilewarning perl -x %perlscr%'
goto :EOF

##############################################################################
#!/usr/bin/perl -w
# .tab=4

$ENV{ 'LANG' } = 'C';
$_ = `svn info`;
/^Revision: (\d+)/m;

$NewNum = $1;

if( -e "rev_num.h" ){
	$_ = `cat rev_num.h`;
	/"(\d+)"/;
	$OldNum = $1;
}else{
	$OldNum = -1;
}

if( $OldNum != $NewNum ){
	open( FP, "> rev_num.h" );
	print( FP "#define PROG_REVISION\t\"r$1\"\n" );
}
