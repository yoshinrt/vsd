// GPX リーダ

LogReaderInfo.push({
	Caption:	"GPX (*.gpx)",
	Filter:		"*.gpx",
	ReaderFunc:	"Read_gpx"
});

function Read_gpx( Files ){
	
	Log.Time		= [];
	Log.Longitude	= [];
	Log.Latitude	= [];
	
	var	Cnt = 0;
	var Line = '';
	var bSpeed = false;
	
	for( var i = 0; i < Files.length; ++i ){
		var file = new File();
		if( file.Open( Files[ i ], "zr" )){
			MessageBox( "ファイルが開けません: " + Files[ i ] );
			return 0;
		}
		
	  NextFile:
		while( 1 ){
			
			// <trkpt lat="35.12345" lon="135.12345">
			// <speed>0.0</speed>
			// <time>2012-04-28T21:27:50Z</time>
			// </trkpt>
			
			// trkpt 先頭サーチ
			while( !Line.match( /<trkpt/ )){
				Line = file.ReadLine();
				if( file.IsEOF()) break NextFile;
			}
			
			// trkpt 先頭が見つかったので，それ以前を破棄
			Line = RegExp.lastMatch ; + RegExp.rightContext;
			
			// trkpt 終了サーチ
			while( !Line.match( /<\/trkpt>/ )){
				Line = Line + file.ReadLine();
				if( file.IsEOF()) break NextFile;
			}
			
			// 1つの point に分解
			var Point = RegExp.leftContext + RegExp.lastMatch;
			Line = RegExp.rightContext;
			Point.replace( /[\x0D\x0A]/g, "" );
			
			// 時間
			if( !Point.match( /<time>(\d+)-(\d+)-(\d+)T(\d+):(\d+):(\d+)/ )){
				continue;
			}
			Log.Time[ Cnt ] = Date.UTC(
				RegExp.$1, RegExp.$2 - 1, RegExp.$3,
				RegExp.$4, RegExp.$5, RegExp.$6, RegExp.$7 * 1000
			);
			
			// long, lat
			if( !Point.match( /lat="([\d\.]+)"/ )) continue;
			Log.Latitude [ Cnt ] = +RegExp.$1;
			if( !Point.match( /lon="([\d\.]+)"/ )) continue;
			Log.Longitude[ Cnt ] = +RegExp.$1;
			
			
			// Speed がある場合は Array 作成
			if( Point.match( /<speed>([\d\.]+)/ )){
				if( !bSpeed ){
					Log.Speed = [];
					bSpeed = true;
				}
				Log.Speed[ Cnt ] = 3.6 * RegExp.$1;
			}
			
			++Cnt;
		}
		file.Close();
	}
	
	return Cnt;
}
