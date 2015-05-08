// GPX リーダ

LogReaderInfo.push({
	Caption:	"GPX (*.gpx)",
	Filter:		"*.gpx;*.gpx.gz",
	ReaderFunc:	"Read_gpx"
});

function Read_gpx( Files ){
	
	Log.Time		= [];
	Log.Speed		= [];
	Log.Altitude	= [];
	Log.Longitude	= [];
	Log.Latitude	= [];
	
	var	Cnt			= 0;
	var Buf			= '';
	
	// 一旦全ファイルを Buf に溜める
	for( var i = 0; i < Files.length; ++i ){
		// kml を普通に open
		var file = new File();
		try{
			file.Open( Files[ i ], "zrb" );
		}catch( e ){
			MessageBox( "ファイルが開けません: " + Files[ i ] );
			return 0;
		}
		
		do{
			Buf += file.ReadLine();
		}while( !file.IsEOF())
		
		file.Close();
	}
	
	// Google My Tracks のバグ? 回避
	var bMyTracks = Buf.match( /Google My Tracks/ );
	var Points;
	
	if( Points = Buf.match( /<trkpt[\S\s]*?<\/trkpt>/g )){
		Points.forEach( function( Point ){
			// <trkpt lat="35.12345" lon="135.12345">
			// <speed>0.0</speed>
			// <time>2012-04-28T21:27:50.328Z</time>
			// <ele>128.1</ele>
			// </trkpt>
			
			// 時間
			if( !Point.match( /<time>(\d+)-(\d+)-(\d+)T(\d+):(\d+):([\d\.]+)(Z|([-+]\d+):(\d+))/ )){
				return;
			}
			Log.Time[ Cnt ] = Date.UTC(
				RegExp.$1, RegExp.$2 - 1, RegExp.$3,
				RegExp.$4 - ( RegExp.$7 == 'Z' ? 0 : RegExp.$8 ),
				RegExp.$5 - ( RegExp.$7 == 'Z' ? 0 : RegExp.$8 >= 0 ? RegExp.$9 : -RegExp.$9 ),
				~~RegExp.$6, ~~( RegExp.$6 * 1000 ) % 1000
			);
			
			// long, lat
			if( !Point.match( /lat="(.*?)"/ )) return;
			Log.Latitude [ Cnt ] = +RegExp.$1;
			if( !Point.match( /lon="(.*?)"/ )) return;
			Log.Longitude[ Cnt ] = +RegExp.$1;
			
			// Speed がある場合は Array 作成
			if( Point.match( /<speed>([\d\.]+)/ )){
				Log.Speed[ Cnt ] = 3.6 * RegExp.$1;
			}
			
			// ele がある場合は Array 作成
			if( Point.match( /<ele>([\d\.]+)/ )){
				Log.Altitude[ Cnt ] = +RegExp.$1;
			}
			++Cnt;
		});
	}else{
		return INVALID_FORMAT;
	}
	
	if( bMyTracks ) SmoothLowFreqLog();
	return Cnt;
}
