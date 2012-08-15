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
BackImg.Resize( Vsd.Width, 120 * Scale );

//*** メーター描画処理 ******************************************************

var Color = 0x00FF00;

function Draw(){
	// 背景
	//Vsd.DrawRect( 0, 0, Vsd.Width - 1, 120, 0x80006000, DRAW_FILL );
	Vsd.PutImage( 0, 0, BackImg );
	
	// スピード
	Vsd.DrawTextAlign(
		Vsd.Width - 10, 100 * Scale, ALIGN_RIGHT | ALIGN_BOTTOM,
		"km/h", FontM, Color
	);
	Vsd.DrawTextAlign(
		Vsd.Width - 100 * Scale, 60 * Scale, ALIGN_RIGHT | ALIGN_VCENTER,
		~~Vsd.Speed, FontL, Color
	);
	
	// ラップタイム
	Vsd.DrawTextAlign( 170 * Scale, 20 * Scale, ALIGN_VCENTER, "Best:", FontM, Color );
	Vsd.DrawTextAlign(
		480 * Scale, 20 * Scale, ALIGN_VCENTER | ALIGN_RIGHT,
		Vsd.FormatTime( Vsd.BestLapTime ), FontM, Color
	);
	Vsd.DrawTextAlign(
		170 * Scale, 60 * Scale, ALIGN_VCENTER,
		"Lap" + Vsd.LapCnt + "/" + Vsd.MaxLapCnt, FontM, Color
	);
	Vsd.DrawTextAlign(
		480 * Scale, 60 * Scale, ALIGN_VCENTER | ALIGN_RIGHT,
		Vsd.FormatTime( Vsd.LapTime ), FontM, Color
	);
	Vsd.DrawTextAlign(
		170 * Scale, 100 * Scale, ALIGN_VCENTER,
		"Diff:", FontM, Color
	);
	Vsd.DrawTextAlign(
		480 * Scale, 100 * Scale, ALIGN_VCENTER | ALIGN_RIGHT,
		Vsd.FormatTime( Vsd.DiffTime ), FontM, Color
	);
	
	// G メータ
	Vsd.DrawCircle( GCx, GCy, GR, Color );
	Vsd.DrawCircle( GCx, GCy, GR * 2 / 3, Color );
	Vsd.DrawCircle( GCx, GCy, GR / 3, Color );
	Vsd.DrawLine( GCx - GR, GCy, GCx + GR, GCy, Color );
	Vsd.DrawLine( GCx, GCy - GR, GCx, GCy + GR, Color );
	Vsd.DrawGSnake(	GCx, GCy, GR / 1.5, 5 * Scale, 2, Color, Color );
	
	// グラフ
	
	// 走行軌跡
	Vsd.DrawMap(
		5, 5, 155 * Scale, 115 * Scale, ALIGN_HCENTER | ALIGN_VCENTER,
		2, 4 * Scale, ( Vsd.FrameCnt & 8 ) ? Color : 0xFF000000,
		Color, Color, Color
	);
	
	// グラフ
	Vsd.DrawGraph(
		620 * Scale, 5, Vsd.Width - 250 * Scale, 120 * Scale - 5, FontS,
		Vsd.MaxTacho > 0 ?
			GRAPH_SPEED | GRAPH_TACHO | GRAPH_GX | GRAPH_GY :
			GRAPH_SPEED |               GRAPH_GX | GRAPH_GY
	);
}
