//*** 初期化処理 ************************************************************

function Initialize(){
	Scale = Vsd.Width / 1280;
	
	// 使用する画像・フォントの宣言
	FontS = new Font( "Arial Black", 20 * Scale );
	FontM = new Font( "Arial Black", 42 * Scale );
	FontL = new Font( "Arial Black", 90 * Scale );
	
	GCx = Vsd.Width - 330 * Scale;
	GCy = 60 * Scale;
	GR  = 55 * Scale;
	
	BackImg = new Image( Vsd.SkinDir + "compact.png" );
	BackImg.Resize( Vsd.Width, 120 * Scale );
	
	MeterCx = Vsd.Width - 140 * Scale;
	MeterCy = 260 * Scale;
	MeterR  = 250 * Scale;
	
	// グラフ用パラメータ生成
	GraphParam = [
		"Speed",	"%.0f km/h",	0x00FFFF,
		"Tacho",	"%.0f rpm",		0xFF4000,
	//	"Accel",	"%.0f %%",		0x00FF00,
	//	"TurnR",	"%.0f m",		0x00FF00,
		"Gx",		"%.2f G[lat]",	0x00FF00,
		"Gy",		"%.2f G[lon]",	0xFF00FF,
	];
}

//*** メーター描画処理 ******************************************************

function Draw(){
	// 背景
	//Vsd.DrawRect( 0, 0, Vsd.Width - 1, 120, 0x80006000, DRAW_FILL );
	Vsd.PutImage( 0, 0, BackImg );
	
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
	
	// ラップタイム
	Vsd.DrawTextAlign( 170 * Scale, 20 * Scale, ALIGN_VCENTER, "Best:", FontM, 0xFFFF00 );
	Vsd.DrawTextAlign(
		470 * Scale, 20 * Scale, ALIGN_VCENTER | ALIGN_RIGHT,
		Vsd.FormatTime( Vsd.BestLapTime ), FontM, 0xFFFFFF
	);
	Vsd.DrawTextAlign(
		170 * Scale, 60 * Scale, ALIGN_VCENTER,
		"Lap" + Vsd.LapCnt + "/" + Vsd.MaxLapCnt, FontM, 0xFFFF00
	);
	Vsd.DrawTextAlign(
		470 * Scale, 60 * Scale, ALIGN_VCENTER | ALIGN_RIGHT,
		Vsd.FormatTime( Vsd.LapTime ), FontM, 0xFFFFFF
	);
	Vsd.DrawTextAlign(
		470 * Scale, 100 * Scale, ALIGN_VCENTER | ALIGN_RIGHT,
		( Vsd.DiffTime > 0 ? '+' : '' ) + Vsd.FormatTime( Vsd.DiffTime ),
		FontM, Vsd.DiffTime <= 0 ? 0x00FFFF : 0xFF6030
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
	Vsd.DrawMap(
		5, 5, 155 * Scale, 115 * Scale, ALIGN_HCENTER | ALIGN_VCENTER,
		2, 4 * Scale, ( Vsd.FrameCnt & 8 ) ? 0x00FFFF : 0xFF000000,
		0xFF6030, 0xFF6030, 0xFF6030
	);
	
	// グラフ
	Vsd.DrawGraph(
		480 * Scale, 5, Vsd.Width - 400 * Scale, 120 * Scale - 5, FontS,
		GRAPH_HTILE, GraphParam
	);
}
