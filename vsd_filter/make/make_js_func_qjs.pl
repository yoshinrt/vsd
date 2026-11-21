#!/usr/bin/perl -w

$ENV{'PATH'} = "$ENV{'HOME'}/bin:" . $ENV{'PATH'};

$OutputFile = $ARGV[0];

open(fpOut, "> $OutputFile");

print fpOut << "-----";
\xEF\xBB\xBF/*****************************************************************************
	
	VSD -- vehicle data logger system  Copyright(C) by DDS
	
	ScriptIF.cpp - C<-->JavaScript interface
	
*****************************************************************************/
#include "StdAfx.h"

#include "CVsdFilter.h"
#include "CScript_qjs.h"
#include "CVsdImage_qjs.h"
#include "CVsdFile_qjs.h"
#include "CVsdFont.h"

void CScript_qjs::RegisterClass(){
-----

#_#### CVsdFilter ###############################################################
#_#
#_#MakeJsIF({
#_#	Class		=> 'CVsdFilter',
#_#	NoDestructor=> 1,
#_#	JsClass		=> '__VSD_System__',
#_#});
#_#
### CVsdImage ################################################################

MakeJsIF({
	Class		=> 'CVsdImage_qjs',
});

### CVsdFont #################################################################

MakeJsIF({
	Class		=> 'CVsdFont',
});

### CVsdFile #################################################################

MakeJsIF({
	Class		=> 'CVsdFile_qjs',
});

### Global ###################################################################

MakeJsIF({
	Class		=> 'CScript_qjs',
	JsClass		=> 'global'
});

##############################################################################

sub MakeJsIF {
	my($param) = @_;
	
	$param->{NewObject} = << "-----" if(!defined($param->{NewObject}));
		$param->{Class} *pC_obj = new $param->{Class}();
-----
	
	$param->{bGlobal}		= $param->{Class} eq 'CScript' ? 1 : 0;
	$param->{FunctionIF}	= '' if(!defined($param->{FunctionIF}));
	$param->{ExtraInit}	= '' if(!defined($param->{ExtraInit}));
	$param->{ExtraNew}	= '' if(!defined($param->{ExtraNew}));
	
	if(defined($param->{Dispose})){
		$UseCustomDispose = 1;
	}else{
		$param->{Dispose} = '';
		$UseCustomDispose = 0;
	}
	
	if(!defined($param->{'JsClass'})){
		$param->{'Class'}		=~ /^CVsd(.+)/;
		$param->{'JsClass'}	= $1;
		$param->{'JsClass'}	=~ s/_qjs$//;
	}
	
	$ind0 = $param->{JsClass} eq 'global' ? "" : "\t";
	$ind1 = "\t$ind0";
	$ind2 = "\t$ind1";
	$ind3 = "\t$ind2";
	
	$Accessor	= '';
	$AccessorIF	= '';
	$Function	= '';
	$Const	= '';
	
	$UseDestructor = $param->{NoDestructor} ? '0' : '1';
	
	$hasConstructor = 0;
	
	print(fpOut << "-----");
	
	//////////////////////////////////////////////////////////////////////////
	// $param->{JsClass}
	
-----
	
	print(fpOut << "-----") if($param->{JsClass} ne 'global');
	qjsb::ClassDef<$param->{Class}>(m_JsContext, "$param->{JsClass}")
-----

	open(fpIn,	"< $param->{Class}.h");
	while(<fpIn>){
		if(/!js_func\b/){
			
			# 関数名
			/(\S+)\s+\*?([\w_][\w_\d]*)\s*\(/;
			($RetType, $FuncName) = ($1, $2);
			
			$ArgNum = 0;
			$ArgMin = 0;
			@Args = ();
			@Defs = ();
			
			$Line = $_;
			
			# 引数，); まで読み込む
			if($Line !~ /\)(?:\s*=\s*0\s*)?;/){
				while(<fpIn>){
					$Line .= $_;
					last if($Line =~ /\)(?:\s*=\s*0\s*)?;/);
				}
			}
			
			$_ = $Line;
			s/[\x0D]//g;
			s/\bconst\b//g;
			s/^.*?\([^\)]*?\n// || s/^.*?\(//;
			s/[\n\s]*\).*$//s;
			s/,\s*/\n/g;
			s#\n\s*//# //#g;
			
			@Line = split(/\n/, $_);
			
			foreach $_ (@Line){
				/(\S+)/;	# 型
				$Type = $1;
				
				$Default = /!default:(\S+)/ ? $1 : undef;
				
				if($Type =~ /^(CVsd.+)/){
					$Args[$ArgNum] = "qjsb::from_js<$1*>(ctx, argv[$ArgNum])";
				}
				
				elsif($Type eq 'char'){
					# string 型
					$Args[$ArgNum] = "qjsb::from_js<std::string>(ctx, argv[$ArgNum]).c_str()";
				}
				
				elsif($Type =~ /^LPC?WSTR$/){
					# WCHAR string 型
					$Args[$ArgNum] = "Utf8ToWide(qjsb::from_js<std::string>(ctx, argv[$ArgNum])).c_str()";
				}
				
				elsif($Type eq 'std::string'){
					# std::string 型
					$Args[$ArgNum] = "std::move(qjsb::from_js<std::string>(ctx, argv[$ArgNum]))";
				}
				
				elsif($Type eq 'double'){
					$Args[$ArgNum] = "qjsb::from_js<double>(ctx, argv[$ArgNum])";
				}
				
				elsif($Type eq 'int' || $Type eq 'UINT'){
					# int/UINT 型
					$Args[$ArgNum] = "qjsb::from_js<int>(ctx, argv[$ArgNum])";
				}
				
				elsif($Type eq 'CPixelArg'){
					# (・∀・)ラヴィ!!
					$Args[$ArgNum] = "CPixel(qjsb::from_js<int>(ctx, argv[$ArgNum]))";
				}
				
				elsif($Type eq 'v8Array'){
					# V8 array object 
					$Args[$ArgNum] = "Local<Array>::Cast(args[$ArgNum])";
				}
				
				elsif($Type eq 'void'){
					next;
				}
				
				else{
					$Args[$ArgNum] = "????";
				}
				
				if(defined($Default)){
					$Args[$ArgNum] = "argc <= $ArgNum ? $Default : $Args[$ArgNum]";
				}else{
					++$ArgMin;
				}
				
				++$ArgNum;
			}
			
			$Defs = join("\n${ind1}", @Defs);
			$Args = join(",\n${ind3}", @Args);
			$Args = "\n${ind3}$Args\n${ind2}" if($Args ne '');
			
			$Len = $ArgMin == $ArgNum ?
				"argc == $ArgNum" :
				"$ArgMin <= argc && argc <= $ArgNum";
			
			# 返り値
			if($RetType eq 'void'){
				$PreRet		= '';
				$PostRet	= '; return JS_UNDEFINED;';
			}
			
			elsif(
				$RetType eq 'int' || $RetType eq 'UINT' || $RetType eq 'double' ||
				$RetType eq 'std::string'
			){
				$PreRet		= 'return qjsb::to_js(ctx, ';
				$PostRet	= ');';
			}
			
			elsif($RetType eq 'char'){
				$PreRet		= 'return qjsb::to_js(ctx, std::string(';
				$PostRet	= '));';
			}
			
			else{
				$PreRet		= '???';
				$PostRet	= " unknown type:$RetType	= ";
			}
			if($param->{JsClass} eq 'global'){
#-----
				print(fpOut << "-----");
	m_JsContext.bindFunctionRaw("$FuncName", [](JSContext* ctx, JSValueConst, int argc, JSValueConst* argv) -> JSValue {
		if(!($Len)) JsErrorNumOfArg_qjs();
		$Defs
		${PreRet}CScript_qjs::$FuncName($Args)${PostRet}
	});
-----
			}else{
#-----
				print(fpOut << "-----");
		.method("$FuncName", [](JSContext* ctx, $param->{Class}* self, int argc, JSValueConst* argv) -> JSValue {
			if(!($Len)) JsErrorNumOfArg_qjs();
			$Defs
			${PreRet}self->$FuncName($Args)${PostRet}
		})
-----
			
			}
		}
		elsif(/!js_var:(\w+)/){
			$JSvar = $1;
			
			s/[\x0D\x0A]//g;
			s/\s*[{=;].*//;
			s/\(\s*void\s*\)/()/;
			s/^\s+//g;
			/(\S+)$/;
			
			/(.+)\s(\w+)\s*(\(\))?$/;
			($type, $var, $func) = ($1, $2, $3);
			$func = '' if(!defined $func);
			
			if(
				$type =~ /int|UINT|double|std::string/ &&
				$func
			){
				# property get 関数を直接指定
				print(fpOut "${ind1}.property(\"$JSvar\", \&$param->{Class}::$var)\n");
			}else{
				# ラムダでリード
				print(fpOut "${ind1}.property(\"$JSvar\", [](const $param->{Class}* self){return self->$var$func;})\n");
			}
		}
		
		elsif(/!js_const:(\w+)/){
			# CVsdFilter 専用
			$JSvar = $1;
			
			s/[\x0D\x0A]//g;
			s/\s*[{=;].*//;
			s/\(.*\)/()/;
			/(\w+\W*)$/;
			
			$RealVar = $1;
			
			$Type =
				/\b(?:int|UINT)\b/	? "Int32::New(pIsolate, " :
				/\bdouble\b/		? "Number::New(pIsolate, " :
				/\bchar\b/			? "String::NewFromOneByte(pIsolate, (uint8_t *)" :
				/\bLPC?WSTR\b/		? "String::NewFromTwoByte(pIsolate, (uint16_t *)" :
									  "???";
#-----
			$Const .= << "-----";
		proto->Set(String::NewFromOneByte(pIsolate, (uint8_t *)"$JSvar"), ${Type}(($param->{Class} *)pClass)->$RealVar));
-----
		}
		
		elsif(/!js_constructor\b/){
			$hasConstructor = 1;
			
			while(<fpIn>){
				last if(m#\*/#);
				s/\r//g;
				print(fpOut);
			}
		}
	}
	close(fpIn);
	
	$AccessorIF =~ s/Get_(\w+)/AddAccessor($1, $param->{Class})/ge;
	$param->{FunctionIF} =~ s/Func_(\w+)/AddFunction($1, $param->{Class})/ge;
	
	if($param->{JsClass} ne 'global'){
		print(fpOut "${ind1}.constructor<>()\n") if(!$hasConstructor);
		print(fpOut "${ind1}.endClass();\n")
	}
}

print(fpOut "}\n");
