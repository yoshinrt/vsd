/// torque log リーダ ////////////////////////////////////////////////////////

LogReaderInfo.push( "Torque log (*.csv)", "*.csv;*.csv.gz", "ReadTorqueLog" );

function ReadTorqueLog( Files ){
	
	var ParamDef = {
		// csv 項目名							Vsd 変数名		function or 乗数
		"Device Time"						: [ "Time",			StrToUTC ],
		"Speed (OBD)(km/h)"					: [ "Speed",		1 ],
		"Engine RPM(rpm)"					: [ "Tacho",		1 ],
		"Trip Distance(km)"					: [	"Distance",		1000 ],
		"Throttle Position(%)"				: [	"Accel",		1 ],
		"Longitude"							: [ "Longitude",	1 ],
		"Latitude"							: [ "Latitude",		1 ],
		"Acceleration Sensor(Z axis)(g)"	: [ "Gy",			-1 ],
		"Acceleration Sensor(X axis)(g)"	: [ "Gx",			1 ],
	};
	
	return ReadCSV( Files, ParamDef );
	
	function StrToUTC( str ){
		// 08-9-2012 11:02:36 (JST)
		str.match( /(\d+)-(\d+)-(\d+) (\d+):(\d+):(\d+)(\.\d+)/ );
		return Date.UTC(
			RegExp.$3, RegExp.$2 - 1, RegExp.$1,
			RegExp.$4, RegExp.$5, RegExp.$6, RegExp.$7 * 1000
		) - 9 * 3600 * 1000;
	}
}

/// 汎用 CSV リーダ //////////////////////////////////////////////////////////

function ReadCSV( Files, ParamDef ){
	var ParamUsed = [];
	
	var	Cnt = 0;
	var GPSValid = true;
	
	for( var i = 0; i < Files.length; ++i ){
		var file = new File();
		if( file.Open( Files[ i ], "zr" )){
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
				
				if( ParamDef[ Header[ i ]][ 0 ] == 'Longitude' ) GPSValid = false;
			}
		}
		
		while( 1 ){
			var Param = file.ReadLine().replace( /[\x0D\x0A]/g, '' ).split( /[,\t]/ );
			if( file.IsEOF()) break;
			
			for( var j = 0; j < ParamUsed.length; ++j ){
				if( typeof( ParamUsed[ j ][ 1 ] ) == 'function' ){
					// function なら，それを実行
					Log[ ParamUsed[ j ][ 0 ]][ Cnt ] =
						ParamUsed[ j ][ 1 ]( Param[ ParamUsed[ j ][ 2 ]] );
				}else if( Param[ ParamUsed[ j ][ 2 ]] != '' && !isNaN( Param[ ParamUsed[ j ][ 2 ]] )){
					// 係数なら，param に掛ける
					Log[ ParamUsed[ j ][ 0 ]][ Cnt ] =
						Param[ ParamUsed[ j ][ 2 ]] * ParamUsed[ j ][ 1 ];
				}else if( Cnt ){
					// データがない場合，1個前をコピー
					Log[ ParamUsed[ j ][ 0 ]][ Cnt ] =
						Log[ ParamUsed[ j ][ 0 ]][ Cnt -1 ];
				}
			}
			
			// GPS が補足できるまでのデータを補正
			if( !GPSValid && !isNaN( Log.Longitude[ Cnt ] )){
				GPSValid = true;
				
				for( var j = 0; j < Cnt; ++j ){
					Log.Longitude[ j ] = Log.Longitude[ Cnt ];
					Log.Latitude[ j ]  = Log.Latitude[ Cnt ];
				}
			}
			
			++Cnt;
		}
		file.Close();
	}
	
	// 最後まで GPS が捕捉できなければ，Long Lati を削除
	if( !GPSValid ){
		delete Log.Longitude;
		delete Log.Latitude;
	}
	
	return Cnt;
}
