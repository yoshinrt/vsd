:@echo off
: -*- tab-width: 4 -*-

cd %~dp0

call "C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build\vcvars32.bat"

set path=c:\cygwin\bin;C:\strawberry\c\bin;C:\strawberry\perl\site\bin;C:\strawberry\perl\bin;C:\ProgramData\chocolatey\lib\zip\tools;%PATH%

msbuild /p:Configuration=ReleaseMTA /t:Rebuild dds_lib\dds_lib\dds_lib.vcxproj
if errorlevel 1 exit /b %errorlevel%
msbuild /p:Configuration=Release make\make.vcxproj
if errorlevel 1 exit /b %errorlevel%
msbuild /p:Configuration=ReleaseMT /t:Rebuild vsd_filter\vsd_filter.vcxproj
if errorlevel 1 exit /b %errorlevel%
msbuild /p:Configuration=ReleaseMT /t:Rebuild vsd_filter_avs\vsd_filter_avs.vcxproj
if errorlevel 1 exit /b %errorlevel%

set LANG=ja_JP.UTF-8

:release

rmdir /s/q zrelease
xcopy /e/i/y installer zrelease

pushd zrelease
	move vsd_filter_gps.exe ..
	mkdir plugins
	
	pushd plugins
		copy ..\..\vsd_filter\ReleaseMT\vsd_filter.dll vsd_filter_gps.auf
		copy ..\..\vsd_filter_avs\ReleaseMT\vsd_filter_avs.dll .
		
		xcopy /e/i/y ..\..\vsd_plugins vsd_plugins
		
		cd vsd_plugins
		rmdir /s/q gallery
		del /s/q _log_reader\vsd_log.js
		move _user_config.js.sample _user_config.js
	popd
	
	zip -9r ../vsd_filter_gps.exe *
	if errorlevel 1 exit /b %errorlevel%
popd

call :rename

if not exist c:\cygwin exit /b 0

if "%APPVEYOR%"=="" pause

rmdir /s/q zrelease
call :clean > nul 2>&1
exit /b 0

:=============================================================================
:rename
for /f "tokens=3" %%r in ( rev_num.h ) do (
	move vsd_filter_gps.exe vsd_filter_gps_r%%r.exe
	exit /b 0
)

:=============================================================================
:clean
rmdir /s/q vsd_filter\Release
rmdir /s/q vsd_filter\ReleaseMT
rmdir /s/q vsd_filter\Debug
rmdir /s/q vsd_filter_avs\Release
rmdir /s/q vsd_filter_avs\ReleaseMT
rmdir /s/q vsd_filter_avs\Debug
rmdir /s/q dds_lib\dds_lib\Win32
