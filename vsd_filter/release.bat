@echo off

mkdir zrelease
mkdir zrelease\avisynth

copy "C:\Program Files\AVIUTL\vsd_filter.auf" zrelease\vsd_filter_gps.auf
copy "C:\Program Files\AviSynth 2.5\plugins\vsd_filter_avs.dll" zrelease\avisynth\vsd_filter_gps.dll

pushd zrelease

lha a -d ..\vsd_filter_gps.lzh *

popd

del /s/y zrelease
