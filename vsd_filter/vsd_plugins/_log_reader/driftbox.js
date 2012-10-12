// Driftbox ログリーダ

LogReaderInfo.push( "Driftbox (*.dbn)", "*.dbn", "Read_dbn" );

// dbn フォーマット (ほとんど [HEADER] に書いてある)
// 1レコード: 33バイト  big endian
// '$'
// SATS(1)
// TIME(3)		GMT 0:00 からの経過時間 1秒=100 日付は失われている(?)
// LATITUDE(4)	1度 = 6000000
// LONGITUDE(4)
// VELOCITY(2)	1km/h = 100
// HEADING(2)	1度 = 100
// HEIGHT(4)
// YAW__(2)		以下なんの値か不明
// YAW_(2)
// YAW(2)
// SLIP(2)
// CHKSUM(2)
// 0x0D 0x0A

function Read_dbn( Files ){
	
	Log.Time		= [];
	Log.Speed		= [];
	Log.Longitude	= [];
	Log.Latitude	= [];
	
	var	Cnt = 0;
	var Line;
	
	for( var i = 0; i < Files.length; ++i ){
		var file = new File();
		if( file.Open( Files[ i ], "rb" )){
			MessageBox( "ファイルが開けません: " + Files[ i ] );
			return 0;
		}
		
		// ヘッダスキップ
		while( !file.IsEOF()){
			if( file.ReadLine().match( /^\[DATA\]/ )) break;
		}
		
		var PrevTime = 0;
		
		// 日付情報が無いので暫定的に 2012/1/1
		var Time0 = Date.UTC( 2012, 0, 1, 0, 0, 0 );
		
		while( 1 ){
			file.Seek( 2, SEEK_CUR );	// '$', SAT のスキップ
			var Time	= file.ReadUShortB() * 256 + file.ReadUChar();
			
			Log.Latitude [ Cnt ] = file.ReadIntB() / 6000000;
			Log.Longitude[ Cnt ] = file.ReadIntB() / 6000000;
			Log.Speed	 [ Cnt ] = file.ReadUShortB() / 100;
			
			if( file.IsEOF()) break;
			
			if( Time < PrevTime ){
				Time0 += 24 * 3600 * 1000;
			}
			PrevTime = Time;
			Log.Time[ Cnt ]	= Time * 10 + Time0;
			
			++Cnt;
			
			// 残りのデータはスキップ
			file.Seek( 33 - 15, SEEK_CUR );
		}
		file.Close();
	}
	
	return Cnt;
}
