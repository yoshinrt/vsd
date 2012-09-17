function Read_nmea( FileName ){
	
	Log				= new Array();
	Log.Time		= new Array();
	Log.Longitude	= new Array();
	Log.Latitude	= new Array();
	
	for( var a = 0; a < 10; ++a ){
		Log.Time		[ a ] = a;
		Log.Longitude	[ a ] = a;
		Log.Latitude	[ a ] = a;
	}
	
//	var file = File();
//	
//	if( file.Open( FileName, "r" )) return 1;
//	
//	var Line;
//	while( Line = file.ReadLine()){
//		
//	}
//	file.Close();
	
	return 0;
}
