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
	var m;
	
	if( Points = Buf.match( /<trkpt[\S\s]*?<\/trkpt>/g )){
		Points.forEach( function( Point ){
			// <trkpt lat="35.12345" lon="135.12345">
			// <speed>0.0</speed>
			// <time>2012-04-28T21:27:50.328Z</time>
			// <ele>128.1</ele>
			// </trkpt>
			
			// 時間
			if( !(m = Point.match( /<time>(\d+)-(\d+)-(\d+)T(\d+):(\d+):([\d\.]+)(Z|([-+]\d+):(\d+))/ ))){
				return;
			}
			Log.Time[ Cnt ] = Date.UTC(
				m[1], m[2] - 1, m[3],
				m[4] - ( m[7] == 'Z' ? 0 : m[8] ),
				m[5] - ( m[7] == 'Z' ? 0 : m[8] >= 0 ? m[9] : -m[9] ),
				~~m[6], ~~( m[6] * 1000 ) % 1000
			);
			
			// long, lat
			if( !Point.match( /lat="(.*?)"/ )) return;
			Log.Latitude [ Cnt ] = +m[1];
			if( !Point.match( /lon="(.*?)"/ )) return;
			Log.Longitude[ Cnt ] = +m[1];
			
			// Speed がある場合は Array 作成
			if( Point.match( /<speed>([\d\.]+)/ )){
				Log.Speed[ Cnt ] = 3.6 * m[1];
			}
			
			// ele がある場合は Array 作成
			if( Point.match( /<ele>([\d\.]+)/ )){
				Log.Altitude[ Cnt ] = +m[1];
			}
			++Cnt;
		});
	}else{
		return INVALID_FORMAT;
	}
	
	if( bMyTracks ) SmoothLowFreqLog();
	return Cnt;
}
