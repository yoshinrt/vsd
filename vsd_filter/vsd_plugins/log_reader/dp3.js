// LAP+ ログリーダ

LogReaderInfo.push( "LAP+ (*.dp3)", "*.dp3", "Read_dp3" );

function Read_dp3( Files ){
	
	Log.Time		= new Array();
	Log.Speed		= new Array();
	Log.Longitude	= new Array();
	Log.Latitude	= new Array();
	
	var	Cnt = 0;
	var Line;
	
	for( var i = 0; i < Files.length; ++i ){
		var file = new File();
		if( file.Open( Files[ i ], "rb" )){
			MessageBox( "ファイルが開けません: " + Files[ i ] );
			return 0;
		}
		
		// ヘッダスキップ
		file.Seek( 0x100, SEEK_SET );
		
		while( 1 ){
			var Time	= file.ReadIntB();
			Log.Longitude[ Cnt ] = file.ReadIntB() / 460800;
			Log.Latitude [ Cnt ] = file.ReadIntB() / 460800;
			Log.Speed	 [ Cnt ] = file.ReadShortB() / 10;
			
			file.ReadShortL();	// ダミー
			
			if( file.IsEOF()) break;
			
			// 日付情報が無いので暫定的に 2012/1/1
			// 時刻は JST らしいので -9:00 する
			Log.Time[ Cnt ]	= Date.UTC( 2012, 0, 1,
				~~( Time / 100000 ),
				~~( Time /   1000 ) % 100,
				~~( Time /     10 ) % 100,
				Time % 10 * 100
			) - ( 9 * 3600 * 1000 );
			
			++Cnt;
		}
		file.Close();
	}
	
	return Cnt;
}
