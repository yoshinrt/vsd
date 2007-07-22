@echo off

set path=%path%;C:/cygwin/bin;C:/cygwin/usr/bin

:perl make_entry.pl 300thew3/monitor/Release/MONITOR.MAP rom_entry.h
perl make_entry2.pl 300thew3/monitor/Release/MONITOR.MAP main2.c sci.c> log
