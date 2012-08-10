//*** 初期化処理 ************************************************************

// 使用する画像・フォントの宣言
var ImgMeter = new Image( Vsd.SkinDir + "meter_bg.png" );
//var ImgMapBG = new Image( Vsd.SkinDir + "square.png" );
var ImgG     = new Image( ImgMeter );

var FontS = new Font( "Impact", Vsd.Height / 30 );
var FontM = new Font( "Impact", Vsd.Height / 23 );
var FontL = new Font( "Impact", Vsd.Height / 12 );
var FontM_Outline = new Font( "Impact", Vsd.Height / 23, FONT_FIXED | FONT_OUTLINE );

// メーターを右に表示する場合 1に設定
var MeterLeft = 0;

// 動画サイズに応じてメーター画像をリサイズ
var Scale = Vsd.Height / 720;
if( Scale != 1 ){
	ImgMeter.Resize( ImgMeter.Width * Scale, ImgMeter.Height * Scale );
	ImgMapBG.Resize( ImgMapBG.Width * Scale, ImgMapBG.Height * Scale );
}

// 座標等を予め計算しておく
var MeterX	= MeterLeft ? Vsd.Width  - ImgMeter.Width : 0;
var MeterY	= Vsd.Height - ImgMeter.Height * 0.85
var MeterR  = 150 * Scale;
var MeterR2 = 126 * Scale;
var MeterCx = MeterX + MeterR;
var MeterCy = MeterY + MeterR;

// G 用画像をリサイズ
ImgG.Resize( ImgG.Width * Scale / 2, ImgG.Height * Scale / 2 );

// G メーター用の座標計算
var MeterGX	 = MeterLeft ? Vsd.Width - ImgMeter.Width * 1.45 : ImgMeter.Width * 0.95;
var MeterGY	 = Vsd.Height - ImgG.Height;
var MeterGR  = MeterR / 2;
var MeterGR2 = MeterR2 / 2;
var MeterGCx = MeterGX + MeterGR;
var MeterGCy = MeterGY + MeterGR;

//*** メーター描画処理 ******************************************************

function Draw(){
	// メーター画像描画
	Vsd.PutImage( MeterX, MeterY, ImgMeter );
	
	// メーター目盛り描画
	Vsd.DrawMeterScale(
		MeterCx, MeterCy, MeterR2,
		MeterR2 * 0.1,  2, 0xFFFFFF,
		MeterR2 * 0.05, 1, 0xFFFFFF,
		5, 135, 45,
		MeterR2 * 0.75,
		Vsd.MaxSpeed, 12, 0xFFFFFF,
		FontM
	);
	
	// スピード数値表示
	var Speed = ~~Vsd.Speed;
	Vsd.DrawTextAlign(
		MeterCx, MeterCy + MeterR * 0.25, 
		ALIGN_HCENTER | ALIGN_VCENTER,
		Speed, FontL, 0xFFFFFF
	);
	
	Vsd.DrawTextAlign(
		MeterCx, MeterCy + MeterR * 0.5,
		ALIGN_HCENTER | ALIGN_VCENTER,
		"km/h", FontS, 0xFFFFFF
	);
	
	// スピードメーター針
	Vsd.DrawNeedle(
		MeterCx, MeterCy, MeterR2 * 0.95, MeterR2 * -0.1,
		135, 45, Vsd.Speed / Vsd.MaxSpeed, 0xFF0000, 3
	);
	
	// Gメーターパネル画像描画
	Vsd.PutImage( MeterGX, MeterGY, ImgG );
	Vsd.DrawLine( MeterGCx - MeterGR2, MeterGCy, MeterGCx + MeterGR2, MeterGCy, 0x802000 );
	Vsd.DrawLine( MeterGCx, MeterGCy - MeterGR2, MeterGCx, MeterGCy + MeterGR2, 0x802000 );
	Vsd.DrawCircle( MeterGCx, MeterGCy, MeterGR2 / 3,     0x802000 );
	Vsd.DrawCircle( MeterGCx, MeterGCy, MeterGR2 / 3 * 2, 0x802000 );
	
	// G 数値
	var Accel = Math.sqrt( Vsd.Gx * Vsd.Gx + Vsd.Gy * Vsd.Gy ).toFixed( 1 ) + "G";
	Vsd.DrawText(
		MeterGX, MeterGCy + MeterR / 2 - FontS.Height,
		Accel, FontS, 0xFFFFFF
	);
	
	// G スネーク
	Vsd.DrawGSnake(	MeterGCx, MeterGCy, MeterGR2 / 1.5, 5 * Scale, 2, 0xFF4000, 0x802000 );
	
	// 走行軌跡
	//Vsd.PutImage( 0, 0, ImgMapBG );
	Vsd.DrawMap(
		8 * Scale, 8 * Scale, 500 * Scale, 300 * Scale,
		ALIGN_TOP | ALIGN_LEFT,
		2, 5, 0xFF0000, 0xFFFF00, 0x00FF00, 0xFF0000
	);
	
	// ラップタイム
	Vsd.DrawLapTime( Vsd.Width - 1, 0, ALIGN_TOP | ALIGN_RIGHT, FontM_Outline, 0xFFFFFF, 0, 0x00FFFF, 0xFF4000 );
	
	Vsd.DrawGraph( 0, 0, Vsd.Width, Vsd.Height, FontM );
}
