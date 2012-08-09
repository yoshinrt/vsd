var FontS = new Font( "Impact", Vsd.Height / 30, FONT_FIXED );
var FontM = new Font( "Impact", Vsd.Height / 20, FONT_FIXED );
var FontM_Outline = new Font( "Impact", Vsd.Height / 20, FONT_OUTLINE | FONT_FIXED );

var MeterR = ~~( Vsd.Height * 100 / 480 );
var MeterCx = Vsd.Width  - MeterR;
var MeterCy = Vsd.Height - MeterR;

var LapTimeX = Vsd.Width - FontM_Outline.GetTextWidth( "Time 0'00.000" );

function Draw(){
	Vsd.DrawCircle( MeterCx, MeterCy, MeterR, 0x80404040, 1 );
	
	// メーター目盛り描画
	Vsd.DrawMeterScale(
		MeterCx, MeterCy, MeterR,
		MeterR * 0.1,  2, 0xFFFFFF,
		MeterR * 0.05, 1, 0xFFFFFF,
		2, 135, 45,
		MeterR * 0.75,
		Vsd.MaxSpeed, 12, 0xFFFFFF,
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
		135, 45, Vsd.Speed / Vsd.MaxSpeed, 0xFF0000, 3
	);
	
	// 走行軌跡
	Vsd.DrawMap( 8, 8, Vsd.Height / 1.5, 2, 5, 0xFF0000, 0xFFFF00, 0x00FF00, 0xFF0000 );
	
	// ラップタイム
	Vsd.DrawLapTime( LapTimeX, 0, FontM_Outline, 0xFFFFFF, 0, 0x00FFFF, 0xFF0000 );
	
	Vsd.DrawGraph( 0, 0, Vsd.Width, Vsd.Height, FontM );
}
