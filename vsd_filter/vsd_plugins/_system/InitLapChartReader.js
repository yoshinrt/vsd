"use strict";

var LapTime = [];

function ReadLapChart( filename ){
	
	var file = new File();
	try{
		file.Open( filename, "zr" );
	}catch( e ){
		MessageBox( "ファイルが開けません: " + filename );
		return 0;
	}
	
	var CameraCarID;
	var CameraCarName = '';
	var Line;
	var Drivers = [];
	
	while( 1 ){
		Line = file.ReadLine();
		if( file.IsEOF()) break;
		
		if( Line.match( /^Name:\s*(.*)/ )){
			// カメラ車の指定
			CameraCarName = RegExp.$1;
			
		}else{
			if( Line.match( /^LapTable:/ )){
				Line = file.ReadLine();
				
				// ドライバー名の取得
				Drivers = Line.replace( /[\x0D\x0A]+/, '' ).split( / *[,\t] */ );
				
				for( var i = 0; i < Drivers.length; ++i ){
					LapTime[ Drivers[ i ]] = [];
					if( Drivers[ i ] == CameraCarName ) CameraCarID = i;
				}
			}else{
				LapTime[ Drivers[ 0 ] = 'default' ] = [ GetMS( Line ) ];
			}
			
			// ラップタイムの取得
			while( 1 ){
				Line = file.ReadLine();
				if( file.IsEOF()) break;
				
				var Times = Line.replace( /[\x0D\x0A]+/, '' ).split( /(?:\s*,\s*| *\t *)/ );
				for( var i = 0; i < Times.length; ++i ){
					if( Times[ i ] != '' ){
						LapTime[ Drivers[ i ]].push( GetMS( Times[ i ] ));
					}
				}
			}
			
			if( Drivers[ 0 ] == 'default' ){
				LapTime.default.push( 0 );
			}
		}
	}
	
	return 1;
	
	function GetMS( str ){
		if( str.match( /^(\d+)[^\d\.]([\d\.]+)/ )){
			return ~~( RegExp.$1 * 60000 + RegExp.$2 * 1000 );
		}
		return ~~( str * 1000 );
	}
}
