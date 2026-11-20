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

#include "CScript_qjs.h"
#include "CVsdFile_qjs.h"

void CScript_qjs::RegisterClass(){
-----

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
	}
	
	$Accessor	= '';
	$AccessorIF	= '';
	$Function	= '';
	$Const	= '';
	
	$UseDestructor = $param->{NoDestructor} ? '0' : '1';
	
	print(fpOut << "-----");
	
	//////////////////////////////////////////////////////////////////////////
	// $param->{JsClass}
	
-----
	
	print(fpOut << "-----") if($param->{JsClass} ne 'global');
	qjsb::ClassDef<$param->{Class}>(m_JsContext, "$param->{JsClass}")
		.constructor<>()
-----

	open(fpIn,	"< $param->{Class}.h");
	while(<fpIn>){
		if(/!js_func\b/){
			# 関数名
			/(\S+)\s+\*?([\w_][\w_\d]*)\s*\(/;
			($RetType, $FuncName) = ($1, $2);
			
			if($param->{JsClass} ne 'global'){
				print(fpOut "\t\t.method(\"$FuncName\", \&$param->{Class}::$FuncName)\n");
			}else{
				print(fpOut "\tm_JsContext.bindFunction(\"$FuncName\", \&$param->{Class}::$FuncName);\n");
			}
			
		}elsif(/!js_func_overload\b/){
			
			# 関数名
			/(\S+)\s+\*?([\w_][\w_\d]*)\s*\(/;
			($RetType, $FuncName) = ($1, $2);
			
			$NoArgNumCheck = '';
			$ArgNum = 0;
			$ArgMin = 0;
			$HiddenArg = 0;	# ReturnValue 等，C にしかない引数の数
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
				$ArgPos  = /!arg:(\d+)/ ? $1 : $ArgNum;
				
				if($Type =~ /^CVsd(.+)/){
					$_ = $1;
					
					$ArgPos_p1 = $ArgPos + 1;
					push(@Defs, "Local<Object> $_$ArgNum = args[$ArgPos]->ToObject();");
					push(@Defs, "if(CheckClass($_$ArgNum, \"$_\", \"arg[$ArgPos_p1] must be $_\")) return;");
					push(@Defs, "$Type *obj$ArgNum = GetThis<$Type>($_$ArgNum);");
					push(@Defs, "if(!obj$ArgNum) return;");
					$Args[$ArgNum + $HiddenArg] = "*obj$ArgNum";
				}
				
				elsif($Type eq 'char'){
					# string 型
					push(@Defs, "String::Utf8Value str$ArgNum(args[$ArgPos]);");
					$Args[$ArgNum + $HiddenArg] = "*str$ArgNum";
				}
				
				elsif($Type =~ /^LPC?WSTR$/){
					# WCHAR string 型
					push(@Defs, "String::Value str$ArgNum(args[$ArgPos]);");
					$Args[$ArgNum + $HiddenArg] = "($Type)*str$ArgNum";
				}
				
				elsif($Type eq 'double'){
					$Args[$ArgNum + $HiddenArg] = "args[$ArgPos]->NumberValue()";
				}
				
				elsif($Type eq 'int' || $Type eq 'UINT'){
					# int/UINT 型
					$Args[$ArgNum + $HiddenArg] = "args[$ArgPos]->Int32Value()";
				}
				
				elsif($Type eq 'CPixelArg'){
					# (・∀・)ラヴィ!!
					$Args[$ArgNum + $HiddenArg] = "CPixel(args[$ArgPos]->Int32Value())";
				}
				
				elsif($Type eq 'v8Array'){
					# V8 array object 
					$Args[$ArgNum + $HiddenArg] = "Local<Array>::Cast(args[$ArgPos])";
				}
				
				elsif($Type eq 'FunctionCallbackInfo<Value>&'){
					$Args[$ArgNum + $HiddenArg] = 'args';
					$NoArgNumCheck = '//';
				}
				
				elsif($Type eq 'ReturnValue<Value>'){
					$Args[$ArgNum + $HiddenArg] = 'args.GetReturnValue()';
					++$HiddenArg;
					--$ArgMin;
					--$ArgNum;
				}
				
				elsif($Type eq 'void'){
					next;
				}
				
				else{
					$Args[$ArgNum + $HiddenArg] = "????";
				}
				
				if(defined($Default)){
					$Args[$ArgNum + $HiddenArg] = "iLen <= $ArgPos ? $Default : $Args[$ArgNum + $HiddenArg]";
				}else{
					++$ArgMin;
				}
				
				++$ArgNum;
			}
			
			$Defs = join("\n\t\t", @Defs);
			$Args = join(",\n\t\t\t", @Args);
			$Args = "\n\t\t\t$Args\n\t\t" if($Args ne '');
			
			$Len = $ArgMin == $ArgNum ?
				"iLen == $ArgNum" : "$ArgMin <= iLen && iLen <= $ArgNum";
			
 			$PreRet		= 'args.GetReturnValue().Set(';
			$PostRet	= '));';
			
			# 返り値
			if($RetType eq 'void'){
				$PreRet		= '';
				$PostRet	= ';';
			}
			
			elsif(
				$RetType eq 'int' || $RetType eq 'UINT' ||
				$RetType eq 'double' || $RetType	=~ /^(Handle|Local)\b/
			){
				$PreRet		.= "(";
			}
			
			elsif($RetType eq 'char'){
				$PreRet		.= "String::NewFromOneByte(args.GetIsolate(), (uint8_t *)";
			}
			
			elsif($RetType	=~ /^LPC?WSTR$/){
				$PreRet		.= "String::NewFromTwoByte(args.GetIsolate(), (uint16_t *)";
			}
			
			else{
				$PreRet		= '???';
				$PostRet	= " unknown type:$RetType	= ";
			}
#-----
			$GetThis = $param->{bGlobal} ? 'CScript::GetCScript' : "GetThis<$param->{Class}>";
			$param->{FunctionIF} .= << "-----";
	static void Func_$FuncName(const FunctionCallbackInfo<Value>& args){
		${NoArgNumCheck}int iLen = args.Length();
		${NoArgNumCheck}if(CheckArgs($Len)) return;
		$Defs
		$param->{Class} *C_obj = $GetThis(args.This());
		if(!C_obj) return;
		${PreRet}C_obj->$FuncName($Args)$PostRet
	}
-----
		}
		
		elsif(/!js_var:(\w+)/){
			$JSvar = $1;
			
			s/[\x0D\x0A]//g;
			s/\s*[{=;].*//;
			s/\(\s*void\s*\)/()/;
			s/\(\s*ReturnValue<.*?\)/(info.GetReturnValue())/;
			s/^\s+//g;
			/(\S+)$/;
			
			$RealVar = $1;
			
			$Ret =
				/\b(?:int|UINT|double|Handle|Local)\b/
									? "pC_obj->$RealVar" :
				/\bchar\b/			? "String::NewFromOneByte(info.GetIsolate(), (uint8_t *)pC_obj->$RealVar)" :
				/\bLPC?WSTR\b/		? "String::NewFromTwoByte(info.GetIsolate(), (uint16_t *)pC_obj->$RealVar)" :
									  "unknown ret type:$_ pC_obj->$RealVar";
			
			if(/\bvoid\b/){
				$Ret = "pC_obj->$RealVar";
			}else{
				$Ret = "info.GetReturnValue().Set($Ret)";
			}
#-----
			$AccessorIF .= << "-----";
	static void Get_$JSvar(Local<String> propertyName, const PropertyCallbackInfo<Value>& info){
		$param->{Class} *pC_obj = GetThis<$param->{Class}>(info.Holder());
		if(pC_obj) $Ret;
	}
-----
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
	}
	close(fpIn);
	
	$AccessorIF =~ s/Get_(\w+)/AddAccessor($1, $param->{Class})/ge;
	$param->{FunctionIF} =~ s/Func_(\w+)/AddFunction($1, $param->{Class})/ge;
	
	print(fpOut "\t\t.endClass();\n") if($param->{JsClass} ne 'global');
}

print(fpOut "}\n");
