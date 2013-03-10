GlobalInstance = this;

SEEK_SET	= 0;
SEEK_CUR	= 1;
SEEK_END	= 2;

INVALID_FORMAT	= -1;

//*** ログファイルリーダ エントリ関数 ****************************************

function ReadLog( FileName, ReaderFunc ){
	
	// '/' で連結された複数ファイルを配列化
	var Files = FileName.split( "/" );
	
	if( Files.length >= 2 ){
		var Dir = Files.shift();
		Files.sort();
		
		for( var i = 0; i < Files.length; ++i ){
			Files[ i ] = Dir + "\\" + Files[ i ];
		}
	}
	
	if( typeof( ReaderFunc ) == 'undefined' ){
		// ReaderFunc 自動判別
		for( var i = 0; i < LogReaderInfo.length; ++i ){
			// *.hoge;*.fuga のようなリストを ; で split
			var Filters = LogReaderInfo[ i ].Filter.split( ";" );
			for( var j = 0; j < Filters.length; ++j ){
				
				// ワイルドカード形式から RegExp オブジェクト作成
				var re = new RegExp( "^" + Filters[ j ]
					.replace( /\./g, "\\." )	// . -> \.
					.replace( /\?/g, "." )		// ? -> .
					.replace( /\*/g, ".*" )		// * -> .*
					+ "$", "i"
				);
				
				// RE にマッチしたら，リーダ関数名を取得
				if( Files[ 0 ].match( re )){
					ReaderFunc = LogReaderInfo[ i ].ReaderFunc;
					var Cnt = ReadLog0( 1 );
					if( Cnt >= 0 ) return Cnt;
				}
			}
		}
		
		MessageBox( Files[ 0 ] + "\nはリードできません．次のことを試してください\n" +
			"・拡張子を適切に変更する\n" +
			"・ファイルを開く ダイアログでファイル形式を「自動判別」以外に設定する"
		);
		delete( Log );
		return 0;
	}else{
		// フォーマット指定オープン
		return ReadLog0( 0 );
	}
	
	function ReadLog0( bAuto ){
		// ReaderFunc を呼ぶ
		if( typeof( ReaderFunc ) == 'undefined' ){
			MessageBox( Files[ 0 ] + "\nはリードできません．次のことを試してください\n" +
				"・拡張子を適切に変更する\n" +
				"・ファイルを開く ダイアログでファイル形式を「自動判別」以外に設定する"
			);
			delete( Log );
			return 0;
		}
		
		if( typeof( GlobalInstance[ ReaderFunc ] ) == 'undefined' ){
			MessageBox( ReaderFunc + "() は定義されていません" );
			delete( Log );
			return 0;
		}
		
		Log = [];
		var Cnt = GlobalInstance[ ReaderFunc ]( Files );
		
		if( Cnt == INVALID_FORMAT && bAuto ){
			// 自動モードで，フォーマットが違うなら次のフォーマット
			return Cnt;
		}
		
		if( Cnt <= 0 ){
			MessageBox( Files[ 0 ] + "\n有効なログが見つかりませんでした" );
			delete( Log );
			return 0;
		}
		
		if( typeof( Log ) != 'object' ){
			MessageBox( Files[ 0 ] + "\nArray 型の変数 'Log' を定義してください" );
			delete( Log );
			return 0;
		}
		
		if( typeof( Log.Time ) != 'object' ){
			MessageBox( Files[ 0 ] + "\nArray 型の変数 'Log.Time' を定義してください" );
			delete( Log );
			return 0;
		}
		
		//DumpLog( "dump.csv" );
		
		return Cnt;
	}
}

//*** LogReaderInfo ソート ***************************************************

function SortLogReaderInfo(){
	LogReaderInfo.sort(
		function( a, b ){
			var aa = isNaN( a.Priority ) ? 0x10000000 : a.Priority;
			var bb = isNaN( b.Priority ) ? 0x10000000 : b.Priority;
			return aa - bb;
		}
	);
}

//*** デバッグ用ログデータダンプ *********************************************

function DumpLog( FileName ){
	var file = new File();
	if( file.Open( FileName, "w" )){
		MessageBox( "ファイルが開けません: " + FileName );
		return 0;
	}
	
	// ヘッダ
	for( var obj in Log ){
		file.WriteLine( obj + "," );
	}
	file.WriteLine( "\n" );
	
	// 要素出力
	for( var i = 0; i < Log.Time.length; ++i ){
		for( var obj in Log ){
			file.WriteLine( Log[ obj ][ i ] + "," );
		}
		file.WriteLine( "\n" );
	}
	
	file.Close();
}

//*** ログスムージング *******************************************************
// ログ記録 Hz よりログ実効 Hz が低い場合，同じデータが連続で記録され
// 表示がカクカクになってしまうので，そのようなデータは捨てて
// ログを線形補間する

function SmoothLowFreqLog( Cnt ){
	
	var PrevVal;
	
	for( var v in Log ) if( v != 'Time' ){
		for( var i = 0; i < Cnt; ++i ){
			if( i == 0 ){
				PrevIdx = 0;
				PrevVal = Log[ v ][ 0 ];
			}else{
				if( PrevVal != Log[ v ][ i ] ){
					if(
						// 2個以上同じデータが連続する
						i - PrevIdx >= 2 &&
						// データが連続時間が 1秒以内
						Log.Time[ i ] - Log.Time[ PrevIdx ] <= 1000
					){
						for( var j = PrevIdx + 1; j < i; ++j ){
							Log[ v ][ j ] = Log[ v ][ PrevIdx ] +
								( Log[ v ][ i ] - Log[ v ][ PrevIdx ] ) /
								( Log.Time[ i ] - Log.Time[ PrevIdx ] ) *
								( Log.Time[ j ] - Log.Time[ PrevIdx ] );
						}
					}
					PrevVal = Log[ v ][ i ];
					PrevIdx = i;
				}
			}
		}
	}
}
