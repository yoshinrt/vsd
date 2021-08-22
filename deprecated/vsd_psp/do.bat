@echo off

cd /d %~dp0\
copy ..\vsd\vsd\Release\vsd.mot vsd_new.mot > nul
luaplayer_win\luaplayer_com4.exe index.lua
