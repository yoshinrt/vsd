#!/usr/bin/perl -w

my( $ExceptList );
my( %ExceptList );

## ビルドログから重複するシンボルエラーを pick up
if( open( $fp, "../exec_sram/List/vsd2.map" )){
	while( <$fp> ){
		if( /duplicate definitions for "(.*)"/ ){
			$ExceptList{ $1 } = 1;
		}
	}
	close( $fp );
}

## 除外する関数名
$ExceptList = <<'EOF';
main
_main
__main
__vector_table
__iar_program_start
_exit
EOF

foreach ( split( /\n/, $ExceptList )){
	$ExceptList{ $_ } = 1;
}

$_ = ();

## 除外する obj 名

my( $ObjList, %ObjList );
$ObjList = <<'EOF';
#cortexm3_macro.o
hw_config.o
main.o
#stm32f10x_flash.o
#stm32f10x_gpio.o
#stm32f10x_it.o
#stm32f10x_nvic.o
#stm32f10x_rcc.o
#stm32f10x_usart.o
stm32f10x_vector.o
#usb_core.o
#usb_desc.o
#usb_endp.o
#usb_init.o
#usb_int.o
#usb_istr.o
#usb_mem.o
#usb_prop.o
#usb_pwr.o
#usb_regs.o
#div.o
#exit.o
low_level_init.o
#sprintf.o
#xdnorm.o
#xdscale.o
#xprintffull.o
#xsprout.o
#ABImemcpy.o
#DblAdd.o
#DblCmpGe.o
#DblCmpLe.o
#DblDiv.o
#DblMul.o
#DblSub.o
#DblSubNrml.o
#DblToI32.o
#I32DivMod.o
#I32ToDbl.o
#I64DivMod.o
#I64DivZer.o
#IntDivZer.o
#cexit.o
cmain.o
copy_init.o
data_init.o
#memchr.o
#strchr.o
#strlen.o
zero_init.o
EOF

$ObjList =~ s/^#.*\n//gm;

foreach ( split( /\n/, $ObjList )){
	$ObjList{ $_ } = '';
}

# SYM list parser

my( $VarName );
my( %Vars );

open( $fp, "../Release/List/vsd2.map" );
while( <$fp> ){
	last if( /^Entry/ );
}
$_ = <$fp>; # 読み捨て

open( fpOut, "> ../src/rom_entry.s" );

print fpOut <<"EOF";
;*****************************************************************************
;	
;	VSD2 - vehicle data logger system2
;	Copyright(C) by DDS
;	
;	rom_entry.s -- rom entry point table
;	
;*****************************************************************************

	RSEG ROM_CODE:CODE(2)
EOF

my( $tmp );
my( @SymbolList );

while( <$fp> ){
	s/[\x0D\x0A]//g;
	last if( $_ eq '' );
	
	# 2行に別れてるやつを結合
	if( !/[\-\]]$/ ){
		$tmp = <$fp>;
		substr( $tmp, 34, 17 ) = ' ';
		$_ .= $tmp;
	}else{
		substr( $_, 34, 17 ) = ' ';
	}
	
	if(
		/^([\w_]+)\s+0x([\w]+)\s+(\S+)/ &&
		!defined( $ObjList{ $3 } ) &&
		!defined( $ExceptList{ $1 } )
	){
		push( @SymbolList, "$2\t$1" );
	}
}

$CurPC = 0x080030EC;

for ( sort @SymbolList ){
	( $Addr, $Symbol ) = split( /\t/, $_ );
	
	if( $CurPC < 0x20000000 && hex( $Addr ) >= 0x20000000 ){
		print( fpOut "\n\tRSEG FLASH_DATA:DATA(4)\n" );
		$CurPC = hex( $Addr );
	}
	
	$Offset = sprintf( "0x%X", hex( $Addr ) - $CurPC );
	$CurPC	= hex( $Addr );
	print fpOut << "EOF";
	DS	$Offset

	PUBWEAK	$Symbol
$Symbol:\t// 0x$Addr
EOF
}

print fpOut "\tEND\n";
