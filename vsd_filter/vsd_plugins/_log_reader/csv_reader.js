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
