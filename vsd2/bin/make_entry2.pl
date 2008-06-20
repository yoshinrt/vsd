#!/usr/bin/perl -w

# $Id$

my( $ExceptList );
my( %ExceptList );

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
	$ExceptList{ $_ } = '';
}

$_ = ();

## 使用する obj 名

my( $ObjList, %ObjList );
$ObjList = <<'EOF';
#cortexm3_macro.o
#hw_config.o
#main.o
stm32f10x_flash.o
stm32f10x_gpio.o
stm32f10x_it.o
stm32f10x_nvic.o
stm32f10x_rcc.o
stm32f10x_usart.o
#stm32f10x_vector.o
usb_core.o
#usb_desc.o
#usb_endp.o
#usb_init.o
#usb_int.o
#usb_istr.o
#usb_mem.o
#usb_prop.o
#usb_pwr.o
#usb_regs.o
div.o
exit.o
#low_level_init.o
sprintf.o
xdnorm.o
xdscale.o
xprintffull.o
xsprout.o
ABImemcpy.o
DblAdd.o
DblCmpGe.o
DblCmpLe.o
DblDiv.o
DblMul.o
DblSub.o
DblSubNrml.o
DblToI32.o
I32DivMod.o
I32ToDbl.o
I64DivMod.o
I64DivZer.o
IntDivZer.o
cexit.o
#cmain.o
#copy_init.o
#data_init.o
memchr.o
strchr.o
strlen.o
#zero_init.o
exit.o
EOF

$ObjList =~ s/^#.*\n//gm;

foreach ( split( /\n/, $ObjList )){
	$ObjList{ $_ } = '';
}

# SYM list parser

my( $VarName );
my( %Vars );
while( <> ){
	last if( /^Entry/ );
}
$_ = <>; # 読み捨て

open( fpOut, "> rom_entry.s" );

$IdString = '$I' . 'd$';

print fpOut <<"EOF";
;*****************************************************************************
;	
;	VSD2 - vehicle data logger system2
;	Copyright(C) by DDS
;	
;	rom_entry.s -- rom entry point table
;	$IdString
;	
;*****************************************************************************

	RSEG ROM_CODE:CODE(2)
EOF

my( $tmp );
my( @SymbolList );

while( <> ){
	s/[\x0D\x0A]//g;
	last if( $_ eq '' );
	
	# 2行に別れてるやつを結合
	if( !/[\-\]]$/ ){
		$tmp = <>;
		substr( $tmp, 34, 17 ) = ' ';
		$_ .= $tmp;
	}else{
		substr( $_, 34, 17 ) = ' ';
	}
	
	if(
		/^([\w_]+)\s+([\w]+)\s+(\S+)/ &&
		defined( $ObjList{ $3 } ) &&
		!defined( $ExceptList{ $1 } )
	){
		push( @SymbolList, "$3\t$1\t$2" );
	}
}

$PrevObj = '';

for ( sort @SymbolList ){
	@_ = split( /\t/, $_ );
	
	if( $PrevObj ne $_[ 0 ] ){
		print fpOut ";*** $_[ 0 ] " . '*' x ( 72 - length( $_[ 0 ] )) . "\n";
		$PrevObj = $_[ 0 ];
	}
	print fpOut "\tEXPORT $_[ 1 ]\n$_[ 1 ] = $_[ 2 ]\n";
}

print fpOut "\tEND\n";
