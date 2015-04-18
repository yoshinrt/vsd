//*** 初期化処理 ************************************************************
function Initialize(){
Scale = Vsd.Width / 1920;

// 使用するフォントの宣言
FontR = new Font("メイリオ",82*Scale,FONT_ITALIC|FONT_BOLD );
FontS = new Font("メイリオ",66*Scale,FONT_ITALIC|FONT_BOLD );
FontD = new Font("メイリオ",30*Scale,FONT_BOLD);
FontA = new Font("Verdana",30*Scale);
	
MeterCx = 290*Scale;
MeterCy = 1020*Scale;
MeterR  = 200*Scale;
}

//*** メーター描画処理 ******************************************************

function Draw(){

//画面右上の走行軌跡表示
Vsd.DrawRect(1550*Scale,0*Scale,1920*Scale,410*Scale,0xE66000000,DRAW_FILL);
Vsd.DrawTextAlign(1570*Scale,40*Scale,ALIGN_LEFT|ALIGN_BOTTOM,"Map",FontA,0x55CCCCCC)
Vsd.DrawTextAlign(1900*Scale,40*Scale,ALIGN_RIGHT|ALIGN_BOTTOM,"☄GPS",FontD,0xFF9933)
Vsd.DrawRect(1570*Scale,60*Scale,1900*Scale,390*Scale,0xEFFFFFF)

//VSD for GPSの仕様上、最大で1000秒間(8分40秒)の軌跡しか表示できません
//「VSDメーター合成」ウィンドウの「走行軌跡長さ」で何秒間の軌跡を表示させるか設定できます
//「走行軌跡回転」で0.1度単位で軌跡を回転させることもできます
Vsd.DrawMap(1580*Scale,70*Scale,1890*Scale,380*Scale,ALIGN_BOTTOM|ALIGN_HCENTER|DRAW_MAP_START,3*Scale,4*Scale,0x00FF66,0xFF9933,0xFFA500,0xFFA500);

//メーターの背景を表示
Vsd.DrawCircle(MeterCx,MeterCy,MeterR*1.17,0x33000000,DRAW_FILL)
//画面下部のパネルを表示
Vsd.DrawRect(0,850*Scale,1920*Scale,1080*Scale,0x66000000,DRAW_FILL);

//メーター背景
Vsd.DrawMeterScale(MeterCx,MeterCy,MeterR*1.11,MeterR*0.08,3*Scale,0xCCCCCC,MeterR*0.04,2*Scale,0xAACCCCCC,3,180,0,MeterR*0.78,100000,8,0xFFFFFFFF,FontR);
Vsd.DrawArc(MeterCx,MeterCy,MeterR*1.125,MeterR*1.125,MeterR*1.11,MeterR*1.11,140,40,0xCCCCCC);
Vsd.DrawArc(MeterCx,MeterCy,MeterR*1.07,MeterR*1.07,MeterR*1.06,MeterR*1.06,180,0,0xAACCCCCC)
Vsd.DrawCircle(MeterCx,MeterCy,MeterR*0.88,0x66FF9933,DRAW_FILL)
Vsd.DrawCircle(MeterCx,MeterCy,MeterR*0.87,0xDD000000,DRAW_FILL)
Vsd.DrawCircle(MeterCx,MeterCy,MeterR*0.86,0xBB000000,DRAW_FILL)
Vsd.DrawCircle(MeterCx,MeterCy,MeterR*0.84,0xAA000000,DRAW_FILL)
Vsd.DrawCircle(MeterCx,MeterCy,MeterR*0.82,0xF0000000,DRAW_FILL)
Vsd.DrawCircle(MeterCx,MeterCy,MeterR*0.80,0xA0000000,DRAW_FILL)
Vsd.DrawCircle(MeterCx,MeterCy,MeterR*0.75,0xB0000000,DRAW_FILL)
Vsd.DrawCircle(MeterCx,MeterCy,MeterR*0.70,0xB0000000,DRAW_FILL)
Vsd.DrawTextAlign(MeterCx-110*Scale,MeterCy-60*Scale,ALIGN_LEFT|ALIGN_VCENTER,"Speed",FontA,0x55CCCCCC)


Vsd.DrawLine(600*Scale,965*Scale,1920*Scale,965*Scale,0x000000,4*Scale)
Vsd.DrawLine(600*Scale,854*Scale,600*Scale,1076*Scale,0x000000,4*Scale)
Vsd.DrawLine(1040*Scale,854*Scale,1040*Scale,1076*Scale,0x000000,3*Scale)
Vsd.DrawLine(1480*Scale,854*Scale,1480*Scale,1076*Scale,0x000000,3*Scale)


Vsd.DrawTextAlign(615*Scale,890*Scale,ALIGN_LEFT|ALIGN_BOTTOM,"Average speed",FontA,0x55CCCCCC)
Vsd.DrawTextAlign(1055*Scale,890*Scale,ALIGN_LEFT|ALIGN_BOTTOM,"Distance",FontA,0x55CCCCCC)
Vsd.DrawTextAlign(1495*Scale,890*Scale,ALIGN_LEFT|ALIGN_BOTTOM,"Date",FontA,0x55CCCCCC)
Vsd.DrawTextAlign(615*Scale,1001*Scale,ALIGN_LEFT|ALIGN_BOTTOM,"Duration time",FontA,0x55CCCCCC)
Vsd.DrawTextAlign(1055*Scale,1001*Scale,ALIGN_LEFT|ALIGN_BOTTOM,"Total time",FontA,0x55CCCCCC)
Vsd.DrawTextAlign(1495*Scale,1001*Scale,ALIGN_LEFT|ALIGN_BOTTOM,"Clock",FontA,0x55CCCCCC)


//日付・時刻表示用
date = new Date();
date.setTime(Vsd.DateTime)
yy = date.getYear();
mo = date.getMonth() + 1;
dd = date.getDate();
if (yy < 2000) { yy += 1900; }
day = date.getDay() 
DayTbl = new Array( "日" , "月" , "火" , "水" , "木" , "金" , "土" ) ;
yb = DayTbl[day]

HH = date.getHours();
GG = "午前 "
if (HH > 11) { HH = HH-12,GG="午後 "}
MM = date.getMinutes();	// 分
if (MM < 10) { MM = "0" + MM;}
SS = date.getSeconds();	// 秒
if (SS < 10) { SS = "0" + SS;}

Vsd.DrawTextAlign(1486*Scale,960*Scale,ALIGN_LEFT|ALIGN_BOTTOM,yy+"/"+mo+"/"+dd+" ("+yb+")",FontS,0xFFFFFF)
Vsd.DrawTextAlign(1890*Scale,1075*Scale,ALIGN_RIGHT|ALIGN_BOTTOM,GG+HH+":"+MM+":"+SS,FontS,0xFFFFFF)



MaxTime = new Date()
MaxTime.setTime(Log.Max.Time)
H2 = MaxTime.getUTCHours();
if (H2 < 10) { H2 = "0" + H2;}
M2 = MaxTime.getUTCMinutes();	// 分
if (M2 < 10) { M2 = "0" + M2;}
S2 = MaxTime.getUTCSeconds();	// 秒
if (S2 < 10) { S2 = "0" + S2;}
Vsd.DrawTextAlign(1450*Scale,1075*Scale,ALIGN_RIGHT|ALIGN_BOTTOM,H2+":"+M2+":"+S2,FontS,0xFFFFFF)


ElapsedTime = new Date()
ElapsedTime.setTime(Log.Time)
H3 = ElapsedTime.getUTCHours();
if (H3 < 10) { H3 = "0" + H3;}
M3 = ElapsedTime.getUTCMinutes();	// 分
if (M3 < 10) { M3 = "0" + M3;}
S3 = ElapsedTime.getUTCSeconds();	// 秒
if (S3 < 10) { S3 = "0" + S3;}

Vsd.DrawTextAlign(1010*Scale,1075*Scale,ALIGN_RIGHT|ALIGN_BOTTOM,H3+":"+M3+":"+S3,FontS,0xFFFFFF)


//移動距離表示
Vsd.DrawTextAlign(1450*Scale,960*Scale,ALIGN_RIGHT|ALIGN_BOTTOM,(Log.Distance/1000).toFixed(1)+"km",FontS,0xFFFFFF)

//平均速度表示
Vsd.DrawTextAlign(1010*Scale,960*Scale,ALIGN_RIGHT|ALIGN_BOTTOM,(60*60*Log.Max.Distance/Log.Max.Time).toFixed(1)+"km/h",FontS,0xFFFFFF)


// スピードを表示
Vsd.DrawTextAlign(MeterCx-5*Scale,MeterCy,ALIGN_LEFT|ALIGN_VCENTER,"km/h",FontR,0xFFFFFF);
Vsd.DrawTextAlign(MeterCx-90*Scale,MeterCy,ALIGN_HCENTER|ALIGN_VCENTER,Log.Speed.toFixed(1),FontR,0xFFFFFF);

if( Log.Speed > 1 ) Vsd.DrawArc(MeterCx,MeterCy,MeterR,MeterR,MeterR*0.95,MeterR*0.95,180,180+180*Log.Speed/Log.Max.Speed, 0xFF9933);
//if( Log.Speed < Log.Max.Speed ) Vsd.DrawArc(MeterCx,MeterCy,MeterR,MeterR,MeterR*0.95,MeterR*0.95,180+180*Log.Speed/Log.Max.Speed,0,0xFFFFFF);
//仕切り板の表示
Vsd.DrawMeterScale(MeterCx,MeterCy,MeterR,MeterR*0.05,2.5*Scale,0x000000,MeterR*0.04,2*Scale,0xFFCCCCCC,0,180,0,MeterR*0.78,100000,60,0xFFFFFFFF,FontR);



}
