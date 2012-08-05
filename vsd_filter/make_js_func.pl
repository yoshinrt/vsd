#!/usr/bin/perl

open( fpOut, "| nkf -s > ScriptIF.h" );

MakeJsIF( 'CVsdFilter', 'Vsd', << '-----', << '-----' );
		CVsdFilter* obj = CScript::m_Vsd;
-----
	/*** DrawArc ****************************************************************/
	
	static v8::Handle<v8::Value> Func_DrawArc( const v8::Arguments& args ){
		int iLen = args.Length();
		if( CheckArgs( 7 <= iLen && iLen <= 10 )) return v8::Undefined();
		
		if( iLen >= 9 ){
			CScript::m_Vsd->DrawArc(
				args[ 0 ]->Int32Value(),
				args[ 1 ]->Int32Value(),
				args[ 2 ]->Int32Value(),
				args[ 3 ]->Int32Value(),
				args[ 4 ]->Int32Value(),
				args[ 5 ]->Int32Value(),
				args[ 6 ]->NumberValue(),
				args[ 7 ]->NumberValue(),
				PIXEL_RABY::Argb2Raby( args[ 8 ]->Int32Value()),
				iLen <= 9 ? 0 : args[ 9 ]->Int32Value()
			);
		}else{
			CScript::m_Vsd->DrawArc(
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
	
	/*** デバッグ用 *************************************************************/
	
	// 関数オブジェクト print の実体
	static v8::Handle<v8::Value> Func_print(const v8::Arguments& args) {
		v8::String::AsciiValue str( args[ 0 ] );
		DebugMsgD( "%s\n", *str );
		return v8::Undefined();
	}
	
-----

MakeJsIF( 'CVsdImage', 'Image', << '-----', '' );
		// 引数チェック
		if ( args.Length() <= 0 ) return v8::Undefined();
		
		CVsdImage* obj;
		
		// arg[ 0 ] が Image だった場合，そのコピーを作る
		if( args[ 0 ]->IsObject()){
			v8::Local<v8::Object> Image0 = args[ 0 ]->ToObject();
			if( strcmp( *( v8::String::AsciiValue )( Image0->GetConstructorName()), "Image" ) == 0 ){
				CVsdImage *obj0 = GetThis<CVsdImage>( Image0 );
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
			v8::String::AsciiValue FileName( args[ 0 ] );
			
			if( obj->Load( *FileName ) != ERR_OK ){
				delete obj;
				return v8::Undefined();
			}
		}
-----

MakeJsIF( 'CVsdFont', 'Font', << '-----', '' );
		// 引数チェック
		if ( args.Length() < 2 ) return v8::Undefined();
		
		v8::String::AsciiValue FontName( args[ 0 ] );
		CVsdFont *obj = new CVsdFont(
			*FontName,
			args[ 1 ]->Int32Value(),
			args.Length() <= 2 ? 0 : args[ 2 ]->Int32Value()
		);
-----

sub MakeJsIF {
	my( $Class, $JsClass, $NewObject, $FunctionIF ) = @_;
	
	$Accessor	= '';
	$AccessorIF	= '';
	$Function	= '';
	
	open( fpIn,	"< $Class.h" );
	while( <fpIn> ){
		if( /!js_func\b/ ){
			
			# 関数名
			/([\w_]+)\s+([\w_]+)\s*\(/;
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
					push( @Defs, "if( CheckClass( $_$ArgNum, \"$_\", \"arg[ $ArgPos_p1 ] must be $_\" )) return v8::Undefined();" );
					push( @Defs, "$Type *obj$ArgNum = GetThis<$Type>( $_$ArgNum );" );
					push( @Defs, "if( !obj$ArgNum ) return v8::Undefined();" );
					$Args[ $ArgNum ] = "*obj$ArgNum";
				}
				
				elsif( $Type eq 'char' ){
					# string 型
					push( @Defs, "v8::String::AsciiValue str$ArgNum( args[ $ArgPos ] );" );
					$Args[ $ArgNum ] = "*str$ArgNum";
				}
				
				elsif( $Type eq 'double' ){
					$Args[ $ArgNum ] = "args[ $ArgPos ]->NumberValue()";
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
			
			elsif( $RetType eq 'double' ){
				$RetVar   = "double ret = ";
				$RetValue = "v8::Number::New( ret )"
			}
			
			else{
				$RetVar   = '??? = ';
				$RetValue = '???';
			}
#-----
			$FunctionIF .= << "-----";
	static v8::Handle<v8::Value> Func_$FuncName( const v8::Arguments& args ){
		int iLen = args.Length();
		if( CheckArgs( $Len )) return v8::Undefined();
		$Defs
		$Class *thisObj = GetThis<$Class>( args.This());
		if( !thisObj ) return v8::Undefined();
		${RetVar}thisObj->$FuncName($Args);
		
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
				/\b(?:double)\b/	? "Number" :
									  "???";
#-----
			$AccessorIF .= << "-----";
	static v8::Handle<v8::Value> Get_$JSvar( v8::Local<v8::String> propertyName, const v8::AccessorInfo& info ){
		$Class *obj = GetThis<$Class>( info.Holder());
		return obj ? v8::${Type}::New( obj->$RealVar ) : v8::Undefined();
	}
-----
		}
	}
	close( fpIn );
	
	$AccessorIF =~ s/Get_(\w+)/AddAccessor( $1, $Class )/ge;
	$FunctionIF =~ s/Func_(\w+)/AddFunction( $1, $Class )/ge;
	
	print fpOut << "-----";
/****************************************************************************/

class ${Class}IF {
  private:
	// クラスコンストラクタ
	static v8::Handle<v8::Value> New( const v8::Arguments& args ){
		
$NewObject
		// internal field にバックエンドオブジェクトを設定
		v8::Local<v8::Object> thisObject = args.This();
		thisObject->SetInternalField( 0, v8::External::New( obj ));
		
		// JS オブジェクトが GC されるときにデストラクタが呼ばれるおまじない
		v8::Persistent<v8::Object> objectHolder = v8::Persistent<v8::Object>::New( thisObject );
		objectHolder.MakeWeak( obj, Dispose );
		
		// コンストラクタは this を返すこと。
		return thisObject;
	}
	
	// クラスデストラクタ
	static void Dispose( v8::Persistent<v8::Value> handle, void* pVoid ){
		delete static_cast<$Class*>( pVoid );
	}
	
	///// プロパティアクセサ /////
$AccessorIF
	///// メソッドコールバック /////
$FunctionIF
  public:
	// this へのアクセスヘルパ
	template<typename T>
	static T* GetThis( v8::Local<v8::Object> handle ){
		if( handle->GetInternalField( 0 )->IsUndefined()){
			v8::ThrowException( v8::Exception::TypeError( v8::String::New( "Invalid object ( maybe \\"new\\" failed )" )));
			return NULL;
		}
		
		void* pThis = v8::Local<v8::External>::Cast( handle->GetInternalField( 0 ))->Value();
		return static_cast<T*>( pThis );
	}
	
	// 引数の数チェック
	static BOOL CheckArgs( BOOL cond ){
		if( !( cond )){
			v8::ThrowException( v8::Exception::Error( v8::String::New(
				"invalid number of args"
			)));
			return TRUE;
		}
		return FALSE;
	}
	
	static BOOL CheckClass( v8::Local<v8::Object> obj, char *name, char *msg ){
		if( strcmp( *( v8::String::AsciiValue )( obj->GetConstructorName()), name )){
			v8::ThrowException( v8::Exception::TypeError( v8::String::New( msg )));
			return TRUE;
		}
		return FALSE;
	}
	
	// クラステンプレートの初期化
	static void InitializeClass( v8::Handle<v8::ObjectTemplate> global ){
		// コンストラクタを作成
		v8::Local<v8::FunctionTemplate> tmpl = v8::FunctionTemplate::New( New );
		tmpl->SetClassName( v8::String::New( "$JsClass" ));
		
		// フィールドなどはこちらに
		v8::Handle<v8::ObjectTemplate> inst = tmpl->InstanceTemplate();
		inst->SetInternalFieldCount( 1 );
$Accessor
		// メソッドはこちらに
		v8::Handle<v8::ObjectTemplate> proto = tmpl->PrototypeTemplate();
$Function
		// グローバルオブジェクトにクラスを定義
		global->Set( v8::String::New( "$JsClass" ), tmpl );
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
