// kml / kmz リーダ

LogReaderInfo.push({
	Caption:	"Google Keyhole ML (*.kml,*.kmz)",
	Filter:		"*.kml;*.kmz",
	ReaderFunc:	"Read_kml"
});

/****************************************************************************/

function Read_kml( Files ){
	
	Log.Time		= [];
	Log.Longitude	= [];
	Log.Latitude	= [];
	Log.Altitude	= [];
	
	var	Cnt = 0;
	var bMyTracks; // Google My Tracks のバグ? 回避
	
	var WshShell = new ActiveXObject( "WScript.Shell" );
	var fso = new ActiveXObject( "Scripting.FileSystemObject" );
	
	for( var i = 0; i < Files.length; ++i ){
		
		if( Files[ i ].match( /\.kmz$/ )){
			// kmz を unzip
			var ExecInfo = WshShell.Exec( 'c:\\cygwin\\bin\\unzip -c "' + Files[ i ] + '"' );
			file = ExecInfo.StdOut;
		}else{
			// kml を普通に open
			var file = fso.OpenTextFile( WScript.arguments( 0 ), 1 );
		}
		
		var Line = '';
		var Buf;
		
	  NextFile:
		while( 1 ){
			
			// <when>2014-09-05T23:45:56.415Z</when>
			// <gx:coord>135.12345 35.12345 160.10000610351563</gx:coord>
			
			// </gx:coord> サーチ
			while( !Line.match( /<\/gx:coord>/ )){
				if( bMyTracks === undefined && Line.match( /Google My Tracks/ )){
					bMyTracks = true;
				}
				
				Line = Line + file.ReadLine();
				if( file.AtEndOfStream ) break NextFile;
			}
			if( bMyTracks === undefined ) bMyTracks = false;
			
			// いらないデータ破棄
			var Point = ( RegExp.leftContext + RegExp.lastMatch ).replace( /[\x0D\x0A]/g, "" );
			Line = RegExp.rightContext;
			
			// 時間
			if( !Point.match( /<when>(\d+)-(\d+)-(\d+)T(\d+):(\d+):([\d\.]+)(Z|([-+]\d+):(\d+))/ )){
				continue;
			}
			Log.Time[ Cnt ] = Date.UTC(
				RegExp.$1, RegExp.$2 - 1, RegExp.$3,
				RegExp.$4 - ( RegExp.$7 == 'Z' ? 0 : RegExp.$8 ),
				RegExp.$5 - ( RegExp.$7 == 'Z' ? 0 : RegExp.$8 >= 0 ? RegExp.$9 : -RegExp.$9 ),
				~~RegExp.$6, ~~( RegExp.$6 * 1000 ) % 1000
			);
			
			// long, lat, alt
			if( !Point.match( /<gx:coord>(\S+)\s+(\S+)\s+([\d\.]+)/ )){
				continue;
			}
			Log.Longitude[ Cnt ] = +RegExp.$1;
			Log.Latitude [ Cnt ] = +RegExp.$2;
			Log.Altitude [ Cnt ] = +RegExp.$3;
			
			// Google My Tracks は変なログを吐くのでその対策
			if( !( bMyTracks && Cnt &&
				Log.Latitude [ Cnt - 1 ] == Log.Latitude [ Cnt ] &&
				Log.Longitude[ Cnt - 1 ] == Log.Longitude[ Cnt ]
			)) ++Cnt;
		}
		file.Close();
	}
	
	return Cnt;
}
