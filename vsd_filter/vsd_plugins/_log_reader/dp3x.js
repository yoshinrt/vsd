// LAP+Android ログリーダ

LogReaderInfo.push( "LAP+Android (*.dp3x)", "*.dp3x", "Read_dp3x_5Hz" );
//LogReaderInfo.push( "LAP+Android 10Hz (*.dp3x)", "*.dp3x", "Read_dp3x_10Hz" );

// dp3x フォーマット
// 0x48-0x4F: ログ開始時刻,ms (GMT-9h, 日本時間に直すためには +18h)
// 0x50-0x53: 原点の経度 1度 = 128 * 360 * 360
// 0x54-0x57: 原点の緯度
// 
// 0x78 以降が 18バイト/1レコード のログで，
// +0x00-0x01: 原点からの経度
// +0x02-0x03: 原点からの緯度
// +0x04-0x05: 速度			1km/h = 10
// +0x06-0x08: G センサー	1G = 40(?)
// +0x09-0x0B: 磁気センサー?
// +0x0C～   : 不明 (all 0)

function Read_dp3x_5Hz( Files ){
	return Read_dp3x( Files, 5 );
}
function Read_dp3x_10Hz( Files ){
	return Read_dp3x( Files, 10 );
}
function Read_dp3x( Files, Hz ){
	
	Log.Time		= [];
	Log.Speed		= [];
	Log.Longitude	= [];
	Log.Latitude	= [];
	Log.Gx			= [];
	Log.Gy			= [];
	
	var	Cnt = 0;
	var Line;
	
	var Long0;
	var Lati0;
	var Time0;
	
	for( var i = 0; i < Files.length; ++i ){
		var file = new File();
		if( file.Open( Files[ i ], "rb" )){
			MessageBox( "ファイルが開けません: " + Files[ i ] );
			return 0;
		}
		
		// ヘッダ情報リード
		file.Seek( 0x48, SEEK_SET );
		Time0 = file.ReadUIntL() + file.ReadUIntL() * ( 0x10000 * 0x10000 ) +
			9 * 3600 * 1000;	// GMT - 9h っぽい
		Long0 = file.ReadIntL() / 460800;
		Lati0 = file.ReadIntL() / 460800;
		
		file.Seek( 0x78, SEEK_SET );
		
		while( 1 ){
			Log.Longitude[ Cnt ] = Long0 + file.ReadShortL() / 460800;
			Log.Latitude [ Cnt ] = Lati0 + file.ReadShortL() / 460800;
			Log.Speed	 [ Cnt ] = file.ReadShortL() / 10;
			
			file.ReadChar(); // dummy
			Log.Gx[ Cnt ] = -file.ReadChar() / 40;
			Log.Gy[ Cnt ] = -file.ReadChar() / 40;
			
			if( file.IsEOF()) break;
			
			Log.Time[ Cnt ] = Time0 + 1000 * Cnt / Hz;
			
			file.Seek( 18 - 0x9, SEEK_CUR );
			
			++Cnt;
		}
		file.Close();
	}
	
	return Cnt;
}
