# -*- tab-width: 4 -*-

.PHONY: rev_num.h
.SILENT:
.PHONY : release

export PATH := /bin:$(PATH)

all: ScriptIF.h rev_num.h

ScriptIF.h : \
	make/make_js_func.pl \
	CVsdFilter.h \
	CVsdImage.h \
	CVsdFont.h \
	CVsdFile.h \
	CScript.h
	./$< $@

rev_num.h:
	make/set_git_version.pl $@
