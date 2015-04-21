// kml / kmz リーダ

LogReaderInfo.push({
	Caption:	"Google Keyhole ML (*.kml,*.kmz)",
	Filter:		"*.kml;*.kmz",
	ReaderFunc:	"Read_kml"
});

/****************************************************************************/

function Read_kml( Files ){
	
	Log.Time		= [];
	Log.Longitude	= [];
	Log.Latitude	= [];
	Log.Altitude	= [];
	
	var	Cnt = 0;
	var bMyTracks; // Google My Tracks のバグ? 回避
	
	var Buf = '';
	var file = new File();
	
	// 一旦全ファイルを Buf に溜める
	for( var i = 0; i < Files.length; ++i ){
		if( Files[ i ].match( /\.kmz$/ )){
			
			// kmz を unzip
			file.Open( Files[ i ], "Zrb" );
			
			do{
				var file_name = file.ZipNextFile();
Print( file_name + "---------\n" );
			}while( !file_name.match( /\.kml$/i ));
		}else{
			// kml を普通に open
			file.Open( Files[ i ], "rb" );
		}
		
		do{
			Buf += file.ReadLine();
		}while( !file.IsEOF())
		
		file.Close();
	}
	
	var bMyTracks = Buf.match( /Google My Tracks/ );
	
	// <when>2014-09-05T23:45:56.415Z</when>
	// <gx:coord>135.12345 35.12345 160.10000610351563</gx:coord>
	
	var Points;
	
	if( Points = Buf.match( /<when>.*?<\/gx:coord>/gm )){
		Buf = undefined;
		
		// 時刻付きの KML
		Points.forEach( function( Point ){
			// 時間
			if( !Point.match( /<when>(\d+)-(\d+)-(\d+)T(\d+):(\d+):([\d\.]+)(Z|([-+]\d+):(\d+))/ )){
				return;
			}
			Log.Time[ Cnt ] = Date.UTC(
				RegExp.$1, RegExp.$2 - 1, RegExp.$3,
				RegExp.$4 - ( RegExp.$7 == 'Z' ? 0 : RegExp.$8 ),
				RegExp.$5 - ( RegExp.$7 == 'Z' ? 0 : RegExp.$8 >= 0 ? RegExp.$9 : -RegExp.$9 ),
				~~RegExp.$6, ~~( RegExp.$6 * 1000 ) % 1000
			);
			
			// long, lat, alt
			if( !Point.match( /<gx:coord>(\S+)\s+(\S+)\s+([\d\.]+)/ )){
				return;
			}
			Log.Longitude[ Cnt ] = +RegExp.$1;
			Log.Latitude [ Cnt ] = +RegExp.$2;
			Log.Altitude [ Cnt ] = +RegExp.$3;
			
			++Cnt;
		});
	}else if( Points = Buf.match( /<coordinates>.*?<\/coordinates>/gm )){
		Buf = undefined;
		
		// 時刻がない KML，Maps Engine の KML を想定
		// <coordinates>135.67543,35.05375,0.0 135.6749,35.05453,0.0</coordinates>
		
		Points.forEach( function( Line ){
			var Coordinates = Line.match( /[+\-\d\.,]+/g );
			
			if( Coordinates && Coordinates.length > 1 ){
				Coordinates.forEach( function( Point ){
					
					Point.match( /^(.*),(.*),(.*)/ );
					Log.Longitude[ Cnt ] = +RegExp.$1;
					Log.Latitude [ Cnt ] = +RegExp.$2;
					Log.Altitude [ Cnt ] = +RegExp.$3;
					
					if( Cnt == 0 ){
						var date = new Date;
						Log.Time[ 0 ] = Date.UTC( date.getFullYear(), date.getMonth(), date.getDate(), 0, 0, 0, 0 );
						++Cnt;
					}else if(
						Log.Longitude[ Cnt - 1 ] != Log.Longitude[ Cnt ] &&
						Log.Latitude [ Cnt - 1 ] != Log.Latitude [ Cnt ]
					){
						// 60km/h における所要時間を求める
						Log.Time[ Cnt ] = Log.Time[ Cnt - 1 ] + GetDistance(
							Log.Longitude[ Cnt - 1 ],
							Log.Latitude [ Cnt - 1 ],
							Log.Longitude[ Cnt ],
							Log.Latitude [ Cnt ]
						) / ( 60 / 3600 );
						++Cnt;
					}
				});
			}
		});
	}else{
		return INVALID_FORMAT;
	}
	if( bMyTracks ) SmoothLowFreqLog();
	return Cnt;
	
	// 緯度・経度から距離算出
	function GetDistance( dLong0, dLati0, dLong1, dLati1 ){
		var a	= 6378137.000;
		var b	= 6356752.314245;
		var e2	= ( a * a - b * b ) / ( a * a );
		var ToRAD = Math.PI / 180;
		
		var dx	= ( dLong1 - dLong0 ) * ToRAD;
		var dy	= ( dLati1 - dLati0 ) * ToRAD;
		var uy	= ( dLati0 + dLati1 ) / 2 * ToRAD;
		var W	= Math.sqrt( 1 - e2 * Math.sin( uy ) * Math.sin( uy ));
		var M	= a * ( 1 - e2 ) / Math.pow( W, 3 );
		var N	= a / W;
		
		return	Math.sqrt( dy * dy * M * M + Math.pow( dx * N * Math.cos( uy ), 2 ));
	}
}
