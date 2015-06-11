#.tab=4

XToolRoot	= /home/yoshi/8.09/staging_dir/toolchain-mips_gcc4.1.2/bin
XTool		= PATH=$PATH:$(XToolRoot) mips-linux-uclibc-

all: seri2eth

seri2eth: seri2eth.c
	$(XTool)gcc -L$(XToolRoot)/mips/usr/lib -O1 -o $@ $<; \
	$(XTool)strip $@

clean:
	rm -f seri2eth seri_eth_fon
