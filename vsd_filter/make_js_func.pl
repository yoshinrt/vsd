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

class CScriptIFBase {
	
  public:
	// this へのアクセスヘルパ
	template<typename T>
	static T* GetThis( Local<Object> handle ){
		if( handle->GetInternalField( 0 )->IsUndefined()){
			V8TypeError( "Object is undefined" );
			return NULL;
		}
		
		void* pThis = Local<External>::Cast( handle->GetInternalField( 0 ))->Value();
		return static_cast<T*>( pThis );
	}
	
	// 引数の数チェック
	static BOOL CheckArgs( BOOL cond ){
		if( !( cond )){
			V8ErrorNumOfArg();
			return TRUE;
		}
		return FALSE;
	}
	
	static BOOL CheckClass( Local<Object> obj, char *name, char *msg ){
		if( strcmp( *( String::Utf8Value )( obj->GetConstructorName()), name )){
			V8TypeError( msg );
			return TRUE;
		}
		return FALSE;
	}
};
-----

### CVsdFilter ###############################################################

MakeJsIF({
	Class		=> 'CVsdFilter',
	NoDestructor=> 1,
	JsClass		=> '__VSD_System__',
	NewObject	=> << '-----',
		int iLen = args.Length();
		if( CheckArgs( iLen == 1 )) return;
		
		CVsdFilter *pC_obj = static_cast<CVsdFilter *>( Local<External>::Cast( args[ 0 ] )->Value());
		if( !pC_obj ){
			V8Error( ERR_NOT_ENOUGH_MEMORY );
			return;
		}
-----
	FunctionIF	=> << '-----',
	/*** DrawArc ****************************************************************/
	
	static void Func_DrawArc( const FunctionCallbackInfo<Value>& args ){
		int iLen = args.Length();
		if( CheckArgs( 7 <= iLen && iLen <= 9 )) return;
		
		CVsdFilter *C_obj = GetThis<CVsdFilter>( args.This());
		if( !C_obj ) return;
		
		if( iLen >= 9 ){
			C_obj->DrawArc(
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
			C_obj->DrawArc(
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
	}
	
	static void Func_DrawMap( const FunctionCallbackInfo<Value>& args ){
		int iLen = args.Length();
		
		CVsdFilter *C_obj = GetThis<CVsdFilter>( args.This());
		UINT uFlag = args[ 4 ]->Int32Value();
		
		if( uFlag & CVsdFilter::IMG_LOADMAP ){
			if( CheckArgs( 8 == iLen )) return;
			
			if( !C_obj ) return;
			C_obj->DrawMap(
				args[ 0 ]->Int32Value(),
				args[ 1 ]->Int32Value(),
				args[ 2 ]->Int32Value(),
				args[ 3 ]->Int32Value(),
				args[ 5 ]->Int32Value(),
				args[ 6 ]->Int32Value(),
				CPixel( args[ 7 ]->Int32Value())
			);
		}else{
			if( CheckArgs( 11 <= iLen && iLen <= 12 )) return;
			
			if( !C_obj ) return;
			C_obj->DrawMap(
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
	}
	
	/*** ログデータ取得用 *******************************************************/
	
	#define DEF_LOG( name ) \
		static void Get_##name( Local<String> propertyName, const PropertyCallbackInfo<Value>& info ){ \
			CVsdFilter *pC_obj = GetThis<CVsdFilter>( info.Holder()); \
			if( pC_obj ) info.GetReturnValue().Set( pC_obj->Get##name()); \
		}
	#include "def_log.h"
	
	static void Get_Value( Local<String> propertyName, const PropertyCallbackInfo<Value>& info ){
		CVsdFilter *pC_obj = GetThis<CVsdFilter>( info.Holder());
		String::Utf8Value str( propertyName );
		if( pC_obj ) pC_obj->GetValue( info.GetReturnValue(), *str ); \
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
		if( CheckArgs( iLen == 1 )) return;
		
		CVsdFilterLog *pC_obj = static_cast<CVsdFilterLog *>( Local<External>::Cast( args[ 0 ] )->Value());
		if( !pC_obj ){
			V8Error( ERR_NOT_ENOUGH_MEMORY );
			return;
		}
-----
	ExtraNew	=> << '-----',
		pC_obj->AddLogAccessor( thisObject );
-----
});

### CVsdImage ################################################################

MakeJsIF({
	Class		=> 'CVsdImage',
	NewObject	=> << '-----',
		// 引数チェック
		if ( args.Length() <= 0 ){
			V8ErrorNumOfArg();
			return;
		}
		
		CVsdImage* pC_obj;
		
		// arg[ 0 ] が Image だった場合，そのコピーを作る
		if( args[ 0 ]->IsObject()){
			Local<Object> Image0 = args[ 0 ]->ToObject();
			if( strcmp( *( String::Utf8Value )( Image0->GetConstructorName()), "Image" ) == 0 ){
				CVsdImage *obj0 = GetThis<CVsdImage>( Image0 );
				if( !obj0 ) return;
				
				pC_obj = new CVsdImage( *obj0 );
			}else{
				V8TypeError( "arg[ 0 ] must be Image or string" );
				return;
			}
		}else{
			// ファイル名指定で画像ロード
			pC_obj = new CVsdImage();
			String::Value FileName( args[ 0 ] );
			
			UINT uRet = pC_obj->Load(
				( LPCWSTR )*FileName,
				args.Length() <= 1 ? 0 : args[ 1 ]->Int32Value()
			);
			if( uRet != ERR_OK ){
				V8Error( uRet );
				delete pC_obj;
				return;
			}
		}
-----
	Dispose	=> << '-----',
					C_obj->DeleteAsync();
-----
});

### CVsdFont #################################################################

MakeJsIF({
	Class		=> 'CVsdFont',
	NewObject	=> << '-----'
		// 引数チェック
		if ( args.Length() < 2 ){
			V8ErrorNumOfArg();
			return;
		}
		
		String::Value FontName( args[ 0 ] );
		CVsdFont *pC_obj = new CVsdFont(
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
		COle *pC_obj = new COle();
		
		// 引数チェック
		if( args.Length() >= 1 ){
			String::Value strServer( args[ 0 ]);
			
			UINT uRet = pC_obj->CreateInstance(( LPCWSTR )*strServer );
			if( uRet != ERR_OK ){
				V8Error( uRet );
				delete pC_obj;
				return;
			}
		}
-----
	ExtraInit	=> << '-----',
		COle::InitJS( tmpl );
-----
	ExtraNew	=> << '-----'
		if( args.Length() >= 1 ) pC_obj->AddOLEFunction( thisObject );
-----
});

### Global ###################################################################

MakeJsIF({
	Class		=> 'CScript',
	JsClass		=> 'dummy'
});

##############################################################################

sub MakeJsIF {
	my( $param ) = @_;
	
	$param->{ NewObject } = << "-----" if( !defined( $param->{ NewObject } ));
		$param->{ Class } *pC_obj = new $param->{ Class }();
-----
	
	$param->{ bGlobal }		= $param->{ Class } eq 'CScript' ? 1 : 0;
	$param->{ FunctionIF }	= '' if( !defined( $param->{ FunctionIF } ));
	$param->{ ExtraInit }	= '' if( !defined( $param->{ ExtraInit } ));
	$param->{ ExtraNew }	= '' if( !defined( $param->{ ExtraNew } ));
	
	if( defined( $param->{ Dispose })){
		$UseCustomDispose = 1;
	}else{
		$param->{ Dispose } = '';
		$UseCustomDispose = 0;
	}
	
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
			$HiddenArg = 0;	# ReturnValue 等，C にしかない引数の数
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
					push( @Defs, "if( CheckClass( $_$ArgNum, \"$_\", \"arg[ $ArgPos_p1 ] must be $_\" )) return;" );
					push( @Defs, "$Type *obj$ArgNum = GetThis<$Type>( $_$ArgNum );" );
					push( @Defs, "if( !obj$ArgNum ) return;" );
					$Args[ $ArgNum + $HiddenArg ] = "*obj$ArgNum";
				}
				
				elsif( $Type eq 'char' ){
					# string 型
					push( @Defs, "String::Utf8Value str$ArgNum( args[ $ArgPos ] );" );
					$Args[ $ArgNum + $HiddenArg ] = "*str$ArgNum";
				}
				
				elsif( $Type =~ /^LPC?WSTR$/ ){
					# WCHAR string 型
					push( @Defs, "String::Value str$ArgNum( args[ $ArgPos ] );" );
					$Args[ $ArgNum + $HiddenArg ] = "( $Type )*str$ArgNum";
				}
				
				elsif( $Type eq 'double' ){
					$Args[ $ArgNum + $HiddenArg ] = "args[ $ArgPos ]->NumberValue()";
				}
				
				elsif( $Type eq 'int' || $Type eq 'UINT' ){
					# int/UINT 型
					$Args[ $ArgNum + $HiddenArg ] = "args[ $ArgPos ]->Int32Value()";
				}
				
				elsif( $Type eq 'CPixelArg' ){
					# (・∀・)ラヴィ!!
					$Args[ $ArgNum + $HiddenArg ] = "CPixel( args[ $ArgPos ]->Int32Value())";
				}
				
				elsif( $Type eq 'v8Array' ){
					# V8 array object 
					$Args[ $ArgNum + $HiddenArg ] = "Local<Array>::Cast( args[ $ArgPos ] )";
				}
				
				elsif( $Type eq 'FunctionCallbackInfo<Value>&' ){
					$Args[ $ArgNum + $HiddenArg ] = 'args';
					$NoArgNumCheck = '//';
				}
				
				elsif( $Type eq 'ReturnValue<Value>' ){
					$Args[ $ArgNum + $HiddenArg ] = 'args.GetReturnValue()';
					++$HiddenArg;
					--$ArgMin;
					--$ArgNum;
				}
				
				elsif( $Type eq 'void' ){
					next;
				}
				
				else{
					$Args[ $ArgNum + $HiddenArg ] = "????";
				}
				
				if( defined( $Default )){
					$Args[ $ArgNum + $HiddenArg ] = "iLen <= $ArgPos ? $Default : $Args[ $ArgNum + $HiddenArg ]";
				}else{
					++$ArgMin;
				}
				
				++$ArgNum;
			}
			
			$Defs = join( "\n\t\t", @Defs );
			$Args = join( ",\n\t\t\t", @Args );
			$Args = "\n\t\t\t$Args\n\t\t" if( $Args ne '' );
			
			$Len = $ArgMin == $ArgNum ?
				"iLen == $ArgNum" : "$ArgMin <= iLen && iLen <= $ArgNum";
			
 			$PreRet		= 'args.GetReturnValue().Set( ';
			$PostRet	= '));';
			
			# 返り値
			if( $RetType eq 'void' ){
				$PreRet		= '';
				$PostRet	= ';';
			}
			
			elsif(
				$RetType eq 'int' || $RetType eq 'UINT' ||
				$RetType eq 'double' || $RetType	=~ /^(Handle|Local)\b/
			){
				$PreRet		.= "(";
			}
			
			elsif( $RetType eq 'char' ){
				$PreRet		.= "String::NewFromOneByte( args.GetIsolate(), ( uint8_t *)";
			}
			
			elsif( $RetType	=~ /^LPC?WSTR$/ ){
				$PreRet		.= "String::NewFromTwoByte( args.GetIsolate(), ( uint16_t *)";
			}
			
			else{
				$PreRet		= '???';
				$PostRet	= " unknown type:$RetType	= ";
			}
#-----
			$GetThis = $param->{ bGlobal } ? 'CScript::GetCScript' : "GetThis<$param->{ Class }>";
			$param->{ FunctionIF } .= << "-----";
	static void Func_$FuncName( const FunctionCallbackInfo<Value>& args ){
		${NoArgNumCheck}int iLen = args.Length();
		${NoArgNumCheck}if( CheckArgs( $Len )) return;
		$Defs
		$param->{ Class } *C_obj = $GetThis( args.This());
		if( !C_obj ) return;
		${PreRet}C_obj->$FuncName($Args)$PostRet
	}
-----
		}
		
		elsif( /!js_var:(\w+)/ ){
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
				/\bchar\b/			? "String::NewFromOneByte( info.GetIsolate(), ( uint8_t *)pC_obj->$RealVar )" :
				/\bLPC?WSTR\b/		? "String::NewFromTwoByte( info.GetIsolate(), ( uint16_t *)pC_obj->$RealVar )" :
									  "unknown ret type:$_ pC_obj->$RealVar";
			
			if( /\bvoid\b/ ){
				$Ret = "pC_obj->$RealVar";
			}else{
				$Ret = "info.GetReturnValue().Set( $Ret )";
			}
#-----
			$AccessorIF .= << "-----";
	static void Get_$JSvar( Local<String> propertyName, const PropertyCallbackInfo<Value>& info ){
		$param->{ Class } *pC_obj = GetThis<$param->{ Class }>( info.Holder());
		if( pC_obj ) $Ret;
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
				/\b(?:int|UINT)\b/	? "Int32::New( pIsolate, " :
				/\bdouble\b/		? "Number::New( pIsolate, " :
				/\bchar\b/			? "String::NewFromOneByte( pIsolate, ( uint8_t *)" :
				/\bLPC?WSTR\b/		? "String::NewFromTwoByte( pIsolate, ( uint16_t *)" :
									  "???";
#-----
			$Const .= << "-----";
		proto->Set( String::NewFromOneByte( pIsolate, ( uint8_t *)"$JSvar" ), ${Type}(( $param->{ Class } *)pClass )->$RealVar ));
-----
		}
	}
	close( fpIn );
	
	$AccessorIF =~ s/Get_(\w+)/AddAccessor( $1, $param->{ Class } )/ge;
	$param->{ FunctionIF } =~ s/Func_(\w+)/AddFunction( $1, $param->{ Class } )/ge;
	
	print fpOut << "-----";
/****************************************************************************/

class $param->{ Class }IF : CScriptIFBase {
  public:
#if !$param->{ bGlobal }
	// クラスコンストラクタ
	static void New( const FunctionCallbackInfo<Value>& args ){
		HandleScope handle_scope( args.GetIsolate());
		
$param->{ NewObject }
		// internal field にバックエンドオブジェクトを設定
		Local<Object> thisObject = args.This();
		thisObject->SetInternalField( 0, External::New( args.GetIsolate(), pC_obj ));
		
		#if $UseDestructor
			// JS オブジェクトが GC されるときにデストラクタが呼ばれるおまじない
			pC_obj->m_pHolder = new Persistent<Object>( args.GetIsolate(), thisObject );
			pC_obj->m_pHolder->SetWeak( pC_obj, Dispose );
		#endif
		
$param->{ ExtraNew }
		#ifdef DEBUG
			DebugMsgD( ">>>new js obj $param->{ Class }:%X\\n", pC_obj );
		#endif
	}
	
	// クラスデストラクタ
	#if $UseDestructor
		static void Dispose( const WeakCallbackData<Object, $param->{ Class }> &data ){
			$param->{ Class } *pC_obj = data.GetParameter();
			
			//release instance. は delete c_Obj で自動的にやる
			//pC_obj->m_Holder->Reset();
			
			delete pC_obj;
			DebugMsgD( "<<<del by WeakCallback $param->{ Class }:%X\\n", pC_obj );
		}
	#endif
	
	// JavaScript からの明示的な破棄
	static void Func_Dispose( const FunctionCallbackInfo<Value>& args ){
		// pC_obj の Dispose() を呼ぶ
		#if $UseDestructor
			$param->{ Class } *C_obj = GetThis<$param->{ Class }>( args.This());
			if( C_obj ){
				
				#if $UseCustomDispose
$param->{ Dispose }
				#else
					delete C_obj;
				#endif
				
				#ifdef DEBUG
					DebugMsgD( "<<<Dispose() js obj $param->{ Class }:%X\\n", C_obj );
				#endif
				
				// internalfield を null っぽくする
				args.This()->SetInternalField( 0, External::New( args.GetIsolate(), NULL ));
			}
		#endif
	}
#endif
	
	///// プロパティアクセサ /////
$AccessorIF
	///// メソッドコールバック /////
$param->{ FunctionIF }
  public:
	// クラステンプレートの初期化
	static void InitializeClass( Isolate *pIsolate, Handle<ObjectTemplate> global, void *pClass = NULL ){
		HandleScope handle_scope( pIsolate );
		
	#if $param->{ bGlobal }
		#define inst	global
		#define proto	global
	#else
		// コンストラクタを作成
		Local<String> ClassName = String::NewFromOneByte( pIsolate, ( uint8_t *)"$param->{ JsClass }" );
		Local<FunctionTemplate> tmpl = FunctionTemplate::New( pIsolate, New );
		tmpl->SetClassName( ClassName );
		// フィールドなどはこちらに
		Local<ObjectTemplate> inst = tmpl->InstanceTemplate();
		inst->SetInternalFieldCount( 1 );
	#endif
	
$Accessor
		// メソッドはこちらに
	#if !$param->{ bGlobal }
		Local<ObjectTemplate> proto = tmpl->PrototypeTemplate();
		proto->Set( String::NewFromOneByte( pIsolate, ( uint8_t *)"Dispose" ), FunctionTemplate::New( pIsolate, Func_Dispose ));
	#endif
$Function
$Const
$param->{ ExtraInit }
	#if !$param->{ bGlobal }
		// グローバルオブジェクトにクラスを定義
		global->Set( ClassName, tmpl );
		#undef inst
		#undef proto
	#endif
	}
};
-----
}

##############################################################################

sub AddAccessor {
	my( $Name, $Class )= @_;
	$Accessor .= << "-----";
		inst->SetAccessor( String::NewFromOneByte( pIsolate, ( uint8_t *)"$Name" ), Get_$Name );
-----
	return "Get_$Name";
}

sub AddFunction {
	my( $Name, $Class )= @_;
	$Function .= << "-----";
		proto->Set( String::NewFromOneByte( pIsolate, ( uint8_t *)"$Name" ), FunctionTemplate::New( pIsolate, Func_$Name ));
-----
	return "Func_$Name";
}
