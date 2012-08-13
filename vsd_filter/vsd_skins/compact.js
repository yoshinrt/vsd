//*** 初期化処理 ************************************************************

var Scale = Vsd.Height / 720;

// 使用する画像・フォントの宣言
var FontS = new Font( "Arial Black", 20 * Scale );
var FontM = new Font( "Arial Black", 44 * Scale );
var FontL = new Font( "Arial Black", 90 * Scale );

var GCx = 540 * Scale;
var GCy = 60 * Scale;
var GR  = 55 * Scale;

var BackImg = new Image( Vsd.SkinDir + "compact.png" );
BackImg.Resize( Vsd.Width, 120 );

//*** メーター描画処理 ******************************************************

function Draw(){
	//Vsd.DrawRect( 0, 0, 1280, 720, 0xFFFFFF, DRAW_FILL );
	
	// 背景
	//Vsd.DrawRect( 0, 0, Vsd.Width - 1, 120, 0x80006000, DRAW_FILL );
	Vsd.PutImage( 0, 0, BackImg );
	
	// スピード
	Vsd.DrawTextAlign(
		Vsd.Width - 10, 100 * Scale, ALIGN_RIGHT | ALIGN_BOTTOM,
		"km/h", FontM, 0x00FF00
	);
	Vsd.DrawTextAlign(
		Vsd.Width - 100 * Scale, 60 * Scale, ALIGN_RIGHT | ALIGN_VCENTER,
		~~Vsd.Speed, FontL, 0x00FF00
	);
	
	// ラップタイム
	Vsd.DrawTextAlign( 170 * Scale, 20 * Scale, ALIGN_VCENTER, "Best:", FontM, 0x00FF00 );
	Vsd.DrawTextAlign(
		480 * Scale, 20 * Scale, ALIGN_VCENTER | ALIGN_RIGHT,
		Vsd.FormatTime( Vsd.BestLapTime ), FontM, 0x00FF00
	);
	Vsd.DrawTextAlign(
		170 * Scale, 60 * Scale, ALIGN_VCENTER,
		"Lap" + Vsd.LapCnt + "/" + Vsd.MaxLapCnt, FontM, 0x00FF00
	);
	Vsd.DrawTextAlign(
		480 * Scale, 60 * Scale, ALIGN_VCENTER | ALIGN_RIGHT,
		Vsd.FormatTime( Vsd.LapTime ), FontM, 0x00FF00
	);
	Vsd.DrawTextAlign(
		170 * Scale, 100 * Scale, ALIGN_VCENTER,
		"Diff:", FontM, 0x00FF00
	);
	Vsd.DrawTextAlign(
		480 * Scale, 100 * Scale, ALIGN_VCENTER | ALIGN_RIGHT,
		Vsd.FormatTime( Vsd.DiffTime ), FontM, 0x00FF00
	);
	
	// G メータ
	Vsd.DrawCircle( GCx, GCy, GR, 0x00FF00 );
	Vsd.DrawCircle( GCx, GCy, GR * 2 / 3, 0x00FF00 );
	Vsd.DrawCircle( GCx, GCy, GR / 3, 0x00FF00 );
	Vsd.DrawLine( GCx - GR, GCy, GCx + GR, GCy, 0x00FF00 );
	Vsd.DrawLine( GCx, GCy - GR, GCx, GCy + GR, 0x00FF00 );
	Vsd.DrawGSnake(	GCx, GCy, GR / 1.5, 5 * Scale, 2, 0x00FF00, 0x00FF00 );
	
	// グラフ
	
	// 走行軌跡
	Vsd.DrawMap(
		5, 5, 155 * Scale, 115 * Scale, ALIGN_HCENTER | ALIGN_VCENTER,
		2, 4 * Scale, ( Vsd.FrameCnt & 8 ) ? 0x00FF00 : 0xFF000000,
		0x00FF00, 0x00FF00, 0x00FF00
	);
	
	// グラフ
	Vsd.DrawGraph(
		620 * Scale, 5, Vsd.Width - 250 * Scale, 120 * Scale - 5, FontS,
		Vsd.MaxTacho > 0 ?
			GRAPH_SPEED | GRAPH_TACHO | GRAPH_GX | GRAPH_GY :
			GRAPH_SPEED |               GRAPH_GX | GRAPH_GY
	);
}
