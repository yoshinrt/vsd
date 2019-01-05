/// VSD log リーダ ///////////////////////////////////////////////////////////

LogReaderInfo.push({
	Caption:	"VSD log (*.log)",
	Filter:		"*.log;*.log.gz",
	ReaderFunc:	"ReadVsdLog",
});

function ReadVsdLog( Files ){
	
	var GpsTime	= [];
	var Lati	= [];
	var Long	= [];
	var Alti	= [];
	
	var ParamDef = {
		// csv 項目名	Vsd 変数名			function or 乗数
		"Date/Time"		: [ "Time",			StrToUTC ],
		"Speed"			: [ "Speed",		1 ],
		"Tacho"			: [ "Tacho",		1 ],
		"Distance"		: [	"Distance",		1 ],
		"Gy"			: [ "Gy",			1 ],
		"Gx"			: [ "Gx",			1 ],
		"Throttle"		: [ "Accel",		1 ],
		"Longitude"		: [ "Longitude",	1 ],
		"Latitude"		: [ "Latitude",		1 ],
		"LapTime"		: [ "LapTime",		StrToLapTime ],
	};
	
	var Cnt = ReadVsdSub( Files, ParamDef );
	
	if( GpsTime.length == 0 ) return Cnt;
	
	// GPS ログを VSD ログにマージ
	var ig = -1;
	var GpsTimeDiff = 0;	// [ms]
	
	Log.Longitude	= [];
	Log.Latitude	= [];
	Log.Altitude	= [];
	
	for( var iv = 0; iv < Cnt; ++iv ){
		
		// GPSTime[ ig ] <= VsdTime < GpsTime[ ig + 1 ] となる ig を探索
		for( ; ig < GpsTime.length - 1; ++ig ){
			if( Log.Time[ iv ] < GpsTime[ ig + 1 ] - GpsTimeDiff ) break;
		}
		
		// GPS ログが存在しない (開始時)
		if( ig == -1 ){
			Log.Longitude[ iv ]	= Long[ 0 ];
			Log.Latitude [ iv ]	= Lati[ 0 ];
			Log.Altitude [ iv ]	= Alti[ 0 ];
		}
		
		// GPS ログが存在しない (開始時)
		else if( ig == GpsTime.length - 1 ){
			Log.Longitude[ iv ]	= Long[ GpsTime.length - 1 ];
			Log.Latitude [ iv ]	= Lati[ GpsTime.length - 1 ];
			Log.Altitude [ iv ]	= Alti[ GpsTime.length - 1 ];
		}
		
		// GPS ログを等比分割
		else{
			var a = ( Log.Time[ iv ] - ( GpsTime[ ig ] - GpsTimeDiff )) / ( GpsTime[ ig + 1 ] - GpsTime[ ig ]);
			Log.Longitude[ iv ]	= ( Long[ ig + 1 ] - Long[ ig ]) * a + Long[ ig ];
			Log.Latitude [ iv ]	= ( Lati[ ig + 1 ] - Lati[ ig ]) * a + Lati[ ig ];
			Log.Altitude [ iv ]	= ( Alti[ ig + 1 ] - Alti[ ig ]) * a + Alti[ ig ];
		}
	}
	
	return Cnt;
	
	function StrToUTC( str ){
		// 2012-01-23T11:02:36.789Z (GMT)
		str.match( /(\d+)-(\d+)-(\d+)T(\d+):(\d+):(\d+)(\.\d+)/ );
		return Date.UTC(
			RegExp.$1, RegExp.$2 - 1, RegExp.$3,
			RegExp.$4, RegExp.$5, RegExp.$6, RegExp.$7 * 1000
		);
	}
	
	function StrToLapTime( str ){
		// 1:23.456
		if( typeof( str ) == "string" && str.match( /(?:(\d+):)?([\d\.]+)/ )){
			return RegExp.$1 * 60000 + RegExp.$2 * 1000;
		}
		return -1;
	}

	function ReadVsdSub( Files, ParamDef ){
		var ParamUsed = [];
		
		var	Cnt = 0;
		
		for( var i = 0; i < Files.length; ++i ){
			var file = new File();
			try{
				file.Open( Files[ i ], "zr" );
			}catch( e ){
				MessageBox( "ファイルが開けません: " + Files[ i ] );
				return 0;
			}
			
			// ヘッダリード
			var Header = file.ReadLine().replace( /[\x0D\x0A]/g, '' ).split( / *[,\t] */ );
			
			for( var i = 0; i < Header.length; ++i ){
				if( typeof ParamDef[ Header[ i ]] != "undefined" ){
					ParamDef[ Header[ i ]][ 2 ] = i;
					ParamUsed.push( ParamDef[ Header[ i ]] );
					Log[ ParamDef[ Header[ i ]][ 0 ]] = [];
				}
			}
			
			while( 1 ){
				var Param = file.ReadLine().replace( /[\x0D\x0A]/g, '' ).split( /[,\t]/ );
				if( file.IsEOF()) break;
				
				if( Param[ 0 ] == 'GPS' ){
					// 0	1							2			3			4		5
					// GPS	2019-01-04T04:34:39.200Z	136.12345	35.12345	92.600	0.037
					
					Long[ GpsTime.length ]		= +Param[ 2 ];
					Lati[ GpsTime.length ]		= +Param[ 3 ];
					Alti[ GpsTime.length ]		= +Param[ 4 ];
					GpsTime[ GpsTime.length ]	= StrToUTC( Param[ 1 ]);
				}
				
				else{
					for( var j = 0; j < ParamUsed.length; ++j ){
						if( typeof( ParamUsed[ j ][ 1 ] ) == 'function' ){
							// function なら，それを実行
							Log[ ParamUsed[ j ][ 0 ]][ Cnt ] =
								ParamUsed[ j ][ 1 ]( Param[ ParamUsed[ j ][ 2 ]] );
						}else if( Param[ ParamUsed[ j ][ 2 ]] != '' && !isNaN( Param[ ParamUsed[ j ][ 2 ]] )){
							// 係数なら，param に掛ける
							Log[ ParamUsed[ j ][ 0 ]][ Cnt ] =
								Param[ ParamUsed[ j ][ 2 ]] * ParamUsed[ j ][ 1 ];
						}
					}
					
					++Cnt;
				}
			}
			file.Close();
		}
		
		return Cnt;
	}
}
