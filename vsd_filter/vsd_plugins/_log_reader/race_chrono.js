// Race Chrono (*.rcz) ログリーダ

// 1: ULONG epoch 時刻 [ms]
// 2: ULONG 走行距離 [1/1000m]
// 3: int latitude, int longitude [1/6000000度]
// 4: UINT 速度 [1/277.7792km/h, キリがいいのに近いのは 1/512knot?]
// 5: 高度 [1/1000m]
// 6: UINT bearing [1/1000度]
// 30002: 捕捉衛生数
// 30003: 位置特定品質 ($GPGGA)
// 30004: DOP 座標精度 [*1/1000]
// 30005: DOP 座標精度 [*1/1000], -128:データなし
// すべてリトルエンディアン

LogReaderInfo.push({
	Caption:	"Race Chrono (*.rcz)",
	Filter:		"*.rcz",
	ReaderFunc:	"Read_rcz"
});

function Read_rcz( Files ){
	
	Log.Time		= [];
	Log.Speed		= [];
	Log.Longitude	= [];
	Log.Latitude	= [];
	
	var	Cnt = 0;
	var Line;
	
	for( var i = 0; i < Files.length; ++i ){
		var file = new File();
		try{
			file.Open( Files[ i ], "Zrb" );
		}catch( e ){
			MessageBox( "ファイルが開けません: " + Files[ i ] );
			return 0;
		}
		
		while( $InnerFile = file.ZipNextFile()){
			// 時刻ログ
			if( $InnerFile.match( /^channel_1_100_0_1_/ )){
				Cnt = Log.Time.length;
				while( 1 ){
					Log.Time[ Cnt++ ]	= file.ReadUIntL() + file.ReadUIntL() * 0x100000000;
					if( file.IsEOF()) break;
				}
			}
			
			// 緯度経度ログ
			else if( $InnerFile.match( /^channel_1_100_0_3_/ )){
				Cnt = Log.Longitude.length;
				while( 1 ){
					Log.Latitude [ Cnt ] = file.ReadIntL() / 6000000;
					Log.Longitude[ Cnt ] = file.ReadIntL() / 6000000;
					++Cnt;
					if( file.IsEOF()) break;
				}
			}
			
			// 速度ログ
			else if( $InnerFile.match( /^channel_1_100_0_4_/ )){
				Cnt = Log.Speed.length;
				while( 1 ){
					Log.Speed[ Cnt++ ] = file.ReadUIntL() / 277.7792;
					if( file.IsEOF()) break;
				}
			}
		}
		
		file.Close();
	}
	
	Printf( "%d\n", Log.Time.length );
	return Log.Time.length;
}
