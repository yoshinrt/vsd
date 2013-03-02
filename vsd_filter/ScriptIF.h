/*****************************************************************************
	
	VSD -- vehicle data logger system  Copyright(C) by DDS
	
	ScriptIF.h - C<-->JavaScript interface
	
*****************************************************************************/

#pragma once

#include "CVsdFilter.h"
#include "CVsdFile.h"
//#include "COle.h"
/****************************************************************************/

class CVsdFilterIF {
  public:
	// クラスコンストラクタ
	static v8::Handle<v8::Value> New( const v8::Arguments& args ){
		
		int iLen = args.Length();
		if( CScript::CheckArgs( iLen == 1 )) return v8::Undefined();
		
		CVsdFilter *obj = static_cast<CVsdFilter *>( v8::Local<v8::External>::Cast( args[ 0 ] )->Value());
		if( !obj ) return v8::Undefined();

		// internal field にバックエンドオブジェクトを設定
		v8::Local<v8::Object> thisObject = args.This();
		thisObject->SetInternalField( 0, v8::External::New( obj ));
		
		// JS オブジェクトが GC されるときにデストラクタが呼ばれるおまじない
		v8::Persistent<v8::Object> objectHolder = v8::Persistent<v8::Object>::New( thisObject );
		objectHolder.MakeWeak( obj, Dispose );
		
		#ifdef DEBUG
			DebugMsgD( ">>>new js obj CVsdFilter:%X\n", obj );
		#endif
		// コンストラクタは this を返すこと。
		return thisObject;
	}
	
	// クラスデストラクタ
	static void Dispose( v8::Persistent<v8::Value> handle, void* pVoid ){
		if( 0 ) {
			v8::HandleScope handle_scope;
			CVsdFilter *thisObj = CScript::GetThis<CVsdFilter>( handle->ToObject());
			if( thisObj ){
				delete static_cast<CVsdFilter*>( thisObj );
				#ifdef DEBUG
					DebugMsgD( "<<<del js obj CVsdFilter:%X\n", thisObj );
				#endif
			}
		}
		handle.Dispose();
	}
	
	// JavaScript からの明示的な破棄
	static v8::Handle<v8::Value> Func_Dispose( const v8::Arguments& args ){
		// obj の Dispose() を呼ぶ
		CVsdFilter *thisObj = CScript::GetThis<CVsdFilter>( args.This());
		if( 0 ) if( thisObj ){
			delete thisObj;
			#ifdef DEBUG
				DebugMsgD( "<<<DISPOSE js obj CVsdFilter:%X\n", thisObj );
			#endif
			
			// internalfield を null っぽくする
			args.This()->SetInternalField( 0, v8::External::New( NULL ));
		}
		return v8::Undefined();
	}
	
	///// プロパティアクセサ /////
	static v8::Handle<v8::Value> Get_ElapsedTime( v8::Local<v8::String> propertyName, const v8::AccessorInfo& info ){
		CVsdFilter *obj = CScript::GetThis<CVsdFilter>( info.Holder());
		return obj ? v8::Integer::New( obj->CurTime() ) : v8::Undefined();
	}
	static v8::Handle<v8::Value> Get_BestLapTime( v8::Local<v8::String> propertyName, const v8::AccessorInfo& info ){
		CVsdFilter *obj = CScript::GetThis<CVsdFilter>( info.Holder());
		return obj ? v8::Integer::New( obj->BestLapTime() ) : v8::Undefined();
	}
	static v8::Handle<v8::Value> Get_DiffTime( v8::Local<v8::String> propertyName, const v8::AccessorInfo& info ){
		CVsdFilter *obj = CScript::GetThis<CVsdFilter>( info.Holder());
		return obj ? v8::Integer::New( obj->DiffTime() ) : v8::Undefined();
	}
	static v8::Handle<v8::Value> Get_LapTime( v8::Local<v8::String> propertyName, const v8::AccessorInfo& info ){
		CVsdFilter *obj = CScript::GetThis<CVsdFilter>( info.Holder());
		return obj ? v8::Integer::New( obj->LapTime() ) : v8::Undefined();
	}
	static v8::Handle<v8::Value> Get_LapCnt( v8::Local<v8::String> propertyName, const v8::AccessorInfo& info ){
		CVsdFilter *obj = CScript::GetThis<CVsdFilter>( info.Holder());
		return obj ? v8::Integer::New( obj->LapCnt() ) : v8::Undefined();
	}
	static v8::Handle<v8::Value> Get_MaxLapCnt( v8::Local<v8::String> propertyName, const v8::AccessorInfo& info ){
		CVsdFilter *obj = CScript::GetThis<CVsdFilter>( info.Holder());
		return obj ? v8::Integer::New( obj->MaxLapCnt() ) : v8::Undefined();
	}
	static v8::Handle<v8::Value> Get_Config_map_length( v8::Local<v8::String> propertyName, const v8::AccessorInfo& info ){
		CVsdFilter *obj = CScript::GetThis<CVsdFilter>( info.Holder());
		return obj ? v8::Integer::New( obj->LineTrace() ) : v8::Undefined();
	}
	static v8::Handle<v8::Value> Get_Config_lap_time( v8::Local<v8::String> propertyName, const v8::AccessorInfo& info ){
		CVsdFilter *obj = CScript::GetThis<CVsdFilter>( info.Holder());
		return obj ? v8::Integer::New( obj->DispLap() ) : v8::Undefined();
	}
	static v8::Handle<v8::Value> Get_Config_graph( v8::Local<v8::String> propertyName, const v8::AccessorInfo& info ){
		CVsdFilter *obj = CScript::GetThis<CVsdFilter>( info.Holder());
		return obj ? v8::Integer::New( obj->DispGraph() ) : v8::Undefined();
	}
	static v8::Handle<v8::Value> Get_Config_sync_mode( v8::Local<v8::String> propertyName, const v8::AccessorInfo& info ){
		CVsdFilter *obj = CScript::GetThis<CVsdFilter>( info.Holder());
		return obj ? v8::Integer::New( obj->DispSyncInfo() ) : v8::Undefined();
	}
	static v8::Handle<v8::Value> Get_FrameCnt( v8::Local<v8::String> propertyName, const v8::AccessorInfo& info ){
		CVsdFilter *obj = CScript::GetThis<CVsdFilter>( info.Holder());
		return obj ? v8::Integer::New( obj->GetFrameCnt() ) : v8::Undefined();
	}

	///// メャbドコールバック /////
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
	
	static v8::Handle<v8::Value> Func_PutPixel( const v8::Arguments& args ){
		int iLen = args.Length();
		if( CScript::CheckArgs( 3 <= iLen && iLen <= 4 )) return v8::Undefined();
		
		CVsdFilter *thisObj = CScript::GetThis<CVsdFilter>( args.This());
		if( !thisObj ) return v8::Undefined();
		thisObj->PutPixel(
			args[ 0 ]->Int32Value(),
			args[ 1 ]->Int32Value(),
			PIXEL_RABY::Argb2Raby( args[ 2 ]->Int32Value()),
			iLen <= 3 ? 0 : args[ 3 ]->Int32Value()
		);
		
		return v8::Undefined();
	}
	static v8::Handle<v8::Value> Func_PutImage( const v8::Arguments& args ){
		int iLen = args.Length();
		if( CScript::CheckArgs( 3 <= iLen && iLen <= 4 )) return v8::Undefined();
		v8::Local<v8::Object> Image2 = args[ 2 ]->ToObject();
		if( CScript::CheckClass( Image2, "Image", "arg[ 3 ] must be Image" )) return v8::Undefined();
		CVsdImage *obj2 = CScript::GetThis<CVsdImage>( Image2 );
		if( !obj2 ) return v8::Undefined();
		CVsdFilter *thisObj = CScript::GetThis<CVsdFilter>( args.This());
		if( !thisObj ) return v8::Undefined();
		int ret = thisObj->PutImage(
			args[ 0 ]->Int32Value(),
			args[ 1 ]->Int32Value(),
			*obj2,
			iLen <= 3 ? 0 : args[ 3 ]->Int32Value()
		);
		
		return v8::Integer::New( ret );
	}
	static v8::Handle<v8::Value> Func_DrawLine( const v8::Arguments& args ){
		int iLen = args.Length();
		if( CScript::CheckArgs( 5 <= iLen && iLen <= 7 )) return v8::Undefined();
		
		CVsdFilter *thisObj = CScript::GetThis<CVsdFilter>( args.This());
		if( !thisObj ) return v8::Undefined();
		thisObj->DrawLine(
			args[ 0 ]->Int32Value(),
			args[ 1 ]->Int32Value(),
			args[ 2 ]->Int32Value(),
			args[ 3 ]->Int32Value(),
			iLen <= 5 ? 1 : args[ 5 ]->Int32Value(),
			PIXEL_RABY::Argb2Raby( args[ 4 ]->Int32Value()),
			iLen <= 6 ? 0 : args[ 6 ]->Int32Value()
		);
		
		return v8::Undefined();
	}
	static v8::Handle<v8::Value> Func_DrawRect( const v8::Arguments& args ){
		int iLen = args.Length();
		if( CScript::CheckArgs( 5 <= iLen && iLen <= 6 )) return v8::Undefined();
		
		CVsdFilter *thisObj = CScript::GetThis<CVsdFilter>( args.This());
		if( !thisObj ) return v8::Undefined();
		thisObj->DrawRect(
			args[ 0 ]->Int32Value(),
			args[ 1 ]->Int32Value(),
			args[ 2 ]->Int32Value(),
			args[ 3 ]->Int32Value(),
			PIXEL_RABY::Argb2Raby( args[ 4 ]->Int32Value()),
			iLen <= 5 ? 0 : args[ 5 ]->Int32Value()
		);
		
		return v8::Undefined();
	}
	static v8::Handle<v8::Value> Func_DrawCircle( const v8::Arguments& args ){
		int iLen = args.Length();
		if( CScript::CheckArgs( 4 <= iLen && iLen <= 5 )) return v8::Undefined();
		
		CVsdFilter *thisObj = CScript::GetThis<CVsdFilter>( args.This());
		if( !thisObj ) return v8::Undefined();
		thisObj->DrawCircle(
			args[ 0 ]->Int32Value(),
			args[ 1 ]->Int32Value(),
			args[ 2 ]->Int32Value(),
			PIXEL_RABY::Argb2Raby( args[ 3 ]->Int32Value()),
			iLen <= 4 ? 0 : args[ 4 ]->Int32Value()
		);
		
		return v8::Undefined();
	}
	static v8::Handle<v8::Value> Func_DrawText( const v8::Arguments& args ){
		int iLen = args.Length();
		if( CScript::CheckArgs( 4 <= iLen && iLen <= 6 )) return v8::Undefined();
		v8::String::Value str2( args[ 2 ] );
		v8::Local<v8::Object> Font3 = args[ 3 ]->ToObject();
		if( CScript::CheckClass( Font3, "Font", "arg[ 4 ] must be Font" )) return v8::Undefined();
		CVsdFont *obj3 = CScript::GetThis<CVsdFont>( Font3 );
		if( !obj3 ) return v8::Undefined();
		CVsdFilter *thisObj = CScript::GetThis<CVsdFilter>( args.This());
		if( !thisObj ) return v8::Undefined();
		thisObj->DrawText(
			args[ 0 ]->Int32Value(),
			args[ 1 ]->Int32Value(),
			( LPCWSTR )*str2,
			*obj3,
			iLen <= 4 ? color_white : PIXEL_RABY::Argb2Raby( args[ 4 ]->Int32Value()),
			iLen <= 5 ? color_black : PIXEL_RABY::Argb2Raby( args[ 5 ]->Int32Value())
		);
		
		return v8::Undefined();
	}
	static v8::Handle<v8::Value> Func_DrawTextAlign( const v8::Arguments& args ){
		int iLen = args.Length();
		if( CScript::CheckArgs( 5 <= iLen && iLen <= 7 )) return v8::Undefined();
		v8::String::Value str3( args[ 3 ] );
		v8::Local<v8::Object> Font4 = args[ 4 ]->ToObject();
		if( CScript::CheckClass( Font4, "Font", "arg[ 5 ] must be Font" )) return v8::Undefined();
		CVsdFont *obj4 = CScript::GetThis<CVsdFont>( Font4 );
		if( !obj4 ) return v8::Undefined();
		CVsdFilter *thisObj = CScript::GetThis<CVsdFilter>( args.This());
		if( !thisObj ) return v8::Undefined();
		thisObj->DrawTextAlign(
			args[ 0 ]->Int32Value(),
			args[ 1 ]->Int32Value(),
			args[ 2 ]->Int32Value(),
			( LPCWSTR )*str3,
			*obj4,
			iLen <= 5 ? color_white : PIXEL_RABY::Argb2Raby( args[ 5 ]->Int32Value()),
			iLen <= 6 ? color_black : PIXEL_RABY::Argb2Raby( args[ 6 ]->Int32Value())
		);
		
		return v8::Undefined();
	}
	static v8::Handle<v8::Value> Func_DrawGraphSingle( const v8::Arguments& args ){
		int iLen = args.Length();
		if( CScript::CheckArgs( iLen == 8 )) return v8::Undefined();
		v8::String::AsciiValue str4( args[ 4 ] );
		v8::String::Value str5( args[ 5 ] );
		v8::Local<v8::Object> Font6 = args[ 6 ]->ToObject();
		if( CScript::CheckClass( Font6, "Font", "arg[ 7 ] must be Font" )) return v8::Undefined();
		CVsdFont *obj6 = CScript::GetThis<CVsdFont>( Font6 );
		if( !obj6 ) return v8::Undefined();
		CVsdFilter *thisObj = CScript::GetThis<CVsdFilter>( args.This());
		if( !thisObj ) return v8::Undefined();
		thisObj->DrawGraphSingle(
			args[ 0 ]->Int32Value(),
			args[ 1 ]->Int32Value(),
			args[ 2 ]->Int32Value(),
			args[ 3 ]->Int32Value(),
			*str4,
			( LPCWSTR )*str5,
			*obj6,
			PIXEL_RABY::Argb2Raby( args[ 7 ]->Int32Value())
		);
		
		return v8::Undefined();
	}
	static v8::Handle<v8::Value> Func_DrawGraphMulti( const v8::Arguments& args ){
		int iLen = args.Length();
		if( CScript::CheckArgs( 5 <= iLen && iLen <= 6 )) return v8::Undefined();
		v8::Local<v8::Object> Font4 = args[ 4 ]->ToObject();
		if( CScript::CheckClass( Font4, "Font", "arg[ 5 ] must be Font" )) return v8::Undefined();
		CVsdFont *obj4 = CScript::GetThis<CVsdFont>( Font4 );
		if( !obj4 ) return v8::Undefined();
		CVsdFilter *thisObj = CScript::GetThis<CVsdFilter>( args.This());
		if( !thisObj ) return v8::Undefined();
		thisObj->DrawGraphMulti(
			args[ 0 ]->Int32Value(),
			args[ 1 ]->Int32Value(),
			args[ 2 ]->Int32Value(),
			args[ 3 ]->Int32Value(),
			*obj4,
			iLen <= 5 ? 1 : args[ 5 ]->Int32Value()
		);
		
		return v8::Undefined();
	}
	static v8::Handle<v8::Value> Func_InitPolygon( const v8::Arguments& args ){
		int iLen = args.Length();
		if( CScript::CheckArgs( iLen == 0 )) return v8::Undefined();
		
		CVsdFilter *thisObj = CScript::GetThis<CVsdFilter>( args.This());
		if( !thisObj ) return v8::Undefined();
		thisObj->InitPolygon();
		
		return v8::Undefined();
	}
	static v8::Handle<v8::Value> Func_DrawPolygon( const v8::Arguments& args ){
		int iLen = args.Length();
		if( CScript::CheckArgs( iLen == 1 )) return v8::Undefined();
		
		CVsdFilter *thisObj = CScript::GetThis<CVsdFilter>( args.This());
		if( !thisObj ) return v8::Undefined();
		thisObj->DrawPolygon(
			PIXEL_RABY::Argb2Raby( args[ 0 ]->Int32Value())
		);
		
		return v8::Undefined();
	}
	static v8::Handle<v8::Value> Func_DrawGSnake( const v8::Arguments& args ){
		int iLen = args.Length();
		if( CScript::CheckArgs( 7 <= iLen && iLen <= 8 )) return v8::Undefined();
		
		CVsdFilter *thisObj = CScript::GetThis<CVsdFilter>( args.This());
		if( !thisObj ) return v8::Undefined();
		thisObj->DrawGSnake(
			args[ 0 ]->Int32Value(),
			args[ 1 ]->Int32Value(),
			args[ 2 ]->Int32Value(),
			args[ 3 ]->Int32Value(),
			args[ 4 ]->Int32Value(),
			PIXEL_RABY::Argb2Raby( args[ 5 ]->Int32Value()),
			PIXEL_RABY::Argb2Raby( args[ 6 ]->Int32Value()),
			iLen <= 7 ? 3 : args[ 7 ]->NumberValue()
		);
		
		return v8::Undefined();
	}
	static v8::Handle<v8::Value> Func_DrawMeterScale( const v8::Arguments& args ){
		int iLen = args.Length();
		if( CScript::CheckArgs( iLen == 17 )) return v8::Undefined();
		v8::Local<v8::Object> Font16 = args[ 16 ]->ToObject();
		if( CScript::CheckClass( Font16, "Font", "arg[ 17 ] must be Font" )) return v8::Undefined();
		CVsdFont *obj16 = CScript::GetThis<CVsdFont>( Font16 );
		if( !obj16 ) return v8::Undefined();
		CVsdFilter *thisObj = CScript::GetThis<CVsdFilter>( args.This());
		if( !thisObj ) return v8::Undefined();
		thisObj->DrawMeterScale(
			args[ 0 ]->Int32Value(),
			args[ 1 ]->Int32Value(),
			args[ 2 ]->Int32Value(),
			args[ 3 ]->Int32Value(),
			args[ 4 ]->Int32Value(),
			PIXEL_RABY::Argb2Raby( args[ 5 ]->Int32Value()),
			args[ 6 ]->Int32Value(),
			args[ 7 ]->Int32Value(),
			PIXEL_RABY::Argb2Raby( args[ 8 ]->Int32Value()),
			args[ 9 ]->Int32Value(),
			args[ 10 ]->Int32Value(),
			args[ 11 ]->Int32Value(),
			args[ 12 ]->Int32Value(),
			args[ 13 ]->Int32Value(),
			args[ 14 ]->Int32Value(),
			PIXEL_RABY::Argb2Raby( args[ 15 ]->Int32Value()),
			*obj16
		);
		
		return v8::Undefined();
	}
	static v8::Handle<v8::Value> Func_DrawMap( const v8::Arguments& args ){
		int iLen = args.Length();
		if( CScript::CheckArgs( iLen == 11 )) return v8::Undefined();
		
		CVsdFilter *thisObj = CScript::GetThis<CVsdFilter>( args.This());
		if( !thisObj ) return v8::Undefined();
		thisObj->DrawMap(
			args[ 0 ]->Int32Value(),
			args[ 1 ]->Int32Value(),
			args[ 2 ]->Int32Value(),
			args[ 3 ]->Int32Value(),
			args[ 4 ]->Int32Value(),
			args[ 5 ]->Int32Value(),
			args[ 6 ]->Int32Value(),
			PIXEL_RABY::Argb2Raby( args[ 7 ]->Int32Value()),
			PIXEL_RABY::Argb2Raby( args[ 8 ]->Int32Value()),
			PIXEL_RABY::Argb2Raby( args[ 9 ]->Int32Value()),
			PIXEL_RABY::Argb2Raby( args[ 10 ]->Int32Value())
		);
		
		return v8::Undefined();
	}
	static v8::Handle<v8::Value> Func_DrawLapTime( const v8::Arguments& args ){
		int iLen = args.Length();
		if( CScript::CheckArgs( 4 <= iLen && iLen <= 8 )) return v8::Undefined();
		v8::Local<v8::Object> Font3 = args[ 3 ]->ToObject();
		if( CScript::CheckClass( Font3, "Font", "arg[ 4 ] must be Font" )) return v8::Undefined();
		CVsdFont *obj3 = CScript::GetThis<CVsdFont>( Font3 );
		if( !obj3 ) return v8::Undefined();
		CVsdFilter *thisObj = CScript::GetThis<CVsdFilter>( args.This());
		if( !thisObj ) return v8::Undefined();
		thisObj->DrawLapTime(
			args[ 0 ]->Int32Value(),
			args[ 1 ]->Int32Value(),
			args[ 2 ]->Int32Value(),
			*obj3,
			iLen <= 4 ? color_white : PIXEL_RABY::Argb2Raby( args[ 4 ]->Int32Value()),
			iLen <= 5 ? color_cyan : PIXEL_RABY::Argb2Raby( args[ 5 ]->Int32Value()),
			iLen <= 6 ? color_red : PIXEL_RABY::Argb2Raby( args[ 6 ]->Int32Value()),
			iLen <= 7 ? color_black : PIXEL_RABY::Argb2Raby( args[ 7 ]->Int32Value())
		);
		
		return v8::Undefined();
	}
	static v8::Handle<v8::Value> Func_DrawLapTimeLog( const v8::Arguments& args ){
		int iLen = args.Length();
		if( CScript::CheckArgs( 5 <= iLen && iLen <= 8 )) return v8::Undefined();
		v8::Local<v8::Object> Font4 = args[ 4 ]->ToObject();
		if( CScript::CheckClass( Font4, "Font", "arg[ 5 ] must be Font" )) return v8::Undefined();
		CVsdFont *obj4 = CScript::GetThis<CVsdFont>( Font4 );
		if( !obj4 ) return v8::Undefined();
		CVsdFilter *thisObj = CScript::GetThis<CVsdFilter>( args.This());
		if( !thisObj ) return v8::Undefined();
		thisObj->DrawLapTimeLog(
			args[ 0 ]->Int32Value(),
			args[ 1 ]->Int32Value(),
			args[ 2 ]->Int32Value(),
			args[ 3 ]->Int32Value(),
			*obj4,
			iLen <= 5 ? color_white : PIXEL_RABY::Argb2Raby( args[ 5 ]->Int32Value()),
			iLen <= 6 ? color_cyan : PIXEL_RABY::Argb2Raby( args[ 6 ]->Int32Value()),
			iLen <= 7 ? color_black : PIXEL_RABY::Argb2Raby( args[ 7 ]->Int32Value())
		);
		
		return v8::Undefined();
	}
	static v8::Handle<v8::Value> Func_DrawNeedle( const v8::Arguments& args ){
		int iLen = args.Length();
		if( CScript::CheckArgs( 8 <= iLen && iLen <= 9 )) return v8::Undefined();
		
		CVsdFilter *thisObj = CScript::GetThis<CVsdFilter>( args.This());
		if( !thisObj ) return v8::Undefined();
		thisObj->DrawNeedle(
			args[ 0 ]->Int32Value(),
			args[ 1 ]->Int32Value(),
			args[ 2 ]->Int32Value(),
			args[ 3 ]->Int32Value(),
			args[ 4 ]->Int32Value(),
			args[ 5 ]->Int32Value(),
			args[ 6 ]->NumberValue(),
			PIXEL_RABY::Argb2Raby( args[ 7 ]->Int32Value()),
			iLen <= 8 ? 1 : args[ 8 ]->Int32Value()
		);
		
		return v8::Undefined();
	}
	static v8::Handle<v8::Value> Func_FormatTime( const v8::Arguments& args ){
		int iLen = args.Length();
		if( CScript::CheckArgs( iLen == 1 )) return v8::Undefined();
		
		CVsdFilter *thisObj = CScript::GetThis<CVsdFilter>( args.This());
		if( !thisObj ) return v8::Undefined();
		LPCWSTR ret = thisObj->FormatTime(
			args[ 0 ]->Int32Value()
		);
		
		return v8::String::New(( uint16_t *)ret );
	}
	static v8::Handle<v8::Value> Func_DateTime( const v8::Arguments& args ){
		int iLen = args.Length();
		if( CScript::CheckArgs( iLen == 0 )) return v8::Undefined();
		
		CVsdFilter *thisObj = CScript::GetThis<CVsdFilter>( args.This());
		if( !thisObj ) return v8::Undefined();
		double ret = thisObj->DateTime();
		
		return v8::Number::New( ret );
	}

  public:
	// クラステンプレートの初期化
	static void InitializeClass( v8::Handle<v8::ObjectTemplate> global, void *pClass = NULL ){
		// コンストラクタを作成
		v8::Local<v8::FunctionTemplate> tmpl = v8::FunctionTemplate::New( New );
		tmpl->SetClassName( v8::String::New( "__VSD_System__" ));
		
		// フィールドなどはこちらに
		v8::Handle<v8::ObjectTemplate> inst = tmpl->InstanceTemplate();
		inst->SetInternalFieldCount( 1 );
		inst->SetAccessor( v8::String::New( "ElapsedTime" ), Get_ElapsedTime );
		inst->SetAccessor( v8::String::New( "BestLapTime" ), Get_BestLapTime );
		inst->SetAccessor( v8::String::New( "DiffTime" ), Get_DiffTime );
		inst->SetAccessor( v8::String::New( "LapTime" ), Get_LapTime );
		inst->SetAccessor( v8::String::New( "LapCnt" ), Get_LapCnt );
		inst->SetAccessor( v8::String::New( "MaxLapCnt" ), Get_MaxLapCnt );
		inst->SetAccessor( v8::String::New( "Config_map_length" ), Get_Config_map_length );
		inst->SetAccessor( v8::String::New( "Config_lap_time" ), Get_Config_lap_time );
		inst->SetAccessor( v8::String::New( "Config_graph" ), Get_Config_graph );
		inst->SetAccessor( v8::String::New( "Config_sync_mode" ), Get_Config_sync_mode );
		inst->SetAccessor( v8::String::New( "FrameCnt" ), Get_FrameCnt );

		// メャbドはこちらに
		v8::Handle<v8::ObjectTemplate> proto = tmpl->PrototypeTemplate();
		proto->Set( v8::String::New( "Dispose" ), v8::FunctionTemplate::New( Func_Dispose ));
		proto->Set( v8::String::New( "DrawArc" ), v8::FunctionTemplate::New( Func_DrawArc ));
		proto->Set( v8::String::New( "print" ), v8::FunctionTemplate::New( Func_print ));
		proto->Set( v8::String::New( "PutPixel" ), v8::FunctionTemplate::New( Func_PutPixel ));
		proto->Set( v8::String::New( "PutImage" ), v8::FunctionTemplate::New( Func_PutImage ));
		proto->Set( v8::String::New( "DrawLine" ), v8::FunctionTemplate::New( Func_DrawLine ));
		proto->Set( v8::String::New( "DrawRect" ), v8::FunctionTemplate::New( Func_DrawRect ));
		proto->Set( v8::String::New( "DrawCircle" ), v8::FunctionTemplate::New( Func_DrawCircle ));
		proto->Set( v8::String::New( "DrawText" ), v8::FunctionTemplate::New( Func_DrawText ));
		proto->Set( v8::String::New( "DrawTextAlign" ), v8::FunctionTemplate::New( Func_DrawTextAlign ));
		proto->Set( v8::String::New( "DrawGraphSingle" ), v8::FunctionTemplate::New( Func_DrawGraphSingle ));
		proto->Set( v8::String::New( "DrawGraphMulti" ), v8::FunctionTemplate::New( Func_DrawGraphMulti ));
		proto->Set( v8::String::New( "InitPolygon" ), v8::FunctionTemplate::New( Func_InitPolygon ));
		proto->Set( v8::String::New( "DrawPolygon" ), v8::FunctionTemplate::New( Func_DrawPolygon ));
		proto->Set( v8::String::New( "DrawGSnake" ), v8::FunctionTemplate::New( Func_DrawGSnake ));
		proto->Set( v8::String::New( "DrawMeterScale" ), v8::FunctionTemplate::New( Func_DrawMeterScale ));
		proto->Set( v8::String::New( "DrawMap" ), v8::FunctionTemplate::New( Func_DrawMap ));
		proto->Set( v8::String::New( "DrawLapTime" ), v8::FunctionTemplate::New( Func_DrawLapTime ));
		proto->Set( v8::String::New( "DrawLapTimeLog" ), v8::FunctionTemplate::New( Func_DrawLapTimeLog ));
		proto->Set( v8::String::New( "DrawNeedle" ), v8::FunctionTemplate::New( Func_DrawNeedle ));
		proto->Set( v8::String::New( "FormatTime" ), v8::FunctionTemplate::New( Func_FormatTime ));
		proto->Set( v8::String::New( "DateTime" ), v8::FunctionTemplate::New( Func_DateTime ));

		proto->Set( v8::String::New( "Width" ), v8::Integer::New((( CVsdFilter *)pClass )->GetWidth() ));
		proto->Set( v8::String::New( "Height" ), v8::Integer::New((( CVsdFilter *)pClass )->GetHeight() ));
		proto->Set( v8::String::New( "MaxFrameCnt" ), v8::Integer::New((( CVsdFilter *)pClass )->GetFrameMax() ));
		proto->Set( v8::String::New( "SkinDir" ), v8::String::New(( uint16_t *)(( CVsdFilter *)pClass )->m_szSkinDirW ));
		proto->Set( v8::String::New( "VsdRootDir" ), v8::String::New(( uint16_t *)(( CVsdFilter *)pClass )->m_szPluginDirW ));

		(( CVsdFilter *)pClass )->InitJS( tmpl );

		// グローバルオブジェクトにクラスを定義
		global->Set( v8::String::New( "__VSD_System__" ), tmpl );
	}
};
/****************************************************************************/

class CVsdImageIF {
  public:
	// クラスコンストラクタ
	static v8::Handle<v8::Value> New( const v8::Arguments& args ){
		
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
			
			if( obj->Load(
				( LPCWSTR )*FileName,
				args.Length() <= 1 ? 0 : args[ 1 ]->Int32Value()
			) != ERR_OK ){
				delete obj;
				return v8::Undefined();
			}
		}

		// internal field にバックエンドオブジェクトを設定
		v8::Local<v8::Object> thisObject = args.This();
		thisObject->SetInternalField( 0, v8::External::New( obj ));
		
		// JS オブジェクトが GC されるときにデストラクタが呼ばれるおまじない
		v8::Persistent<v8::Object> objectHolder = v8::Persistent<v8::Object>::New( thisObject );
		objectHolder.MakeWeak( obj, Dispose );
		
		#ifdef DEBUG
			DebugMsgD( ">>>new js obj CVsdImage:%X\n", obj );
		#endif
		// コンストラクタは this を返すこと。
		return thisObject;
	}
	
	// クラスデストラクタ
	static void Dispose( v8::Persistent<v8::Value> handle, void* pVoid ){
		 {
			v8::HandleScope handle_scope;
			CVsdImage *thisObj = CScript::GetThis<CVsdImage>( handle->ToObject());
			if( thisObj ){
				delete static_cast<CVsdImage*>( thisObj );
				#ifdef DEBUG
					DebugMsgD( "<<<del js obj CVsdImage:%X\n", thisObj );
				#endif
			}
		}
		handle.Dispose();
	}
	
	// JavaScript からの明示的な破棄
	static v8::Handle<v8::Value> Func_Dispose( const v8::Arguments& args ){
		// obj の Dispose() を呼ぶ
		CVsdImage *thisObj = CScript::GetThis<CVsdImage>( args.This());
		 if( thisObj ){
			delete thisObj;
			#ifdef DEBUG
				DebugMsgD( "<<<DISPOSE js obj CVsdImage:%X\n", thisObj );
			#endif
			
			// internalfield を null っぽくする
			args.This()->SetInternalField( 0, v8::External::New( NULL ));
		}
		return v8::Undefined();
	}
	
	///// プロパティアクセサ /////
	static v8::Handle<v8::Value> Get_Width( v8::Local<v8::String> propertyName, const v8::AccessorInfo& info ){
		CVsdImage *obj = CScript::GetThis<CVsdImage>( info.Holder());
		return obj ? v8::Integer::New( obj->m_iWidth ) : v8::Undefined();
	}
	static v8::Handle<v8::Value> Get_Height( v8::Local<v8::String> propertyName, const v8::AccessorInfo& info ){
		CVsdImage *obj = CScript::GetThis<CVsdImage>( info.Holder());
		return obj ? v8::Integer::New( obj->m_iHeight ) : v8::Undefined();
	}
	static v8::Handle<v8::Value> Get_Status( v8::Local<v8::String> propertyName, const v8::AccessorInfo& info ){
		CVsdImage *obj = CScript::GetThis<CVsdImage>( info.Holder());
		return obj ? v8::Integer::New( obj->m_iStatus ) : v8::Undefined();
	}

	///// メャbドコールバック /////
	static v8::Handle<v8::Value> Func_Resize( const v8::Arguments& args ){
		int iLen = args.Length();
		if( CScript::CheckArgs( iLen == 2 )) return v8::Undefined();
		
		CVsdImage *thisObj = CScript::GetThis<CVsdImage>( args.This());
		if( !thisObj ) return v8::Undefined();
		int ret = thisObj->Resize(
			args[ 0 ]->Int32Value(),
			args[ 1 ]->Int32Value()
		);
		
		return v8::Integer::New( ret );
	}
	static v8::Handle<v8::Value> Func_Rotate( const v8::Arguments& args ){
		int iLen = args.Length();
		if( CScript::CheckArgs( iLen == 3 )) return v8::Undefined();
		
		CVsdImage *thisObj = CScript::GetThis<CVsdImage>( args.This());
		if( !thisObj ) return v8::Undefined();
		int ret = thisObj->Rotate(
			args[ 0 ]->Int32Value(),
			args[ 1 ]->Int32Value(),
			args[ 2 ]->NumberValue()
		);
		
		return v8::Integer::New( ret );
	}

  public:
	// クラステンプレートの初期化
	static void InitializeClass( v8::Handle<v8::ObjectTemplate> global, void *pClass = NULL ){
		// コンストラクタを作成
		v8::Local<v8::FunctionTemplate> tmpl = v8::FunctionTemplate::New( New );
		tmpl->SetClassName( v8::String::New( "Image" ));
		
		// フィールドなどはこちらに
		v8::Handle<v8::ObjectTemplate> inst = tmpl->InstanceTemplate();
		inst->SetInternalFieldCount( 1 );
		inst->SetAccessor( v8::String::New( "Width" ), Get_Width );
		inst->SetAccessor( v8::String::New( "Height" ), Get_Height );
		inst->SetAccessor( v8::String::New( "Status" ), Get_Status );

		// メャbドはこちらに
		v8::Handle<v8::ObjectTemplate> proto = tmpl->PrototypeTemplate();
		proto->Set( v8::String::New( "Dispose" ), v8::FunctionTemplate::New( Func_Dispose ));
		proto->Set( v8::String::New( "Resize" ), v8::FunctionTemplate::New( Func_Resize ));
		proto->Set( v8::String::New( "Rotate" ), v8::FunctionTemplate::New( Func_Rotate ));



		// グローバルオブジェクトにクラスを定義
		global->Set( v8::String::New( "Image" ), tmpl );
	}
};
/****************************************************************************/

class CVsdFontIF {
  public:
	// クラスコンストラクタ
	static v8::Handle<v8::Value> New( const v8::Arguments& args ){
		
		// 引数チェック
		if ( args.Length() < 2 ) return v8::Undefined();
		
		v8::String::Value FontName( args[ 0 ] );
		CVsdFont *obj = new CVsdFont(
			( LPCWSTR )*FontName,
			args[ 1 ]->Int32Value(),
			args.Length() <= 2 ? 0 : args[ 2 ]->Int32Value()
		);

		// internal field にバックエンドオブジェクトを設定
		v8::Local<v8::Object> thisObject = args.This();
		thisObject->SetInternalField( 0, v8::External::New( obj ));
		
		// JS オブジェクトが GC されるときにデストラクタが呼ばれるおまじない
		v8::Persistent<v8::Object> objectHolder = v8::Persistent<v8::Object>::New( thisObject );
		objectHolder.MakeWeak( obj, Dispose );
		
		#ifdef DEBUG
			DebugMsgD( ">>>new js obj CVsdFont:%X\n", obj );
		#endif
		// コンストラクタは this を返すこと。
		return thisObject;
	}
	
	// クラスデストラクタ
	static void Dispose( v8::Persistent<v8::Value> handle, void* pVoid ){
		 {
			v8::HandleScope handle_scope;
			CVsdFont *thisObj = CScript::GetThis<CVsdFont>( handle->ToObject());
			if( thisObj ){
				delete static_cast<CVsdFont*>( thisObj );
				#ifdef DEBUG
					DebugMsgD( "<<<del js obj CVsdFont:%X\n", thisObj );
				#endif
			}
		}
		handle.Dispose();
	}
	
	// JavaScript からの明示的な破棄
	static v8::Handle<v8::Value> Func_Dispose( const v8::Arguments& args ){
		// obj の Dispose() を呼ぶ
		CVsdFont *thisObj = CScript::GetThis<CVsdFont>( args.This());
		 if( thisObj ){
			delete thisObj;
			#ifdef DEBUG
				DebugMsgD( "<<<DISPOSE js obj CVsdFont:%X\n", thisObj );
			#endif
			
			// internalfield を null っぽくする
			args.This()->SetInternalField( 0, v8::External::New( NULL ));
		}
		return v8::Undefined();
	}
	
	///// プロパティアクセサ /////
	static v8::Handle<v8::Value> Get_Height( v8::Local<v8::String> propertyName, const v8::AccessorInfo& info ){
		CVsdFont *obj = CScript::GetThis<CVsdFont>( info.Holder());
		return obj ? v8::Integer::New( obj->GetHeight() ) : v8::Undefined();
	}

	///// メャbドコールバック /////
	static v8::Handle<v8::Value> Func_GetTextWidth( const v8::Arguments& args ){
		int iLen = args.Length();
		if( CScript::CheckArgs( iLen == 1 )) return v8::Undefined();
		v8::String::Value str0( args[ 0 ] );
		CVsdFont *thisObj = CScript::GetThis<CVsdFont>( args.This());
		if( !thisObj ) return v8::Undefined();
		int ret = thisObj->GetTextWidth(
			( LPCWSTR )*str0
		);
		
		return v8::Integer::New( ret );
	}

  public:
	// クラステンプレートの初期化
	static void InitializeClass( v8::Handle<v8::ObjectTemplate> global, void *pClass = NULL ){
		// コンストラクタを作成
		v8::Local<v8::FunctionTemplate> tmpl = v8::FunctionTemplate::New( New );
		tmpl->SetClassName( v8::String::New( "Font" ));
		
		// フィールドなどはこちらに
		v8::Handle<v8::ObjectTemplate> inst = tmpl->InstanceTemplate();
		inst->SetInternalFieldCount( 1 );
		inst->SetAccessor( v8::String::New( "Height" ), Get_Height );

		// メャbドはこちらに
		v8::Handle<v8::ObjectTemplate> proto = tmpl->PrototypeTemplate();
		proto->Set( v8::String::New( "Dispose" ), v8::FunctionTemplate::New( Func_Dispose ));
		proto->Set( v8::String::New( "GetTextWidth" ), v8::FunctionTemplate::New( Func_GetTextWidth ));



		// グローバルオブジェクトにクラスを定義
		global->Set( v8::String::New( "Font" ), tmpl );
	}
};
/****************************************************************************/

class CVsdFileIF {
  public:
	// クラスコンストラクタ
	static v8::Handle<v8::Value> New( const v8::Arguments& args ){
		
		CVsdFile *obj = new CVsdFile();

		// internal field にバックエンドオブジェクトを設定
		v8::Local<v8::Object> thisObject = args.This();
		thisObject->SetInternalField( 0, v8::External::New( obj ));
		
		// JS オブジェクトが GC されるときにデストラクタが呼ばれるおまじない
		v8::Persistent<v8::Object> objectHolder = v8::Persistent<v8::Object>::New( thisObject );
		objectHolder.MakeWeak( obj, Dispose );
		
		#ifdef DEBUG
			DebugMsgD( ">>>new js obj CVsdFile:%X\n", obj );
		#endif
		// コンストラクタは this を返すこと。
		return thisObject;
	}
	
	// クラスデストラクタ
	static void Dispose( v8::Persistent<v8::Value> handle, void* pVoid ){
		 {
			v8::HandleScope handle_scope;
			CVsdFile *thisObj = CScript::GetThis<CVsdFile>( handle->ToObject());
			if( thisObj ){
				delete static_cast<CVsdFile*>( thisObj );
				#ifdef DEBUG
					DebugMsgD( "<<<del js obj CVsdFile:%X\n", thisObj );
				#endif
			}
		}
		handle.Dispose();
	}
	
	// JavaScript からの明示的な破棄
	static v8::Handle<v8::Value> Func_Dispose( const v8::Arguments& args ){
		// obj の Dispose() を呼ぶ
		CVsdFile *thisObj = CScript::GetThis<CVsdFile>( args.This());
		 if( thisObj ){
			delete thisObj;
			#ifdef DEBUG
				DebugMsgD( "<<<DISPOSE js obj CVsdFile:%X\n", thisObj );
			#endif
			
			// internalfield を null っぽくする
			args.This()->SetInternalField( 0, v8::External::New( NULL ));
		}
		return v8::Undefined();
	}
	
	///// プロパティアクセサ /////

	///// メャbドコールバック /////
	static v8::Handle<v8::Value> Func_Open( const v8::Arguments& args ){
		int iLen = args.Length();
		if( CScript::CheckArgs( iLen == 2 )) return v8::Undefined();
		v8::String::Value str0( args[ 0 ] );
		v8::String::Value str1( args[ 1 ] );
		CVsdFile *thisObj = CScript::GetThis<CVsdFile>( args.This());
		if( !thisObj ) return v8::Undefined();
		int ret = thisObj->Open(
			( LPCWSTR )*str0,
			( LPCWSTR )*str1
		);
		
		return v8::Integer::New( ret );
	}
	static v8::Handle<v8::Value> Func_Close( const v8::Arguments& args ){
		int iLen = args.Length();
		if( CScript::CheckArgs( iLen == 0 )) return v8::Undefined();
		
		CVsdFile *thisObj = CScript::GetThis<CVsdFile>( args.This());
		if( !thisObj ) return v8::Undefined();
		thisObj->Close();
		
		return v8::Undefined();
	}
	static v8::Handle<v8::Value> Func_ReadLine( const v8::Arguments& args ){
		int iLen = args.Length();
		if( CScript::CheckArgs( iLen == 0 )) return v8::Undefined();
		
		CVsdFile *thisObj = CScript::GetThis<CVsdFile>( args.This());
		if( !thisObj ) return v8::Undefined();
		char *ret = thisObj->ReadLine();
		
		return v8::String::New( ret );
	}
	static v8::Handle<v8::Value> Func_WriteLine( const v8::Arguments& args ){
		int iLen = args.Length();
		if( CScript::CheckArgs( iLen == 1 )) return v8::Undefined();
		v8::String::AsciiValue str0( args[ 0 ] );
		CVsdFile *thisObj = CScript::GetThis<CVsdFile>( args.This());
		if( !thisObj ) return v8::Undefined();
		int ret = thisObj->WriteLine(
			*str0
		);
		
		return v8::Integer::New( ret );
	}
	static v8::Handle<v8::Value> Func_Seek( const v8::Arguments& args ){
		int iLen = args.Length();
		if( CScript::CheckArgs( iLen == 2 )) return v8::Undefined();
		
		CVsdFile *thisObj = CScript::GetThis<CVsdFile>( args.This());
		if( !thisObj ) return v8::Undefined();
		int ret = thisObj->Seek(
			args[ 0 ]->Int32Value(),
			args[ 1 ]->Int32Value()
		);
		
		return v8::Integer::New( ret );
	}
	static v8::Handle<v8::Value> Func_IsEOF( const v8::Arguments& args ){
		int iLen = args.Length();
		if( CScript::CheckArgs( iLen == 0 )) return v8::Undefined();
		
		CVsdFile *thisObj = CScript::GetThis<CVsdFile>( args.This());
		if( !thisObj ) return v8::Undefined();
		int ret = thisObj->IsEOF();
		
		return v8::Integer::New( ret );
	}
	static v8::Handle<v8::Value> Func_ReadChar( const v8::Arguments& args ){
		int iLen = args.Length();
		if( CScript::CheckArgs( iLen == 0 )) return v8::Undefined();
		
		CVsdFile *thisObj = CScript::GetThis<CVsdFile>( args.This());
		if( !thisObj ) return v8::Undefined();
		int ret = thisObj->ReadChar();
		
		return v8::Integer::New( ret );
	}
	static v8::Handle<v8::Value> Func_ReadUChar( const v8::Arguments& args ){
		int iLen = args.Length();
		if( CScript::CheckArgs( iLen == 0 )) return v8::Undefined();
		
		CVsdFile *thisObj = CScript::GetThis<CVsdFile>( args.This());
		if( !thisObj ) return v8::Undefined();
		int ret = thisObj->ReadUChar();
		
		return v8::Integer::New( ret );
	}
	static v8::Handle<v8::Value> Func_ReadShortL( const v8::Arguments& args ){
		int iLen = args.Length();
		if( CScript::CheckArgs( iLen == 0 )) return v8::Undefined();
		
		CVsdFile *thisObj = CScript::GetThis<CVsdFile>( args.This());
		if( !thisObj ) return v8::Undefined();
		int ret = thisObj->ReadShortL();
		
		return v8::Integer::New( ret );
	}
	static v8::Handle<v8::Value> Func_ReadUShortL( const v8::Arguments& args ){
		int iLen = args.Length();
		if( CScript::CheckArgs( iLen == 0 )) return v8::Undefined();
		
		CVsdFile *thisObj = CScript::GetThis<CVsdFile>( args.This());
		if( !thisObj ) return v8::Undefined();
		int ret = thisObj->ReadUShortL();
		
		return v8::Integer::New( ret );
	}
	static v8::Handle<v8::Value> Func_ReadIntL( const v8::Arguments& args ){
		int iLen = args.Length();
		if( CScript::CheckArgs( iLen == 0 )) return v8::Undefined();
		
		CVsdFile *thisObj = CScript::GetThis<CVsdFile>( args.This());
		if( !thisObj ) return v8::Undefined();
		int ret = thisObj->ReadIntL();
		
		return v8::Integer::New( ret );
	}
	static v8::Handle<v8::Value> Func_ReadUIntL( const v8::Arguments& args ){
		int iLen = args.Length();
		if( CScript::CheckArgs( iLen == 0 )) return v8::Undefined();
		
		CVsdFile *thisObj = CScript::GetThis<CVsdFile>( args.This());
		if( !thisObj ) return v8::Undefined();
		double ret = thisObj->ReadUIntL();
		
		return v8::Number::New( ret );
	}
	static v8::Handle<v8::Value> Func_ReadFloat( const v8::Arguments& args ){
		int iLen = args.Length();
		if( CScript::CheckArgs( iLen == 0 )) return v8::Undefined();
		
		CVsdFile *thisObj = CScript::GetThis<CVsdFile>( args.This());
		if( !thisObj ) return v8::Undefined();
		double ret = thisObj->ReadFloat();
		
		return v8::Number::New( ret );
	}
	static v8::Handle<v8::Value> Func_ReadDouble( const v8::Arguments& args ){
		int iLen = args.Length();
		if( CScript::CheckArgs( iLen == 0 )) return v8::Undefined();
		
		CVsdFile *thisObj = CScript::GetThis<CVsdFile>( args.This());
		if( !thisObj ) return v8::Undefined();
		double ret = thisObj->ReadDouble();
		
		return v8::Number::New( ret );
	}
	static v8::Handle<v8::Value> Func_ReadShortB( const v8::Arguments& args ){
		int iLen = args.Length();
		if( CScript::CheckArgs( iLen == 0 )) return v8::Undefined();
		
		CVsdFile *thisObj = CScript::GetThis<CVsdFile>( args.This());
		if( !thisObj ) return v8::Undefined();
		int ret = thisObj->ReadShortB();
		
		return v8::Integer::New( ret );
	}
	static v8::Handle<v8::Value> Func_ReadUShortB( const v8::Arguments& args ){
		int iLen = args.Length();
		if( CScript::CheckArgs( iLen == 0 )) return v8::Undefined();
		
		CVsdFile *thisObj = CScript::GetThis<CVsdFile>( args.This());
		if( !thisObj ) return v8::Undefined();
		int ret = thisObj->ReadUShortB();
		
		return v8::Integer::New( ret );
	}
	static v8::Handle<v8::Value> Func_ReadIntB( const v8::Arguments& args ){
		int iLen = args.Length();
		if( CScript::CheckArgs( iLen == 0 )) return v8::Undefined();
		
		CVsdFile *thisObj = CScript::GetThis<CVsdFile>( args.This());
		if( !thisObj ) return v8::Undefined();
		int ret = thisObj->ReadIntB();
		
		return v8::Integer::New( ret );
	}
	static v8::Handle<v8::Value> Func_ReadUIntB( const v8::Arguments& args ){
		int iLen = args.Length();
		if( CScript::CheckArgs( iLen == 0 )) return v8::Undefined();
		
		CVsdFile *thisObj = CScript::GetThis<CVsdFile>( args.This());
		if( !thisObj ) return v8::Undefined();
		double ret = thisObj->ReadUIntB();
		
		return v8::Number::New( ret );
	}
	static v8::Handle<v8::Value> Func_WriteChar( const v8::Arguments& args ){
		int iLen = args.Length();
		if( CScript::CheckArgs( iLen == 1 )) return v8::Undefined();
		
		CVsdFile *thisObj = CScript::GetThis<CVsdFile>( args.This());
		if( !thisObj ) return v8::Undefined();
		int ret = thisObj->WriteChar(
			args[ 0 ]->Int32Value()
		);
		
		return v8::Integer::New( ret );
	}
	static v8::Handle<v8::Value> Func_WriteShortL( const v8::Arguments& args ){
		int iLen = args.Length();
		if( CScript::CheckArgs( iLen == 1 )) return v8::Undefined();
		
		CVsdFile *thisObj = CScript::GetThis<CVsdFile>( args.This());
		if( !thisObj ) return v8::Undefined();
		int ret = thisObj->WriteShortL(
			args[ 0 ]->Int32Value()
		);
		
		return v8::Integer::New( ret );
	}
	static v8::Handle<v8::Value> Func_WriteIntL( const v8::Arguments& args ){
		int iLen = args.Length();
		if( CScript::CheckArgs( iLen == 1 )) return v8::Undefined();
		
		CVsdFile *thisObj = CScript::GetThis<CVsdFile>( args.This());
		if( !thisObj ) return v8::Undefined();
		int ret = thisObj->WriteIntL(
			args[ 0 ]->NumberValue()
		);
		
		return v8::Integer::New( ret );
	}
	static v8::Handle<v8::Value> Func_WriteFloat( const v8::Arguments& args ){
		int iLen = args.Length();
		if( CScript::CheckArgs( iLen == 1 )) return v8::Undefined();
		
		CVsdFile *thisObj = CScript::GetThis<CVsdFile>( args.This());
		if( !thisObj ) return v8::Undefined();
		int ret = thisObj->WriteFloat(
			args[ 0 ]->NumberValue()
		);
		
		return v8::Integer::New( ret );
	}
	static v8::Handle<v8::Value> Func_WriteDouble( const v8::Arguments& args ){
		int iLen = args.Length();
		if( CScript::CheckArgs( iLen == 1 )) return v8::Undefined();
		
		CVsdFile *thisObj = CScript::GetThis<CVsdFile>( args.This());
		if( !thisObj ) return v8::Undefined();
		int ret = thisObj->WriteDouble(
			args[ 0 ]->NumberValue()
		);
		
		return v8::Integer::New( ret );
	}
	static v8::Handle<v8::Value> Func_WriteShortB( const v8::Arguments& args ){
		int iLen = args.Length();
		if( CScript::CheckArgs( iLen == 1 )) return v8::Undefined();
		
		CVsdFile *thisObj = CScript::GetThis<CVsdFile>( args.This());
		if( !thisObj ) return v8::Undefined();
		int ret = thisObj->WriteShortB(
			args[ 0 ]->Int32Value()
		);
		
		return v8::Integer::New( ret );
	}
	static v8::Handle<v8::Value> Func_WriteIntB( const v8::Arguments& args ){
		int iLen = args.Length();
		if( CScript::CheckArgs( iLen == 1 )) return v8::Undefined();
		
		CVsdFile *thisObj = CScript::GetThis<CVsdFile>( args.This());
		if( !thisObj ) return v8::Undefined();
		int ret = thisObj->WriteIntB(
			args[ 0 ]->NumberValue()
		);
		
		return v8::Integer::New( ret );
	}

  public:
	// クラステンプレートの初期化
	static void InitializeClass( v8::Handle<v8::ObjectTemplate> global, void *pClass = NULL ){
		// コンストラクタを作成
		v8::Local<v8::FunctionTemplate> tmpl = v8::FunctionTemplate::New( New );
		tmpl->SetClassName( v8::String::New( "File" ));
		
		// フィールドなどはこちらに
		v8::Handle<v8::ObjectTemplate> inst = tmpl->InstanceTemplate();
		inst->SetInternalFieldCount( 1 );

		// メャbドはこちらに
		v8::Handle<v8::ObjectTemplate> proto = tmpl->PrototypeTemplate();
		proto->Set( v8::String::New( "Dispose" ), v8::FunctionTemplate::New( Func_Dispose ));
		proto->Set( v8::String::New( "Open" ), v8::FunctionTemplate::New( Func_Open ));
		proto->Set( v8::String::New( "Close" ), v8::FunctionTemplate::New( Func_Close ));
		proto->Set( v8::String::New( "ReadLine" ), v8::FunctionTemplate::New( Func_ReadLine ));
		proto->Set( v8::String::New( "WriteLine" ), v8::FunctionTemplate::New( Func_WriteLine ));
		proto->Set( v8::String::New( "Seek" ), v8::FunctionTemplate::New( Func_Seek ));
		proto->Set( v8::String::New( "IsEOF" ), v8::FunctionTemplate::New( Func_IsEOF ));
		proto->Set( v8::String::New( "ReadChar" ), v8::FunctionTemplate::New( Func_ReadChar ));
		proto->Set( v8::String::New( "ReadUChar" ), v8::FunctionTemplate::New( Func_ReadUChar ));
		proto->Set( v8::String::New( "ReadShortL" ), v8::FunctionTemplate::New( Func_ReadShortL ));
		proto->Set( v8::String::New( "ReadUShortL" ), v8::FunctionTemplate::New( Func_ReadUShortL ));
		proto->Set( v8::String::New( "ReadIntL" ), v8::FunctionTemplate::New( Func_ReadIntL ));
		proto->Set( v8::String::New( "ReadUIntL" ), v8::FunctionTemplate::New( Func_ReadUIntL ));
		proto->Set( v8::String::New( "ReadFloat" ), v8::FunctionTemplate::New( Func_ReadFloat ));
		proto->Set( v8::String::New( "ReadDouble" ), v8::FunctionTemplate::New( Func_ReadDouble ));
		proto->Set( v8::String::New( "ReadShortB" ), v8::FunctionTemplate::New( Func_ReadShortB ));
		proto->Set( v8::String::New( "ReadUShortB" ), v8::FunctionTemplate::New( Func_ReadUShortB ));
		proto->Set( v8::String::New( "ReadIntB" ), v8::FunctionTemplate::New( Func_ReadIntB ));
		proto->Set( v8::String::New( "ReadUIntB" ), v8::FunctionTemplate::New( Func_ReadUIntB ));
		proto->Set( v8::String::New( "WriteChar" ), v8::FunctionTemplate::New( Func_WriteChar ));
		proto->Set( v8::String::New( "WriteShortL" ), v8::FunctionTemplate::New( Func_WriteShortL ));
		proto->Set( v8::String::New( "WriteIntL" ), v8::FunctionTemplate::New( Func_WriteIntL ));
		proto->Set( v8::String::New( "WriteFloat" ), v8::FunctionTemplate::New( Func_WriteFloat ));
		proto->Set( v8::String::New( "WriteDouble" ), v8::FunctionTemplate::New( Func_WriteDouble ));
		proto->Set( v8::String::New( "WriteShortB" ), v8::FunctionTemplate::New( Func_WriteShortB ));
		proto->Set( v8::String::New( "WriteIntB" ), v8::FunctionTemplate::New( Func_WriteIntB ));



		// グローバルオブジェクトにクラスを定義
		global->Set( v8::String::New( "File" ), tmpl );
	}
};
/****************************************************************************/

class CScriptIF {
  public:
	///// プロパティアクセサ /////

	///// メャbドコールバック /////
	static v8::Handle<v8::Value> Func_DebugPrint( const v8::Arguments& args ){
		int iLen = args.Length();
		if( CScript::CheckArgs( iLen == 1 )) return v8::Undefined();
		v8::String::Value str0( args[ 0 ] );
		CScript::DebugPrint(
			( LPCWSTR )*str0
		);
		
		return v8::Undefined();
	}
	static v8::Handle<v8::Value> Func_MessageBox( const v8::Arguments& args ){
		int iLen = args.Length();
		if( CScript::CheckArgs( iLen == 1 )) return v8::Undefined();
		v8::String::Value str0( args[ 0 ] );
		CScript::MessageBox(
			( LPCWSTR )*str0
		);
		
		return v8::Undefined();
	}

  public:
	// クラステンプレートの初期化
	static void InitializeClass( v8::Handle<v8::ObjectTemplate> GlobalTmpl ){
		#define inst	GlobalTmpl
		#define proto	GlobalTmpl
		// フィールドなどはこちらに

		// メャbドはこちらに
		proto->Set( v8::String::New( "DebugPrint" ), v8::FunctionTemplate::New( Func_DebugPrint ));
		proto->Set( v8::String::New( "MessageBox" ), v8::FunctionTemplate::New( Func_MessageBox ));



		#undef inst
		#undef GlobalTmpl
	}
};
