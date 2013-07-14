//*** 設定ここから ***********************************************************

// ギア比の設定  各ギアの エンジン回転[rpm]／スピード[km/h] を設定
GEAR_RATIO = [
	101.9192993,	// 1速
	61.08177186,	// 2速
	45.72406922,	// 3速
	35.95098573,	// 4速
	29.66828919,	// 5速  6速以降がある場合は同じように追加
];

REV_LIMIT		= 6500;	// レブリミット (上限)

//*** 設定ここまで ***********************************************************

Vsd = new __VSD_System__( __CVsdFilter );

DRAW_FILL			= 1;
FONT_BOLD			= 1 << 0;
FONT_ITALIC			= 1 << 1;
FONT_OUTLINE		= 1 << 2;
FONT_FIXED			= 1 << 3;
FONT_NOANTIALIAS	= 1 << 4;
ALIGN_LEFT			= 0;
ALIGN_TOP			= 0;
ALIGN_HCENTER		= 1 << 0;
ALIGN_RIGHT			= 1 << 1;
ALIGN_VCENTER		= 1 << 2;
ALIGN_BOTTOM		= 1 << 3;
DRAW_MAP_START		= 1 << 4;
GRAPH_HTILE			= 1 << 30;
GRAPH_VTILE			= 1 << 31;
IMG_INET_ASYNC				= 1 << 0;
IMG_STATUS_LOAD_COMPLETE	= 0;
IMG_STATUS_LOAD_INCOMPLETE	= 1;
IMG_STATUS_LOAD_FAILED		= 2;
SEEK_SET	= 0;
SEEK_CUR	= 1;
SEEK_END	= 2;

function GetGear( GearRatio ){
	var Gear;
	for( Gear = 1; Gear < GEAR_RATIO.length; ++Gear ){
		if( GearRatio > ( GEAR_RATIO[ Gear - 1 ] + GEAR_RATIO[ Gear ] ) / 2 ){
			break;
		}
	}
	return Gear;
}

GlobalInstance = this;

function DisposeAll(){
	if( typeof GlobalInstance[ "Dispose" ] == 'function' ){
		Dispose();
	}
	
	for( var obj in GlobalInstance ){
		if(
			obj != "GlobalInstance" &&
			typeof GlobalInstance[ obj ] == 'object' &&
			typeof GlobalInstance[ obj ].Dispose == 'function'
		){
			GlobalInstance[ obj ].Dispose();
			GlobalInstance[ obj ] = null;
		}
	}
}

//*** グラフ描画 *************************************************************

Vsd.MakeGraphParam = function( params ){
	for( var i = 0; i < params.length; ){
		if( Vsd[ params[ i ]] === undefined ){
			params.splice( i, 3 );	// データがない要素を削除
		}else{
			i += 3;
		}
	}
}

Vsd.DrawGraph = function( x1, y1, x2, y2, font, flags, params ){
	// グラフを表示しない条件
	if( !Vsd.Config_graph ) return;
	
	var GrpNum = ~~( params.length / 3 );
	var Width  = x2 - x1 + 1;
	var Height = y2 - y1 + 1;
	
	var X1 = x1;
	var X2 = ( flags & GRAPH_HTILE ) ? x1 - 1 : x2;
	var Y1 = y1;
	var Y2 = ( flags & GRAPH_VTILE ) ? y1 - 1 : y2;
	
	for( var i = 0; i < GrpNum; ++i ){
		if( flags & GRAPH_HTILE ){
			X1 = X2 + 1;
			X2 = x1 - 1 + ~~( Width * ( i + 1 ) / GrpNum );
		}else if( flags & GRAPH_VTILE ){
			Y1 = Y2 + 1;
			Y2 = y1 - 1 + ~~( Height * ( i + 1 ) / GrpNum );
		}
		
		Vsd.DrawGraphSingle(
			X1, Y1, X2, Y2,
			params[ i * 3 + 0 ],	// key
			params[ i * 3 + 1 ],	// unit
			font,					// font
			params[ i * 3 + 2 ]		// color
		);
	}
}

//*** Google Map 描画 ********************************************************

Vsd.DrawGoogleMaps = function( param ){
	if( Vsd.Longitude === undefined ) return;
	
	GMapURL = "http://maps.googleapis.com/maps/api/staticmap?sensor=false&language=ja" +
		( param.APIKey != '' ? "&key=" + param.APIKey : '' ) +
		"&maptype=" + param.Maptype +
		"&zoom=" + param.Zoom +
		"&size=" + Math.floor( param.Width ) + "x" + Math.floor( param.Height ) +
		"&center=";
	
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
	
	Vsd.PutImage( param.X, param.Y, param.MapImg );
	
	DrawArrow(
		param.X + param.Width  / 2 + PixLong,
		param.Y + param.Height / 2 + PixLati,
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
