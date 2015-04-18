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
	
	StreetViewParam = {
		// Google Maps の API キーを指定します．
		// 例: APIKey: "ABCDEFGHIJKLMNOPQRSTUVWXYZ",
		APIKey: [
			"AIzaSyABCDEFGHIJKLMNOPQRSTUVWXYZabcdefg",
			"AIzaSyABCDEFGHIJKLMNOPQRSTUVWXYZabcdefg",
			"AIzaSyABCDEFGHIJKLMNOPQRSTUVWXYZabcdefg",
		],
		
		// 地図表示位置，サイズ(最大 640x640)
		X:		0,
		Y:		0,
		Width:	min( 640, Vsd.Width ),
		Height:	min( 480, Vsd.Height ),
		
		// 地図更新間隔
		// 前回地図更新時から指定秒以上経過し，
		// かつ指定距離以上移動した場合のみ地図を更新します
		UpdateTime:		Vsd.IsSaving ? 1 : 4,		// [frame]
		
		// 画像先読み数
		ImgCacheCnt:	30,
	};
	
	MapParam = {
		// Google Maps の API キーを指定します．
		// 例: APIKey: "ABCDEFGHIJKLMNOPQRSTUVWXYZ",
		APIKey: "AIzaSyABCDEFGHIJKLMNOPQRSTUVWXYZabcdefg",
		
		// ズームレベルを 0～21 で指定します
		Zoom: 14,
		
		// 地図タイプ
		// roadmap:地図  satellite:航空写真  terrain:地形図  hybrid:地図+航空写真
		//Maptype: "roadmap",
		Maptype: "openstreetmap",
		//Maptype: "hybrid",
		//Maptype: "satellite",
		
		// 地図表示位置，サイズ(最大 640x640)
		X:		Vsd.Width - 300 * Scale,
		Y:		0,
		Width:	300 * Scale,
		Height:	300 * Scale,
		
		// 自車インジケータ
		IndicatorSize:	12 * Scale,		// サイズ
		IndicatorColor:	0x0080FF,		// 色
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
	
	if( MapParam.APIKey == '' ){
		MessageBox(
			"google_maps.js スキンを使用するためには初期設定が必要です．詳しくは\n" +
			Vsd.SkinDir + "google_maps.js\n" +
			"をメモ帳等で開き，その先頭に書かれている説明をお読みください．\n" +
			"(設定なしでも短時間なら使えるようです)"
		);
	}
	
	MeterRight = 1;
	
	// 使用する画像・フォントの宣言
	FontM = new Font( "Impact", 20, FONT_FIXED | FONT_OUTLINE );
	FontS = new Font( "Impact", 18 * Scale );
	FontL = new Font( "Impact", 48 * Scale );
	
	// 座標等を予め計算しておく
	MeterR  = 100 * Scale;
	MeterX	= MeterRight ? Vsd.Width  - MeterR * 2: 0;
	MeterY	= Vsd.Height - MeterR * 2 * 0.88;
	MeterCx = MeterX + MeterR;
	MeterCy = MeterY + MeterR;
	
	// スピードメータ用最高速計算
	MaxSpeed = ~~( Log.Max.Speed / 10 ) * 10;
	
	FontColor   = 0;
	FontColorOL = 0xFFFFFF;
	BGColor = 0x80001020;
}

//*** StreetView 描画 *******************************************************

DrawStreetView = function( param ){
	if( Log.Longitude === undefined ) return;
	
	var FrameCnt	= ~~( Vsd.FrameCnt / param.UpdateTime ) * param.UpdateTime;
	var ImgIdx		= ( FrameCnt / param.UpdateTime ) % param.ImgCacheCnt;
	
	// 一番最初の画像を同期モードで取得
	if( param.StViewImg === undefined ){
		// 画像 cache 数だけ先読み
		param.StViewImg = [];
		
		for( var i = 1; i < param.ImgCacheCnt; ++i ){
			param.StViewImg[ i ] = new Image( GetImageURL( FrameCnt + i * param.UpdateTime ), IMG_INET_ASYNC );
		}
		
		param.StViewImg[ 0 ] = new Image( GetImageURL( FrameCnt ));
		param.DispIdx = 0;
	}
	
	if( Vsd.IsSaving ){
		// 画像データ取得完了まで待つ
		while( param.StViewImg[ ImgIdx ].Status == IMG_STATUS_LOAD_INCOMPLETE );
		Vsd.PutImage( param.X, param.Y, param.StViewImg[ ImgIdx ]);
		
		// 次の画像データを非同期モードで取得
		if(( Vsd.FrameCnt + 1 ) % param.UpdateTime == 0 ){
			param.StViewImg[ ImgIdx ].Dispose();
			param.StViewImg[ ImgIdx ] = new Image(
				GetImageURL( FrameCnt + param.ImgCacheCnt * param.UpdateTime ),
				IMG_INET_ASYNC
			);
		}
	}else{
		if(
			ImgIdx != param.DispIdx &&
			param.StViewImg[( param.DispIdx + 1 ) % param.ImgCacheCnt ].Status == IMG_STATUS_LOAD_COMPLETE
		){
			param.StViewImg[ param.DispIdx ].Dispose();
			param.StViewImg[ param.DispIdx ] = new Image(
				GetImageURL(
					( ImgIdx > param.DispIdx ?
						param.DispIdx + param.ImgCacheCnt - ImgIdx :
						param.DispIdx - ImgIdx
					) * param.UpdateTime + FrameCnt
				),
				IMG_INET_ASYNC
			);
			param.DispIdx = ( param.DispIdx + 1 ) % param.ImgCacheCnt;
		}
Print( "ImgIdx:" + ImgIdx + "  DispIdx:" + param.DispIdx + "\n" );
		Vsd.PutImage( param.X, param.Y, param.StViewImg[ param.DispIdx ]);
	}
	
	// 画像 URL 生成
	function GetImageURL( FrameCnt ){
Print( "Req:" + FrameCnt + " / " + ( ~~( FrameCnt / param.UpdateTime )% param.ImgCacheCnt ) + "\n" );
		var key = '';
		
		if( typeof( param.APIKey ) == 'object' ){
			key = "&key=" + param.APIKey[ Vsd.FrameCnt % param.APIKey.length ];
		}else if( param.APIKey != '' ){
			key = "&key=" + param.APIKey;
		}
		
		return "http://maps.googleapis.com/maps/api/streetview?sensor=false" +
			"&size=" + ~~param.Width + "x" + ~~param.Height + "&location=" +
			Vsd.AccessLog( "Latitude", FrameCnt ) + "," +
			Vsd.AccessLog( "Longitude", FrameCnt ) + "&heading=" +
			Vsd.AccessLog( "Direction", FrameCnt ) + key;
	}
}

//*** メーター描画処理 ******************************************************

function Draw(){
	// Google マップ表示
	DrawStreetView( StreetViewParam );
	Vsd.DrawRoadMap( MapParam );
	//Vsd.DrawRect( MapParam2.X, MapParam2.Y, MapParam2.X + MapParam2.Width -1 , MapParam2.Y + MapParam2.Height - 1, 0 );
	
	// メーター画像描画
	Vsd.DrawCircle( MeterCx, MeterCy, MeterR, BGColor, DRAW_FILL );
	
	// スピードメーター目盛り描画
	Vsd.DrawMeterScale(
		MeterCx, MeterCy, MeterR,
		MeterR * 0.1,  2, 0xFFFFFF,
		MeterR * 0.05, 1, 0xFFFFFF,
		2, 135, 45,
		MeterR * 0.78,
		MaxSpeed, 12, 0xFFFFFF,
		FontS
	);
	
	// スピード数値表示
	Vsd.DrawTextAlign(
		MeterCx, MeterCy + MeterR * 0.25, 
		ALIGN_HCENTER | ALIGN_VCENTER,
		~~Log.Speed, FontL, 0xFFFFFF
	);
	
	Vsd.DrawTextAlign(
		MeterCx, MeterCy + MeterR * 0.5,
		ALIGN_HCENTER | ALIGN_VCENTER,
		"km/h", FontS, 0xFFFFFF
	);
	
	// スピードメーター針
	Vsd.DrawNeedle(
		MeterCx, MeterCy, MeterR * 0.95, MeterR * -0.1,
		135, 45, Log.Speed / MaxSpeed, 0xFF0000, 3
	);
	
	// 文字データ
	var Y = 0;
	var X = 0;
	
	var date = new Date();
	date.setTime( Vsd.DateTime );
	
	Vsd.DrawText( X, Y,
			date.getFullYear() + "/" +
			( date.getMonth() < 9 ? "0" : "" ) + ( date.getMonth() + 1 ) + "/" +
			( date.getDate() < 10 ? "0" : "" ) + date.getDate() + " " +
			( date.getHours() < 10 ? "0" : "" ) + date.getHours() + ":" +
			( date.getMinutes() < 10 ? "0" : "" ) + date.getMinutes() + ":" +
			( date.getSeconds() < 10 ? "0" : "" ) + date.getSeconds(),
		FontM, FontColor, FontColorOL
	);
	Y += FontM.Height;
	
	Vsd.DrawText( X, Y, "Alt.: " + ( Vsd.Altitude !== undefined ? Vsd.Altitude.toFixed( 1 ) + "m" : "---" ), FontM, FontColor, FontColorOL );
	Y += FontM.Height;
	Vsd.DrawText( X, Y, "Dist.:" + ( Log.Distance / 1000 ).toFixed( 2 ) + "km", FontM, FontColor, FontColorOL );
}
