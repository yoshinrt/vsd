:@echo off

pushd ..

if exist c:\cygwin\bin\ (
	echo cygwin detected
	
	c:\cygwin\bin\make.exe
) else (
	echo cygwin not detected
	
	set path=C:\strawberry\c\bin;C:\strawberry\perl\site\bin;C:\strawberry\perl\bin;%PATH%
	set path=c:\cygwin\bin
	
	perl make/make_js_func.pl ScriptIF.h
	perl make/set_git_version.pl rev_num.h
)

popd
