@echo off

mkdir zrelease
mkdir zrelease\avisynth

pushd zrelease

copy "C:\Program Files\AVIUTL\vsd_filter.auf" vsd_filter_gps.auf
copy "C:\Program Files\AviSynth 2.5\plugins\vsd_filter_avs.dll" avisynth\vsd_filter_gps.dll
copy ..\manual.url .

lha a -d ..\vsd_filter_gps.lzh *

popd

rmdir /s/q zrelease
