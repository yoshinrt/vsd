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
xcopy /i/s "D:\Program Files\AVIUTL\vsd_plugins" vsd_plugins

rmdir /s/q vsd_plugins\negi
del vsd_plugins\_log_reader\vsd_log.js
move vsd_plugins\google_maps_release.js vsd_plugins\google_maps.js

d:\dds\bin\lha a -d ..\vsd_filter_gps.lzh *

popd

rmdir /s/q zrelease

perl -ne 'rename( "vsd_filter_gps.lzh", "vsd_filter_gps_$1.lzh" ) if( /#define\s+PROG_REVISION\s+"(r\d+)"/ );' rev_num.h

pause
