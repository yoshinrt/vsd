// VSD log reader

LogReaderInfo.push({
	Caption:	"サーキットの友 (*.ptd)",
	Filter:		"*.ptd",
	ReaderFunc:	"ReadTomoLog"
});

function ReadTomoLog( Files ){
	
	Log.Time		= [];
	Log.Speed		= [];
	Log.Tacho		= [];
	Log.Distance	= [];
	
	var	Cnt = 0;
	var Line;
	var bG = false;
	
	for( var i = 0; i < Files.length; ++i ){
		var file = new File();
		if( file.Open( Files[ i ], "zr" )){
			MessageBox( "ファイルが開けません: " + Files[ i ] );
			return 0;
		}
		
		Files[ i ].match( /(\d+)_(\d+)/ );
		var	Time	= ~~RegExp.$2;
		var Hour	= ~~( Time / 10000 );
		var Min		= ~~( Time / 100 ) % 100;
		var Sec		= Time % 100;
		
		var	DateTime= ~~RegExp.$1;
		var Day		= DateTime % 100;
		var Mon		= ~~( DateTime / 100 ) % 100;
		var Year	= ~~( DateTime / 10000 );
		
		while( !file.IsEOF()){
			Line = file.ReadLine();
			
			//tacho spd		distance	Gy		Gx
			// 0	1		2			3		4
			// 2336	60.93	12213.32	0.1572	0.1785
			
			var Param = Line.split( "\t" );
			
			Log.Tacho[ Cnt ]	= Param[ 0 ];
			Log.Speed[ Cnt ]	= Param[ 1 ];
			Log.Distance[ Cnt ]	= Param[ 2 ];
			
			// Speed がある場合は Array 作成
			if( Param.length >= 5 ){
				if( !bG ){
					Log.Gx = [];
					Log.Gy = [];
					bG = true;
				}
				Log.Gy[ Cnt ] = Param[ 3 ];
				Log.Gx[ Cnt ] = Param[ 4 ];
			}
			
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
