function ReadLog( FileName ){
	
	// '/' で連結された複数ファイルを配列化
	var Files = FileName.split( "/" );
	
	if( Files.length >= 2 ){
		var Dir = Files.shift();
		Files.sort();
		
		for( var i = 0; i < Files.length; ++i ){
			Files[ i ] = Dir + "\\" + Files[ i ];
		}
	}
	
	// Read_拡張子 を呼ぶ
	var Ext = Files[ 0 ].toLowerCase().replace( /\.gz$/, '' );
	Ext.match( /([^\.]+)$/ );
	
	return eval( "Read_" + RegExp.$1 + "( Files )" );
}

function Read_nmea( Files ){
	
	Log				= new Array();
	Log.Time		= new Array();
	Log.Longitude	= new Array();
	Log.Latitude	= new Array();
	
	for( var FileName in Files ){
		var file = new File();
		if( file.Open( FileName, "zr" )) return 0;
		
		var	Cnt = 0;
		var Line;
		var bSpeed = false;
		
		while( !file.IsEOF()){
			Line = file.ReadLine();
			
			if( Line.substr( 0, 6 ) == "$GPRMC" ){
				
				//        時間         lat           long           knot  方位   日付
				// 0      1          2 3           4 5            6 7     8      9
				// $GPRMC,043431.200,A,3439.997825,N,13523.377978,E,0.602,178.29,240612,,,A*59
				
				var Param = Line.split( "," );
				
				// Speed がある場合は Array 作成
				if( Param[ 7 ] != '' ){
					if( !bSpeed ){
						Log.Speed = new Array();
						bSpeed = true;
					}
					Log.Speed[ Cnt ] = +Param[ 7 ] * 1.85200;
				}
				
				var Time	= +Param[ 1 ];
				var Hour	= ~~( Time / 10000 );
				var Min		= ~~( Time / 100 ) % 100;
				var Sec		= Time % 100;
				
				var DateTime= ~~Param[ 9 ];
				var Day		= ~~( DateTime / 10000 );
				var Mon		= ~~( DateTime / 100 ) % 100;
				var Year	= DateTime % 100 + 2000;
				
				Log.Time[ Cnt ]	= Date.UTC( Year, Mon - 1, Day, Hour, Min, Sec );
				
				var Long = +Param[ 5 ];
				Long = ~~( Long / 100 ) + ( Long % 100 / 60 );
				if( Param[ 6 ] == 'W' ) Long = -Long;
				
				var Lati = +Param[ 3 ];
				Lati = ~~( Lati / 100 ) + ( Lati % 100 / 60 );
				if( Param[ 4 ] == 'S' ) Lati = -Lati;
				
				Log.Longitude[ Cnt ] = Long;
				Log.Latitude [ Cnt ] = Lati;
				
				++Cnt;
			}
		}
		file.Close();
	}
	
	return Cnt;
}
