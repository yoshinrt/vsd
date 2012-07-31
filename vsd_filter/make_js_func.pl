#!/usr/bin/perl

open( fpIn,			"< CVsdFilter.h" );
open( fpFunc,		"> CVsdFilterJsFunc.h" );
open( fpFuncProto,	"> def_vsd_func.h" );
open( fpVar,		"> def_vsd_var.h" );

@Var  = ();

while( <fpIn> ){
	if( /!js_func\b/ ){
		
		# ´Ø¿ôÌ¾
		/([\w_]+)\s*\(/;
		$FuncName = $1;
		
		print( fpFuncProto "DEF_SCR_FUNC( $FuncName )\n" );
		
		$ArgNum = 0;
		$ArgMin = 0;
		@Args = ();
		@Defs = ();
		
		$Line = $_;
		
		# °ú¿ô
		if( $Line !~ /\)(?:\s*=\s*0\s*)?;/ ){
			while( <fpIn> ){
				$Line .= $_;
				last if( $Line =~ /\)(?:\s*=\s*0\s*)?;/ );
			}
		}
		$_ = $Line;
		s/[\x0D]//g;
		s/^.*?\([^\)]*?\n// || s/^.*?\(//;
		s/[\n\s]*\).*$//s;
		s/,\s*/\n/g;
		s#\n\s*//# //#g;
		
		@Line = split( /\n/, $_ );
		
		foreach $_ ( @Line ){
			/(\S+)/;	# ·¿
			$Type = $1;
			
			$Default = /!default:(\S+)/ ? $1 : undef;
			$ArgPos  = /!arg:(\d+)/ ? $1 : $ArgNum;
			
			if( $Type eq 'CVsdImage' ){
				push( @Defs, "v8::Local<v8::Object> img$ArgNum = args[ $ArgPos ]->ToObject();" );
				push( @Defs, "CheckClass( img$ArgNum, \"Image\", \"arg[ $ArgPos ] must be Image\" );" );
				$Args[ $ArgNum ] = "*CVsdImage::GetThis( img$ArgNum )";
			}
			
			elsif( $Type eq 'CVsdFont' ){
				push( @Defs, "v8::Local<v8::Object> font$ArgNum = args[ $ArgPos ]->ToObject();" );
				push( @Defs, "CheckClass( font$ArgNum, \"Font\", \"arg[ $ArgPos ] must be Font\" );" );
				$Args[ $ArgNum ] = "*CVsdFont::GetThis( font$ArgNum )";
			}
			
			elsif( $Type eq 'char' ){
				# string ·¿
				push( @Defs, "v8::String::AsciiValue str$ArgNum( args[ $ArgPos ] );" );
				$Args[ $ArgNum ] = "*str$ArgNum";
			}
			
			elsif( $Type eq 'double' ){
				$Args[ $ArgNum ] = "args[ $ArgPos ]->NumberValue()";
			}
			
			elsif( $Type eq 'int' || $Type eq 'UINT' ){
				# int/UINT ·¿
				if( defined( $Default )){
					$Args[ $ArgNum ] = "iLen <= $ArgPos ? $Default : args[ $ArgPos ]->Int32Value()";
					--$ArgMin;
				}else{
					$Args[ $ArgNum ] = "args[ $ArgPos ]->Int32Value()";
				}
			}
			
			elsif( $Type eq 'void' ){
				next;
			}
			
			#else{
			#	$Args[ $ArgNum ] = "????";
			#}
			++$ArgMin;
			++$ArgNum;
		}
		
		$Defs = join( "\n\t", @Defs );
		$Args = join( ",\n\t\t", @Args );
		$Args = "\n\t\t$Args\n\t" if( $Args ne '' );
		
		$Len = $ArgMin == $ArgNum ?
			"iLen == $ArgNum" :
			"$ArgMin <= iLen && iLen <= $ArgNum";
		
		print fpFunc << "EOF";
static v8::Handle<v8::Value> Func_$FuncName( const v8::Arguments& args ){
	int iLen = args.Length();
	CheckArgs( "$FuncName", $Len );
	$Defs
	CScript::m_Vsd->$FuncName($Args);
	
	return v8::Undefined();
}
EOF
	}
	
	elsif( /!js_var:(\w+)/ ){
		$JSvar = $1;
		
		s/[\x0D\x0A]//g;
		s/\s*[\=;].*//;
		s/\(.*\)/()/;
		/(\w+\W*)$/;
		
		$RealVar = $1;
		
		$Type =
			/\b(?:int|UINT)\b/	? "Integer" :
			/\b(?:double)\b/	? "Number" :
								  "???";
		print( fpVar "DEF_SCR_VAR( $JSvar, $Type, $RealVar )\n" );
	}
}

print( fpFuncProto << "EOF" );
DEF_SCR_FUNC( DrawArc )
DEF_SCR_FUNC( print )
#undef DEF_SCR_FUNC
EOF

print( fpVar "#undef DEF_SCR_VAR\n" );
