//*** 設定ここから ***********************************************************

// ギア比の設定  各ギアの エンジン回転[rpm]／スピード[km/h] を設定
GEAR_RATIO1 = 101.9192993;
GEAR_RATIO2 = 61.08177186;
GEAR_RATIO3 = 45.72406922;
GEAR_RATIO4 = 35.95098573;
GEAR_RATIO5 = 29.66828919;
GEAR_RATIO6 = 0;
GEAR_RATIO7 = 0;

// レブリミット
REV_LIMIT = 6500;

//*** 設定ここまで ***********************************************************

Vsd = new __VSD_System__();

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
GRAPH_SPEED			= 1 << 0;
GRAPH_TACHO			= 1 << 1;
GRAPH_GX			= 1 << 2;
GRAPH_GY			= 1 << 3;
GRAPH_HTILE			= 1 << 30;
GRAPH_VTILE			= 1 << 31;

GEAR_RATIO1 = ( GEAR_RATIO1 + GEAR_RATIO2 ) / 2;
GEAR_RATIO2 = ( GEAR_RATIO2 + GEAR_RATIO3 ) / 2;
GEAR_RATIO3 = ( GEAR_RATIO3 + GEAR_RATIO4 ) / 2;
GEAR_RATIO4 = ( GEAR_RATIO4 + GEAR_RATIO5 ) / 2;
GEAR_RATIO5 = ( GEAR_RATIO5 + GEAR_RATIO6 ) / 2;
GEAR_RATIO6 = ( GEAR_RATIO6 + GEAR_RATIO7 ) / 2;

function GetGear( GearRatio ){
	if( GearRatio <= 0 ) return 'N';
	if( GearRatio > GEAR_RATIO1 ) return 1;
	if( GearRatio > GEAR_RATIO2 ) return 2;
	if( GearRatio > GEAR_RATIO3 ) return 3;
	if( GearRatio > GEAR_RATIO4 ) return 4;
	if( GearRatio > GEAR_RATIO5 ) return 5;
	if( GearRatio > GEAR_RATIO6 ) return 6;
	return 7;
}

GlobalInstance = this;

function DisposeAll(){
	if( typeof GlobalInstance[ "Dispose" ] == 'function' ){
		Dispose();
	}
	
	for( var obj in GlobalInstance ){
		if(
			obj != "GlobalInstance" &&
			typeof( GlobalInstance[ obj ] ) == 'object' &&
			typeof( GlobalInstance[ obj ].Dispose ) == 'function'
		){
			GlobalInstance[ obj ].Dispose();
			GlobalInstance[ obj ] = null;
		}
	}
}
