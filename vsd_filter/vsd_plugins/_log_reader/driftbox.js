// Driftbox ログリーダ

LogReaderInfo.push({
	Caption:	"Driftbox (*.dbn)",
	Filter:		"*.dbn",
	ReaderFunc:	"Read_dbn"
});

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
		try{
			file.Open( Files[ i ], "rb" );
		}catch( e ){
			MessageBox( "ファイルが開けません: " + Files[ i ] );
			return 0;
		}
		
		// [HEADER] までスキップ
		while( !file.IsEOF()){
			if( file.ReadLine().match( /^\[HEADER\]/ )) break;
		}
		
		var ParamName = [];
		var ParamSize = [];
		
		// ヘッダ解析
		while( !file.IsEOF()){
			Line = file.ReadLine();
			if( Line.match( /^(.+)\((\d+)\)/ )){
				ParamName.push( RegExp.$1 );
				ParamSize.push( ~~RegExp.$2 );
			}else{
				break;
			}
		}
		if( file.IsEOF()) return INVALID_FORMAT;
		
		// [DATA] までスキップ
		if( !Line.match( /^\[DATA\]/ )){
			while( !file.IsEOF()){
				if( file.ReadLine().match( /^\[DATA\]/ )) break;
			}
		}
		if( file.IsEOF()) return INVALID_FORMAT;
		
		var PrevTime = 0;
		
		// 日付情報が無いので暫定的に 2012/1/1
		var Time0 = Date.UTC( 2012, 0, 1, 0, 0, 0 );
		
		while( 1 ){
			file.Seek( 1, SEEK_CUR );	// '$' のスキップ
			
			for( var j = 0; j < ParamName.length; ++j ){
				var Val;
				
				switch( ParamSize[ j ] ){
					case 1: Val = file.ReadUChar(); break;
					case 2: Val = file.ReadUShortB(); break;
					case 3: Val = file.ReadUShortB() * 256 + file.ReadUChar(); break;
					default:Val = file.ReadIntB();
				}
				
				switch( ParamName[ j ] ){
					case "TIME":		Time = Val * 10; break;
					case "LATITUDE":	Log.Latitude [ Cnt ] = Val / 6000000; break;
					case "LONGITUDE":	Log.Longitude[ Cnt ] = Val / 6000000; break;
					case "VELOCITY":	Log.Speed	 [ Cnt ] = Val / 100;
				}
			}
			
			if( file.IsEOF()) break;
			
			if( Time < PrevTime ){
				Time0 += 24 * 3600 * 1000;
			}
			PrevTime = Time;
			Log.Time[ Cnt ]	= Time + Time0;
			
			++Cnt;
			
			// 0D 0A をスキップ
			file.Seek( 2, SEEK_CUR );
		}
		file.Close();
	}
	
	return Cnt;
}
