#.tab=4

.PHONY: rev_num.h
.SILENT:
.PHONY : release

export PATH := /bin:$(PATH)

all: ScriptIF.h rev_num.h

ScriptIF.h : \
	make_js_func.pl \
	CVsdFilter.h \
	CVsdImage.h \
	CVsdFont.h \
	CVsdFile.h \
	CScript.h
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

SVN	=	file:///home/yoshi/.svnrepos/vsd/trunk/vsd_filter/vsd_plugins
release:
	rm -rf zrelease; cp -r release_files zrelease; cd zrelease; \
	cp "/cygdrive/d/Program Files/AVIUTL/Plugins/vsd_filter.auf" vsd_filter_gps.auf; \
	svn export -N $(SVN); \
	svn export -N $(SVN)/_log_reader \
		vsd_plugins/_log_reader; \
	cd vsd_plugins; \
	rm _log_reader/vsd_log.js; \
	for a in `find . -name '*.js'`; do \
		perl -pe 's@.*//#DEL#.*\n@@; s@//(.*\S)\s*#REL#@$$1@' $$a > $$a.z; mv $$a.z $$a; \
	done
