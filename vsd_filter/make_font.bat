@echo off

set path=%path%;C:/cygwin/bin

perl make_font.pl font_9p.bmp > font_9p.h
