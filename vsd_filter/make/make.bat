:@echo off

pushd ..

set path=c:\cygwin\bin;C:\strawberry\c\bin;C:\strawberry\perl\site\bin;C:\strawberry\perl\bin;%PATH%

if exist c:\cygwin\bin\ (
	echo cygwin detected
	
	c:\cygwin\bin\make.exe
) else (
	echo cygwin not detected
	
	perl make/make_js_func.pl ScriptIF.h
	perl make/set_git_version.pl rev_num.h
)

popd
