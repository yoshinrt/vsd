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
#include "CVsdFile.h"
#include "COle.h"
-----

### CVsdFilter ###############################################################

MakeJsIF({
	Class		=> 'CVsdFilter',
	JsClass		=> '__VSD_System__',
	NewObject	=> << '-----',
		int iLen = args.Length();
		if( CScript::CheckArgs( iLen == 1 )) return v8::Undefined();
		
		CVsdFilter *obj = static_cast<CVsdFilter *>( v8::Local<v8::External>::Cast( args[ 0 ] )->Value());
		if( !obj ) return v8::Undefined();
-----
	FunctionIF	=> << '-----',
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
		return v8::Undefined();
	}
	
	static v8::Handle<v8::Value> Func_DrawMap( const v8::Arguments& args ){
		int iLen = args.Length();
		
		CVsdFilter *thisObj = CScript::GetThis<CVsdFilter>( args.This());
		UINT uFlag = args[ 4 ]->Int32Value();
		
		if( uFlag & CVsdFilter::IMG_LOADMAP ){
			if( CScript::CheckArgs( 8 == iLen )) return v8::Undefined();
			
			if( !thisObj ) return v8::Undefined();
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
			if( CScript::CheckArgs( 11 <= iLen && iLen <= 12 )) return v8::Undefined();
			
			if( !thisObj ) return v8::Undefined();
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
		return obj ? obj->GetValue( *str ) : v8::Undefined();
	}
-----
	ExtraInit	=> << '-----',
		(( CVsdFilter *)pClass )->AddAccessor( tmpl );
-----
});

### CVsdFilter_Log ###############################################################

MakeJsIF({
	Class		=> 'CVsdFilter',
	JsClass		=> '__VSD_SystemLog__',
	vsdlog		=> 1,
	NewObject	=> << '-----',
		int iLen = args.Length();
		if( CScript::CheckArgs( iLen == 1 )) return v8::Undefined();
		
		CVsdFilter *obj = static_cast<CVsdFilter *>( v8::Local<v8::External>::Cast( args[ 0 ] )->Value());
		if( !obj ) return v8::Undefined();
-----
	FunctionIF	=> << '-----',
	static v8::Handle<v8::Value> Func_ValueOfIndex( const v8::Arguments& args ){
		int iLen = args.Length();
		if( CScript::CheckArgs( iLen == 2 )) return v8::Undefined();
		v8::String::AsciiValue str0( args[ 0 ] );
		CVsdFilter *thisObj = CScript::GetThis<CVsdFilter>( args.This());
		if( !thisObj ) return v8::Undefined();
		return thisObj->AccessLog(
			*str0,
			args[ 1 ]->NumberValue()
		);
	}
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
				V8TypeError( "arg[ 0 ] must be Image or string" );
				return v8::Undefined();
			}
		}else{
			// ファイル名指定で画像ロード
			obj = new CVsdImage();
			v8::String::Value FileName( args[ 0 ] );
			
			if( obj->Load(
				( LPCWSTR )*FileName,
				args.Length() <= 1 ? 0 : args[ 1 ]->Int32Value()
			) != ERR_OK ){
				delete obj;
				return v8::Undefined();
			}
		}
-----
});

### CVsdFont #################################################################

MakeJsIF({
	Class		=> 'CVsdFont',
	NewObject	=> << '-----'
		// 引数チェック
		if ( args.Length() < 2 ) return v8::Undefined();
		
		v8::String::Value FontName( args[ 0 ] );
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
			v8::String::Value strServer( args[ 0 ] );
			
			if( obj->CreateInstance(( LPCWSTR )*strServer ) != S_OK ){
				return v8::ThrowException( v8::Exception::Error(
					v8::String::Concat(
						v8::String::New( "Can't open OLE server: " ),
						args[ 0 ]->ToString()
					)
				));
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
	
	$IfNotVsd = $param->{ Class } eq 'CVsdFilter' ? 'if( 0 )' : '';
	
	open( fpIn,	"< $param->{ Class }.h" );
	while( <fpIn> ){
		# インデントを深くしたくないので苦肉の策
		last if( $param->{ vsdlog });
		
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
					$Args[ $ArgNum ] = "v8::Local<v8::Array>::Cast( args[ $ArgPos ] )";
				}
				
				elsif( $Type eq 'v8::Arguments&' ){
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
				$PostRet	= '; return v8::Undefined();';
			}
			
			elsif( $RetType eq 'int' || $RetType eq 'UINT' ){
				$PreRet		= "return v8::Int32::New( ";
			}
			
			elsif( $RetType eq 'char' ){
				$PreRet		= "return v8::String::New( ";
			}
			
			elsif( $RetType	=~ /^LPC?WSTR$/ ){
				$PreRet		= "return v8::String::New(( uint16_t *)";
			}
			
			elsif( $RetType eq 'double' ){
				$PreRet		= "return v8::Number::New( ";
			}
			
			elsif( $RetType	=~ /^v8::(Handle|Local)\b/ ){
				$PreRet		= "return ( ";
			}
			
			else{
				$PreRet		= '???';
				$PostRet	= " unknown type:$RetType	= ";
			}
#-----
			$GetThis = $param->{ bGlobal } ? 'CScript::GetCScript' : "CScript::GetThis<$param->{ Class }>";
			$param->{ FunctionIF } .= << "-----";
	static v8::Handle<v8::Value> Func_$FuncName( const v8::Arguments& args ){
		${NoArgNumCheck}int iLen = args.Length();
		${NoArgNumCheck}if( CScript::CheckArgs( $Len )) return v8::Undefined();
		$Defs
		$param->{ Class } *thisObj = $GetThis( args.This());
		if( !thisObj ) return v8::Undefined();
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
				/\b(?:int|UINT)\b/		? "v8::Int32::New( obj->$RealVar )" :
				/\bdouble\b/			? "v8::Number::New( obj->$RealVar )" :
				/\bchar\b/				? "v8::String::New( obj->$RealVar )" :
				/\bLPC?WSTR\b/			? "v8::String::New(( uint16_t *)obj->$RealVar )" :
				/^v8::(Handle|Local)\b/	? "obj->$RealVar" :
										  "unknown ret type obj->$RealVar";
#-----
			$AccessorIF .= << "-----";
	static v8::Handle<v8::Value> Get_$JSvar( v8::Local<v8::String> propertyName, const v8::AccessorInfo& info ){
		$param->{ Class } *obj = CScript::GetThis<$param->{ Class }>( info.Holder());
		return obj ? $Ret : v8::Undefined();
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
		proto->Set( v8::String::New( "$JSvar" ), v8::${Type}::New($Cast(( $param->{ Class } *)pClass )->$RealVar ));
-----
		}
	}
	close( fpIn );
	
	$AccessorIF =~ s/Get_(\w+)/AddAccessor( $1, $param->{ Class } )/ge;
	$param->{ FunctionIF } =~ s/Func_(\w+)/AddFunction( $1, $param->{ Class } )/ge;
	
	$ClassIfName = $param->{ vsdlog } ?
		"$param->{ Class }_LogIF" :
		"$param->{ Class }IF";
	
	print fpOut << "-----" if( !$param->{ bGlobal } );
/****************************************************************************/

class $ClassIfName {
  public:
	// クラスコンストラクタ
	static v8::Handle<v8::Value> New( const v8::Arguments& args ){
		v8::HandleScope handle_scope;
		
$param->{ NewObject }
		// internal field にバックエンドオブジェクトを設定
		v8::Local<v8::Object> thisObject = args.This();
		thisObject->SetInternalField( 0, v8::External::New( obj ));
		
		// JS オブジェクトが GC されるときにデストラクタが呼ばれるおまじない
		v8::Persistent<v8::Object> objectHolder = v8::Persistent<v8::Object>::New( thisObject );
		objectHolder.MakeWeak( obj, Dispose );
		
$param->{ ExtraNew }
		#ifdef DEBUG
			DebugMsgD( ">>>new js obj $param->{ Class }:%X\\n", obj );
		#endif
		// コンストラクタは this を返すこと。
		return handle_scope.Close( thisObject );
	}
	
	// クラスデストラクタ
	static void Dispose( v8::Persistent<v8::Value> handle, void* pVoid ){
		$IfNotVsd {
			v8::HandleScope handle_scope;
			$param->{ Class } *thisObj = CScript::GetThis<$param->{ Class }>( handle->ToObject());
			if( thisObj ){
				delete static_cast<$param->{ Class }*>( thisObj );
				#ifdef DEBUG
					DebugMsgD( "<<<del js obj $param->{ Class }:%X\\n", thisObj );
				#endif
			}
		}
		handle.Dispose();
	}
	
	// JavaScript からの明示的な破棄
	static v8::Handle<v8::Value> Func_Dispose( const v8::Arguments& args ){
		// obj の Dispose() を呼ぶ
		$param->{ Class } *thisObj = CScript::GetThis<$param->{ Class }>( args.This());
		$IfNotVsd if( thisObj ){
			delete thisObj;
			#ifdef DEBUG
				DebugMsgD( "<<<DISPOSE js obj $param->{ Class }:%X\\n", thisObj );
			#endif
			
			// internalfield を null っぽくする
			args.This()->SetInternalField( 0, v8::External::New( NULL ));
		}
		return v8::Undefined();
	}
	
	///// プロパティアクセサ /////
$AccessorIF
	///// メソッドコールバック /////
$param->{ FunctionIF }
  public:
	// クラステンプレートの初期化
	static void InitializeClass( v8::Handle<v8::ObjectTemplate> global, void *pClass = NULL ){
		v8::HandleScope handle_scope;
		
		// コンストラクタを作成
		v8::Local<v8::FunctionTemplate> tmpl = v8::FunctionTemplate::New( New );
		tmpl->SetClassName( v8::String::New( "$param->{ JsClass }" ));
		
		// フィールドなどはこちらに
		v8::Local<v8::ObjectTemplate> inst = tmpl->InstanceTemplate();
		inst->SetInternalFieldCount( 1 );
$Accessor
		// メソッドはこちらに
		v8::Local<v8::ObjectTemplate> proto = tmpl->PrototypeTemplate();
		proto->Set( v8::String::New( "Dispose" ), v8::FunctionTemplate::New( Func_Dispose ));
$Function
$Const
$param->{ ExtraInit }
		// グローバルオブジェクトにクラスを定義
		global->Set( v8::String::New( "$param->{ JsClass }" ), tmpl );
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
	static void InitializeClass( v8::Handle<v8::ObjectTemplate> GlobalTmpl ){
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
