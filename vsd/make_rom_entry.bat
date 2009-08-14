@echo off
set perlscr=%0 %*
set perlscr=%perlscr:\=\\\\%
C:\cygwin\bin\bash --login -i -c 'cd "%CD%";perl -x %perlscr%'
goto :EOF

#.tab=4
##############################################################################
#!/usr/bin/perl -w

open( fpIn, '< 300thew3/monitor/Release/MONITOR.MAP' );
open( fpOut, '> rom_entry.src' );

print fpOut<<'EOF';
	.CPU	300HN
	.SECTION    P,CODE,ALIGN=2
EOF

$Symbol = '';
$bSkip = 1;
while( <fpIn> ){
	s/[\x0D\x0A]//;
	
	if( / Symbol List / ){
		$bSkip = 0;
		next;
	}
	
	if( $bSkip ){
		next;
	}elsif( /^\s+(\$\S+)$/ ){
		$Symbol = $1;
	}elsif( $Symbol ne '' && /^\s+([\w\d]{8})\s/ ){
		print( fpOut "\t.EXPORT\t$Symbol\n$Symbol:\t.EQU\tH'$1\n" );
		$Symbol = '';
	}
}

print( fpOut "	.END\n" );
