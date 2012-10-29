@echo off
set perlscr=%0 %*
set perlscr=%perlscr:\=/%
C:\cygwin\bin\bash --login -i -c 'cd "%CD%";CYGWIN=nodosfilewarning perl -x %perlscr%'
goto :EOF

##############################################################################
#!/usr/bin/perl -w
# .tab=4

$ENV{ 'PATH' } = "$ENV{ 'HOME' }/bin:" . $ENV{ 'PATH' };

open( fpOut, "| nkf -s > ScriptIF.h" );

print fpOut << "-----";
/*****************************************************************************
	
	VSD -- vehicle data logger system  Copyright(C) by DDS
	
	ScriptIF.h - C<-->JavaScript interface
	
*****************************************************************************/

#pragma once

#include "CVsdFilter.h"
#include "CVsdFile.h"
//#include "COle.h"
-----

### CVsdFilter ###############################################################

MakeJsIF( 'CVsdFilter', '__VSD_System__', << '-----', << '-----', << '-----' );
		int iLen = args.Length();
		if( CScript::CheckArgs( iLen == 1 )) return v8::Undefined();
		
		CVsdFilter *obj = static_cast<CVsdFilter *>( v8::Local<v8::External>::Cast( args[ 0 ] )->Value());
		if( !obj ) return v8::Undefined();
-----
	/*** DrawArc ****************************************************************/
	
	static v8::Handle<v8::Value> Func_DrawArc( const v8::Arguments& args ){
		int iLen = args.Length();
		if( CScript::CheckArgs( 7 <= iLen && iLen <= 9 )) return v8::Undefined();
		
		CVsdFilter *thisObj = CScript::GetThis<CVsdFilter>( args.This());
		if( !thisObj ) return v8::Undefined();
		
		if( iLen >= 9 ){
			thisObj->DrawArc(
				args[ 0 ]->Int32Value(),
				args[ 1 ]->Int32Value(),
				args[ 2 ]->Int32Value(),
				args[ 3 ]->Int32Value(),
				args[ 4 ]->Int32Value(),
				args[ 5 ]->Int32Value(),
				args[ 6 ]->NumberValue(),
				args[ 7 ]->NumberValue(),
				PIXEL_RABY::Argb2Raby( args[ 8 ]->Int32Value())
			);
		}else{
			thisObj->DrawArc(
				args[ 0 ]->Int32Value(),
				args[ 1 ]->Int32Value(),
				args[ 2 ]->Int32Value(),
				args[ 3 ]->Int32Value(),
				args[ 4 ]->NumberValue(),
				args[ 5 ]->NumberValue(),
				PIXEL_RABY::Argb2Raby( args[ 6 ]->Int32Value()),
				iLen <= 7 ? 0 : args[ 7 ]->Int32Value()
			);
		}
		return v8::Undefined();
	}
	
	/*** ログデータ取得用 *******************************************************/
	
	#define DEF_LOG( name ) \
		static v8::Handle<v8::Value> Get_##name( v8::Local<v8::String> propertyName, const v8::AccessorInfo& info ){ \
			CVsdFilter *obj = CScript::GetThis<CVsdFilter>( info.Holder()); \
			return obj ? v8::Number::New( obj->Get##name() ) : v8::Undefined(); \
		}
	#include "def_log.h"
	
	static v8::Handle<v8::Value> Get_Value( v8::Local<v8::String> propertyName, const v8::AccessorInfo& info ){
		CVsdFilter *obj = CScript::GetThis<CVsdFilter>( info.Holder());
		v8::String::AsciiValue str( propertyName );
		return obj ? v8::Number::New( obj->GetValue( *str )) : v8::Undefined();
	}
	
	/*** デバッグ用 *************************************************************/
	
	// 関数オブジェクト print の実体
	static v8::Handle<v8::Value> Func_print(const v8::Arguments& args) {
		v8::String::AsciiValue str( args[ 0 ] );
		DebugMsgD( "%s\n", *str );
		return v8::Undefined();
	}
	
-----
		(( CVsdFilter *)pClass )->InitJS( tmpl );
-----

### CVsdImage ################################################################

MakeJsIF( 'CVsdImage', 'Image', << '-----' );
		// 引数チェック
		if ( args.Length() <= 0 ) return v8::Undefined();
		
		CVsdImage* obj;
		
		// arg[ 0 ] が Image だった場合，そのコピーを作る
		if( args[ 0 ]->IsObject()){
			v8::Local<v8::Object> Image0 = args[ 0 ]->ToObject();
			if( strcmp( *( v8::String::AsciiValue )( Image0->GetConstructorName()), "Image" ) == 0 ){
				CVsdImage *obj0 = CScript::GetThis<CVsdImage>( Image0 );
				if( !obj0 ) return v8::Undefined();
				
				obj = new CVsdImage( *obj0 );
			}else{
				return v8::ThrowException( v8::Exception::Error( v8::String::New(
					"arg[ 0 ] must be Image or string"
				)));
			}
		}else{
			// ファイル名指定で画像ロード
			obj = new CVsdImage();
			v8::String::Value FileName( args[ 0 ] );
			
			if( obj->Load(( LPCWSTR )*FileName ) != ERR_OK ){
				delete obj;
				return v8::Undefined();
			}
		}
-----

### CVsdFont #################################################################

MakeJsIF( 'CVsdFont', 'Font', << '-----' );
		// 引数チェック
		if ( args.Length() < 2 ) return v8::Undefined();
		
		v8::String::Value FontName( args[ 0 ] );
		CVsdFont *obj = new CVsdFont(
			( LPCWSTR )*FontName,
			args[ 1 ]->Int32Value(),
			args.Length() <= 2 ? 0 : args[ 2 ]->Int32Value()
		);
-----

### CVsdFile #################################################################

MakeJsIF( 'CVsdFile', 'File' );

### COle #####################################################################

#MakeJsIF( 'COle', 'ActiveXObject', << '-----', undef, << '-----' );
#		COle *obj = new COle();
#		
#		// 引数チェック
#		if ( args.Length() >= 1 ){
#			v8::String::Value strServer( args[ 0 ] );
#			obj->CreateInstance(( LPCWSTR )*strServer );
#		}
#-----
#		COle::InitJS( tmpl );
#-----

### Global ###################################################################

MakeJsIF( 'CScript' );

##############################################################################

sub MakeJsIF {
	my( $Class, $JsClass, $NewObject, $FunctionIF, $ExtraInit ) = @_;
	
	$NewObject = << "-----" if( !defined( $NewObject ));
		$Class *obj = new $Class();
-----
	
	$bGlobal = !defined( $JsClass );
	
	$FunctionIF = '' if( !defined( $FunctionIF ));
	$ExtraInit  = '' if( !defined( $ExtraInit ));
	
	$Accessor	= '';
	$AccessorIF	= '';
	$Function	= '';
	$Const	= '';
	
	$IfNotVsd = $Class eq 'CVsdFilter' ? 'if( 0 )' : '';
	
	open( fpIn,	"< $Class.h" );
	while( <fpIn> ){
		if( /!js_func\b/ ){
			
			# 関数名
			/([\w_]+)\s+\*?([\w_]+)\s*\(/;
			( $RetType, $FuncName ) = ( $1, $2 );
			
			$ArgNum = 0;
			$ArgMin = 0;
			@Args = ();
			@Defs = ();
			
			$Line = $_;
			
			# 引数
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
				/(\S+)/;	# 型
				$Type = $1;
				
				$Default = /!default:(\S+)/ ? $1 : undef;
				$ArgPos  = /!arg:(\d+)/ ? $1 : $ArgNum;
				
				if( $Type =~ /^CVsd(.+)/ ){
					$_ = $1;
					
					$ArgPos_p1 = $ArgPos + 1;
					push( @Defs, "v8::Local<v8::Object> $_$ArgNum = args[ $ArgPos ]->ToObject();" );
					push( @Defs, "if( CScript::CheckClass( $_$ArgNum, \"$_\", \"arg[ $ArgPos_p1 ] must be $_\" )) return v8::Undefined();" );
					push( @Defs, "$Type *obj$ArgNum = CScript::GetThis<$Type>( $_$ArgNum );" );
					push( @Defs, "if( !obj$ArgNum ) return v8::Undefined();" );
					$Args[ $ArgNum ] = "*obj$ArgNum";
				}
				
				elsif( $Type eq 'char' ){
					# string 型
					push( @Defs, "v8::String::AsciiValue str$ArgNum( args[ $ArgPos ] );" );
					$Args[ $ArgNum ] = "*str$ArgNum";
				}
				
				elsif( $Type =~ /^LPC?WSTR$/ ){
					# WCHAR string 型
					push( @Defs, "v8::String::Value str$ArgNum( args[ $ArgPos ] );" );
					$Args[ $ArgNum ] = "( $Type )*str$ArgNum";
				}
				
				elsif( $Type eq 'double' ){
					if( defined( $Default )){
						$Args[ $ArgNum ] = "iLen <= $ArgPos ? $Default : args[ $ArgPos ]->NumberValue()";
						--$ArgMin;
					}else{
						$Args[ $ArgNum ] = "args[ $ArgPos ]->NumberValue()";
					}
				}
				
				elsif( $Type eq 'int' || $Type eq 'UINT' ){
					# int/UINT 型
					if( defined( $Default )){
						$Args[ $ArgNum ] = "iLen <= $ArgPos ? $Default : args[ $ArgPos ]->Int32Value()";
						--$ArgMin;
					}else{
						$Args[ $ArgNum ] = "args[ $ArgPos ]->Int32Value()";
					}
				}
				
				elsif( $Type eq 'tRABY' ){
					# (・∀・)ラヴィ!!
					if( defined( $Default )){
						$Args[ $ArgNum ] = "iLen <= $ArgPos ? $Default : PIXEL_RABY::Argb2Raby( args[ $ArgPos ]->Int32Value())";
						--$ArgMin;
					}else{
						$Args[ $ArgNum ] = "PIXEL_RABY::Argb2Raby( args[ $ArgPos ]->Int32Value())";
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
			
			$Defs = join( "\n\t\t", @Defs );
			$Args = join( ",\n\t\t\t", @Args );
			$Args = "\n\t\t\t$Args\n\t\t" if( $Args ne '' );
			
			$Len = $ArgMin == $ArgNum ?
				"iLen == $ArgNum" :
				"$ArgMin <= iLen && iLen <= $ArgNum";
			
			# 返り値
			if( $RetType eq 'void' ){
				$RetVar   = '';
				$RetValue = 'v8::Undefined()';
			}
			
			elsif( $RetType eq 'int' || $RetType eq 'UINT' ){
				$RetVar   = "int ret = ";
				$RetValue = "v8::Integer::New( ret )"
			}
			
			elsif( $RetType eq 'char' ){
				$RetVar   = "char *ret = ";
				$RetValue = "v8::String::New( ret )"
			}
			
			elsif( $RetType =~ /^LPC?WSTR$/ ){
				$RetVar   = "$RetType ret = ";
				$RetValue = "v8::String::New(( uint16_t *)ret )"
			}
			
			elsif( $RetType eq 'double' ){
				$RetVar   = "double ret = ";
				$RetValue = "v8::Number::New( ret )"
			}
			
			else{
				$RetVar   = '??? = ';
				$RetValue = '???';
			}
#-----
			$FunctionIF .= << "-----" if( !$bGlobal );
	static v8::Handle<v8::Value> Func_$FuncName( const v8::Arguments& args ){
		int iLen = args.Length();
		if( CScript::CheckArgs( $Len )) return v8::Undefined();
		$Defs
		$Class *thisObj = CScript::GetThis<$Class>( args.This());
		if( !thisObj ) return v8::Undefined();
		${RetVar}thisObj->$FuncName($Args);
		
		return $RetValue;
	}
-----
			$FunctionIF .= << "-----" if( $bGlobal );
	static v8::Handle<v8::Value> Func_$FuncName( const v8::Arguments& args ){
		int iLen = args.Length();
		if( CScript::CheckArgs( $Len )) return v8::Undefined();
		$Defs
		CScript::$FuncName($Args);
		
		return $RetValue;
	}
-----
		}
		
		elsif( /!js_var:(\w+)/ ){
			$JSvar = $1;
			
			s/[\x0D\x0A]//g;
			s/\s*[{=;].*//;
			s/\(.*\)/()/;
			/(\w+\W*)$/;
			
			$RealVar = $1;
			
			$Type =
				/\b(?:int|UINT)\b/	? "Integer" :
				/\bdouble\b/		? "Number" :
				/\bchar\b/			? "String" :
				/\bLPC?WSTR\b/		? "String" :
									  "???";
			
			$Cast = '';
			if( /\bLPC?WSTR\b/ ){
				$Cast	= '( uint16_t *)';
			}
#-----
			$AccessorIF .= << "-----";
	static v8::Handle<v8::Value> Get_$JSvar( v8::Local<v8::String> propertyName, const v8::AccessorInfo& info ){
		$Class *obj = CScript::GetThis<$Class>( info.Holder());
		return obj ? v8::${Type}::New($Cast obj->$RealVar ) : v8::Undefined();
	}
-----
		}
		
		elsif( /!js_const:(\w+)/ ){
			# CVsdFilter 専用
			$JSvar = $1;
			
			s/[\x0D\x0A]//g;
			s/\s*[{=;].*//;
			s/\(.*\)/()/;
			/(\w+\W*)$/;
			
			$RealVar = $1;
			
			$Type =
				/\b(?:int|UINT)\b/	? "Integer" :
				/\bdouble\b/		? "Number" :
				/\bchar\b/			? "String" :
				/\bLPC?WSTR\b/		? "String" :
									  "???";
			
			$Cast = '';
			if( /\bLPC?WSTR\b/ ){
				$Cast	= '( uint16_t *)';
			}
#-----
			$Const .= << "-----";
		proto->Set( v8::String::New( "$JSvar" ), v8::${Type}::New($Cast(( $Class *)pClass )->$RealVar ));
-----
		}
	}
	close( fpIn );
	
	$AccessorIF =~ s/Get_(\w+)/AddAccessor( $1, $Class )/ge;
	$FunctionIF =~ s/Func_(\w+)/AddFunction( $1, $Class )/ge;
	
	print fpOut << "-----" if( !$bGlobal );
/****************************************************************************/

class ${Class}IF {
  public:
	// クラスコンストラクタ
	static v8::Handle<v8::Value> New( const v8::Arguments& args ){
		
$NewObject
		// internal field にバックエンドオブジェクトを設定
		v8::Local<v8::Object> thisObject = args.This();
		thisObject->SetInternalField( 0, v8::External::New( obj ));
		
		// JS オブジェクトが GC されるときにデストラクタが呼ばれるおまじない
		v8::Persistent<v8::Object> objectHolder = v8::Persistent<v8::Object>::New( thisObject );
		objectHolder.MakeWeak( obj, Dispose );
		
		#ifdef DEBUG
			DebugMsgD( ">>>new js obj $Class:%X\\n", obj );
		#endif
		// コンストラクタは this を返すこと。
		return thisObject;
	}
	
	// クラスデストラクタ
	static void Dispose( v8::Persistent<v8::Value> handle, void* pVoid ){
		$IfNotVsd {
			v8::HandleScope handle_scope;
			$Class *thisObj = CScript::GetThis<$Class>( handle->ToObject());
			if( thisObj ){
				delete static_cast<$Class*>( thisObj );
				#ifdef DEBUG
					DebugMsgD( "<<<del js obj $Class:%X\\n", thisObj );
				#endif
			}
		}
		handle.Dispose();
	}
	
	// JavaScript からの明示的な破棄
	static v8::Handle<v8::Value> Func_Dispose( const v8::Arguments& args ){
		// obj の Dispose() を呼ぶ
		$Class *thisObj = CScript::GetThis<$Class>( args.This());
		$IfNotVsd if( thisObj ){
			delete thisObj;
			#ifdef DEBUG
				DebugMsgD( "<<<DISPOSE js obj $Class:%X\\n", thisObj );
			#endif
			
			// internalfield を null っぽくする
			args.This()->SetInternalField( 0, v8::External::New( NULL ));
		}
		return v8::Undefined();
	}
	
	///// プロパティアクセサ /////
$AccessorIF
	///// メソッドコールバック /////
$FunctionIF
  public:
	// クラステンプレートの初期化
	static void InitializeClass( v8::Handle<v8::ObjectTemplate> global, void *pClass = NULL ){
		// コンストラクタを作成
		v8::Local<v8::FunctionTemplate> tmpl = v8::FunctionTemplate::New( New );
		tmpl->SetClassName( v8::String::New( "$JsClass" ));
		
		// フィールドなどはこちらに
		v8::Handle<v8::ObjectTemplate> inst = tmpl->InstanceTemplate();
		inst->SetInternalFieldCount( 1 );
$Accessor
		// メソッドはこちらに
		v8::Handle<v8::ObjectTemplate> proto = tmpl->PrototypeTemplate();
		proto->Set( v8::String::New( "Dispose" ), v8::FunctionTemplate::New( Func_Dispose ));
$Function
$Const
$ExtraInit
		// グローバルオブジェクトにクラスを定義
		global->Set( v8::String::New( "$JsClass" ), tmpl );
	}
};
-----
	
	print fpOut << "-----" if( $bGlobal );
/****************************************************************************/

class ${Class}IF {
  public:
	///// プロパティアクセサ /////
$AccessorIF
	///// メソッドコールバック /////
$FunctionIF
  public:
	// クラステンプレートの初期化
	static void InitializeClass( v8::Handle<v8::ObjectTemplate> GlobalTmpl ){
		#define inst	GlobalTmpl
		#define proto	GlobalTmpl
		// フィールドなどはこちらに
$Accessor
		// メソッドはこちらに
$Function
$Const
$ExtraInit
		#undef inst
		#undef GlobalTmpl
	}
};
-----
}

##############################################################################

sub AddAccessor {
	my( $Name, $Class )= @_;
	$Accessor .= << "-----";
		inst->SetAccessor( v8::String::New( "$Name" ), Get_$Name );
-----
	return "Get_$Name";
}

sub AddFunction {
	my( $Name, $Class )= @_;
	$Function .= << "-----";
		proto->Set( v8::String::New( "$Name" ), v8::FunctionTemplate::New( Func_$Name ));
-----
	return "Func_$Name";
}
