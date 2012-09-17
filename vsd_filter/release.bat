@echo off

set path=%path%;c:\cygwin\bin

mkdir zrelease
mkdir zrelease\avisynth

pushd zrelease

copy "D:\Program Files\AVIUTL\vsd_filter.auf" vsd_filter_gps.auf
copy "D:\Program Files\AviSynth 2.5\plugins\vsd_filter_avs.dll" avisynth\vsd_filter_gps.dll
copy ..\manual.url .
copy ..\split_nmea.hta .

mkdir vsd_plugins
cd vsd_plugins
xcopy /i "D:\Program Files\AVIUTL\vsd_plugins"

cd ..
d:\dds\bin\lha a -d ..\vsd_filter_gps.lzh *

popd

rmdir /s/q zrelease

perl -ne 'rename( "vsd_filter_gps.lzh", "vsd_filter_gps$1$2.lzh" ) if( /#define\s+PROG_VERSION\s+"v(.+)\.(.+)"/ );' CVsdFilter.h
