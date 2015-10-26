#!/usr/bin/perl -w
# .tab=4

$ENV{ 'PATH' } = "$ENV{ 'HOME' }/bin:" . $ENV{ 'PATH' };

$OutputFile = $ARGV[ 0 ];

open( fpOut, "| nkf -sLw > $OutputFile" );

print fpOut << "-----";
/*****************************************************************************
	
	VSD -- vehicle data logger system  Copyright(C) by DDS
	
	ScriptIF.h - C<-->JavaScript interface
	
*****************************************************************************/

#pragma once

#include "CVsdFilter.h"
#include "CVsdFilterLog.h"
#include "CVsdFile.h"
#include "COle.h"
-----

### CVsdFilter ###############################################################

MakeJsIF({
	Class		=> 'CVsdFilter',
	NoDestructor=> 1,
	JsClass		=> '__VSD_System__',
	NewObject	=> << '-----',
		int iLen = args.Length();
		if( CScript::CheckArgs( iLen == 1 )) return Undefined( Isolate::GetCurrent());
		
		CVsdFilter *obj = static_cast<CVsdFilter *>( Local<External>::Cast( args[ 0 ] )->Value());
		if( !obj ) return Undefined( Isolate::GetCurrent());
-----
	FunctionIF	=> << '-----',
	/*** DrawArc ****************************************************************/
	
	static Handle<Value> Func_DrawArc( const FunctionCallbackInfo<Value>& args ){
		int iLen = args.Length();
		if( CScript::CheckArgs( 7 <= iLen && iLen <= 9 )) return Undefined( Isolate::GetCurrent());
		
		CVsdFilter *thisObj = CScript::GetThis<CVsdFilter>( args.This());
		if( !thisObj ) return Undefined( Isolate::GetCurrent());
		
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
				CPixel( args[ 8 ]->Int32Value())
			);
		}else{
			thisObj->DrawArc(
				args[ 0 ]->Int32Value(),
				args[ 1 ]->Int32Value(),
				args[ 2 ]->Int32Value(),
				args[ 3 ]->Int32Value(),
				args[ 4 ]->NumberValue(),
				args[ 5 ]->NumberValue(),
				CPixel( args[ 6 ]->Int32Value()),
				iLen <= 7 ? 0 : args[ 7 ]->Int32Value()
			);
		}
		return Undefined( Isolate::GetCurrent());
	}
	
	static Handle<Value> Func_DrawMap( const FunctionCallbackInfo<Value>& args ){
		int iLen = args.Length();
		
		CVsdFilter *thisObj = CScript::GetThis<CVsdFilter>( args.This());
		UINT uFlag = args[ 4 ]->Int32Value();
		
		if( uFlag & CVsdFilter::IMG_LOADMAP ){
			if( CScript::CheckArgs( 8 == iLen )) return Undefined( Isolate::GetCurrent());
			
			if( !thisObj ) return Undefined( Isolate::GetCurrent());
			thisObj->DrawMap(
				args[ 0 ]->Int32Value(),
				args[ 1 ]->Int32Value(),
				args[ 2 ]->Int32Value(),
				args[ 3 ]->Int32Value(),
				args[ 5 ]->Int32Value(),
				args[ 6 ]->Int32Value(),
				CPixel( args[ 7 ]->Int32Value())
			);
		}else{
			if( CScript::CheckArgs( 11 <= iLen && iLen <= 12 )) return Undefined( Isolate::GetCurrent());
			
			if( !thisObj ) return Undefined( Isolate::GetCurrent());
			thisObj->DrawMap(
				args[ 0 ]->Int32Value(),
				args[ 1 ]->Int32Value(),
				args[ 2 ]->Int32Value(),
				args[ 3 ]->Int32Value(),
				args[ 4 ]->Int32Value(),
				args[ 5 ]->Int32Value(),
				args[ 6 ]->Int32Value(),
				CPixel( args[ 7 ]->Int32Value()),
				CPixel( args[ 8 ]->Int32Value()),
				CPixel( args[ 9 ]->Int32Value()),
				CPixel( args[ 10 ]->Int32Value()),
				iLen <= 11 ? INVALID_INT : args[ 11 ]->Int32Value()
			);
		}
		return Undefined( Isolate::GetCurrent());
	}
	
	/*** ログデータ取得用 *******************************************************/
	
	#define DEF_LOG( name ) \
		static Handle<Value> Get_##name( Local<String> propertyName, const PropertyCallbackInfo<Value>& info ){ \
			CVsdFilter *obj = CScript::GetThis<CVsdFilter>( info.Holder()); \
			return obj ? Number::New( Isolate::GetCurrent(), obj->Get##name() ) : Undefined( Isolate::GetCurrent()); \
		}
	#include "def_log.h"
	
	static Handle<Value> Get_Value( Local<String> propertyName, const PropertyCallbackInfo<Value>& info ){
		CVsdFilter *obj = CScript::GetThis<CVsdFilter>( info.Holder());
		String::Utf8Value str( propertyName );
		return obj ? obj->GetValue( *str ) : Undefined( Isolate::GetCurrent());
	}
-----
});

### CVsdFilter_Log ###############################################################

MakeJsIF({
	Class		=> 'CVsdFilterLog',
	NoDestructor=> 1,
	JsClass		=> '__VSD_SystemLog__',
	NewObject	=> << '-----',
		int iLen = args.Length();
		if( CScript::CheckArgs( iLen == 1 )) return Undefined( Isolate::GetCurrent());
		
		CVsdFilterLog *obj = static_cast<CVsdFilterLog *>( Local<External>::Cast( args[ 0 ] )->Value());
		if( !obj ) return Undefined( Isolate::GetCurrent());
-----
	ExtraNew	=> << '-----',
		obj->AddLogAccessor( thisObject );
-----
});

### CVsdImage ################################################################

MakeJsIF({
	Class		=> 'CVsdImage',
	NewObject	=> << '-----',
		// 引数チェック
		if ( args.Length() <= 0 ) return Undefined( Isolate::GetCurrent());
		
		CVsdImage* obj;
		
		// arg[ 0 ] が Image だった場合，そのコピーを作る
		if( args[ 0 ]->IsObject()){
			Local<Object> Image0 = args[ 0 ]->ToObject();
			if( strcmp( *( String::Utf8Value )( Image0->GetConstructorName()), "Image" ) == 0 ){
				CVsdImage *obj0 = CScript::GetThis<CVsdImage>( Image0 );
				if( !obj0 ) return Undefined( Isolate::GetCurrent());
				
				obj = new CVsdImage( *obj0 );
			}else{
				V8TypeError( "arg[ 0 ] must be Image or string" );
				return Undefined( Isolate::GetCurrent());
			}
		}else{
			// ファイル名指定で画像ロード
			obj = new CVsdImage();
			String::Value FileName( args[ 0 ] );
			
			UINT uRet = obj->Load(
				( LPCWSTR )*FileName,
				args.Length() <= 1 ? 0 : args[ 1 ]->Int32Value()
			);
			if( uRet != ERR_OK ){
				V8Error( uRet );
				delete obj;
				return Undefined( Isolate::GetCurrent());
			}
		}
-----
});

### CVsdFont #################################################################

MakeJsIF({
	Class		=> 'CVsdFont',
	NewObject	=> << '-----'
		// 引数チェック
		if ( args.Length() < 2 ) return Undefined( Isolate::GetCurrent());
		
		String::Value FontName( args[ 0 ] );
		CVsdFont *obj = new CVsdFont(
			( LPCWSTR )*FontName,
			args[ 1 ]->Int32Value(),
			args.Length() <= 2 ? 0 : args[ 2 ]->Int32Value()
		);
-----
});

### CVsdFile #################################################################

MakeJsIF({
	Class		=> 'CVsdFile',
});

### COle #####################################################################

MakeJsIF({
	Class		=> 'COle',
	JsClass		=> 'ActiveXObject',
	NewObject	=> << '-----',
		COle *obj = new COle();
		
		// 引数チェック
		if( args.Length() >= 1 ){
			String::Value strServer( args[ 0 ]);
			
			UINT uRet = obj->CreateInstance(( LPCWSTR )*strServer );
			if( uRet != ERR_OK ){
				V8Error( uRet );
				delete obj;
				return Undefined( Isolate::GetCurrent());
			}
		}
-----
	ExtraInit	=> << '-----',
		COle::InitJS( tmpl );
-----
	ExtraNew	=> << '-----'
		if( args.Length() >= 1 ) obj->AddOLEFunction( thisObject );
-----
});

### Global ###################################################################

MakeJsIF({
	Class		=> 'CScript',
});

##############################################################################

sub MakeJsIF {
	my( $param ) = @_;
	
	$param->{ NewObject } = << "-----" if( !defined( $param->{ NewObject } ));
		$param->{ Class } *obj = new $param->{ Class }();
-----
	
	$param->{ bGlobal }		= $param->{ Class } eq 'CScript';
	$param->{ FunctionIF }	= '' if( !defined( $param->{ FunctionIF } ));
	$param->{ ExtraInit }	= '' if( !defined( $param->{ ExtraInit } ));
	$param->{ ExtraNew }	= '' if( !defined( $param->{ ExtraNew } ));
	
	if( !defined( $param->{ 'JsClass' })){
		$param->{ 'Class' }		=~ /^CVsd(.+)/;
		$param->{ 'JsClass' }	= $1;
	}
	
	$Accessor	= '';
	$AccessorIF	= '';
	$Function	= '';
	$Const	= '';
	
	$UseDestructor = $param->{ NoDestructor } ? '0' : '1';
	
	open( fpIn,	"< $param->{ Class }.h" );
	while( <fpIn> ){
		if( /!js_func\b/ ){
			
			# 関数名
			/(\S+)\s+\*?([\w_][\w_\d]*)\s*\(/;
			( $RetType, $FuncName ) = ( $1, $2 );
			
			$NoArgNumCheck = '';
			$ArgNum = 0;
			$ArgMin = 0;
			@Args = ();
			@Defs = ();
			
			$Line = $_;
			
			# 引数， ); まで読み込む
			if( $Line !~ /\)(?:\s*=\s*0\s*)?;/ ){
				while( <fpIn> ){
					$Line .= $_;
					last if( $Line =~ /\)(?:\s*=\s*0\s*)?;/ );
				}
			}
			
			$_ = $Line;
			s/[\x0D]//g;
			s/\bconst\b//g;
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
					push( @Defs, "Local<Object> $_$ArgNum = args[ $ArgPos ]->ToObject();" );
					push( @Defs, "if( CScript::CheckClass( $_$ArgNum, \"$_\", \"arg[ $ArgPos_p1 ] must be $_\" )) return Undefined( Isolate::GetCurrent());" );
					push( @Defs, "$Type *obj$ArgNum = CScript::GetThis<$Type>( $_$ArgNum );" );
					push( @Defs, "if( !obj$ArgNum ) return Undefined( Isolate::GetCurrent());" );
					$Args[ $ArgNum ] = "*obj$ArgNum";
				}
				
				elsif( $Type eq 'char' ){
					# string 型
					push( @Defs, "String::Utf8Value str$ArgNum( args[ $ArgPos ] );" );
					$Args[ $ArgNum ] = "*str$ArgNum";
				}
				
				elsif( $Type =~ /^LPC?WSTR$/ ){
					# WCHAR string 型
					push( @Defs, "String::Value str$ArgNum( args[ $ArgPos ] );" );
					$Args[ $ArgNum ] = "( $Type )*str$ArgNum";
				}
				
				elsif( $Type eq 'double' ){
					$Args[ $ArgNum ] = "args[ $ArgPos ]->NumberValue()";
				}
				
				elsif( $Type eq 'int' || $Type eq 'UINT' ){
					# int/UINT 型
					$Args[ $ArgNum ] = "args[ $ArgPos ]->Int32Value()";
				}
				
				elsif( $Type eq 'CPixelArg' ){
					# (・∀・)ラヴィ!!
					$Args[ $ArgNum ] = "CPixel( args[ $ArgPos ]->Int32Value())";
				}
				
				elsif( $Type eq 'v8Array' ){
					# V8 array object 
					$Args[ $ArgNum ] = "Local<Array>::Cast( args[ $ArgPos ] )";
				}
				
				elsif( $Type eq 'FunctionCallbackInfo<Value>&' ){
					$Args[ $ArgNum ] = 'args';
					$NoArgNumCheck = '//';
				}
				
				elsif( $Type eq 'void' ){
					next;
				}
				
				else{
					$Args[ $ArgNum ] = "????";
				}
				
				if( defined( $Default )){
					$Args[ $ArgNum ] = "iLen <= $ArgPos ? $Default : $Args[ $ArgNum ]";
				}else{
					++$ArgMin;
				}
				
				++$ArgNum;
			}
			
			$Defs = join( "\n\t\t", @Defs );
			$Args = join( ",\n\t\t\t", @Args );
			$Args = "\n\t\t\t$Args\n\t\t" if( $Args ne '' );
			
			$Len = $ArgMin == $ArgNum ?
				"iLen == $ArgNum" :
				"$ArgMin <= iLen && iLen <= $ArgNum";
			
			
			$PostRet	= ');';
			
			# 返り値
			if( $RetType eq 'void' ){
				$PreRet		= '';
				$PostRet	= '; return Undefined( Isolate::GetCurrent());';
			}
			
			elsif( $RetType eq 'int' || $RetType eq 'UINT' ){
				$PreRet		= "return Int32::New( Isolate::GetCurrent(), ";
			}
			
			elsif( $RetType eq 'char' ){
				$PreRet		= "return String::NewFromOneByte( Isolate::GetCurrent(), ( uint8_t *)";
			}
			
			elsif( $RetType	=~ /^LPC?WSTR$/ ){
				$PreRet		= "return String::NewFromTwoByte( Isolate::GetCurrent(), ( uint16_t *)";
			}
			
			elsif( $RetType eq 'double' ){
				$PreRet		= "return Number::New( Isolate::GetCurrent(), ";
			}
			
			elsif( $RetType	=~ /^(Handle|Local)\b/ ){
				$PreRet		= "return ( ";
			}
			
			else{
				$PreRet		= '???';
				$PostRet	= " unknown type:$RetType	= ";
			}
#-----
			$GetThis = $param->{ bGlobal } ? 'CScript::GetCScript' : "CScript::GetThis<$param->{ Class }>";
			$param->{ FunctionIF } .= << "-----";
	static Handle<Value> Func_$FuncName( const FunctionCallbackInfo<Value>& args ){
		${NoArgNumCheck}int iLen = args.Length();
		${NoArgNumCheck}if( CScript::CheckArgs( $Len )) return Undefined( Isolate::GetCurrent());
		$Defs
		$param->{ Class } *thisObj = $GetThis( args.This());
		if( !thisObj ) return Undefined( Isolate::GetCurrent());
		${PreRet}thisObj->$FuncName($Args)$PostRet
	}
-----
		}
		
		elsif( /!js_var:(\w+)/ ){
			$JSvar = $1;
			
			s/[\x0D\x0A]//g;
			s/\s*[{=;].*//;
			s/\(.*\)/()/;
			s/^\s+//g;
			/(\w+\W*)$/;
			
			$RealVar = $1;
			
			$Ret =
				/\b(?:int|UINT)\b/		? "Int32::New( Isolate::GetCurrent(), obj->$RealVar )" :
				/\bdouble\b/			? "Number::New( Isolate::GetCurrent(), obj->$RealVar )" :
				/\bchar\b/				? "String::NewFromOneByte( Isolate::GetCurrent(), ( uint8_t *)obj->$RealVar )" :
				/\bLPC?WSTR\b/			? "String::NewFromTwoByte( Isolate::GetCurrent(), ( uint16_t *)obj->$RealVar )" :
				/^(Handle|Local)\b/	? "obj->$RealVar" :
										  "unknown ret type obj->$RealVar";
#-----
			$AccessorIF .= << "-----";
	static Handle<Value> Get_$JSvar( Local<String> propertyName, const PropertyCallbackInfo<Value>& info ){
		$param->{ Class } *obj = CScript::GetThis<$param->{ Class }>( info.Holder());
		return obj ? $Ret : Undefined( Isolate::GetCurrent());
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
				/\b(?:int|UINT)\b/	? "Int32" :
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
		proto->Set( String::NewFromOneByte( Isolate::GetCurrent(), ( uint8_t *)"$JSvar" ), ${Type}::New($Cast(( $param->{ Class } *)pClass )->$RealVar ));
-----
		}
	}
	close( fpIn );
	
	$AccessorIF =~ s/Get_(\w+)/AddAccessor( $1, $param->{ Class } )/ge;
	$param->{ FunctionIF } =~ s/Func_(\w+)/AddFunction( $1, $param->{ Class } )/ge;
	
	print fpOut << "-----" if( !$param->{ bGlobal } );
/****************************************************************************/

class $param->{ Class }IF {
  public:
	// クラスコンストラクタ
	static Handle<Value> New( const FunctionCallbackInfo<Value>& args ){
		EscapableHandleScope handle_scope( Isolate::GetCurrent() );
		
$param->{ NewObject }
		// internal field にバックエンドオブジェクトを設定
		Local<Object> thisObject = args.This();
		thisObject->SetInternalField( 0, External::New( Isolate::GetCurrent(), obj ));
		
		// JS オブジェクトが GC されるときにデストラクタが呼ばれるおまじない
		Persistent<Object> objectHolder = Persistent<Object>::New( thisObject );
		objectHolder.MakeWeak( obj, Dispose );
		
$param->{ ExtraNew }
		#ifdef DEBUG
			DebugMsgD( ">>>new js obj $param->{ Class }:%X\\n", obj );
		#endif
		// コンストラクタは this を返すこと。
		return handle_scope.Escape( thisObject );
	}
	
	// クラスデストラクタ
	static void Dispose( Persistent<Value> handle, void* pVoid ){
		#if $UseDestructor
			HandleScope handle_scope( Isolate::GetCurrent() );
			$param->{ Class } *thisObj = CScript::GetThis<$param->{ Class }>( handle->ToObject());
			if( thisObj ){
				delete static_cast<$param->{ Class }*>( thisObj );
				#ifdef DEBUG
					DebugMsgD( "<<<del js obj $param->{ Class }:%X\\n", thisObj );
				#endif
			}
		#endif
		handle.Dispose();
	}
	
	// JavaScript からの明示的な破棄
	static Handle<Value> Func_Dispose( const FunctionCallbackInfo<Value>& args ){
		// obj の Dispose() を呼ぶ
		$param->{ Class } *thisObj = CScript::GetThis<$param->{ Class }>( args.This());
		#if $UseDestructor
			if( thisObj ){
				delete thisObj;
				#ifdef DEBUG
					DebugMsgD( "<<<DISPOSE js obj $param->{ Class }:%X\\n", thisObj );
				#endif
				
				// internalfield を null っぽくする
				args.This()->SetInternalField( 0, External::New( Isolate::GetCurrent(), NULL ));
			}
		#endif
		return Undefined( Isolate::GetCurrent());
	}
	
	///// プロパティアクセサ /////
$AccessorIF
	///// メソッドコールバック /////
$param->{ FunctionIF }
  public:
	// クラステンプレートの初期化
	static void InitializeClass( Handle<ObjectTemplate> global, void *pClass = NULL ){
		HandleScope handle_scope( Isolate::GetCurrent() );
		
		// コンストラクタを作成
		Local<FunctionTemplate> tmpl = FunctionTemplate::New( Isolate::GetCurrent(), New );
		tmpl->SetClassName( String::NewFromOneByte( Isolate::GetCurrent(), ( uint8_t *)"$param->{ JsClass }" ));
		
		// フィールドなどはこちらに
		Local<ObjectTemplate> inst = tmpl->InstanceTemplate();
		inst->SetInternalFieldCount( 1 );
$Accessor
		// メソッドはこちらに
		Local<ObjectTemplate> proto = tmpl->PrototypeTemplate();
		proto->Set( String::NewFromOneByte( Isolate::GetCurrent(), ( uint8_t *)"Dispose" ), FunctionTemplate::New( Isolate::GetCurrent(), Func_Dispose ));
$Function
$Const
$param->{ ExtraInit }
		// グローバルオブジェクトにクラスを定義
		global->Set( String::NewFromOneByte( Isolate::GetCurrent(), ( uint8_t *)"$param->{ JsClass }" ), tmpl );
	}
};
-----
	
	print fpOut << "-----" if( $param->{ bGlobal } );
/****************************************************************************/

class $param->{Class}IF {
  public:
	///// プロパティアクセサ /////
$AccessorIF
	///// メソッドコールバック /////
$param->{ FunctionIF }
  public:
	// クラステンプレートの初期化
	static void InitializeClass( Handle<ObjectTemplate> GlobalTmpl ){
		#define inst	GlobalTmpl
		#define proto	GlobalTmpl
		// フィールドなどはこちらに
$Accessor
		// メソッドはこちらに
$Function
$Const
$param->{ ExtraInit }
		#undef inst
		#undef proto
	}
};
-----
}

##############################################################################

sub AddAccessor {
	my( $Name, $Class )= @_;
	$Accessor .= << "-----";
		inst->SetAccessor( String::NewFromOneByte( Isolate::GetCurrent(), ( uint8_t *)"$Name" ), Get_$Name );
-----
	return "Get_$Name";
}

sub AddFunction {
	my( $Name, $Class )= @_;
	$Function .= << "-----";
		proto->Set( String::NewFromOneByte( Isolate::GetCurrent(), ( uint8_t *)"$Name" ), FunctionTemplate::New( Isolate::GetCurrent(), Func_$Name ));
-----
	return "Func_$Name";
}
