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
		
		// ズームレベルを 0～21 で指定します
		Zoom: 17,
		
		// 地図タイプ
		// roadmap:地図  satellite:航空写真  terrain:地形図  hybrid:地図+航空写真
		Maptype: "hybrid",
		Maptype: "terrain",
		Maptype: "roadmap",
		
		// 地図表示位置，サイズ(最大 640x640)
		X: 0,
		Y: 0,
		Width:	min( 400 * Scale, 640 ),
		Height:	min( 300 * Scale, 640 ),
		
		// 自車インジケータ
		IndicatorSize:	12 * Scale,		// サイズ
		IndicatorColor:	0x0080FF,		// 色
		
		// 地図更新間隔
		// 前回地図更新時から指定秒以上経過し，
		// かつ指定距離以上移動した場合のみ地図を更新します
		UpdateTime:		1000,	// [ミリ秒]
		UpdateDistance:	16,		// [ピクセル]
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
	
	// 使用する画像・フォントの宣言
	font = new Font( "ＭＳ ゴシック", 24 * Scale, FONT_OUTLINE );
}

//*** メーター描画処理 ******************************************************

function Draw(){
	if( Vsd.Longitude === undefined ){
		Vsd.DrawTextAlign(
			Vsd.Width / 2, ( Vsd.Height - font.Height ) / 2, ALIGN_HCENTER | ALIGN_VCENTER,
			"GPS データが読み込まれていません", font, 0xFFFFFF
		);
		return;
	}
	
	// Google マップ表示
	Vsd.DrawGoogleMaps2( GoogleMapsParam );
	
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

//*** Google Map 描画 ********************************************************

Vsd.DrawGoogleMaps2 = function( param ){
	if( Vsd.Longitude === undefined ) return;
	
	GMapURL = "http://maps.googleapis.com/maps/api/staticmap?sensor=false&language=ja" +
		( param.APIKey != '' ? "&key=" + param.APIKey : '' ) +
		"&maptype=" + param.Maptype +
		"&zoom=" + param.Zoom +
		"&size=640x640&center=";
	
	// 一番最初の地図データを同期モードで取得
	if( param.MapImg === undefined ){
		param.MapImg	= new Image( GMapURL + Vsd.Latitude + "," + Vsd.Longitude );
		param.Dir		= Vsd.Direction;
		param.Time		= Vsd.DateTime;
		param.DispLong	= param.Long = Vsd.Longitude;
		param.DispLati	= param.Lati = Vsd.Latitude;
		param.Distance	= Vsd.Distance;
	}
	
	// 次のマップデータ取得が完了した
	if(
		param.MapImgNext !== undefined &&
		param.MapImgNext.Status == IMG_STATUS_LOAD_COMPLETE
	){
		param.MapImg.Dispose();
		param.MapImg		= param.MapImgNext;
		param.MapImgNext	= undefined;
		param.Dir			= param.NextDir;
		param.DispLong		= param.Long;
		param.DispLati		= param.Lati;
	}
	
	// ピクセル移動量を計算  参考: http://hosohashi.blog59.fc2.com/blog-entry-5.html
	var PixLong = ( Vsd.Longitude - param.DispLong ) / 180 * ( 1 << ( 7 + param.Zoom ));
	var sin_pic = Math.sin( Math.PI / 180 * param.DispLati );
	var sin_now = Math.sin( Math.PI / 180 * Vsd.Latitude );
	var PixLati = (
		Math.log(( 1 + sin_pic ) / ( 1 - sin_pic )) -
		Math.log(( 1 + sin_now ) / ( 1 - sin_now ))
	) / Math.PI * ( 1 << ( 7 - 1 + param.Zoom ));
	
	Vsd.PutImage(
		param.X, param.Y, param.MapImg, 0,
		320 - ( param.Width  >> 1 ) + PixLong,
		320 - ( param.Height >> 1 ) + PixLati,
		param.Width, param.Height
	);
	
	DrawArrow(
		param.X + param.Width  / 2,
		param.Y + param.Height / 2,
		Vsd.Direction, param.IndicatorSize
	);
	
	// 次のマップデータを非同期モードで取得
	if(
		Math.abs( param.Time - Vsd.DateTime ) >= param.UpdateTime &&
		( PixLong * PixLong + PixLati * PixLati ) >= param.UpdateDistance * param.UpdateDistance
	){
		param.MapImgNext = new Image(
			GMapURL + Vsd.Latitude + "," + Vsd.Longitude,
			IMG_INET_ASYNC
		);
		param.Time		= Vsd.DateTime;
		param.Long		= Vsd.Longitude;
		param.Lati		= Vsd.Latitude;
		param.NextDir	= Vsd.Direction;
	}
	
	
	// 自車マーク描画
	function DrawArrow( cx, cy, angle, size ){
		angle *= Math.PI / 180;
		var cos = Math.cos( angle ) * size;
		var sin = Math.sin( angle ) * size;
		
		apex = [
			0, -1,
			0.707, 0.707,
			0, 0.25,
			-0.707, 0.707,
		];
		
		// 回転 + x,y 座標加算
		for( var i = 0; i < apex.length; i += 2 ){
			var x = cx + apex[ i ] * cos - apex[ i + 1 ] * sin;
			var y = cy + apex[ i ] * sin + apex[ i + 1 ] * cos;
			apex[ i     ] = x;
			apex[ i + 1 ] = y;
		}
		
		Vsd.DrawPolygon( apex, param.IndicatorColor, DRAW_FILL );
		Vsd.DrawCircle( cx, cy, size, param.IndicatorColor );
	}
}
