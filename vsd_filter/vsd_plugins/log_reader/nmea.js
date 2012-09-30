// NMEA-0183 リーダ

LogReaderInfo.push( "NMEA-0183 (*.nme*)", "*.hoge;*.nme*", "Read_nmea" );

function Read_nmea( Files ){
	
	Log.Time		= new Array();
	Log.Longitude	= new Array();
	Log.Latitude	= new Array();
	
	var	Cnt = 0;
	var Line;
	var bSpeed = false;
	
	for( var i = 0; i < Files.length; ++i ){
		var file = new File();
		if( file.Open( Files[ i ], "zr" )){
			MessageBox( "ファイルが開けません: " + Files[ i ] );
			return 0;
		}
		
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
				var Sec		= ~~Time % 100;
				var Milli	= ~~( Time * 1000 ) % 1000;
				
				var DateTime= ~~Param[ 9 ];
				var Day		= ~~( DateTime / 10000 );
				var Mon		= ~~( DateTime / 100 ) % 100;
				var Year	= DateTime % 100 + 2000;
				
				Log.Time[ Cnt ]	= Date.UTC( Year, Mon - 1, Day, Hour, Min, Sec, Milli );
				
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
