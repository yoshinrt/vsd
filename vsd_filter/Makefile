.PHONY: rev_num.h
.SILENT:

export PATH := /bin:$(PATH)

all: ScriptIF.h rev_num.h

ScriptIF.h : \
	make_js_func.pl \
	CVsdFilter.cpp \
	CVsdFilterDraw.cpp \
	CVsdImage.cpp \
	CVsdFont.cpp \
	CVsdFile.cpp \
	CScript.cpp
	./$< $@

rev_num.h:
	export LANG; \
	rev=`LANG=C svn info -r HEAD | grep Revision | sed -r 's/Revision:\s*//'`; \
	echo "#define PROG_REVISION	$$rev"		> $@.$$$$; \
	echo '#define PROG_REVISION_STR	"r'$$rev'"'	>> $@.$$$$; \
	if [ ! -e $@ ]; then \
		mv $@.$$$$ $@; \
	elif diff -q $@.$$$$ $@ > /dev/null; then \
		rm $@.$$$$; \
	else \
		mv $@.$$$$ $@; \
		echo $@ updated; \
	fi
