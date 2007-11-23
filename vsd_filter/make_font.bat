@echo off

set path=%path%;C:/cygwin/bin;C:/cygwin/usr/bin

perl make_font.pl font_9p.bmp > font_9p.h
perl make_font.pl font_18p.bmp > font_18p.h

pause
