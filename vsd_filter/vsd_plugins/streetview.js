//*** 初期化処理 ************************************************************

function Initialize(){
	Scale = Vsd.Height / 720;
	
	//////////////////////////////////////////////////////////////////////////
	/// ↓↓↓↓↓Google ストリートビュー の設定 ここから↓↓↓↓↓ //////////
	//////////////////////////////////////////////////////////////////////////
	
	// 設定を行うためには，以下の設定値を直接書き換えてください．
	
	// ★補足説明
	// このスキンは Google サーバにアクセスし画像データを得ています．
	//   Google サーバにアクセスするためには「API キー」が必要です．
	//   キーは無料で取得出来ます．キー取得方法は
	//   https://developers.google.com/maps/documentation/streetview/index?hl=ja#api_key
	//   を参照してください．
	// また，Google によって
	//   ストリートビュー画像の取得は 1日あたり 25,000 枚
	//   ジオコーディングの取得は 1日あたり 2,500 回
	//   に制限されています．
	
	StreetViewParam = {
		// Google ストリートビューの API キーを指定します．
		// 例: APIKey: "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
		APIKey: [										//#DEL#
			"AIzaSyABCDEFGHIJKLMNOPQRSTUVWXYZabcdefg",	//#DEL#
			"AIzaSyABCDEFGHIJKLMNOPQRSTUVWXYZabcdefg",	//#DEL#
			"AIzaSyABCDEFGHIJKLMNOPQRSTUVWXYZabcdefg",	//#DEL#
		],												//#DEL#
//#REL#	APIKey: "",
		
		//////////////////////////////////////////////////////////////////////////
		/// ↑↑↑↑↑Google ストリートビュー の設定 ここまで↑↑↑↑↑ //////////
		//////////////////////////////////////////////////////////////////////////
		
		// ストリートビュー表示位置，サイズ(最大 640x640)
		X:		0,
		Y:		0,
		Width:	min( 640, Vsd.Width ),
		Height:	min( 480, Vsd.Height ),
		
		// ストリートビュー更新間隔
		// 前回ストリートビュー更新時から指定秒以上経過し，
		// かつ指定距離以上移動した場合のみストリートビューを更新します
		UpdateTime:		Vsd.IsSaving ? 1 : 4,		// [frame]
		
		// 画像先読み数
		ImgCacheCnt:	30,
	};
	
	MapParam = {
		APIKey: "AIzaSyABCDEFGHIJKLMNOPQRSTUVWXYZabcdefg",	//#DEL#
		
		// ズームレベルを 0～21 で指定します
		Zoom: 14,
		
		// 地図タイプ
		// roadmap:地図  satellite:航空写真  terrain:地形図  hybrid:地図+航空写真 //#DEL#
		Maptype: "roadmap",	//#DEL#
//#REL#	Maptype: "openstreetmap",
		
		// 地図表示位置，サイズ(最大 640x640)
		X:		Vsd.Width - 308 * Scale,
		Y:		8 * Scale,
		Width:	300 * Scale,
		Height:	300 * Scale,
		
		// 自車インジケータ
		IndicatorSize:	12 * Scale,		// サイズ
		IndicatorColor:	0x0080FF,		// 色
	};
	
	// Geocoding の設定
	GeocodingParam = {
		// Geocoding 更新間隔
		UpdateTime:	100000,	// [ミリ秒]
	};
	
	function min( a, b ){ return ( a < b ) ? a : b; }
	
	if( StreetViewParam.APIKey == '' ){
		MessageBox(
			"streetview.js スキンを使用するためには初期設定が必要です．詳しくは\n" +
			Vsd.SkinDir + "streetview.js\n" +
			"をメモ帳等で開き，その先頭に書かれている説明をお読みください．\n" +
			"(設定なしでも短時間なら使えるようです)"
		);
	}
	
	MeterRight = 1;
	
	// 使用する画像・フォントの宣言
	FontJ = new Font( "ＭＳ Ｐゴシック", 31 * Scale );
	FontM = new Font( "Impact", 31 * Scale, FONT_FIXED );
	FontS = new Font( "Impact", 24 * Scale );
	
	// スピードメータ用最高速計算
	MaxSpeed = Math.ceil( Log.Max.Speed / 10 ) * 10;
	
	// メータ用 param
	
	MeterParam = {
		Flag:	LMS_HORIZONTAL | ALIGN_HCENTER | ALIGN_TOP,
		X:			8 * Scale,
		Y:			36 * Scale,
		Width:		Vsd.Width - 332 * Scale,
		Line1Len:	20 * Scale,
		Line1Width:	3 * Scale,
		Line1Color:	0xFFFFFF,
		Line1Cnt:	10,
		Line2Len:	15 * Scale,
		Line2Width:	1,
		Line2Color:	0xFFFFFF,
		Line2Cnt:	5,
		MinVal:		0,
		MaxVal:		MaxSpeed,
		NumPos:		20 * Scale,
		Font:		FontS,
		FontColor:	0xFFFFFF
	};
	
	FontColor   = 0xFFFFFF;
	FontColorOL = 0xFFFFFF;
	BGColor = 0x80001020;
}

//*** StreetView 描画 *******************************************************

DrawStreetView = function( param ){
	if( Log.Longitude === undefined ) return;
	
	var FrameCnt	= ~~( Vsd.FrameCnt / param.UpdateTime ) * param.UpdateTime;
	var ImgIdx		= ( FrameCnt / param.UpdateTime ) % param.ImgCacheCnt;
	
	// 一番最初の画像を同期モードで取得
	if( param.StViewImg === undefined || param.FrameCnt != Vsd.FrameCnt ){
		// 画像 cache 数だけ先読み
		param.StViewImg = [];
		
		for( var i = 1; i < param.ImgCacheCnt; ++i ){
			param.StViewImg[ i ] = new Image( GetImageURL( FrameCnt + i * param.UpdateTime ), IMG_INET_ASYNC );
		}
		
		param.StViewImg[ 0 ] = new Image( GetImageURL( FrameCnt ));
		param.DispIdx = 0;
		param.FrameCnt = Vsd.FrameCnt + 1;
	}else{
		++param.FrameCnt;
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
		Vsd.PutImage( param.X, param.Y, param.StViewImg[ param.DispIdx ]);
	}
	
	// 画像 URL 生成
	function GetImageURL( FrameCnt ){
		var key = '';
		
		if( typeof( param.APIKey ) == 'object' ){
			key = "&key=" + param.APIKey[ Vsd.FrameCnt % param.APIKey.length ];
		}else if( param.APIKey != '' ){
			key = "&key=" + param.APIKey;
		}
		
		return "http://maps.googleapis.com/maps/api/streetview?sensor=false" +
			"&size=" + ~~param.Width + "x" + ~~param.Height + "&location=" +
			Log.ValueOfIndex( "Latitude", FrameCnt ) + "," +
			Log.ValueOfIndex( "Longitude", FrameCnt ) + "&heading=" +
			Log.ValueOfIndex( "Direction", FrameCnt ) + key;
	}
}

//*** メーター描画処理 ******************************************************

function Draw(){
	// ストリートビュー表示
	DrawStreetView( StreetViewParam );
	
	// 背景
	Vsd.DrawRect( Vsd.Width - 316 * Scale, 0, Vsd.Width - 1, 316 * Scale, BGColor, DRAW_FILL );
	Vsd.DrawRect( 0, 0, Vsd.Width - 316 * Scale - 1, 84 * Scale - 1, BGColor, DRAW_FILL );
	Vsd.DrawRect( 0, 84 * Scale, FontM.GetTextWidth( ' ' ) * 16.5, 84 * Scale + FontM.Height * 6.2, BGColor, DRAW_FILL );
	//Vsd.DrawRect( MapParam2.X, MapParam2.Y, MapParam2.X + MapParam2.Width -1 , MapParam2.Y + MapParam2.Height - 1, 0 );
	
	// 逆 Geo コーディング
	Vsd.Geocoding( GeocodingParam );
	Vsd.DrawTextAlign(
		0, 0, 0,
		GeocodingParam.Address,
		FontJ, FontColor
	);
	
	// Google マップ表示
	Vsd.DrawRoadMap( MapParam );
	
	// スピードメーター描画
	Vsd.DrawRect(
		8 * Scale, 36 * Scale,
		( 8 * Scale ) + ( Vsd.Width - 332 * Scale ) * Log.Speed / MaxSpeed,
		48 * Scale,
		0x00FFFF, DRAW_FILL
	);
	Vsd.DrawLinearMeterScale( MeterParam );
	
	// 文字データ
	var Y = 84 * Scale;
	
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
	
	if( Log.Longitude === undefined ) return;
	
	Vsd.DrawText( 0, Y, "Lat.: " + Log.Latitude.toFixed( 6 ), FontM, FontColor );
	Y += FontM.Height;
	Vsd.DrawText( 0, Y, "Lng.: " + Log.Longitude.toFixed( 6 ), FontM, FontColor );
	Y += FontM.Height;
	Vsd.DrawText( 0, Y, "Alt.: " + ( Log.Altitude !== undefined ? Log.Altitude.toFixed( 1 ) + "m" : "---" ), FontM, FontColor );
	Y += FontM.Height;
	Vsd.DrawText( 0, Y, "Dist.:" + ( Log.Distance / 1000 ).toFixed( 2 ) + "km", FontM, FontColor );
}
