GlobalInstance = this;

SEEK_SET	= 0;
SEEK_CUR	= 1;
SEEK_END	= 2;

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
	
	// ReaderFunc 自動判別
	ExitLoop: if( typeof( ReaderFunc ) == 'undefined' ){
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
					break ExitLoop;
				}
			}
		}
	}
	
	// ReaderFunc を呼ぶ
	if( typeof( ReaderFunc ) == 'undefined' ){
		MessageBox( Files[ 0 ] + " はリードできません．次のことを試してください\n" +
			"・拡張子を適切に変更する\n" +
			"・ファイルを開く ダイアログでファイル形式を「自動判別」以外に設定する\n"
		);
		delete( Log );
		return 0;
	}
	
	if( typeof( GlobalInstance[ ReaderFunc ] ) == 'undefined' ){
		MessageBox( ReaderFunc + "() は定義されていません" );
		delete( Log );
		return 0;
	}
	
	var Cnt = GlobalInstance[ ReaderFunc ]( Files );
	
	if( Cnt == 0 ){
		MessageBox( Files.join( "\n" ) + "\n有効なログが見つかりませんでした" );
		delete( Log );
		return 0;
	}
	
	if( typeof( Log ) != 'object' ){
		MessageBox( Files.join( "\n" ) + "\nArray 型の変数 'Log' を定義してください" );
		delete( Log );
		return 0;
	}
	
	if( typeof( Log.Time ) != 'object' ){
		MessageBox( Files.join( "\n" ) + "\nArray 型の変数 'Log.Time' を定義してください" );
		delete( Log );
		return 0;
	}
	
	//DumpLog( "dump.csv" );
	
	return Cnt;
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
