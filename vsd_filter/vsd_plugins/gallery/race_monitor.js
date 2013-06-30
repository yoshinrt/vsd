//*** 初期化処理 ************************************************************

function Initialize(){
	Scale = Vsd.Width / 1280;
	
	// 使用する画像・フォントの宣言
	FontSOL = new Font( "Impact", 30 * Scale, FONT_FIXED | FONT_OUTLINE );
	FontS = new Font( "Impact", 36 * Scale, FONT_FIXED | FONT_OUTLINE );
	FontM = new Font( "Arial Black", 42 * Scale, FONT_OUTLINE );
	FontL = new Font( "Arial Black", 90 * Scale, FONT_OUTLINE );
}

//*** メーター描画処理 ******************************************************

function Draw(){
	// スピード
	Vsd.DrawTextAlign(
		Vsd.Width, Vsd.Height, ALIGN_RIGHT | ALIGN_BOTTOM,
		"km/h", FontM, 0xFF6030
	);
	Vsd.DrawTextAlign(
		Vsd.Width - 90 * Scale, Vsd.Height, ALIGN_RIGHT | ALIGN_BOTTOM,
		~~Vsd.Speed, FontL, 0xFF6030
	);
	
	// 走行軌跡
	Vsd.DrawMapPosition(
		8, 8, Vsd.Width - 8, Vsd.Height - 8,
		ALIGN_VCENTER | ALIGN_HCENTER,
		4 * Scale, 0x00E0FF, FontSOL
	);
	
	Vsd.DrawRaceLapTime( 0, 0, 0, 10, FontS, 0xFFE080 );
}
