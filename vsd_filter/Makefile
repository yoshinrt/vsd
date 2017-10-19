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
	rev=$$((`git log --oneline | wc -l` + 44)); \
	if [ `git diff --stat | wc -l` != 0 ]; then rev=$$((rev + 1)); fi; \
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

release:
	rm -rf zrelease; cp -r installer zrelease; \
	cd zrelease; \
		mv vsd_filter_gps.exe ..; \
		mkdir plugins; \
	cd plugins; \
		cp ../../vsd_filter/ReleaseMT/vsd_filter.auf vsd_filter_gps.auf; \
		cp ../../vsd_filter_avs/ReleaseMT/vsd_filter_avs.dll .; \
		( cd ../../; git archive --format tar HEAD vsd_plugins ) | tar -x; \
	cd vsd_plugins; \
		rm -rf _log_reader/vsd_log.js gallery; \
		mv _user_config.js.sample _user_config.js; \
	cd ../../; \
		lha a -d ../vsd_filter_gps.exe *; \
	cd ..; \
		perl -ne 'rename( "vsd_filter_gps.exe", "vsd_filter_gps_r$$1.exe" ) if( /#define\s+PROG_REVISION\s+(\d+)/ );' rev_num.h
	rm -rf zrelease
