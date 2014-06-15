//*** 初期化処理 ************************************************************

function Initialize(){
	// 使用する画像・フォントの宣言
	ImgMeter = new Image( Vsd.SkinDir + "bluearc.png" );
	
	Scale = Vsd.Height / 720;
	
	FontS = new Font( "Impact", 24 * Scale );
	FontM = new Font( "Impact", 31 * Scale );
	FontL = new Font( "Impact", 60 * Scale );
	FontM_Outline = new Font( "Impact", 36 * Scale, FONT_FIXED | FONT_OUTLINE );
	
	// 動画サイズに応じてメーター画像をリサイズ
	if( Scale != 1 ){
		ImgMeter.Resize( ImgMeter.Width * Scale, ImgMeter.Height * Scale );
	}
	
	// 座標等を予め計算しておく
	MeterX	= MeterRight ? Vsd.Width  - ImgMeter.Width : 0;
	MeterY	= Vsd.Height - ImgMeter.Height;
	MeterR  = 150 * Scale;
	
	MeterParam = {
		X:			MeterX + MeterR,
		Y:			MeterY + MeterR,
		R:			120 * Scale,
		Line1Len:	10 * Scale,
		Line1Width:	2,
		Line1Color:	0xFFFFFF,
		Line1Cnt:	12,
		Line2Len:	MeterR * 0.05,
		Line2Width:	1,
		Line2Color:	0xFFFFFF,
		Line2Cnt:	1,
		NumR:		135 * Scale,
		FontColor:	0x808080,
		Font:		FontS,
		MinAngle:	135,
		MaxAngle:	45,
		MinVal:		0,
		MaxVal:		0,	// 暫定
	};
	
	// G メーター用の座標計算
	MeterGX	 = MeterRight ? Vsd.Width - ImgMeter.Width * 1.46 : ImgMeter.Width * 0.96;
	MeterGR  = 150 / 2 * Scale;
	MeterGY	 = Vsd.Height - MeterGR * 2;
	MeterGCx = MeterGX + MeterGR;
	MeterGCy = MeterGY + MeterGR;
	
	// スピードグラフサイズ計算
	SpdX1 = MeterRight ? 8 : ImgMeter.Width * 1.6;
	SpdX2 = MeterRight ? Vsd.Width - ImgMeter.Width * 1.6 : Vsd.Width - 8;
	SpdY1 = Vsd.Height - 300 * Scale;
	SpdY2 = Vsd.Height - 8;
	
	// スピードメータ用最高速計算
	if( Vsd.MaxTacho > 0 ){
		MaxTacho			= Math.ceil( Vsd.MaxTacho / 1000 ) * 1000;
		MeterParam.MaxVal	= ~~( MaxTacho / 1000 );
	}else{
		MaxSpeed			= Math.ceil( Vsd.MaxSpeed / 10 ) * 10;
		MeterParam.MaxVal	= ~~( MaxSpeed / 10 );
	}
	
	// グラフ用パラメータ生成
	GraphParam = [
		"Speed",	"%.0f km/h",	0x00FFFF,
		"Tacho",	"%.0f rpm",		0xFF4000,
		"Accel",	"%.0f %%",		0xFFFF00,
		"Gx",		"%.2f G[lat]",	0x00FF00,
		"Gy",		"%.2f G[lon]",	0xFF00FF,
	];
}

//*** メーター描画処理 ******************************************************

function Draw(){
	Vsd.DrawCircle( MeterParam.X, MeterParam.Y, 150 * Scale, 0x80000000, DRAW_FILL );
	
	if( Vsd.MaxTacho > 0 ){
		if( Vsd.Tacho >= 1 ){
			var GauageColor = Vsd.Tacho > REV_LIMIT && ( Vsd.FrameCnt & 0x2 ) ? 0xFF4040 : 0x804040FF;
			
			// タコメーター針
			Vsd.DrawArc(
				MeterParam.X, MeterParam.Y, 120 * Scale, 120 * Scale, 80 * Scale, 80 * Scale,
				135, 135 + 270 * Vsd.Tacho / MaxTacho, GauageColor
			);
		}
	}else if( Vsd.Speed > 0 ){
		// スピードメーター針
		Vsd.DrawArc(
			MeterParam.X, MeterParam.Y, 120 * Scale, 120 * Scale, 80 * Scale, 80 * Scale,
			135, 135 + 270 * Vsd.Speed / MaxSpeed, 0x804040FF
		);
	}
	
	// スロットル・ブレーキ
	//   データがないときは縦 G を表示
	if( Vsd.MaxAccel > 0 ){
		Vsd.DrawRect(
			MeterParam.X - 70 * Scale,
			MeterParam.Y + 80 * Scale,
			MeterParam.X + ( -70 + 140 * Vsd.Accel / Vsd.MaxAccel ) * Scale,
			MeterParam.Y + 100 * Scale - 1,
			0x00FF00, DRAW_FILL
		);
	}else if( Vsd.Gy >= 0 ){
		Vsd.DrawRect(
			MeterParam.X - 70 * Scale,
			MeterParam.Y + 80 * Scale,
			MeterParam.X + ( -70 + 140 * ( Vsd.Gy < Vsd.MaxGy ? Vsd.Gy : Vsd.MaxGy ) / Vsd.MaxGy ) * Scale,
			MeterParam.Y + 100 * Scale - 1,
			0x00FF00, DRAW_FILL
		);
	}
	
	if( Vsd.MaxBrake > 0 ){
		Vsd.DrawRect(
			MeterParam.X - 70 * Scale,
			MeterParam.Y + 100 * Scale,
			MeterParam.X + ( -70 + 140 * Vsd.Brake / Vsd.MaxBrake ) * Scale,
			MeterParam.Y + 120 * Scale - 1,
			0xFF0000, DRAW_FILL
		);
	}else if( Vsd.Gy < 0 ){
		Vsd.DrawRect(
			MeterParam.X - 70 * Scale,
			MeterParam.Y + 100 * Scale,
			MeterParam.X + ( -70 + 140 * ( Vsd.Gy > Vsd.MinGy ? Vsd.Gy : Vsd.MinGy ) / Vsd.MinGy ) * Scale,
			MeterParam.Y + 120 * Scale - 1,
			0xFF0000, DRAW_FILL
		);
	}
	
	// メーター画像描画
	Vsd.PutImage( MeterX, MeterY, ImgMeter );
	
	if( Vsd.MaxTacho > 0 ){
		
		// タコメーター目盛り描画
		Vsd.DrawRoundMeterScale( MeterParam );
		
		// ギア
		Vsd.DrawTextAlign(
			MeterParam.X, MeterParam.Y + MeterR * 0.25,
			ALIGN_HCENTER | ALIGN_VCENTER,
			GetGear( Vsd.Tacho / Vsd.Speed ), FontL, 0xFFFFFF
		);
	}else{
		// スピードメーター目盛り描画
		Vsd.DrawRoundMeterScale( MeterParam );
	}
	
	// スピード数値表示
	Vsd.DrawTextAlign(
		MeterParam.X, MeterParam.Y - MeterR * 0.35, 
		ALIGN_HCENTER | ALIGN_VCENTER,
		~~Vsd.Speed, FontL, 0xFFFFFF
	);
	
	Vsd.DrawTextAlign(
		MeterParam.X, MeterParam.Y - MeterR * 0.1,
		ALIGN_HCENTER | ALIGN_VCENTER,
		"km/h", FontS, 0xFFFFFF
	);
	
	if( Vsd.Gx !== undefined ){
		// Gメーターパネル画像描画
		Vsd.DrawCircle( MeterGCx, MeterGCy, MeterGR, 0x80000000, DRAW_FILL );
		Vsd.DrawLine( MeterGCx - MeterGR, MeterGCy, MeterGCx + MeterGR, MeterGCy, 0x606060 );
		Vsd.DrawLine( MeterGCx, MeterGCy - MeterGR, MeterGCx, MeterGCy + MeterGR, 0x606060 );
		Vsd.DrawCircle( MeterGCx, MeterGCy, MeterGR / 3,     0x606060 );
		Vsd.DrawCircle( MeterGCx, MeterGCy, MeterGR / 3 * 2, 0x606060 );
		
		// G 数値
		var Accel = Math.sqrt( Vsd.Gx * Vsd.Gx + Vsd.Gy * Vsd.Gy ).toFixed( 1 ) + "G";
		Vsd.DrawTextAlign(
			MeterGCx, MeterGCy + MeterR / 2, ALIGN_HCENTER | ALIGN_BOTTOM,
			Accel, FontS, 0xFFFFFF
		);
		
		// G スネーク
		Vsd.DrawGSnake(	MeterGCx, MeterGCy, MeterGR / 1.5, 5 * Scale, 2, 0xFF4000, 0x802000 );
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
