@echo off
: .tab=4

set path=%path%;c:\cygwin\bin
del "D:\Program Files\AVIUTL\Plugins\vsd_filter.auf"

call "D:\Program Files (x86)\Microsoft Visual Studio 14.0\VC\bin\vcvars32.bat"
msbuild /p:Configuration=Release make\make.vcxproj
msbuild /p:Configuration=ReleaseMT /t:Rebuild vsd_filter\vsd_filter.vcxproj

set LANG=ja_JP.UTF-8
make release

msbuild /p:Configuration=Release vsd_filter\vsd_filter.vcxproj

pause
