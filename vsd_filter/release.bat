@echo off

set path=%path%;c:\cygwin\bin

mkdir zrelease
pushd zrelease

copy "D:\Program Files\AVIUTL\Plugins\vsd_filter.auf" vsd_filter_gps.auf
copy ..\manual.url .
copy ..\split_nmea.hta .

mkdir vsd_plugins
xcopy /i/s "D:\Program Files\AVIUTL\Plugins\vsd_plugins" vsd_plugins

rmdir /s/q vsd_plugins\gallery
del vsd_plugins\_log_reader\vsd_log.js

: API ƒL[íœ
perl -pe 's/AIzaSyCcW8fww9nc2dP6BceauZPfnKHEamj8Fu4//' ../vsd_plugins/google_maps.js > vsd_plugins/google_maps.js

d:\dds\bin\lha a -d ..\vsd_filter_gps.lzh *

popd

rmdir /s/q zrelease

perl -ne 'rename( "vsd_filter_gps.lzh", "vsd_filter_gps_r$1.lzh" ) if( /#define\s+PROG_REVISION\s+(\d+)/ );' rev_num.h

pause
