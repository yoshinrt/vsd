//*** 初期化処理 ************************************************************

function Initialize(){
	Scale = Vsd.Height / 720;
	
	// ストリートビューパラメータ設定
	StreetViewParam = {
		// ストリートビュー表示位置，サイズ(最大 640x640)
		X:		0,
		Y:		0,
		Width:	Math.min( 640, Vsd.Width ),
		Height:	Math.min( 480, Vsd.Height ),
		
		// ストリートビュー更新間隔 (フレーム数)
		UpdateTime:		Vsd.IsSaving ? 1 : 4,		// [frame]
		
		// 画像先読み数
		ImgCacheCnt:	Vsd.IsSaving ? 30 : 10,
	};
	
	MapParam = {
		// ズームレベルを 0～21 で指定します
		Zoom: 14,
		
		// 地図タイプ
		// roadmap:地図  satellite:航空写真  terrain:地形図  hybrid:地図+航空写真 //#DEL#
		Maptype: "roadmap",	//#DEL#
//		Maptype: "openstreetmap",	//#REL#
		
		// 地図表示位置，サイズ(最大 640x640)
		X:		Vsd.Width - 308 * Scale,
		Y:		8 * Scale,
		Width:	300 * Scale,
		Height:	300 * Scale,
		
		// 走行軌跡
		PathColor:	0xFF0000,	// -1 で非表示
		PathWidth:	5 * Scale,
		
		// 自車インジケータ
		IndicatorSize:	12 * Scale,		// サイズ
		IndicatorColor:	0x0080FF,		// 色
	};
	
	// Geocoding の設定
	GeocodingParam = {
		// Geocoding 更新間隔
		UpdateTime:	10000,	// [ミリ秒]
	};
	
	if( GoogleAPIKey[ 0 ] == '' ){
		if( MessageBox(
			"本スキンを使用するためには初期設定が必要です．詳しくは\n" +
			"VSD for GPS インストール手順の web ページを参照してください．\n" +
			"(OK を押すと web ブラウザを開きます)",
			undefined,
			MB_OKCANCEL | MB_ICONINFORMATION
		) == IDOK ){
			var WshShell = new ActiveXObject( "WScript.Shell" );
			WshShell.Run( "cmd /c start https://sites.google.com/site/vsdforgps/home/vsd-for-gps/install#GoogleAPIKey" );
		}
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
	BGColor		= 0x80001020;
}

//*** StreetView 描画 *******************************************************

DrawStreetView = function( param ){
	if( Log.Longitude === undefined ) return;
	
	var FrameCnt	= ~~( Vsd.FrameCnt / param.UpdateTime ) * param.UpdateTime;
	var ImgIdx		= ( FrameCnt / param.UpdateTime ) % param.ImgCacheCnt;
	
	if( param.StViewImg === undefined ){
		param.StViewImg = [];
		param.FrameCnt = -99999;
	}
	
	// 初回・シークバーを動かした時に，画像 cache 数だけ先読み
	if(
		param.FrameCnt - ~~Vsd.FramePerSecond > Vsd.FrameCnt ||
		param.FrameCnt + ~~Vsd.FramePerSecond < Vsd.FrameCnt
	){
		for( var i = 0; i < param.ImgCacheCnt; ++i ){
			var Idx = ( ImgIdx + i ) % param.ImgCacheCnt;
			
			if( param.StViewImg[ Idx ] !== undefined ) param.StViewImg[ Idx ].Dispose();
			param.StViewImg[ Idx ] =
				new Image( GetImageURL( FrameCnt + i * param.UpdateTime ), IMG_INET_ASYNC );
		}
		
		param.DispIdx = ( ImgIdx + param.ImgCacheCnt - 1 ) % param.ImgCacheCnt;
		param.FrameCnt = Vsd.FrameCnt + 1;
		
		param.StViewImg[ ImgIdx ].WaitAsyncLoadComplete( 10000 );
	}else{
		++param.FrameCnt;
	}
	
	if( Vsd.IsSaving ){
		// 画像データ取得完了まで待つ
		param.StViewImg[ ImgIdx ].WaitAsyncLoadComplete( 10000 );
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
		if( ImgIdx != param.DispIdx ){
			var DispIdxNext = ( param.DispIdx + 1 ) % param.ImgCacheCnt;
			
			if( param.StViewImg[ DispIdxNext ].Status != IMG_STATUS_LOAD_COMPLETE ){
				// DispIdx + 1 を表示する時間になっても DispIdx + 1 が
				// ロードできていない時は DispIdx + 1 ⇔ DispIdx を swap
				// して，DispIdx を引き続き表示
				
				var tmp = param.StViewImg[ DispIdxNext ];
				param.StViewImg[ DispIdxNext ] = param.StViewImg[ param.DispIdx ];
				param.StViewImg[ param.DispIdx ] = tmp;
			}
			
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
			param.DispIdx = DispIdxNext;
		}
		Vsd.PutImage( param.X, param.Y, param.StViewImg[ param.DispIdx ]);
	}
	//Print( ImgIdx + ":" + param.DispIdx + "\n" );
	
	// 画像 URL 生成
	function GetImageURL( FrameCnt ){
		return Sprintf(
			"http://maps.googleapis.com/maps/api/streetview?sensor=false&size=%dx%d&location=%.6f,%.6f&heading=%d%s",
			param.Width, param.Height,
			Log.ValueOfIndex( "Latitude", FrameCnt ),
			Log.ValueOfIndex( "Longitude", FrameCnt ),
			Log.ValueOfIndex( "Direction", FrameCnt ), GetAPIKey( GoogleAPIKey, "&key=" )
		);
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
		Sprintf( "Date: %04d/%02d/%02d", date.getFullYear(), date.getMonth() + 1, date.getDate()),
		FontM, FontColor
	);
	Y += FontM.Height;
	
	Vsd.DrawText( 0, Y,
		Sprintf( "Time: %02d:%02d:%02d", date.getHours(), date.getMinutes(), date.getSeconds()),
		FontM, FontColor
	);
	Y += FontM.Height;
	
	if( Log.Longitude === undefined ) return;
	
	Vsd.DrawText( 0, Y, Sprintf( "Lat.:%10.5f%s", Math.abs( Log.Latitude ), ( Log.Latitude >= 0 ? 'N' : 'S' )), FontM, FontColor );
	Y += FontM.Height;
	Vsd.DrawText( 0, Y, Sprintf( "Lng.:%10.5f%s", Math.abs( Log.Longitude ), ( Log.Longitude >= 0 ? 'E' : 'W' )), FontM, FontColor );
	Y += FontM.Height;
	Vsd.DrawText( 0, Y, "Alt.: " + ( Log.Altitude !== undefined ? Log.Altitude.toFixed( 1 ) + "m" : "---" ), FontM, FontColor );
	Y += FontM.Height;
	Vsd.DrawText( 0, Y, Sprintf( "Dist.:%6.2fkm", Log.Distance / 1000 ), FontM, FontColor );
	
	if( !Vsd.IsSaving ){
		Y += FontM.Height;
		
		var Cnt = 0;
		for( var i = 0; i < StreetViewParam.ImgCacheCnt; ++i ){
			if( StreetViewParam.StViewImg[ i ].Status == IMG_STATUS_LOAD_COMPLETE ) ++Cnt;
		}
		Vsd.DrawText( 0, Y,
			Sprintf( "%2d/%2d", Cnt, StreetViewParam.ImgCacheCnt ),
			FontM, FontColor
		);
	}
}
