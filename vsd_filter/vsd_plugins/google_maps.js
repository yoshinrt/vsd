//*** 初期化処理 ************************************************************

function Initialize(){
	Scale = Vsd.Height / 720;
	
	//////////////////////////////////////////////////////////////////////////
	/// ↓↓↓↓↓Google Maps の設定 ここから↓↓↓↓↓ //////////////////////
	//////////////////////////////////////////////////////////////////////////
	
	// 設定を行うためには，以下の設定値を直接書き換えてください．
	
	// ★補足説明
	// このスキンは Google Maps にアクセスし地図データを得ています．
	//   Google Maps にアクセスするためには「API キー」が必要です．
	//   キーは無料で取得出来ます．キー取得方法は
	//   https://developers.google.com/maps/documentation/staticmaps/?hl=ja#api_key
	//   を参照してください．
	// また，Google によって
	//   マップデータの取得は 1日あたり 25,000 枚
	//   ジオコーディングの取得は 1日あたり 2,500 回
	//   に制限されています．
	
	GoogleMapsParam = {
		// Google Maps の API キーを指定します．
		// 例: APIKey: "ABCDEFGHIJKLMNOPQRSTUVWXYZ",
		APIKey: "AIzaSyABCDEFGHIJKLMNOPQRSTUVWXYZabcdefg",
		
		// ズームレベルを 0～21 で指定します
		Zoom: 14,
		
		// 地図タイプ
		// roadmap:地図  satellite:航空写真  terrain:地形図  hybrid:地図+航空写真
		Maptype: "roadmap",
		
		// 地図表示位置，サイズ(最大 640x640)
		X:		8 * Scale,
		Y:		8 * Scale,
		Width:	min( 300 * Scale, 640 ),
		Height:	min( 300 * Scale, 640 ),
		
		// 自車インジケータ
		IndicatorSize:	12 * Scale,		// サイズ
		IndicatorColor:	0x0080FF,		// 色
		
		// 地図更新間隔
		// 前回地図更新時から指定秒以上経過し，
		// かつ指定距離以上移動した場合のみ地図を更新します
		UpdateTime:		1000,	// [ミリ秒]
		UpdateDistance:	10,		// [ピクセル]
	};
	
	// Geocoding の設定
	GeocodingParam = {
		// Geocoding 更新間隔
		UpdateTime:	10000,	// [ミリ秒]
	};
	
	//////////////////////////////////////////////////////////////////////////
	/// ↑↑↑↑↑Google Maps の設定 ここまで↑↑↑↑↑ //////////////////////
	//////////////////////////////////////////////////////////////////////////
	
	function min( a, b ){ return ( a < b ) ? a : b; }
	
	if( GoogleMapsParam.APIKey == '' ){
		MessageBox(
			"google_maps.js スキンを使用するためには初期設定が必要です．詳しくは\n" +
			Vsd.SkinDir + "google_maps.js\n" +
			"をメモ帳等で開き，その先頭に書かれている説明をお読みください．\n" +
			"(設定なしでも短時間なら使えるようです)"
		);
	}
	
	MeterRight = 1;
	
	// 使用する画像・フォントの宣言
	FontS = new Font( "Impact", 24 * Scale );
	FontJ = new Font( "ＭＳ Ｐゴシック", 31 * Scale );
	FontM = new Font( "Impact", 31 * Scale, FONT_FIXED );
	FontL = new Font( "Impact", 60 * Scale );
	
	// スピードメータ用最高速計算
	MaxSpeed = Math.ceil( Vsd.MaxSpeed / 10 ) * 10;
	
	// 座標等を予め計算しておく
	MeterR  = 120 * Scale;
	MeterX	= MeterRight ? Vsd.Width  - MeterR * 2: 0;
	MeterY	= Vsd.Height - MeterR * 2 * 0.88;
	
	MeterParam = {
		X:			MeterX + MeterR,
		Y:			MeterY + MeterR,
		R:			MeterR,
		Line1Len:	MeterR * 0.1,
		Line1Width:	2,
		Line1Color:	0xFFFFFF,
		Line1Cnt:	12,
		Line2Len:	MeterR * 0.05,
		Line2Width:	1,
		Line2Color:	0xFFFFFF,
		Line2Cnt:	2,
		NumR:		MeterR * 0.78,
		FontColor:	0xFFFFFF,
		Font:		FontS,
		MinAngle:	135,
		MaxAngle:	45,
		MinVal:		0,
		MaxVal:		MaxSpeed,
	};
	
	FontColor = 0xC0C0C0;
	BGColor = 0x80001020;
}

//*** メーター描画処理 ******************************************************

function Draw(){
	// メーター画像描画
	Vsd.DrawCircle( MeterParam.X, MeterParam.Y, MeterR, BGColor, DRAW_FILL );
	
	// スピードメーター目盛り描画
	Vsd.DrawRoundMeterScale( MeterParam );
	
	// スピード数値表示
	Vsd.DrawTextAlign(
		MeterParam.X, MeterParam.Y + MeterR * 0.25, 
		ALIGN_HCENTER | ALIGN_VCENTER,
		~~Vsd.Speed, FontL, 0xFFFFFF
	);
	
	Vsd.DrawTextAlign(
		MeterParam.X, MeterParam.Y + MeterR * 0.5,
		ALIGN_HCENTER | ALIGN_VCENTER,
		"km/h", FontS, 0xFFFFFF
	);
	
	// スピードメーター針
	Vsd.DrawNeedle(
		MeterParam.X, MeterParam.Y, MeterR * 0.95, MeterR * -0.1,
		135, 45, Vsd.Speed / MaxSpeed, 0xFF0000, 3
	);
	
	Vsd.DrawRect( 0, 0, 316 * Scale - 1, 316 * Scale - 1, BGColor, DRAW_FILL );
	Vsd.DrawRect( 316 * Scale, 0, Vsd.Width - 1, FontJ.Height - 1, BGColor, DRAW_FILL );
	Vsd.DrawRect( 0, 324 * Scale, 316 * Scale - 1, 324 * Scale + FontJ.Height * 6 - 1, BGColor, DRAW_FILL );
	
	// Google マップ表示
	Vsd.DrawGoogleMaps( GoogleMapsParam );
	
	// 文字データ
	var Y = 324 * Scale;
	
	var date = new Date();
	date.setTime( Vsd.DateTime );
	
	Vsd.DrawText( 0, Y,
		"Date: " + date.getFullYear() + "/" +
			( date.getMonth() < 9 ? "0" : "" ) + ( date.getMonth() + 1 ) + "/" +
			( date.getDate() < 10 ? "0" : "" ) + date.getDate(),
		FontM, FontColor
	);
	Y += FontM.Height;
	
	Vsd.DrawText( 0, Y,
		"Time: " + ( date.getHours() < 10 ? "0" : "" ) + date.getHours() + ":" +
			( date.getMinutes() < 10 ? "0" : "" ) + date.getMinutes() + ":" +
			( date.getSeconds() < 10 ? "0" : "" ) + date.getSeconds(),
		FontM, FontColor
	);
	Y += FontM.Height;
	
	if( Vsd.Longitude === undefined ) return;
	
	Vsd.DrawText( 0, Y, "Lat.: " + Vsd.Latitude.toFixed( 6 ), FontM, FontColor );
	Y += FontM.Height;
	Vsd.DrawText( 0, Y, "Lng.: " + Vsd.Longitude.toFixed( 6 ), FontM, FontColor );
	Y += FontM.Height;
	Vsd.DrawText( 0, Y, "Alt.: " + ( Vsd.Altitude !== undefined ? Vsd.Altitude.toFixed( 1 ) + "m" : "---" ), FontM, FontColor );
	Y += FontM.Height;
	Vsd.DrawText( 0, Y, "Dist.:" + ( Vsd.Distance / 1000 ).toFixed( 2 ) + "km", FontM, FontColor );
	
	Vsd.Geocoding( GeocodingParam );
	Vsd.DrawTextAlign(
		316 * Scale, 0, 0,
		GeocodingParam.Address,
		FontJ, FontColor
	);
}
