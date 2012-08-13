//*** 初期化処理 ************************************************************

// 使用する画像・フォントの宣言
var FontS = new Font( "Impact", Vsd.Height / 30, FONT_FIXED );
var FontM = new Font( "Impact", Vsd.Height / 20, FONT_FIXED );
var FontM_Outline = new Font( "Impact", Vsd.Height / 20, FONT_OUTLINE | FONT_FIXED );

// メーターを右に表示する場合 1に設定
var MeterRight = 1;

// 動画サイズに応じてメーター画像をリサイズ
var Scale = Vsd.Height / 720;

var MeterR = 150 * Scale;
var MeterX	= MeterRight ? Vsd.Width  - MeterR * 2: 0;
var MeterY	= Vsd.Height - MeterR * 2;
var MeterCx = MeterX + MeterR;
var MeterCy = MeterY + MeterR;

// スピードグラフサイズ計算
var SpdX1 = MeterRight ? 8 : 300 * Scale * 1.1;
var SpdX2 = MeterRight ? Vsd.Width - 300 * Scale * 1.1 : Vsd.Width - 8;
var SpdY1 = Vsd.Height - 300 * Scale;
var SpdY2 = Vsd.Height - 8;

// スピードメータ用最高速計算
var MaxSpeed = Math.ceil( Vsd.MaxSpeed / 10 ) * 10;

//*** メーター描画処理 ******************************************************

function Draw(){
	Vsd.DrawCircle( MeterCx, MeterCy, MeterR, 0x80404040, DRAW_FILL );
	
	// メーター目盛り描画
	Vsd.DrawMeterScale(
		MeterCx, MeterCy, MeterR,
		MeterR * 0.1,  2, 0xFFFFFF,
		MeterR * 0.05, 1, 0xFFFFFF,
		2, 135, 45,
		MeterR * 0.75,
		MaxSpeed, 12, 0xFFFFFF,
		FontM
	);
	
	Vsd.DrawGSnake(	MeterCx, MeterCy, MeterR / 1.5, 5, 2, 0x00FF00, 0x008000 );
	
	var Speed = ~~Vsd.Speed;
	if     ( Speed <  10 ) Speed = "  " + Speed;
	else if( Speed < 100 ) Speed = " "  + Speed;
	
	Vsd.DrawText(
		MeterCx - FontM.GetTextWidth( Speed ) / 2, MeterCy + MeterR / 2,
		Speed, FontM, 0xFFFFFF
	);
	
	// G 数値
	var Accel = Math.sqrt( Vsd.Gx * Vsd.Gx + Vsd.Gy * Vsd.Gy ).toFixed( 1 ) + "G";
	Vsd.DrawText(
		MeterCx - FontS.GetTextWidth( Accel ) / 2, MeterCy + MeterR / 2 - FontS.Height,
		Accel, FontS, 0xFFFFFF
	);
	
	// スピードメーター針
	Vsd.DrawNeedle(
		MeterCx, MeterCy, MeterR * 0.95, 0,
		135, 45, Vsd.Speed / MaxSpeed, 0xFF0000, 3
	);
	
	// 走行軌跡
	//Vsd.PutImage( 0, 0, ImgMapBG );
	Vsd.DrawMap(
		8 * Scale, 8 * Scale, 500 * Scale, 300 * Scale,
		ALIGN_TOP | ALIGN_LEFT,
		2, 5, 0xFF0000, 0xFFFF00, 0x00FF00, 0xFF0000
	);
	
	// ラップタイム
	Vsd.DrawLapTime( Vsd.Width - 1, 0, ALIGN_TOP | ALIGN_RIGHT, FontM_Outline );
	
	// スピードグラフ
	Vsd.DrawGraph( SpdX1, SpdY1, SpdX2, SpdY2, FontM, GRAPH_SPEED | GRAPH_GX | GRAPH_GY | GRAPH_TILE );
}
