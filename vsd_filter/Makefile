# -*- tab-width: 4 -*-

.PHONY: rev_num.h
.SILENT:
.PHONY : release

export PATH := /bin:$(PATH)

all: ScriptIF.h ScriptIF.cpp rev_num.h

ScriptIF.h : \
	make/make_js_func.pl \
	CVsdFilter.h \
	CVsdImage.h \
	CVsdFont.h \
	CVsdFile.h \
	CScript.h
	./$< $@

ScriptIF.cpp : \
	make/make_js_func_qjs.pl \
	CVsdFilter.h \
	CVsdImage_qjs.h \
	CVsdFont.h \
	CVsdFile_qjs.h \
	CScript_qjs.h
	./$< $@

rev_num.h:
	make/set_git_version.pl $@
