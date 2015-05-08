// LAP+ ログリーダ

// dp3 フォーマット
// ビッグエンディアン，0x100 以降が 16バイト/1レコード のログで，
// +0x00-0x03: 日本時間 0:00 からの時間，日付は失われている  1秒 = 10
// +0x04-0x07: 経度 1度 = 128 * 360 * 360
// +0x08-0x0B: 緯度
// +0x0C-0x0D: 時速 1km/h = 10
// +0x0E-0x0F: 方位，このリーダでは不使用

LogReaderInfo.push({
	Caption:	"LAP+ (*.dp3)",
	Filter:		"*.dp3",
	ReaderFunc:	"Read_dp3"
});

function Read_dp3( Files ){
	
	Log.Time		= [];
	Log.Speed		= [];
	Log.Longitude	= [];
	Log.Latitude	= [];
	
	var	Cnt = 0;
	var Line;
	
	// 日付情報が無いので暫定的に 2012/1/1
	// 時刻は JST らしいので -9:00 する
	var Time0 = Date.UTC( 2012, 0, 1, 0, 0, 0 ) + ( new Date ).getTimezoneOffset() * 60000;
	
	for( var i = 0; i < Files.length; ++i ){
		var file = new File();
		try{
			file.Open( Files[ i ], "rb" );
		}catch( e ){
			MessageBox( "ファイルが開けません: " + Files[ i ] );
			return 0;
		}
		
		// ヘッダスキップ
		file.Seek( 0x100, SEEK_SET );
		
		while( 1 ){
			var Time	= file.ReadIntB() * 100; // 1/10s → ms
			Log.Longitude[ Cnt ] = file.ReadIntB() / 460800;
			Log.Latitude [ Cnt ] = file.ReadIntB() / 460800;
			Log.Speed	 [ Cnt ] = file.ReadShortB() / 10;
			
			file.ReadShortL();	// ダミー
			
			if( file.IsEOF()) break;
			
			Log.Time[ Cnt ]	= Time0 +
				( ~~( Time / 10000000 )       ) * 3600000 +
				( ~~( Time /   100000 ) % 100 ) *   60000 +
				Time % 100000;
			
			++Cnt;
		}
		file.Close();
	}
	
	return Cnt;
}
