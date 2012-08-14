@echo off

mkdir zrelease
mkdir zrelease\avisynth

pushd zrelease

copy "D:\Program Files\AVIUTL\vsd_filter.auf" vsd_filter_gps.auf
copy "D:\Program Files\AviSynth 2.5\plugins\vsd_filter_avs.dll" avisynth\vsd_filter_gps.dll
copy ..\manual.url .
copy ..\split_nmea.hta .

mkdir vsd_skins
cd vsd_skins
xcopy /s /i "D:\Program Files\AVIUTL\vsd_skins"

cd ..
d:\dds\bin\lha a -d ..\vsd_filter_gps.lzh *

popd

rmdir /s/q zrelease

find "#define PROG_VERSION" CVsdFilter.h
pause
