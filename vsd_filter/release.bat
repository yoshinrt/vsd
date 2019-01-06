@echo off
: -*- tab-width: 4 -*-

call "C:\Program Files (x86)\Microsoft Visual Studio\2017\Community\VC\Auxiliary\Build\vcvars32.bat"

set path=%path%;c:\cygwin\bin

msbuild /p:Configuration=ReleaseMT /t:Rebuild dds_lib\dds_lib\dds_lib.vcxproj
msbuild /p:Configuration=Release make\make.vcxproj
msbuild /p:Configuration=ReleaseMT /t:Rebuild vsd_filter\vsd_filter.vcxproj
msbuild /p:Configuration=ReleaseMT /t:Rebuild vsd_filter_avs\vsd_filter_avs.vcxproj

set LANG=ja_JP.UTF-8
make release

call :clean > nul 2>&1
pause
exit /b 0

:clean
rmdir /s/q Release
rmdir /s/q ReleaseMT
rmdir /s/q Debug
rmdir /s/q vsd_filter_avs\Release
rmdir /s/q vsd_filter_avs\ReleaseMT
rmdir /s/q vsd_filter_avs\Debug
rmdir /s/q vsd_filter\Release
rmdir /s/q vsd_filter\ReleaseMT
rmdir /s/q vsd_filter\Debug
rmdir /s/q dds_lib\dds_lib\Release
rmdir /s/q dds_lib\dds_lib\ReleaseMT
rmdir /s/q dds_lib\dds_lib\Debug
