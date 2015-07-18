#.tab=4

.PHONY: rev_num.h
.SILENT:
.PHONY : release

export PATH := /bin:/cygdrive/d/DDS/bin:$(PATH)

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

SVN_PLUGINS	=	file:///home/yoshi/.svnrepos/vsd/trunk/vsd_filter/vsd_plugins
release:
	rm -rf zrelease; cp -r release_files zrelease; cd zrelease; \
	mv "/cygdrive/d/Program Files/AVIUTL/Plugins/vsd_filter.auf" vsd_filter_gps.auf; \
	svn export $(SVN_PLUGINS); \
	cd vsd_plugins; \
	rm -rf _log_reader/vsd_log.js gallery; \
	for a in `find . -name '*.js'`; do \
		perl -pe 's@.*//#DEL#.*\n@@; s@//(.*\S)\s*//#REL#@$$1@' $$a > $$a.z; mv $$a.z $$a; \
	done; \
#	cd ..; zip -9 -r ../vsd_filter_gps.zip *; \
#	cd ..; perl -ne 'rename( "vsd_filter_gps.zip", "vsd_filter_gps_r$$1.zip" ) if( /#define\s+PROG_REVISION\s+(\d+)/ );' rev_num.h; \
	cd ..; lha a -d ../vsd_filter_gps.lzh *; \
	cd ..; perl -ne 'rename( "vsd_filter_gps.lzh", "vsd_filter_gps_r$$1.lzh" ) if( /#define\s+PROG_REVISION\s+(\d+)/ );' rev_num.h; \
	rm -rf zrelease
