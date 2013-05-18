/// torque log リーダ ////////////////////////////////////////////////////////

LogReaderInfo.push({
	Caption:	"Torque Lite (スムージング無) (*.csv)",
	Filter:		"*.csv;*.csv.gz",
	ReaderFunc:	"ReadTorqueLiteLog",
	Priority:	0x11000001,
});

LogReaderInfo.push({
	Caption:	"Torque Lite (スムージング有) (*.csv)",
	Filter:		"*.csv;*.csv.gz",
	ReaderFunc:	"ReadTorqueLiteLogSmooth",
	Priority:	0x11000000,
});

function ReadTorqueLiteLog( Files ){
	
	// 汎用 CSV リーダ用パラメータ指定例:
	// ・CSV ファイルの 1行目は項目名が入っていると仮定．
	// ・「CSV 項目名」に一致する項目があれば，その値を「VSD プロパティ名」
	//   のプロパティに代入する．
	// ・「function or 乗数」に数値が指定されている場合は，その数値を乗じてから
	//   VSD プロパティに代入する．
	//   function 名が指定されている場合は，CSV から得た文字列を引数として function
	//   を呼び，function から返された数値を VSD プロパティに代入する．
	
	var ParamDef = {
		// CSV 項目名						VSD プロパティ名	function or 乗数
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
		) + ( new Date ).getTimezoneOffset() * 60000;
	}
}

function ReadTorqueLiteLogSmooth( Files ){
	var Cnt = ReadTorqueLiteLog( Files );
	if( Cnt > 0 ) SmoothLowFreqLog( Cnt );
	return Cnt;
}

/// 汎用 CSV リーダ //////////////////////////////////////////////////////////

function ReadCSV( Files, ParamDef ){
	var ParamUsed = [];
	
	var	Cnt = 0;
	
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
				}
			}
			++Cnt;
		}
		file.Close();
	}
	
	// 最後まで GPS が捕捉できなければ，Long Lati を削除
	if( typeof Log.Latitude == 'object' && Log.Latitude.length == 0 ){
		delete Log.Longitude;
		delete Log.Latitude;
	}
	
	return Cnt;
}
