//*** 初期化処理 ************************************************************

function Initialize(){
	Scale = Vsd.Width / 1280;
	
	// 使用する画像・フォントの宣言
	FontSOL = new Font( "Impact", 30 * Scale, FONT_FIXED | FONT_OUTLINE );
	FontTime = new Font( "Impact", 36 * Scale, FONT_FIXED | FONT_OUTLINE );
	
	FontS = new Font( "Arial Black", 20 * Scale );
	FontM = new Font( "Arial Black", 42 * Scale );
	FontL = new Font( "Arial Black", 90 * Scale );
	
	GCx = Vsd.Width - 330 * Scale;
	GCy = 60 * Scale;
	GR  = 55 * Scale;
	
	MeterCx = Vsd.Width - 140 * Scale;
	MeterCy = 260 * Scale;
	MeterR  = 250 * Scale;
}

//*** メーター描画処理 ******************************************************

function Draw(){
	// 背景
	Vsd.DrawRect(( 1280 - 390 ) * Scale, 0, Vsd.Width - 1, 120 * Scale, 0xE0000000, DRAW_FILL );
	
	// スピード
	Vsd.DrawTextAlign(
		Vsd.Width, 115 * Scale, ALIGN_RIGHT | ALIGN_BOTTOM,
		"km/h", FontM, 0x00FFFF
	);
	Vsd.DrawTextAlign(
		Vsd.Width - 90 * Scale, 80 * Scale, ALIGN_RIGHT | ALIGN_VCENTER,
		~~Vsd.Speed, FontL, 0x00FFFF
	);
	
	if( Vsd.Speed > 0 ) Vsd.DrawArc(
		MeterCx, MeterCy,
		MeterR, MeterR,
		MeterR * 0.9, MeterR * 0.9,
		240, 240 + 60 * Vsd.Speed / Vsd.MaxSpeed, 0x00FFFF
	);
	
	if( Vsd.Speed < Vsd.MaxSpeed ) Vsd.DrawArc(
		MeterCx, MeterCy,
		MeterR, MeterR,
		MeterR * 0.9, MeterR * 0.9,
		240 + 60 * Vsd.Speed / Vsd.MaxSpeed, 300, 0x004040
	);
	
	// G メータ
	Vsd.DrawCircle( GCx, GCy, GR, 0x008000 );
	Vsd.DrawCircle( GCx, GCy, GR * 2 / 3, 0x008000 );
	Vsd.DrawCircle( GCx, GCy, GR / 3, 0x008000 );
	Vsd.DrawLine( GCx - GR, GCy, GCx + GR, GCy, 0x008000 );
	Vsd.DrawLine( GCx, GCy - GR, GCx, GCy + GR, 0x008000 );
	Vsd.DrawGSnake(	GCx, GCy, GR / 1.5, 5 * Scale, 2, 0x00FF00, 0x008000 );
	
	var Accel = Math.sqrt( Vsd.Gx * Vsd.Gx + Vsd.Gy * Vsd.Gy ).toFixed( 1 ) + "G";
	Vsd.DrawTextAlign(
		GCx + GR, GCy + GR, ALIGN_BOTTOM | ALIGN_RIGHT, Accel, FontS
	);
	
	// 走行軌跡
	Vsd.DrawMapPosition(
		8, 120 * Scale, Vsd.Width - 8, Vsd.Height - 8,
		ALIGN_BOTTOM | ALIGN_RIGHT,
		//4 * Scale, 0x00E0FF, FontSOL
		4 * Scale, 0xFF6030, FontSOL
	);
	
	Vsd.DrawRaceLapTime( 0, 0, 0, -15, FontTime, 0xFFE080 );
}
