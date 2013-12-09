// NMEA-0183 リーダ

LogReaderInfo.push({
	Caption:	"NMEA-0183 (*.nme*)",
	Filter:		"*.nmea;*.nme;*.nmea.gz;*.nme.gz",
	ReaderFunc:	"Read_nmea"
});

function Read_nmea( Files ){
	
	Log.Time		= [];
	Log.Longitude	= [];
	Log.Latitude	= [];
	
	var	Cnt = 0;
	var Line;
	var bSpeed		= false;
	var bAltitude	= false;
	
	for( var i = 0; i < Files.length; ++i ){
		var file = new File();
		if( file.Open( Files[ i ], "zr" )){
			MessageBox( "ファイルが開けません: " + Files[ i ] );
			return 0;
		}
		
		while( 1 ){
			Line = file.ReadLine();
			if( file.IsEOF()) break;
			
			if( Line.substr( 0, 6 ) == "$GPRMC" ){
				//        時間         lat           long           knot  方位   日付
				// 0      1          2 3           4 5            6 7     8      9
				// $GPRMC,043431.200,A,3439.997825,N,13523.377978,E,0.602,178.29,240612,,,A*59
				
				var Param = Line.split( "," );
				
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
				
				// Speed がある場合は Array 作成
				if( Param[ 7 ] != '' ){
					if( !bSpeed ){
						Log.Speed = [];
						bSpeed = true;
					}
					Log.Speed[ Cnt ] = +Param[ 7 ] * 1.85200;
				}
				
				++Cnt;
			}else if( Line.substr( 0, 6 ) == "$GPGGA" ){
				//        時間       lat           long               高度
				// 0      1          2           3 4            5 6 789
				// $GPGGA,233132.000,3439.997825,N,13523.377978,E,1,,,293.425,M,,,,*21
				
				var Param = Line.split( "," );
				
				// 高度がある場合は Array 作成
				if( Param[ 9 ] != '' ){
					if( !bAltitude ){
						Log.Altitude = [];
						bAltitude = true;
					}
					Log.Altitude[ +Param[ 1 ] == Time ? Cnt - 1 : Cnt ] = +Param[ 9 ];
				}
				var Time	= +Param[ 1 ];
			}
		}
		file.Close();
	}
	
	return Cnt;
}
