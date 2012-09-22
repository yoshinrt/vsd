function ReadLog( FileName ){
	
	// '/' で連結された複数ファイルを配列化
	var Files = FileName.split( "/" );
	
	if( Files.length >= 2 ){
		var Dir = Files.shift();
		Files.sort();
		
		for( var i = 0; i < Files.length; ++i ){
			Files[ i ] = Dir + "\\" + Files[ i ];
		}
	}
	
	// Read_拡張子 を呼ぶ
	var Ext = Files[ 0 ].toLowerCase().replace( /\.gz$/, '' );
	Ext.match( /([^\.]+)$/ );
	
	var Cnt = eval( "Read_" + RegExp.$1 + "( Files )" );
	
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
	
	if( Cnt == 0 ){
		MessageBox( Files.join( "\n" ) + "\n有効なログが見つかりませんでした" );
		delete( Log );
		return 0;
	}
	
	return Cnt;
}
