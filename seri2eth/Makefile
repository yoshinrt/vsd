#.tab=4

XToolRoot	= /home/yoshi/ddwrt/mips/staging_dir
XTool		= PATH=$PATH:$(XToolRoot)/toolchain-mips_gcc4.1.2/bin/ mips-linux-

all: seri2eth

seri2eth: seri2eth.c
	$(XTool)uclibc-gcc -L$(XToolRoot)/mips/usr/lib -O1 -o $@ $<; \
	$(XTool)strip $@

clean:
	rm seri2eth seri_eth_fon
