//*** 初期化処理 ************************************************************

function Initialize(){
	// 使用する画像・フォントの宣言
	Scale = Vsd.Height / 720;
	
	font = new Font( "ＭＳ　ゴシック", 28 * Scale, FONT_OUTLINE | FONT_NOANTIALIAS );
}

//*** メーター描画処理 ******************************************************

X = 0;
Y = 0;
Align = 0;

function print( msg, param ){
		if( typeof param == 'object' ){
		if( param.x !== undefined ) X = param.x;
		if( param.y !== undefined ) Y = param.y;
		if( param.align !== undefined ) Align = param.align;
	}
	Vsd.DrawTextAlign( X, Y, Align, msg, font );
	
	Y += font.Height;
}

function Draw(){
	// リビジョン番号表示
//	Vsd.DrawTextAlign( Vsd.Width - 1, Vsd.Height - 1, ALIGN_RIGHT | ALIGN_BOTTOM,
//		"VSD for GPS rev." + Vsd.Revision, font, 0x008080
//	);
	
	// 走行軌跡
	Vsd.DrawMap(
		4, 4, Vsd.Width / 1.5, Vsd.Height / 1.5,
		ALIGN_TOP | ALIGN_LEFT | DRAW_MAP_START,
		3 * Scale, 6 * Scale, 0xFF0000, 0xFFFF00, 0x00FF00, 0xFF0000
	);
	
	// ラップタイム
	if( !Vsd.Config_lap_time ){
		print( "ラップタイム非表示", { x: Vsd.Width - 1, y: 0, align: ALIGN_RIGHT | ALIGN_TOP } );
	}else if( Vsd.MaxLapCnt == 0 ){
		print( "スタート地点の映像を表示し", { x: Vsd.Width - 1, y: 0, align: ALIGN_RIGHT | ALIGN_TOP } );
		print( "Mキーでラップタイム計測" );
	}else{
		Vsd.DrawLapTimeLog( Vsd.Width - 1, 0, ALIGN_RIGHT | ALIGN_TOP, 10, font );
	}
	
	// グラフ
	Vsd.DrawSyncGraph( Vsd.Width / 3, Vsd.Height / 2, Vsd.Width - 2, Vsd.Height - 2, font );
	
	// 同期文字情報
	Vsd.DrawSyncInfo( 0, Vsd.Height - 1, font, ALIGN_LEFT | ALIGN_BOTTOM );
}
