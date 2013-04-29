/// VSD log リーダ ///////////////////////////////////////////////////////////

LogReaderInfo.push({
	Caption:	"VSD log (*.log)",
	Filter:		"*.log;*.log.gz",
	ReaderFunc:	"ReadVsdLog",
});

function ReadVsdLog( Files ){
	
	var ParamDef = {
		// csv 項目名	Vsd 変数名			function or 乗数
		"Date/Time"		: [ "Time",			StrToUTC ],
		"Speed"			: [ "Speed",		1 ],
		"Tacho"			: [ "Tacho",		1 ],
		"Distance"		: [	"Distance",		1 ],
		"Gy"			: [ "Gy",			1 ],
		"Gx"			: [ "Gx",			1 ],
		"Longitude"		: [ "Longitude",	1 ],
		"Latitude"		: [ "Latitude",		1 ],
		"LapTime"		: [ "LapTime",		StrToLapTime ],
	};
	
	var Ret = ReadCSV( Files, ParamDef );
	
	// Accel 偽造
	Log.Accel = [ 0 ];
	
	for( var i = 1; i < Ret; ++i ){
		Accel = Log.Tacho[ i + 1 ] - Log.Tacho[ i - 1 ];
		if( Accel < 0 ) Accel = 0;
		else if( Log.Gy[ i ] < -0.4 ) Accel *= 0.1;
		if( Accel > 50 ) Accel = 50;
		Log.Accel[ i ] = Accel;
	}
	
	if( typeof Log.Longitude == 'object' ){
		SmoothLowFreqLogSingle( Log.Longitude, Ret );
		SmoothLowFreqLogSingle( Log.Latitude,  Ret );
	}
	return Ret;
	
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
}
