//*** 初期化処理 ************************************************************

function Initialize(){
	// 使用する画像・フォントの宣言
	ImgMeter = new Image( Vsd.SkinDir + "standard.png" );
	ImgG     = new Image( ImgMeter );
	
	FontS = new Font( "Impact", Vsd.Height / 30 );
	FontM = new Font( "Impact", Vsd.Height / 23 );
	FontG = new Font( "Impact", Vsd.Height / 18 );
	FontL = new Font( "Impact", Vsd.Height / 12 );
	FontM_Outline = new Font( "Impact", Vsd.Height / 20, FONT_FIXED | FONT_OUTLINE );
	
	// 動画サイズに応じてメーター画像をリサイズ
	Scale = Vsd.Height / 720;
	if( Scale != 1 ){
		ImgMeter.Resize( ImgMeter.Width * Scale, ImgMeter.Height * Scale );
	}
	
	// 座標等を予め計算しておく
	MeterX	= MeterRight ? Vsd.Width  - ImgMeter.Width : 0;
	MeterY	= Vsd.Height - ImgMeter.Height * 0.85;
	MeterR  = 150 * Scale;
	MeterR2 = 126 * Scale;
	MeterCx = MeterX + MeterR;
	MeterCy = MeterY + MeterR;
	
	// G 用画像をリサイズ
	ImgG.Resize( ImgG.Width * Scale / 2, ImgG.Height * Scale / 2 );
	
	// G メーター用の座標計算
	MeterGX	 = MeterRight ? Vsd.Width - ImgMeter.Width * 1.45 : ImgMeter.Width * 0.95;
	MeterGY	 = Vsd.Height - ImgG.Height;
	MeterGR  = MeterR / 2;
	MeterGR2 = MeterR2 / 2;
	MeterGCx = MeterGX + MeterGR;
	MeterGCy = MeterGY + MeterGR;
	
	// スピードグラフサイズ計算
	SpdX1 = MeterRight ? 8 : ImgMeter.Width * 1.6;
	SpdX2 = MeterRight ? Vsd.Width - ImgMeter.Width * 1.6 : Vsd.Width - 8;
	SpdY1 = Vsd.Height - 300 * Scale;
	SpdY2 = Vsd.Height - 8;
	
	// スピードメータ用最高速計算
	
	if( Vsd.MaxTacho > 0 ){
		MaxTacho = Math.ceil( Vsd.MaxTacho / 1000 ) * 1000;
	}else{
		MaxSpeed = Math.ceil( Vsd.MaxSpeed / 10 ) * 10;
	}
}

//*** メーター描画処理 ******************************************************

function Draw(){
	// メーター画像描画
	Vsd.PutImage( MeterX, MeterY, ImgMeter );
	
	if( Vsd.MaxTacho > 0 ){
		var MeterColor = Vsd.Tacho > REV_LIMIT && ( Vsd.FrameCnt & 0x2 ) ? 0xFF0000 : 0xFFFFFF;
		
		// タコメーター目盛り描画
		Vsd.DrawMeterScale(
			MeterCx, MeterCy, MeterR2,
			MeterR2 * 0.1,  2, MeterColor,
			MeterR2 * 0.05, 1, MeterColor,
			5, 135, 45,
			MeterR2 * 0.80,
			MaxTacho / 1000, 12, MeterColor,
			FontM
		);
		
		// ギア
		Vsd.DrawRect(
			MeterCx - MeterR * 0.09, MeterCy - MeterR * 0.48,
			MeterCx + MeterR * 0.09, MeterCy - MeterR * 0.22,
			0xFF4000, DRAW_FILL
		);
		
		Vsd.DrawTextAlign(
			MeterCx, MeterCy - MeterR * 0.35,
			ALIGN_HCENTER | ALIGN_VCENTER,
			GetGear( Vsd.Tacho / Vsd.Speed ), FontG, 0
		);
	}else{
		// スピードメーター目盛り描画
		Vsd.DrawMeterScale(
			MeterCx, MeterCy, MeterR2,
			MeterR2 * 0.1,  2, 0xFFFFFF,
			MeterR2 * 0.05, 1, 0xFFFFFF,
			5, 135, 45,
			MeterR2 * 0.78,
			MaxSpeed, 12, 0xFFFFFF,
			FontS
		);
	}
	
	// スピード数値表示
	Vsd.DrawTextAlign(
		MeterCx, MeterCy + MeterR * 0.25, 
		ALIGN_HCENTER | ALIGN_VCENTER,
		~~Vsd.Speed, FontL, 0xFFFFFF
	);
	
	Vsd.DrawTextAlign(
		MeterCx, MeterCy + MeterR * 0.5,
		ALIGN_HCENTER | ALIGN_VCENTER,
		"km/h", FontS, 0xFFFFFF
	);
	
	if( Vsd.MaxTacho > 0 ){
		// タコメーター針
		Vsd.DrawNeedle(
			MeterCx, MeterCy, MeterR2 * 0.95, MeterR2 * -0.1,
			135, 45, Vsd.Tacho / MaxTacho, 0xFF0000, 3
		);
	}else{
		// スピードメーター針
		Vsd.DrawNeedle(
			MeterCx, MeterCy, MeterR2 * 0.95, MeterR2 * -0.1,
			135, 45, Vsd.Speed / MaxSpeed, 0xFF0000, 3
		);
	}
	
	if( typeof Vsd.Gx != 'undefined' ){
		// Gメーターパネル画像描画
		Vsd.PutImage( MeterGX, MeterGY, ImgG );
		Vsd.DrawLine( MeterGCx - MeterGR2, MeterGCy, MeterGCx + MeterGR2, MeterGCy, 0x802000 );
		Vsd.DrawLine( MeterGCx, MeterGCy - MeterGR2, MeterGCx, MeterGCy + MeterGR2, 0x802000 );
		Vsd.DrawCircle( MeterGCx, MeterGCy, MeterGR2 / 3,     0x802000 );
		Vsd.DrawCircle( MeterGCx, MeterGCy, MeterGR2 / 3 * 2, 0x802000 );
		
		// G 数値
		var Accel = Math.sqrt( Vsd.Gx * Vsd.Gx + Vsd.Gy * Vsd.Gy ).toFixed( 1 ) + "G";
		Vsd.DrawTextAlign(
			MeterGCx, MeterGCy + MeterR / 2, ALIGN_HCENTER | ALIGN_BOTTOM,
			Accel, FontS, 0xFFFFFF
		);
		
		// G スネーク
		Vsd.DrawGSnake(	MeterGCx, MeterGCy, MeterGR2 / 1.5, 5 * Scale, 2, 0xFF4000, 0x802000 );
	}
	
	// 走行軌跡
	Vsd.DrawMap(
		8 * Scale, 8 * Scale, 500 * Scale, 300 * Scale,
		ALIGN_TOP | ALIGN_LEFT,
		2 * Scale, 5 * Scale, 0xFF0000, 0xFFFF00, 0x00FF00, 0xFF0000
	);
	
	// ラップタイム
	Vsd.DrawLapTime( Vsd.Width - 1, 0, ALIGN_TOP | ALIGN_RIGHT, FontM_Outline );
	
	// グラフ
	Vsd.DrawGraph(
		SpdX1, SpdY1, SpdX2, SpdY2, FontM,
		Vsd.MaxTacho > 0 ?
			GRAPH_SPEED | GRAPH_TACHO | GRAPH_GX | GRAPH_GY | GRAPH_VTILE :
			GRAPH_SPEED |               GRAPH_GX | GRAPH_GY | GRAPH_VTILE
	);
}
