//*** 初期化処理 ************************************************************

function Initialize(){
	// 使用する画像・フォントの宣言
	FontM = new Font( "ＭＳ ゴシック", 12, FONT_OUTLINE | FONT_NOANTIALIAS );
}

//*** メーター描画処理 ******************************************************

var x;
var y;

function print( str ){
	Vsd.DrawText( x, y, str, FontM );
	y += FontM.Height;
	
	if( y > Vsd.Height - FontM.Height ){
		y = 0;
		x = Vsd.Width / 2;
	}
}

function PrintObj( prefix, obj ){
	for( var v in obj ){
		if( typeof obj[ v ] == 'object' ){
			PrintObj( prefix + v + '.', obj[ v ] );
		}else if(
			typeof obj[ v ] != 'function' &&
			!( prefix == "Vsd." && v.match( /^(Min|Max)?(Time|Speed|Tacho|Distance|Direction|Longitude|Latitude|X|Y|Gx|Gy|TurnR)$/ ))
		){
			print( prefix + v + ": " + obj[ v ] );
		}
	}
}
function Draw(){
	x = 0;
	y = 0;
	
	PrintObj( 'Vsd.', Vsd );
	PrintObj( 'Log.', Log );
	
	var date = new Date();
	date.setTime( Vsd.DateTime )
	print( date.toLocaleString());
	Vsd.DrawLapTimeLog( Vsd.Width - 1, 0, ALIGN_TOP | ALIGN_RIGHT, 10, FontM );
}
