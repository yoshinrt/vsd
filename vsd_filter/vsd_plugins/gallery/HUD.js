//*** 初期化処理 ************************************************************

function Initialize(){
	if( Vsd.Revision < 724 ){
		MessageBox( "このスキンはリビジョンr724 以降が必要です" );
	}
	
	// 使用する画像・フォントの宣言
	Scale	= Vsd.Height / 720;
	Color	= 0x00FF00;
	
	FontS = new Font( "Arial", 24 * Scale, FONT_FIXED );
	FontM = new Font( "MS ゴシック", 30 * Scale, FONT_FIXED );
	FontM_Outline = new Font( "Impact", 36 * Scale, FONT_FIXED | FONT_OUTLINE );
	
	// スピード表示用パラメータ
	SpeedMeterParam = {
		Flag:		ALIGN_VCENTER | ALIGN_RIGHT | LMS_VERTICAL,
		X:			120 * Scale,
		Y:			Vsd.Height - 100 * Scale,
		Width:		-( Vsd.Height - 200 * Scale ),
		Line1Len:	-20 * Scale,
		Line1Width:	2,
		Line1Color:	Color,
		Line1Cnt:	8,
		Line2Len:	-15 * Scale,
		Line2Width:	1,
		Line2Color:	Color,
		Line2Cnt:	5,
		MinVal:		0,
		MaxVal:		0,
		NumPos:		-20 * Scale,
		FontColor:	Color,
		Font:		FontS,
	};
	
	SpeedBox = [
		SpeedMeterParam.X - 100 * Scale,                            Vsd.Height / 2 - 16 * Scale,
		SpeedMeterParam.X -  16 * Scale + SpeedMeterParam.Line2Len, Vsd.Height / 2 - 16 * Scale,
		SpeedMeterParam.X               + SpeedMeterParam.Line2Len, Vsd.Height / 2,
		SpeedMeterParam.X -  16 * Scale + SpeedMeterParam.Line2Len, Vsd.Height / 2 + 16 * Scale,
		SpeedMeterParam.X - 100 * Scale,                            Vsd.Height / 2 + 16 * Scale,
	];
	
	// タコ表示用パラメータ
	TachoMeterParam = {
		Flag:		ALIGN_VCENTER | ALIGN_LEFT | LMS_VERTICAL,
		X:			Vsd.Width - 120 * Scale,
		Y:			Vsd.Height - 100 * Scale,
		Width:		-( Vsd.Height - 200 * Scale ),
		Line1Len:	20 * Scale,
		Line1Width:	2,
		Line1Color:	Color,
		Line1Cnt:	8,
		Line2Len:	15 * Scale,
		Line2Width:	1,
		Line2Color:	Color,
		Line2Cnt:	5,
		MinVal:		0,
		MaxVal:		0,
		NumPos:		20 * Scale,
		FontColor:	Color,
		Font:		FontS,
	};
	
	TachoBox = [
		TachoMeterParam.X + 100 * Scale,                            Vsd.Height / 2 - 16 * Scale,
		TachoMeterParam.X +  16 * Scale + TachoMeterParam.Line2Len, Vsd.Height / 2 - 16 * Scale,
		TachoMeterParam.X               + TachoMeterParam.Line2Len, Vsd.Height / 2,
		TachoMeterParam.X +  16 * Scale + TachoMeterParam.Line2Len, Vsd.Height / 2 + 16 * Scale,
		TachoMeterParam.X + 100 * Scale,                            Vsd.Height / 2 + 16 * Scale,
	];
	
	// 方位表示用パラメータ
	DirectionMeterParam = {
		Flag:		ALIGN_BOTTOM | ALIGN_HCENTER | LMS_HORIZONTAL,
		X:			Vsd.Width / 4,
		Y:			80 * Scale,
		Width:		Vsd.Width / 2,
		Line1Len:	-20 * Scale,
		Line1Width:	2,
		Line1Color:	Color,
		Line1Cnt:	8,
		Line2Len:	-15 * Scale,
		Line2Width:	1,
		Line2Color:	Color,
		Line2Cnt:	5,
		MinVal:		0,
		MaxVal:		0,
		NumPos:		-20 * Scale,
		FontColor:	Color,
		Font:		FontS,
	};
	
	DirectionBox = [
		Vsd.Width / 2,              DirectionMeterParam.Y + DirectionMeterParam.Line2Len,
		Vsd.Width / 2 +  8 * Scale, DirectionMeterParam.Y + DirectionMeterParam.Line2Len -  8 * Scale,
		Vsd.Width / 2 + 30 * Scale, DirectionMeterParam.Y + DirectionMeterParam.Line2Len -  8 * Scale,
		Vsd.Width / 2 + 30 * Scale, DirectionMeterParam.Y + DirectionMeterParam.Line2Len - 40 * Scale,
		Vsd.Width / 2 - 30 * Scale, DirectionMeterParam.Y + DirectionMeterParam.Line2Len - 40 * Scale,
		Vsd.Width / 2 - 30 * Scale, DirectionMeterParam.Y + DirectionMeterParam.Line2Len -  8 * Scale,
		Vsd.Width / 2 -  8 * Scale, DirectionMeterParam.Y + DirectionMeterParam.Line2Len -  8 * Scale,
	];
	
	// W マーク
	WhiskyMark = [
		Vsd.Width / 2 - 40, Vsd.Height / 2,
		Vsd.Width / 2 - 20, Vsd.Height / 2,
		Vsd.Width / 2 - 10, Vsd.Height / 2 + 12,
		Vsd.Width / 2,      Vsd.Height / 2,
		Vsd.Width / 2 + 10, Vsd.Height / 2 + 12,
		Vsd.Width / 2 + 20, Vsd.Height / 2,
		Vsd.Width / 2 + 40, Vsd.Height / 2,
	];
}

//*** メーター描画処理 ******************************************************

function Draw(){
	// ロール・ピッチ角表示
	var Pitch = 5 * Vsd.Gy;
	var Roll  = -5 * Vsd.Gx;
	
	var R = Vsd.Height * 3 / 8;
	Vsd.DrawCircle( Vsd.Width / 2, Vsd.Height / 2, R, Color );
	
	for( var i = ~~(( Pitch - 20 ) / 10 ) * 10; i <= ~~(( Pitch + 20 ) / 10 ) * 10; i += 10 ){
		var P0 = [ -R * ( i ? 0.6 : 2 ), ( Pitch - i ) * R / 20 ];
		var P1 = [ -P0[ 0 ], P0[ 1 ]];
		var P2 = [  P0[ 0 ] * 1.2, P0[ 1 ]];
		var P3 = [ -P0[ 0 ] * 1.2, P0[ 1 ]];
		
		RotatePoint( P0, Roll );
		RotatePoint( P1, Roll );
		
		Vsd.DrawLine(
			Vsd.Width / 2 + P0[ 0 ], Vsd.Height / 2 + P0[ 1 ],
			Vsd.Width / 2 + P1[ 0 ], Vsd.Height / 2 + P1[ 1 ],
			Color, 1, i >= 0 ? 0xFFFFFFFF : 0xFFFF0000
		);
		
		if( i ){
			RotatePoint( P2, Roll );
			RotatePoint( P3, Roll );
			Vsd.DrawTextAlign(
				Vsd.Width / 2 + P2[ 0 ], Vsd.Height / 2 + P2[ 1 ],
				ALIGN_VCENTER | ALIGN_HCENTER, i, FontS, Color
			);
			Vsd.DrawTextAlign(
				Vsd.Width / 2 + P3[ 0 ], Vsd.Height / 2 + P3[ 1 ],
				ALIGN_VCENTER | ALIGN_HCENTER, i, FontS, Color
			);
		}
	}
	
	function RotatePoint( Points, angle ){
		angle *= Math.PI / 180;
		var cos = Math.cos( angle );
		var sin = Math.sin( angle );
		
		var x = Points[ 0 ] * cos - Points[ 1 ] * sin;
		var y = Points[ 0 ] * sin + Points[ 1 ] * cos;
		Points[ 0 ] = x;
		Points[ 1 ] = y;
	}
	
	Vsd.DrawPolygon( WhiskyMark, Color, DRAW_NOCLOSE );
	
	// スピード表示
	SpeedMeterParam.MinVal = Vsd.Speed - 50;
	SpeedMeterParam.MaxVal = Vsd.Speed + 50;
	Vsd.DrawLinearMeterScale( SpeedMeterParam );
	Vsd.DrawPolygon( SpeedBox, 0x80000000, DRAW_FILL );
	Vsd.DrawPolygon( SpeedBox, Color );
	Vsd.DrawTextAlign(
		SpeedMeterParam.X + SpeedMeterParam.Line2Len - 16 * Scale, Vsd.Height / 2,
		ALIGN_RIGHT | ALIGN_VCENTER, ~~Vsd.Speed, FontM, Color
	);
	
	// タコ表示
	TachoMeterParam.MinVal = Vsd.Tacho - 2000;
	TachoMeterParam.MaxVal = Vsd.Tacho + 2000;
	Vsd.DrawLinearMeterScale( TachoMeterParam );
	Vsd.DrawPolygon( TachoBox, 0x80000000, DRAW_FILL );
	Vsd.DrawPolygon( TachoBox, Color );
	Vsd.DrawTextAlign(
		TachoMeterParam.X + 100 * Scale, Vsd.Height / 2,
		ALIGN_RIGHT | ALIGN_VCENTER, ~~Vsd.Tacho, FontM, Color
	);
	
	// 方位表示
	DirectionMeterParam.MinVal = Vsd.Direction - 90;
	DirectionMeterParam.MaxVal = Vsd.Direction + 90;
	Vsd.DrawLinearMeterScale( DirectionMeterParam );
	Vsd.DrawPolygon( DirectionBox, 0x80000000, DRAW_FILL );
	Vsd.DrawPolygon( DirectionBox, Color );
	Vsd.DrawTextAlign(
		Vsd.Width / 2, DirectionMeterParam.Y - 23 * Scale,
		ALIGN_HCENTER | ALIGN_BOTTOM, ~~Vsd.Direction, FontM, Color
	);
	
	// 走行軌跡表示
	var MapSize = 200;
	
	Vsd.DrawRect(
		8 * Scale, Vsd.Height - ( MapSize + 8 ) * Scale,
		( MapSize + 8 ) * Scale, Vsd.Height - 8 * Scale,
		0x80000000, DRAW_FILL
	);
	
	Vsd.DrawRect(
		8 * Scale, Vsd.Height - ( MapSize + 8 ) * Scale,
		( MapSize + 8 ) * Scale, Vsd.Height - 8 * Scale,
		Color
	);
	
	Vsd.DrawMap(
		8 * Scale, Vsd.Height - ( MapSize + 8 ) * Scale,
		( MapSize + 8 ) * Scale, Vsd.Height - 8 * Scale,
		ALIGN_HCENTER | ALIGN_VCENTER,
		1, 4 * Scale, Vsd.FrameCnt & 4 ? Color : 0xFF000000, Color, Color, Color
	);
	
	// 走行軌跡表示
	Vsd.DrawRect(
		Vsd.Width - ( MapSize + 8 ) * Scale, Vsd.Height - ( MapSize + 8 ) * Scale,
		Vsd.Width -   8 * Scale, Vsd.Height -   8 * Scale,
		0x80000000, DRAW_FILL
	);
	
	Vsd.DrawRect(
		Vsd.Width - ( MapSize + 8 ) * Scale, Vsd.Height - ( MapSize + 8 ) * Scale,
		Vsd.Width -   8 * Scale, Vsd.Height -   8 * Scale,
		Color
	);
	
	Vsd.DrawLine( 
		Vsd.Width - ( MapSize + 8 ) * Scale, Vsd.Height - ( MapSize + 8 ) / 2 * Scale,
		Vsd.Width -   8 * Scale, Vsd.Height - ( MapSize + 8 ) / 2 * Scale,
		Color, 1, 0xF0F0F0F0
	);
	
	Vsd.DrawLine( 
		Vsd.Width - ( MapSize + 8 ) / 2 * Scale, Vsd.Height - ( MapSize + 8 ) * Scale,
		Vsd.Width - ( MapSize + 8 ) / 2 * Scale, Vsd.Height -   8 * Scale,
		Color, 1, 0xF0F0F0F0
	);
	
	Vsd.DrawGSnake(
		Vsd.Width - ( MapSize + 8 ) / 2 * Scale, Vsd.Height - ( MapSize + 8 ) / 2 * Scale,
		80 * Scale, 4 * Scale, 1, Color, Color, 3
	);
	
	// 文字情報
	var Accel = Math.sqrt( Vsd.Gx * Vsd.Gx + Vsd.Gy * Vsd.Gy ).toFixed( 1 ) + "G";
	Vsd.DrawText(
		( MapSize + 20 ) * Scale, Vsd.Height - FontS.Height * 3,
		Accel, FontS, Color
	);
	Vsd.DrawText(
		( MapSize + 20 ) * Scale, Vsd.Height - FontS.Height * 2,
		"Thr" + ~~Vsd.Accel + "%", FontS, Color
	);
	
	// ラップタイム
	Vsd.DrawTextAlign(
		Vsd.Width - ( MapSize + 20 ) * Scale, Vsd.Height - FontS.Height * 4, ALIGN_TOP | ALIGN_RIGHT,
		"Best " + Vsd.FormatTime( Vsd.BestLapTime ), FontS, Color
	);
	Vsd.DrawTextAlign(
		Vsd.Width - ( MapSize + 20 ) * Scale, Vsd.Height - FontS.Height * 3, ALIGN_TOP | ALIGN_RIGHT,
		"Lap" + Vsd.LapCnt + "/" + Vsd.MaxLapCnt + " " + Vsd.FormatTime( Vsd.LapTime ), FontS, Color
	);
	Vsd.DrawTextAlign(
		Vsd.Width - ( MapSize + 20 ) * Scale, Vsd.Height - FontS.Height * 2, ALIGN_TOP | ALIGN_RIGHT,
		( Vsd.DiffTime > 0 ? '+' : '' ) + Vsd.FormatTime( Vsd.DiffTime ),
		FontS, Color
	);
}
