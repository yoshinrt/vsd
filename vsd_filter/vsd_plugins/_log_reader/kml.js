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
	
	var Buf = '';
	var file = new File();
	var m;
	
	// 一旦全ファイルを Buf に溜める
	for( var i = 0; i < Files.length; ++i ){
		if( Files[ i ].match( /\.kmz$/ )){
			
			// kmz を unzip
			try{
				file.Open( Files[ i ], "Zrb" );
			}catch( e ){
				MessageBox( "ファイルが開けません: " + Files[ i ] );
				return 0;
			}
			while( !file.ZipNextFile().match( /\.kml$/i ));
		}else{
			// kml を普通に open
			try{
				file.Open( Files[ i ], "rb" );
			}catch( e ){
				MessageBox( "ファイルが開けません: " + Files[ i ] );
				return 0;
			}
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
	
	if( Points = Buf.match( /<when>[\S\s]*?<\/gx:coord>/g )){
		
		Log.Speed = Buf
			.match( /<gx:SimpleArrayData name="speed">[\S\s]*?<\/gx:SimpleArrayData>/g )
			.join( "\n" )
			.match( /<gx:value>[^<]*/g )
			.map( function( speed ){
				return speed.match( m = /([\d\.]+)/ ) ? m[1] * 3.6 : undefined;
			});
		
		Buf = undefined;
		
		// 時刻付きの KML
		Points.forEach( function( Point ){
			// 時間
			if( !( m = Point.match( /<when>(\d+)-(\d+)-(\d+)T(\d+):(\d+):([\d\.]+)(Z|([-+]\d+):(\d+))/ ))){
				return;
			}
			Log.Time[ Cnt ] = Date.UTC(
				m[1], m[2] - 1, m[3],
				m[4] - ( m[7] == 'Z' ? 0 : m[8] ),
				m[5] - ( m[7] == 'Z' ? 0 : m[8] >= 0 ? m[9] : -m[9] ),
				~~m[6], ~~( m[6] * 1000 ) % 1000
			);
			
			// long, lat, alt
			if( !(m = Point.match( /<gx:coord>(\S+)\s+(\S+)\s+([\d\.]+)/ ))){
				return;
			}
			Log.Longitude[ Cnt ] = +m[1];
			Log.Latitude [ Cnt ] = +m[2];
			Log.Altitude [ Cnt ] = +m[3];
			
			++Cnt;
		});
	}else if( Points = Buf.match( /<coordinates>[\S\s]*?<\/coordinates>/g )){
		Buf = undefined;
		var MaxStraightLen = 100;	// 直線を切る長さ [m]
		
		// 時刻がない KML，Maps Engine の KML を想定
		// <coordinates>135.67543,35.05375,0.0 135.6749,35.05453,0.0</coordinates>
		
		Points.forEach( function( Line ){
			var Coordinates = Line.match( /[+\-\d\.,]+/g );
			
			if( Coordinates && Coordinates.length > 1 ){
				Coordinates.forEach( function( Point ){
					
					m = Point.match( /^(.*),(.*),(.*)/ ) || Point.match( /^(.*),(.*)/ );
					Log.Longitude[ Cnt ] = +m[1];
					Log.Latitude [ Cnt ] = +m[2];
					if( m[3] !== undefined ) Log.Altitude [ Cnt ] = +m[3];
					
					if( Cnt == 0 ){
						var date = new Date;
						Log.Time[ 0 ] = Date.UTC( date.getFullYear(), date.getMonth(), date.getDate(), 0, 0, 0, 0 );
						++Cnt;
					}else if(
						Log.Longitude[ Cnt - 1 ] != Log.Longitude[ Cnt ] ||
						Log.Latitude [ Cnt - 1 ] != Log.Latitude [ Cnt ]
					){
						var Distance = GetDistanceByLngLat(
							Log.Longitude[ Cnt - 1 ],
							Log.Latitude [ Cnt - 1 ],
							Log.Longitude[ Cnt ],
							Log.Latitude [ Cnt ]
						);
						
						// 直線が長いと方位がおかしくなので一旦切る
						if( Distance > MaxStraightLen + 5 ){
							Log.Longitude[ Cnt + 1 ] = Log.Longitude[ Cnt ];
							Log.Latitude [ Cnt + 1 ] = Log.Latitude [ Cnt ];
							Log.Altitude [ Cnt + 1 ] = Log.Altitude [ Cnt ];
							
							Log.Longitude[ Cnt ] = Log.Longitude[ Cnt - 1 ] + ( Log.Longitude[ Cnt ] - Log.Longitude[ Cnt - 1 ]) / Distance * MaxStraightLen;
							Log.Latitude [ Cnt ] = Log.Latitude [ Cnt - 1 ] + ( Log.Latitude [ Cnt ] - Log.Latitude [ Cnt - 1 ]) / Distance * MaxStraightLen;
							Log.Altitude [ Cnt ] = Log.Altitude [ Cnt - 1 ] + ( Log.Altitude [ Cnt ] - Log.Altitude [ Cnt - 1 ]) / Distance * MaxStraightLen;
							
							Log.Time[ Cnt ] = Log.Time[ Cnt - 1 ] + MaxStraightLen / ( 60 / 3600 );
							++Cnt;
							Distance -= MaxStraightLen;
						}
						
						// 60km/h における所要時間を求める
						Log.Time[ Cnt ] = Log.Time[ Cnt - 1 ] + Distance / ( 60 / 3600 );
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
}
