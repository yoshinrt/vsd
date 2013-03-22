//*** 初期化処理 ************************************************************

function Initialize(){
	// 使用する画像・フォントの宣言
	Scale = Vsd.Height / 720;
	
	FontS = new Font( "Impact", 24 * Scale, FONT_FIXED );
	FontM = new Font( "Impact", 36 * Scale, FONT_FIXED );
	FontM_Outline = new Font( "Impact", 36 * Scale, FONT_FIXED | FONT_OUTLINE );
	
	// 座標等を予め計算しておく
	MeterR = 150 * Scale;
	MeterX	= MeterRight ? Vsd.Width  - MeterR * 2 : 0;
	MeterY	= Vsd.Height - MeterR * 2;
	MeterCx = MeterX + MeterR;
	MeterCy = MeterY + MeterR;
	
	// スピードグラフサイズ計算
	SpdX1 = MeterRight ? 8 : 300 * Scale * 1.1;
	SpdX2 = MeterRight ? Vsd.Width - 300 * Scale * 1.1 : Vsd.Width - 8;
	SpdY1 = Vsd.Height - 300 * Scale;
	SpdY2 = Vsd.Height - 8;
	
	// スピードメータ用最高速計算
	if( Vsd.MaxTacho > 0 ){
		MaxTacho = Math.ceil( Vsd.MaxTacho / 1000 ) * 1000;
	}else{
		MaxSpeed = Math.ceil( Vsd.MaxSpeed / 10 ) * 10;
	}
	
	// グラフ用パラメータ生成
	GraphParam = [
		"Speed",	"%.0f km/h",	0x00FFFF,
		"Tacho",	"%.0f rpm",		0xFF4000,
	//	"Accel",	"%.0f %%",		0x00FF00,
		"Gx",		"%.2f G[lon]",	0x00FF00,
		"Gy",		"%.2f G[lat]",	0xFF00FF,
	];
	Vsd.MakeGraphParam( GraphParam );
}

//*** メーター描画処理 ******************************************************

function Draw(){
	Vsd.DrawCircle( MeterCx, MeterCy, MeterR, 0x80404040, DRAW_FILL );
	
	if( Vsd.MaxTacho > 0 ){
		var MeterColor = Vsd.Tacho > REV_LIMIT && ( Vsd.FrameCnt & 0x2 ) ? 0xFF0000 : 0xFFFFFF;
		
		// タコメーター目盛り描画
		Vsd.DrawMeterScale(
			MeterCx, MeterCy, MeterR,
			MeterR * 0.1,  2, MeterColor,
			MeterR * 0.05, 1, MeterColor,
			5, 135, 45,
			MeterR * 0.80,
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
			GetGear( Vsd.Tacho / Vsd.Speed ), FontM, 0
		);
	}else{
		// スピードメーター目盛り描画
		Vsd.DrawMeterScale(
			MeterCx, MeterCy, MeterR,
			MeterR * 0.1,  2, 0xFFFFFF,
			MeterR * 0.05, 1, 0xFFFFFF,
			2, 135, 45,
			MeterR * 0.75,
			MaxSpeed, 12, 0xFFFFFF,
			FontM
		);
	}
	
	// G スネーク・数値
	if( typeof Vsd.Gx != 'undefined' ){
		Vsd.DrawGSnake(	MeterCx, MeterCy, MeterR / 1.5, 5, 2, 0x00FF00, 0x008000 );
		var Accel = Math.sqrt( Vsd.Gx * Vsd.Gx + Vsd.Gy * Vsd.Gy ).toFixed( 1 ) + "G";
		Vsd.DrawText(
			MeterCx - FontS.GetTextWidth( Accel ) / 2, MeterCy + MeterR / 2 - FontS.Height,
			Accel, FontS, 0xFFFFFF
		);
	}
	
	// スピード数値表示
	var Speed = ~~Vsd.Speed;
	if     ( Speed <  10 ) Speed = "  " + Speed;
	else if( Speed < 100 ) Speed = " "  + Speed;
	
	Vsd.DrawText(
		MeterCx - FontM.GetTextWidth( Speed ) / 2, MeterCy + MeterR / 2,
		Speed, FontM, 0xFFFFFF
	);
	
	if( Vsd.MaxTacho > 0 ){
		// タコメーター針
		Vsd.DrawNeedle(
			MeterCx, MeterCy, MeterR * 0.95, 0,
			135, 45, Vsd.Tacho / MaxTacho, 0xFF0000, 3
		);
	}else{
		// スピードメーター針
		Vsd.DrawNeedle(
			MeterCx, MeterCy, MeterR * 0.95, 0,
			135, 45, Vsd.Speed / MaxSpeed, 0xFF0000, 3
		);
	}
	
	// アクセル
	if( Vsd.MaxAccel > 0 ){
		Vsd.DrawLine(
			MeterCx - MeterR * 0.45,
			MeterCy + MeterR * 0.75,
			MeterCx + MeterR * 0.45,
			MeterCy + MeterR * 0.75,
			0x404040, 4 * Scale
		);
		Vsd.DrawLine(
			MeterCx - MeterR * 0.45,
			MeterCy + MeterR * 0.75,
			MeterCx + MeterR * 0.45 * ( 2 * Vsd.Accel / Vsd.MaxAccel - 1 ),
			MeterCy + MeterR * 0.75,
			0x00C0C0, 4 * Scale
		);
	}
	
	// 走行軌跡
	Vsd.DrawMap(
		8 * Scale, 8 * Scale, 500 * Scale, 300 * Scale,
		ALIGN_TOP | ALIGN_LEFT,
		3 * Scale, 6 * Scale, 0xFF0000, 0xFFFF00, 0x00FF00, 0xFF0000
	);
	
	// ラップタイム
	Vsd.DrawLapTime( Vsd.Width - 1, 0, ALIGN_TOP | ALIGN_RIGHT, FontM_Outline );
	
	// グラフ
	Vsd.DrawGraph( SpdX1, SpdY1, SpdX2, SpdY2, FontM, GRAPH_VTILE, GraphParam );
}
