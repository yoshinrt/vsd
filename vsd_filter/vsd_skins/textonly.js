//*** 初期化処理 ************************************************************

// 使用する画像・フォントの宣言
var FontM = new Font( "ＭＳ Ｐゴシック", 20, FONT_OUTLINE | FONT_NOANTIALIAS );

//*** メーター描画処理 ******************************************************

var y;

function print( str ){
	Vsd.DrawText( 0, y, str, FontM );
	y += FontM.Height;
}

function Draw(){
	y = 0;
	print( "Speed: " + Vsd.Speed.toFixed( 2 ) + "/" + Vsd.MaxSpeed );
	print( "Tacho: " + Vsd.Tacho.toFixed( 0 ) + "/" + Vsd.MaxTacho );
	print( "Gx: " + Vsd.Gx.toFixed( 2 ) + "/" + Vsd.MaxGx.toFixed( 2 ));
	print( "Gy: " + Vsd.Gy.toFixed( 2 ) + "/" + Vsd.MaxGy.toFixed( 2 ));
	print( "Lap: " + Vsd.LapCnt + "/" + Vsd.MaxLapCnt );
	print( "Time: " + Vsd.FormatTime( Vsd.ElapsedTime ));
	print( "LapTime: " + Vsd.FormatTime( Vsd.LapTime ));
	print( "BestLapTime: " + Vsd.FormatTime( Vsd.BestLapTime ));
	print( "DiffTime: " + Vsd.FormatTime( Vsd.DiffTime ));
	print( "Resolution: " + Vsd.Width + "*" + Vsd.Height );
	print( "Frame: " + Vsd.FrameCnt + "/" + Vsd.MaxFrameCnt );
	print( "SkinDir: " + Vsd.SkinDir );
	print( "VsdRootDir: " + Vsd.VsdRootDir );
	
	Vsd.DrawLapTimeLog( Vsd.Width - 1, 0, ALIGN_TOP | ALIGN_RIGHT, 10, FontM );
}
