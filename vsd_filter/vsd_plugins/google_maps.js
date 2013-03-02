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
	
	// Google Maps の API キーを指定します．
	// 例: APIKey = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
	APIKey		= "AIzaSyABCDEFGHIJKLMNOPQRSTUVWXYZabcdefg";
	
	// ズームレベルを 1～21 で指定します
	Zoom		= 15;
	
	// 地図タイプ
	// roadmap:地図  satellite:航空写真  terrain:地形図  hybrid:航空写真
	Maptype		= "roadmap";
	
	// 地図サイズ
	Width		= 400 * Scale;
	Height		= 300 * Scale;
	
	//////////////////////////////////////////////////////////////////////////
	/// ↑↑↑↑↑Google Maps の設定 ここまで↑↑↑↑↑ //////////////////////
	//////////////////////////////////////////////////////////////////////////
	
	if( APIKey == '' ){
		MessageBox(
			"google_maps.js スキンを使用するためには初期設定が必要です．詳しくは\n" +
			Vsd.SkinDir + "google_maps.js\n" +
			"をメモ帳等で開き，その先頭に書かれている説明をお読みください．\n" +
			"(設定なしでも短時間なら使えるようです)"
		);
	}
	
	Width = Math.floor( Width );
	Height = Math.floor( Height );
	
	// 使用する画像・フォントの宣言
	Font = new Font( "ＭＳ ゴシック", 24 * Scale, FONT_OUTLINE );
	
	GMapURL = "http://maps.googleapis.com/maps/api/staticmap?sensor=false&language=ja" +
		( APIKey != '' ? "&key=" + APIKey : '' ) +
		"&maptype=" + Maptype +
		"&zoom=" + Zoom +
		"&size=" + Width + "x" + Height +
		"&center=";
}

//*** メーター描画処理 ******************************************************

function Draw(){
	// 背景
	if( typeof( MapImg ) == 'undefined' ){
		MapImg = new Image( GMapURL + Vsd.Latitude + "," + Vsd.Longitude );
		Dir = Vsd.Direction;
	}
	
	if( Vsd.FrameCnt % ( 30 * 1 ) == 0 ){
		if( Vsd.Speed >= 1 ){
			MapImgNext = new Image(
				GMapURL + Vsd.Latitude + "," + Vsd.Longitude,
				IMG_INET_ASYNC
			);
			NextDir = Vsd.Direction;
		}
		
		Caption =
			"緯度:" + Vsd.Latitude.toFixed( 6 ) +
			"  経度:" + Vsd.Longitude.toFixed( 6 ) +
			"  距離:" + ( Vsd.Distance / 1000 ).toFixed( 2 ) + "km" +
			"  速度:" + Vsd.Speed.toFixed( 0 ) + "km/h";
	}
	
	if(
		typeof( MapImgNext ) != 'undefined' &&
		MapImgNext.Status == IMG_STATUS_LOAD_COMPLETE
	){
		MapImg.Dispose();
		MapImg = MapImgNext;
		MapImgNext = undefined;
		Dir = NextDir;
	}
	
	Vsd.PutImage( 0, 0, MapImg );
	DrawArrow( Width / 2, Height / 2, Dir, Scale );
	
	if( typeof( Caption ) == 'undefined' ) Caption = '';
	Vsd.DrawTextAlign(
		0, Vsd.Height - 1, ALIGN_BOTTOM,
		Caption, Font, 0xFFFFFF
	);
}

function DrawArrow( x, y, angle, scale ){
	angle *= Math.PI / 180;
	var cos = Math.cos( angle ) * Scale;
	var sin = Math.sin( angle ) * Scale;
	
	var x0 =  0 * cos - 15 * -sin;
	var y0 =  0 * sin - 15 *  cos;
	var x1 =  6 * cos +  8 * -sin;
	var y1 =  6 * sin +  8 *  cos;
	var x2 = -6 * cos +  8 * -sin;
	var y2 = -6 * sin +  8 *  cos;
	
	Vsd.DrawLine( x + x0, y + y0, x + x1, y + y1, 0, 1, DRAW_FILL );
	Vsd.DrawLine( x + x1, y + y1, x + x2, y + y2, 0, 1, DRAW_FILL );
	Vsd.DrawLine( x + x2, y + y2, x + x0, y + y0, 0, 1, DRAW_FILL );
	Vsd.DrawPolygon( 0x0080FF );
}
