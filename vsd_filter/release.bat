@echo off
: -*- tab-width: 4 -*-

set path=%path%;c:\cygwin\bin

call "D:\Program Files (x86)\Microsoft Visual Studio 14.0\VC\bin\vcvars32.bat"
msbuild /p:Configuration=Release make\make.vcxproj
msbuild /p:Configuration=ReleaseMT /t:Rebuild vsd_filter\vsd_filter.vcxproj
msbuild /p:Configuration=ReleaseMT /t:Rebuild vsd_filter_avs\vsd_filter_avs.vcxproj

set LANG=ja_JP.UTF-8
make release

rmdir /s/q vsd_filter_avs\Release
rmdir /s/q vsd_filter_avs\ReleaseMT
rmdir /s/q vsd_filter_avs\Debug
rmdir /s/q vsd_filter\Release
rmdir /s/q vsd_filter\ReleaseMT
rmdir /s/q vsd_filter\Debug

pause
