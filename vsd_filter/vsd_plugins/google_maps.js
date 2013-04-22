//*** 初期化処理 ************************************************************

function Initialize(){
	Scale = Vsd.Height / 720;
	
	//////////////////////////////////////////////////////////////////////////
	/// ↓↓↓↓↓Google Maps の設定 ここから↓↓↓↓↓ //////////////////////
	//////////////////////////////////////////////////////////////////////////
	
	// 設定を行うためには，以下の設定値を直接書き換えてください．
	
	// ★補足説明
	// このスキンは Google Maps にアクセスし地図データを得ています．
	// Google Maps にアクセスするためには「API キー」が必要です．
	// キーは無料で取得出来ます．キー取得方法は
	// https://developers.google.com/maps/documentation/staticmaps/?hl=ja#api_key
	// を参照してください．
	// また，マップデータの取得は Google によって
	// 1日あたり 25,000 枚に制限されています．
	
	GoogleMapsParam = {
		// Google Maps の API キーを指定します．
		// 例: APIKey: "ABCDEFGHIJKLMNOPQRSTUVWXYZ",
		APIKey: "AIzaSyABCDEFGHIJKLMNOPQRSTUVWXYZabcdefg",
		
		// ズームレベルを 1～21 で指定します
		Zoom: 14,
		
		// 地図タイプ
		// roadmap:地図  satellite:航空写真  terrain:地形図  hybrid:航空写真
		Maptype: "roadmap",
		
		// 地図表示位置，サイズ
		X:		0,
		Y:		0,
		Width:	400 * Scale,
		Height:	300 * Scale,
		
		// 自車インジケータ
		IndicatorSize:	12 * Scale,		// サイズ
		IndicatorColor:	0x0080FF,		// 色
		
		// 地図更新間隔
		// 前回地図更新時から指定秒以上経過し，
		// かつ指定距離以上移動した場合のみ地図を更新します
		UpdateTime:		1000,	// [ミリ秒]
		UpdateDistance:	5,		// [m]
	};
	
	//////////////////////////////////////////////////////////////////////////
	/// ↑↑↑↑↑Google Maps の設定 ここまで↑↑↑↑↑ //////////////////////
	//////////////////////////////////////////////////////////////////////////
	
	if( GoogleMapsParam.APIKey == '' ){
		MessageBox(
			"google_maps.js スキンを使用するためには初期設定が必要です．詳しくは\n" +
			Vsd.SkinDir + "google_maps.js\n" +
			"をメモ帳等で開き，その先頭に書かれている説明をお読みください．\n" +
			"(設定なしでも短時間なら使えるようです)"
		);
	}
	
	// 使用する画像・フォントの宣言
	font = new Font( "ＭＳ ゴシック", 24 * Scale, FONT_OUTLINE );
}

//*** メーター描画処理 ******************************************************

function Draw(){
	if( Vsd.Longitude === undefined ){
		Vsd.DrawTextAlign(
			Vsd.Width, ( Vsd.Height - font.Height ) / 2, ALIGN_HCENTER | ALIGN_VCENTER,
			"GPS データが読み込まれていません", font, 0xFFFFFF
		);
		return;
	}
	
	// Google マップ表示
	Vsd.DrawGoogleMaps( GoogleMapsParam );
	
	// 文字データ
	Vsd.DrawTextAlign(
		0, Vsd.Height - 1, ALIGN_BOTTOM,
		"緯度:" + Vsd.Latitude.toFixed( 6 ) +
		"  経度:" + Vsd.Longitude.toFixed( 6 ) +
		"  距離:" + ( Vsd.Distance / 1000 ).toFixed( 2 ) + "km" +
		"  速度:" + Vsd.Speed.toFixed( 0 ) + "km/h",
		font, 0xFFFFFF
	);
}
