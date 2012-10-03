// torque log

LogReaderInfo.push( "Torque log (*.csv)", "*.csv;*.csv.gz", "ReadTorqueLog" );

function ReadTorqueLog( Files ){
	
	var	Cnt = 0;
	
	for( var i = 0; i < Files.length; ++i ){
		var file = new File();
		if( file.Open( Files[ i ], "zr" )){
			MessageBox( "ファイルが開けません: " + Files[ i ] );
			return 0;
		}
		
		IdxTime =
		IdxSpeed =
		IdxTacho =
		IdxDistance =
		IdxAccel = -1;
		
		// ヘッダリード
		var Header = file.ReadLine().split( /\s*,\s*/ );
		for( var i = 0; i < Header.length; ++i ){
			if( Header[ i ] == 'Device Time' ){
				IdxTime = i;
				Log.Time = new Array();
			}else if( Header[ i ] == 'Speed (OBD)(km/h)' ){
				Log.Speed = new Array();
				IdxSpeed  = i;
			}else if( Header[ i ] == 'Engine RPM(rpm)' ){
				Log.Tacho = new Array();
				IdxTacho  = i;
			}else if( Header[ i ] == 'Trip Distance(km)' ){
				Log.Distance	= new Array();
				IdxDistance  = i;
			}else if( Header[ i ] == 'Throttle Position(%)' ){
				Log.Accel = new Array();
				IdxAccel  = i;
			}
		}
		
		while( !file.IsEOF()){
			var Param = file.ReadLine().split( "\t" );
			
			if( IdxTacho >= 0 ) Log.Tacho[ Cnt ]	= Param[ IdxTacho ];
			if( IdxSpeed >= 0 ) Log.Speed[ Cnt ]	= Param[ IdxSpeed ];
			if( IdxAccel >= 0 ) Log.Accel[ Cnt ]	= Param[ IdxAccel ];
			if( IdxDistance >= 0 ) Log.Distance[ Cnt ]	= Param[ IdxDistance ] * 1000;
			
			var Time	= +Param[ 1 ];
			var Milli	= ~~( Time * 1000 ) % 1000;
			
			var DateTime= ~~Param[ 9 ];
			
			Log.Time[ Cnt ]	= Date.UTC( Year, Mon - 1, Day, Hour, Min, Sec ) + Cnt * 1000 / 16;
			++Cnt;
		}
		file.Close();
	}
	
	return Cnt;
}
