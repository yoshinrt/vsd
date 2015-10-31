//*** 初期化処理 ************************************************************

function Initialize(){
	// 使用する画像・フォントの宣言
	Scale = Vsd.Height / 720;
	
	font = new Font( "ＭＳ　ゴシック", 20 * Scale, FONT_OUTLINE | FONT_NOANTIALIAS );
}

//*** メーター描画処理 ******************************************************

X = 0;
Y = 0;
Align = 0;

function print( msg, param ){
	var Color = 0xFFFFFF;
	
	if( typeof param == 'object' ){
		if( param.x !== undefined ) X = param.x;
		if( param.y !== undefined ) Y = param.y;
		if( param.align !== undefined ) Align = param.align;
		if( param.color !== undefined ) Color = param.color;
	}
	Vsd.DrawTextAlign( X, Y, Align, msg, font, Color );
	
	Y += font.Height;
}

function Draw(){
	// リビジョン番号表示
//	Vsd.DrawTextAlign( Vsd.Width - 1, Vsd.Height - 1, ALIGN_RIGHT | ALIGN_BOTTOM,
//		"VSD for GPS rev." + Vsd.Revision, font, 0x008080
//	);
	
	// 走行軌跡
	var MapX = Log.Max.X - Log.Min.X;
	var MapY = Log.Max.Y - Log.Min.Y;
	if( MapX >= 10000 || MapY >= 10000 ){
		print(
		"走行軌跡大きさ: " + ~~( MapX / 1000 ) + "km×" + ~~( MapY / 1000 ) + "km",
		{ x: Vsd.Width - 1, y: Vsd.Height - 1, color: 0xFF8000, align: ALIGN_RIGHT | ALIGN_BOTTOM } );
	}
	
	Vsd.DrawMap(
		4, 4, Vsd.Width - 4, Vsd.Height - 4,
		ALIGN_TOP | ALIGN_LEFT | DRAW_MAP_START,
		1, 6 * Scale, 0x00FF00, 0x40FF8000, 0x40FF8000, 0x40FF8000
	);
	
	// ラップタイム
	if( Vsd.Config_lap_time ){
		Vsd.DrawLapTimeLog( Vsd.Width - 1, 0, ALIGN_RIGHT | ALIGN_TOP, 10, font );
	}
	
	// グラフ
	Vsd.DrawSyncGraph( Vsd.Width / 3, Vsd.Height / 2, Vsd.Width - 2, Vsd.Height - 2, font );
	
	// 同期文字情報
	Vsd.DrawSyncInfo( 0, Vsd.Height - 1, font, ALIGN_LEFT | ALIGN_BOTTOM );
	
	// 説明文
	if( Vsd.PrivateBuild ) return;
	Vsd.DrawRect( 0, 0, font.GetTextWidth( "画" ) * 35.5, font.Height * 12.5, 0xC0000000, DRAW_FILL );
	
	print( "●同期手順", { x: 0, y: 0, align: ALIGN_LEFT | ALIGN_TOP } );
	if( typeof( Log.Time ) === "undefined" ){
		print( "・「GPSログ」の「開く」ボタンでGPSログを開きます" );
	}else{
		print( "・この画面下のメインスライダを動かし，" );
		print( "  車両が動き出した瞬間の映像を表示します" );
		print( "・「GPSログ位置調整」スライダを動かし，スピードグラフが" );
		print( "  0km/hから動く瞬間のグラフにグラフカーソルを合わせます" );
		print( "・映像とログが少しずつずれていく場合は，映像の終わり" );
		print( "  付近でもう一度「GPSログ位置調整」スライダを調整します" );
		
	if( Vsd.MaxLapCnt ){
		print( "・ラップ分割がうまくいっていない場合は「計測地点幅」を調整してください" );
	}else if( Vsd.Config_lap_time ){
		print( "・「計測地点幅」をコース幅[m]×10 に設定します" );
		print( "・メインスライダを動かし，コントロールラインを通過した瞬間の" );
		print( "  映像を表示します" );
		print( "・「M」キーを押します" );
	}else{
		print( "・ラップタイム表示する場合は「ラップタイム表示」をONにします" );
	}
		print( "・すべての調整が終わったら「スキン」をお好みのスキンに変更してください" );
	}
}
