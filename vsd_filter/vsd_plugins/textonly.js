//*** 初期化処理 ************************************************************

function Initialize(){
	// 使用する画像・フォントの宣言
	FontM = new Font( "ＭＳ ゴシック", 18, FONT_OUTLINE | FONT_NOANTIALIAS );
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

function Draw(){
	x = 0;
	y = 0;
	for( var v in Vsd ){
		if( typeof Vsd[ v ] != 'function' ){
			print( v + ": " + Vsd[ v ] );
		}
	}
	
	var date = new Date();
	date.setTime( Vsd.DateTime());
	print( date.toLocaleString());
	Vsd.DrawLapTimeLog( Vsd.Width - 1, 0, ALIGN_TOP | ALIGN_RIGHT, 10, FontM );
}
