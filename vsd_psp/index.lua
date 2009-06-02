------------------------------------------------------------------------------
--		VSD -- vehicle data logger system  Copyright(C) by DDS
--		$Id$
------------------------------------------------------------------------------
-- .tab=4

--- def, enum
MODE_LAPTIME	= 0
MODE_GYMKHANA	= 1
MODE_ZERO_FOUR	= 2
MODE_ZERO_ONE	= 3
MODE_NUM		= 4

H8HZ			= 16030000
SERIAL_DIVCNT	= 16		-- シリアル出力を行う周期
LOG_FREQ		= 16

-- スピード * 100/Taco 比
-- ELISE
-- ギア比 * マージンじゃなくて，ave( ギアn, ギアn+1 ) に変更
GEAR_RATIO1 = 1.2381712993947
GEAR_RATIO2 = 1.82350889069989
GEAR_RATIO3 = 2.37581451065366
GEAR_RATIO4 = 2.95059529470571

-- たぶん，ホイル一周が30パルス
PULSE_PER_1KM	= 15473.76689	-- ELISE(CE28N)

ACC_1G_X	= 6762.594337
ACC_1G_Y	= 6667.738702
ACC_1G_Z	= 6842.591839

-- シフトインジケータの表示
TachoBar = { 334, 200, 150, 118, 97 }
RevLimit = 6500

-- config
FirmWare			= "vsd_rom.mot"	-- ファームウェア
StartGThrethold		= 500	-- 車がスタートしたとみなすGセンサの数値
GymkhanaStartMargin	= 0.5 * ( PULSE_PER_1KM / 1000 )	-- スタートまでの移動距離
SectorCntMax		= 1		-- マグネット数

--dofile( "sio_emulation.lua" )
dofile( "config.lua" )

--- gloval vars --------------------------------------------------------------

Tacho			= 0
Speed			= 0
Mileage			= 0
MileageWA		= 0
MileageWAPrev	= 0
MileageWACnt	= 0
GSensorY		= 0
GSensorX		= 0
IRSensor		= 0
LogCnt			= 0

-- 画面モード・動作モード
VSDMode	= MODE_LAPTIME
RedrawLap	= 2

-- ラップタイム記録
LapTimeTable	= {}
BestLap			= nil
BestLapDiff		= nil
LapTimePrev		= nil
LapTimeRaw		= 0
SectorCnt		= 0

OS = os.getenv( "OS" )

------------------------------------------------------------------------------
-- Utility -------------------------------------------------------------------
------------------------------------------------------------------------------

--- ファイルリストアップ -----------------------------------------------------

function ListupFiles( Ext )
	local RetFiles = {}
	local files = System.listDirectory()
	
	for i = 1, #files do
		if( files[ i ].name:sub( -#Ext ):lower() == Ext ) then
			RetFiles[ #RetFiles + 1 ] = files[ i ].name
		end
	end
	
	return RetFiles
end

--- キーパッド ---------------------------------------------------------------

Ctrl = {}

Ctrl.Prev = Controls.read()
Ctrl.Now  = Controls.read()

Ctrl.Read = function( this )
	this.Prev = this.Now
	this.Now = Controls.read()
	-- return this.Now:buttons() ~= this.Prev:buttons()
end

Ctrl.Pushed	= function( this, key )
	return ( not this.Prev[ key ]( this.Prev )) and this.Now[ key ]( this.Now )
end

--- config 設定 --------------------------------------------------------------

function SaveConfig()
	fpCfg = io.open( "config.lua", "wb" )
	fpCfg:write(
		"GymkhanaStartMargin=" .. GymkhanaStartMargin .. "\n" ..
		"SectorCntMax=" .. SectorCntMax .. "\n" ..
		'FirmWare="' .. FirmWare .. '"\n'
	)
	fpCfg:close()
end

function SetupMagnet( Cnt )
	SectorCntMax = Cnt
	SaveConfig()
end

function SetupStartDist( Dist )
	GymkhanaStartMargin = Dist * ( PULSE_PER_1KM / 1000 )
	SaveConfig()
end

function SetupFirmware( Name )
	FirmWare = Name
	SaveConfig()
	LoadFirmware()
end

--- ログファイル リオープン --------------------------------------------------

function OpenLog( bAppend )
	if fpLog then fpLog:close() end
	
	if(( not bAppend ) or ( not LogFile )) then
		LogFile = os.date( "vsd%Y%m%d_%H%M%S.log" )
	end
	
	fpLog = io.open( LogFile, "ab" )
	fpLog:setvbuf( "full", 1024 )
	
	LogCnt = 0
end

--- コンソール風出力 ---------------------------------------------------------

Console = {
	Color		= Color.new( 0, 160, 160 ),
	ColorDbg	= Color.new( 255, 255, 255 ),
	x = 0,
	y = 0,
}

Console.print = function( this, str, Color )
	screen:print( this.x, this.y, str, Color or this.Color )
	this.y = this.y + 8
end

Console.DbgPrint = function( this, str, Color )
	if( this.y >= 272 ) then this.y = 0 end
	
	screen.flip()
	Console:print( str, Color or this.ColorDbg )
	screen.flip()
	if( OS ) then
		screen.waitVblankStart()
	end
end

Console.SetPos = function( this, x, y )
	this.x, this.y = x, y
end

Console.Open = function( this, w, h, x, y, Color )
	x = ( x or ( 480 - ( w + 2 ) * 8 ) / 2 / 8 ) * 8
	y = ( y or ( 272 - ( h + 2 ) * 8 ) / 2 / 8 ) * 8
	
	screen:fillRect(
		x, y,
		( w + 2 ) * 8, ( h + 2 ) * 8,
		Color or ColorLapBG
	)
	
	screen:drawRect(
		x + 4, y + 4, ( w + 1 ) * 8, ( h + 1 ) * 8,
		Color or ColorInfo
	)
	
	this:SetPos( x + 8, y + 8 )
end

------------------------------------------------------------------------------
-- Display Driver ------------------------------------------------------------
------------------------------------------------------------------------------

Console:SetPos( 0, 0 )
Console:DbgPrint( "Loading font" );

-- フォント初期化
FontSpeed = Font.createMonoSpaced()
FontSpeed:setPixelSizes( 0, 100 )
ColorSpeed = Color.new( 102, 255, 255 )
ColorMeter = ColorSpeed

FontHist = Font.createMonoSpaced()
FontHist:setPixelSizes( 22, 25 )
ColorHist = ColorSpeed

FontLap = Font.createMonoSpaced()
FontLap:setPixelSizes( 0, 45 )
ColorLap = ColorSpeed
ColorLapBad = Color.new( 255, 80, 0 )

ColorMenuCursor = ColorLapBad

-- 背景ロード
Console:DbgPrint( "Loading image" );

fpImg = io.open( "vsd_tacho.png", "rb" ); ImgData = fpImg:read( "*a" ); fpImg:close()
ImageTacho = Image.loadFromMemory( ImgData )

ImageSpeed = {}

for i = 1, 5 do
	-- Console:DbgPrint( "Loading speed image" .. i );
	fpImg = io.open( "vsd_speed" .. i .. ".png", "rb" ); ImgData = fpImg:read( "*a" ); fpImg:close()
	ImageSpeed[ i ] = Image.loadFromMemory( ImgData )
end

-- Console:DbgPrint( "Loading G image" );
fpImg = io.open( "vsd_g.png", "rb" ); ImgData = fpImg:read( "*a" ); fpImg:close()
ImageG = Image.loadFromMemory( ImgData )

-- 画面パラメータ
TachoCx			= 60
TachoCy			= 60
TachoMeterR		= 48
TachoMeterStart	= math.pi / 4
TachoMeterMaxRev= 8000

GMeterCx	= 366 + 106 / 2 - ( 480 - ImageG:width())
GMeterCy	= TachoCy
GMeterIndicatorSize	= 2
GMeterR		= 106 / 4

SpeedY		= 38
SpeedH		= 113
FontSize	= FontSpeed:getTextSize( "888" )
SpeedX		= 240 - FontSize.width / 2
SpeedY		= SpeedH / 2 + SpeedY + FontSize.height / 2

HistX	= 22
HistY	= 181
HistW	= 174
HistH	= 84

LapX	= 219
LapY	= HistY
LapW	= 242
LapH	= HistH

LapDiffX	= LapX + ( LapW - FontHist:getTextSize( '-01"28.555' ).width ) / 2
LapClockX	= LapX + ( LapW - FontHist:getTextSize( '12:34' ).width )
ColorLapBG	= Color.new( 51, 51, 51 )
RefreshFlag = nil

ColorInfo	= Color.new( 0, 160, 160 )

LapChartW	= 58
LapChartH	= 32

--- Image 拡張 ---------------------------------------------------------------

Image.drawRect = function( this, x, y, w, h, Color )
	this:drawLine( x,		y,		x + w,	y,		Color )
	this:drawLine( x,		y,		x,		y + h,	Color )
	this:drawLine( x + w,	y,		x + w,	y + h,	Color )
	this:drawLine( x,		y + h,	x + w,	y + h,	Color )
end

screen.drawRect = Image.drawRect

--- ラップタイムウィンドウ描画 -----------------------------------------------

function DrawLap()
	local str
	local Color
	
	-- ラップタイム履歴
	screen:clear()
	screen:fillRect( HistX, HistY, HistW, HistH, ColorLapBG )
	screen:fillRect( LapX, LapY, LapW, LapH, ColorLapBG )
	
	if( BestLap ) then
		str = 'Fst ' .. FormatLapTime( BestLap )
	else
		str = 'Fst --"--.---', ColorHist
	end
	screen:fontPrint( FontHist, HistX, HistY + HistH / 3 * 1, str, ColorHist )
	
	if( #LapTimeTable >= 2 ) then
		screen:fontPrint(
			FontHist, HistX, HistY + HistH / 3 * 2,
			string.format( "%3d ", #LapTimeTable - 1 ) .. FormatLapTime( LapTimeTable[ #LapTimeTable - 1 ] ), ColorHist
		)
	end
	
	if( #LapTimeTable >= 3 ) then
		screen:fontPrint(
			FontHist, HistX, HistY + HistH / 3 * 3,
			string.format( "%3d ", #LapTimeTable - 2 ) .. FormatLapTime( LapTimeTable[ #LapTimeTable - 2 ] ), ColorHist
		)
	end
	
	-- ラップタイム
	if( #LapTimeTable > 0 ) then
		str = FormatLapTime( LapTimeTable[ #LapTimeTable ] )
	else
		str = '--"--.---'
	end
	screen:fontPrint( FontLap, LapX, LapY + LapH / 3 * 1.2, str, ColorLap )
	
	if( BestLapDiff == nil ) then
		str = '  -"--.---'
		Color = ColorLap
	elseif( BestLapDiff < 0 ) then
		str = "-" .. FormatLapTime( -BestLapDiff )
		Color = ColorLap
	else
		str = "+" .. FormatLapTime( BestLapDiff )
		Color = ColorLapBad
	end
	screen:fontPrint( FontHist, LapDiffX, LapY + LapH / 3 * 2, str, Color )
	
	-- その他 info 描画
	if    ( VSDMode == MODE_LAPTIME		) then str = "LAP"
	elseif( VSDMode == MODE_GYMKHANA	) then str = "GYMKA"
	elseif( VSDMode == MODE_ZERO_FOUR	) then str = "0-400 "
	elseif( VSDMode == MODE_ZERO_ONE	) then str = "0-100 "
	end
	str = str .. #LapTimeTable
	if( LapTimePrev == nil ) then str = str .. " RDY" end
	
	screen:fontPrint( FontHist, LapX, LapY + LapH, str, ColorInfo )
	screen:fontPrint( FontHist, LapClockX, LapY + LapH, os.date( "%k:%M" ), ColorInfo )
end

--- メーター類描画 -----------------------------------------------------------

Blink = nil

function DrawMeters()
	local BarLv
	local Gear
	local GearRatio
	
	-- ギアを求める
	GearRatio = Speed / Tacho
	if    ( GearRatio < GEAR_RATIO1 ) then Gear = 1
	elseif( GearRatio < GEAR_RATIO2 ) then Gear = 2
	elseif( GearRatio < GEAR_RATIO3 ) then Gear = 3
	elseif( GearRatio < GEAR_RATIO4 ) then Gear = 4
	else								   Gear = 5
	end
	
	-- スピードメーター
	if(( Speed >= 30000 ) and ( Tacho == 0 )) then
		-- キャリブレーション表示
		BarLv = 5
		Blink = nil
	else
		-- LED の表示 LV を求める
		BarLv = math.floor(( Tacho - RevLimit ) / TachoBar[ Gear ] ) + 5
		
		if( BarLv >= 5 ) then
			BarLv = 5
			if( Blink ) then BarLv = 1; end
			Blink = not Blink
		elseif( BarLv < 1 ) then
			BarLv = 1
			Blink = nil
		else
			Blink = nil
		end
	end
	
	screen:blit( ImageTacho:width(), 0, ImageSpeed[ BarLv ] )
	screen:fontPrint( FontSpeed, SpeedX, SpeedY, string.format( "%3d", Speed / 100 ), ColorSpeed )
	
	-- タコメータの描画
	screen:blit( 0, 0, ImageTacho )
	TachoRad = Tacho / TachoMeterMaxRev * 2 * math.pi + TachoMeterStart
	screen:drawLine(
		TachoCx, TachoCy,
		TachoCx + TachoMeterR * math.cos( TachoRad ),
		TachoCy + TachoMeterR * math.sin( TachoRad ),
		ColorMeter
	)
	screen:print( TachoCx + 20, TachoCy + 10, string.format( "%4d", Tacho ), ColorMeter )
	
	-- Gセンサ描画
	
	if( GxTmp ) then
		ImageG:fillRect(
			GxTmp, GyTmp,
			GMeterIndicatorSize,
			GMeterIndicatorSize,
			ColorLapBG
		)
	end
	
	if( GSensorCaribCnt == 0 ) then
		GxTmp, GyTmp =
			GMeterCx + GSensorX * GMeterR - GMeterIndicatorSize / 2,
			GMeterCy - GSensorY * GMeterR - GMeterIndicatorSize / 2
		
		ImageG:fillRect(
			GxTmp, GyTmp,
			GMeterIndicatorSize,
			GMeterIndicatorSize,
			ColorMeter
		)
	end
	screen:blit( 480 - ImageG:width(), 0, ImageG )
	
	-- その他の情報
	if( bDispInfo ) then
		Console:Open( 10, 4, 47, 15 )
	--	Console:print( os.date( "%y/%m/%d" ))
		Console:print( string.format( "%8.3fkm", Mileage / PULSE_PER_1KM ))
		Console:print( string.format( "Sector:%d", SectorCnt ))
		Console:print( string.format( "GPS:%d", GPS_Valid ))
	end
end

--- ラップチャート表示 -------------------------------------------------------

function DrawLapChart()
	local Color
	
	Console:Open( LapChartW, LapChartH )
	Console:print( "Lap  Time" )
	Console:print( "---------------" )
	local y = Console.y
	
	if( #LapTimeTable > 0 ) then
		for i = 1, #LapTimeTable do
			
			if( BestLap and LapTimeTable[ i ] == BestLap ) then
				Color = ColorLapBad
			else
				Color = ColorInfo
			end
			
			Console:print(
				string.format( "%3d %s", i, FormatLapTime( LapTimeTable[ i ] )), Color
			)
			if( math.fmod( i, LapChartH - 2 ) == 0 ) then
				Console:SetPos( Console.x + 15 * 8, y )
			end
		end
	else
		Console:print( "No results." )
	end
	
	screen.flip()
	while( not Ctrl:Pushed( "cross" )) do
		DoIntervalProc()
		Ctrl:Read()
	end
	RedrawLap = 2
end

------------------------------------------------------------------------------
-- VSD HW Driver -------------------------------------------------------------
------------------------------------------------------------------------------

RxBuf	= ""

-- Gセンサキャリブレーション

GSensorCaribCntMax = 15
GSensorCaribCnt = GSensorCaribCntMax
GSensorCx	= 0
GSensorCy	= 0

-- サウンドロード

Console:DbgPrint( "Loading sound" );
SndBestLap = Sound.load( "best_lap.wav" )
SndNewLap  = Sound.load( "new_lap.wav" )

--- load firmware ------------------------------------------------------------

function LoadFirmware()
	
	local pos
	
	-- ログファイル リオープン
	if not Controls.read():r() then
		OpenLog()
	end
	
	-- SIO 初期化
	Console:DbgPrint( ".Init SIO" )
	
	if( not bSIOActive ) then
		System.sioInit( 38400 )
		bSIOActive = true
	end
	
	-- firmware ロード
	
	repeat
		local fpFirm = io.open( FirmWare, "rb" )
		
		if( fpFirm ) then
			Console:DbgPrint( ".Transferring firmware" )
			
			System.sioWrite( "z\r" )
			screen.waitVblankStart( 6 )
			System.sioWrite( "l\r" )
			
			System.sioWrite( string.gsub( fpFirm:read( "*a" ), "\r\n", "\r" ))
			fpFirm:close()
		end
		
		screen.waitVblankStart( 6 )
		System.sioWrite( "g\r" )
		
		-- バッファクリア
		System.sioRead()
		RxBuf = ""
		
		-- オープニングメッセージスキップ
		local TimeoutCnt = 120
		Console:DbgPrint( ".waiting sync. code" )
		
		repeat
			RxBuf = System.sioRead()
			pos = RxBuf:find( "\255", 1, true )
			TimeoutCnt = TimeoutCnt - 1
			
			if( TimeoutCnt == 0 ) then
				Console:DbgPrint( ".Timeout. Retrying... [X] to cancel" )
				if( Controls.read():cross()) then return false end
				
				pos = 0	-- 内側ループを抜ける手段
			end
			
			screen.waitVblankStart( 1 )
		until pos
	until pos > 0
	
	RxBuf = RxBuf:sub( pos + 1 )
	
	-- VSD モード設定
	System.sioWrite( "s1a" )
	
	return true
end

--- FormatLapTime ------------------------------------------------------------

function FormatLapTime( Time, Ch )
	return string.format(
		'%2d%s%06.3f',
		math.floor( Time / 60 ),
		( Ch or '"' ),
		math.fmod( Time, 60 )
	)
end

--- シリアルデータ→16bit 数値復元 -------------------------------------------

RxBufPos = 1

function SerialUnpack( str )
	local Ret
	
	if( str:byte( RxBufPos ) == 0xFE ) then
		RxBufPos = RxBufPos + 1
		Ret = str:byte( RxBufPos ) + 0xFE
	else
		Ret = str:byte( RxBufPos )
	end
	RxBufPos = RxBufPos + 1
	
	if( str:byte( RxBufPos ) == 0xFE ) then
		RxBufPos = RxBufPos + 1
		Ret = Ret + ( str:byte( RxBufPos ) + 0xFE ) * 256
	else
		Ret = Ret + str:byte( RxBufPos ) * 256
	end
	RxBufPos = RxBufPos + 1
	
	return Ret
end

--- シリアルデータ処理 -------------------------------------------------------

function ProcessSio()
	
	local LapTimeStr = ""
	
	if( RxBufPos == 1 ) then
		-- 処理残りデータが無いので，Sio からリードする
		RxBuf = RxBuf .. System.sioRead()
		
		-- \xFF を検索，無ければ何もせず return
		LogPos = RxBuf:find( "\255", 1, true )
		if( not LogPos ) then return end
		
		RxBufPos = 1
		Tacho = SerialUnpack( RxBuf )
		Speed = SerialUnpack( RxBuf )
		
		RefreshFlag			= true
		
		-- 動いたらログ記録開始  carib でも開始するはず
		if( Speed > 0 ) then bStartLog = true end
		
		return
	end
	
	-- Speed/Tacho は処理済，残りを処理する
	
	MileageWAPrev = MileageWA
	
	MileageWA = SerialUnpack( RxBuf )
	IRSensor  = SerialUnpack( RxBuf )
	GSensorX  = SerialUnpack( RxBuf )
	GSensorY  = SerialUnpack( RxBuf )
	
	if( MileageWAPrev > MileageWA ) then
		MileageWACnt = MileageWACnt + 1
	end
	
	Mileage = MileageWA + MileageWACnt * 0x10000
	
	-- G センサー処理 --------------------------------------------------------
	
	if( GSensorCaribCnt > 0 ) then
		GSensorCaribCnt = GSensorCaribCnt - 1
		GSensorCx = GSensorCx + GSensorX
		GSensorCy = GSensorCy + GSensorY
		
		if( GSensorCaribCnt == 0 ) then
			GSensorCx = GSensorCx / GSensorCaribCntMax
			GSensorCy = GSensorCy / GSensorCaribCntMax
		end
		
		GSensorX	= 0
		GSensorY	= 0
	else
		GSensorX	= -( GSensorX - GSensorCx ) / ACC_1G_Y
		GSensorY	=  ( GSensorY - GSensorCy ) / ACC_1G_Z
	end
	
	-- ラップタイム処理 ------------------------------------------------------
	
	if( RxBuf:byte( RxBufPos ) ~= 0xFF ) then
		local LapTime = SerialUnpack( RxBuf )
		local tmp     = SerialUnpack( RxBuf )
		
		LapTime = LapTime + tmp * 0x10000
		local LapTimeDiff
		
		SectorCnt = SectorCnt + 1
		if SectorCnt >= SectorCntMax then
			SectorCnt = 0
			
			-- チェックポイントを通過済みならば，周回タイムを求める
			local bBestLap = false
			
			if( LapTimePrev ) then
				LapTimeDiff = ( LapTime - LapTimePrev ) / 256
				LapTimeTable[ #LapTimeTable + 1 ] = LapTimeDiff
				LapTimeStr = "\tLAP" .. #LapTimeTable .. " " .. FormatLapTime( LapTimeDiff, ':' )
				-- ベストラップか?
				if( BestLap ) then BestLapDiff = LapTimeDiff - BestLap end
				if( BestLap == nil or LapTimeDiff < BestLap ) then
					if( BestLap ) then
						bBestLap = true
					end
					BestLap = LapTimeDiff
				end
			else
				-- スタートラインを始めて通過したので，マーカー出力
				LapTimeStr = "\tLAP" .. ( #LapTimeTable + 1 ) .. " start"
			end
			
			LapTimePrev = LapTime
			
			if( bBestLap ) then
				-- ベストラップサウンド
				SndBestLap:play()
			else
				-- ラップサウンド
				SndNewLap:play()
			end
			
			RedrawLap = 2
		elseif( LapTimePrev ) then
			-- セクター通過
			LapTimeDiff = ( LapTime - LapTimePrev ) / 256
			LapTimeStr = "\tSector" .. SectorCnt .. " " .. FormatLapTime( LapTimeDiff, ':' )
		end
	end
	
	-- ログに改行が付いたので，可視化ログに出力 ------------------------------
	
	--if( type( NoSio ) ~= "string" ) then
	if( fpLog and bStartLog ) then
		-- テキストログ
		fpLog:write( string.format(
			"%u\t%.2f\t%.2f\t%.4f\t%.4f\t%u",
			Tacho, Speed / 100, Mileage / PULSE_PER_1KM * 1000,
			GSensorY, GSensorX, IRSensor
		))
		
		-- GPS ログ
		if( GetGPSData()) then
			fpLog:write( string.format(
				"\tGPS\t%.10f\t%.10f\t%.10f\t%.10f",
				GPS_Lati,
				GPS_Long,
				GPS_Speed,
				GPS_Bearing
			))
		end
		
		-- ラップタイム
		fpLog:write( LapTimeStr .. "\r\n" )
		
		LogCnt = LogCnt + 1
	end
	
	--DebugRefresh = DebugRefresh + 1
	
	RxBuf = RxBuf:sub( LogPos + 1 )
	RxBufPos = 1
end

--- VSD モード設定 -----------------------------------------------------------

function SetVSDMode( mode )
	if( bSIOActive ) then
		mode = math.fmod( mode + MODE_NUM, MODE_NUM )
		if( NoSio ) then fpLog:write( string.format( "%d-->%d\n", VSDMode, mode )) end
		
		if( mode == MODE_LAPTIME ) then
			System.sioWrite( "l" )
		elseif( mode == MODE_GYMKHANA	) then
			System.sioWrite( string.format( "%Xg", GymkhanaStartMargin + 0.5 ))
		elseif( mode == MODE_ZERO_FOUR	) then
			System.sioWrite( string.format( "%Xf", StartGThrethold ))
		elseif( mode == MODE_ZERO_ONE	) then
			System.sioWrite( string.format( "%Xo", StartGThrethold ))
		end
		
		LapTimePrev = nil
		RedrawLap = 2
		SectorCnt = 0
	end
	
	return mode
end

--- シリアルアクティブ確認 ---------------------------------------------------

bSIOActive = false

------------------------------------------------------------------------------
--- メニュー処理 -------------------------------------------------------------
------------------------------------------------------------------------------

--- Delete bestlap 表示 ------------------------------------------------------

function DeleteLap()
	if( BestLap == nil ) then return end
	
	-- 最速ラップ削除
	local NewBestLap = nil
	
	for i = 1, #LapTimeTable do
		if( LapTimeTable[ i ] == BestLap ) then
			LapTimeTable[ i ] = 599.999
		elseif( NewBestLap == nil or ( LapTimeTable[ i ] < NewBestLap and LapTimeTable[ i ] < 599 )) then
			NewBestLap = LapTimeTable[ i ]
		end
	end
	BestLap = NewBestLap
end

--- toggle info window -------------------------------------------------------

function ToggleInfo()
	bDispInfo = not bDispInfo
end

--- VSD スペシャルコマンド ---------------------------------------------------

function SendCmd_IR()
	System.sioWrite( "0ai" )
end

function SendCmd_Mileage()
	System.sioWrite( "0aM" )
end

--- メニュー -----------------------------------------------------------------

function DoMenu( Item, x, y )
	local MenuID = 1
	local Color
	local BreakMenu
	local left
	local top
	
	BreakMenu = false
	if(( y ) and ( 32 - #Item < y )) then
		y = 32 - #Item
	end
	
	left = x
	top  = y
	
	while( not BreakMenu ) do
		screen.flip()
		Console:Open( Item.width, #Item, x, y )
		if( left == nil ) then
			left = Console.x / 8
			top  = Console.y / 8
		end
		
		-- タイトル
		--if( type( Item.title ) == "string" ) then
		--	Console.y = Console.y - 8
		--	Console:print( Item.title )
		--end
		
		-- アイテム
		
		for i = 1, #Item do
			Color = nil
			if( i == MenuID ) then Color = ColorMenuCursor end
			
			if( type( Item[ i ] ) == "table" ) then
				Console:print( Item[ i ].title, Color )
			else
				Console:print( Item[ i ], Color )
			end
		end
		
		screen.flip()
		while( 1 ) do
			DoIntervalProc()
			Ctrl:Read()
			
			if( Ctrl:Pushed( "up" )) then
				MenuID = MenuID - 1
				if( MenuID <= 0 ) then MenuID = #Item end
				break
			elseif( Ctrl:Pushed( "down" )) then
				MenuID = MenuID + 1
				if( MenuID > #Item ) then MenuID = 1 end
				break
			elseif( Ctrl:Pushed( "circle" )) then
				if( type( Item[ MenuID ] ) == "table" ) then
					if( type( Item[ MenuID ][ 1 ] ) == "function" ) then
						-- 終端 function 呼び出し
						Item[ MenuID ][ 1 ]()
					elseif( not DoMenu( Item[ MenuID ], left + 1, top + MenuID )) then
						-- サブメニュー展開
						break	-- サブが×なので，このメニューを再開
					end
				elseif( type( Item.proc ) == "function" ) then
					-- 共通 proc 呼び出し
					Item.proc( Item[ MenuID ] )
				end
				do return true end
			elseif( Ctrl:Pushed( "cross" )) then
				do return false end
			end
		end
	end
end

-- firm リストアップ

FirmList		= ListupFiles( ".mot" )
FirmList.title	= "Firmware"
FirmList.width	= 15
FirmList.proc	= SetupFirmware

MainMenu = {
	title = "Main menu";
	width = 20;
	{
		title = "Magnet setting";
		width = 5;
		proc = SetupMagnet;
		1, 2, 3, 4, 5
	},
	{
		title = "Start distance";
		width = 5;
		proc = SetupStartDist;
		     0.1, 0.2, 0.3, 0.4, 0.5, 0.6, 0.7, 0.8, 0.9,
		1.0, 1.1, 1.2, 1.3, 1.4, 1.5, 1.6, 1.7, 1.8, 1.9,
		2.0, 2.1, 2.2, 2.3, 2.4, 2.5, 2.6, 2.7, 2.8, 2.9,
		3.0
	},
	
	FirmList,
	
	{ title = "Re-open log";	OpenLog },
	{
		title = "Delete fastest lap";
		width = 13;
		{
			title = "O:ok X:cancel";
			DeleteLap
		}
	},
	{ title = "Toggle info window";	ToggleInfo },
	{
		title = "Special command";
		width = 20;
		{ title = "Disp IR cnt";  SendCmd_IR },
		{ title = "Disp mileage"; SendCmd_Mileage },
	},
	{
		title = "Help";
		width = 20;
	--	"UP:      delete best lap",
		"DOWN:    lap chart",
		"LEFT:    change mode",
		"RIGHT:   change mode",
	--	"LTRIG:",
		"RTRIG:   restart",
		"CIRCLE:  main menu",
		"TRIANGLE:calibration",
		"START:   exit",
	--	"SELECT:  this help",
	--	"-------",
	--	"OS:" .. ( OS or "PSP" ),
	}
}

------------------------------------------------------------------------------
--- GPS ----------------------------------------------------------------------
------------------------------------------------------------------------------

if( not UsbGps ) then
	UsbGps = {}
	UsbGps.open			= function() return 0 end
	UsbGps.close		= function() end
	UsbGps.get_data		= function() return 0 end
	UsbGps.set_init_loc	= function() end
end

GPS_PrevSec = 99;
GPS_Valid = 0;

function GetGPSData()
	
	local tmp
	
	GPS_Valid,		-- valid
	tmp,			-- year
	tmp,			-- month
	tmp,			-- date
	tmp,			-- hour
	tmp,			-- minute
	GPS_Second,		-- second
	GPS_Lati,		-- lati
	GPS_Long,		-- long
	tmp,			-- altitude
	GPS_Speed,		-- speed
	GPS_Bearing		-- bearing
	= UsbGps.get_data()
	
	if( GPS_Valid >= 1 and GPS_PrevSec ~= GPS_Second ) then
		-- GPS データ更新
		GPS_PrevSec = GPS_Second
		return true
	end
	
	return false
end

------------------------------------------------------------------------------
--- メインループ -------------------------------------------------------------
------------------------------------------------------------------------------

-- 一定時間ごとに処理するルーチン --------------------------------------------

function DoIntervalProc()
	if( OS ) then screen.waitVblankStart() end
	
	if( bSIOActive ) then
		-- シリアルデータ処理
		ProcessSio()
		
		-- ログサイズが既定を上回ったら，autosave 用ログファイル リオープン
		-- ただし画面更新時以外
		if( RefreshFlag == nil and fpLog and LogCnt >= 60 * LOG_FREQ ) then
			OpenLog( true )
		end
	elseif( GetGPSData()) then
		-- GPS のデータで，スピード表示更新
		Tacho	= Tacho + GPS_Valid
		Speed	= math.floor( GPS_Speed * 100 + 0.5 )
		RefreshFlag = true
	end
end

-- メイン処理 ----------------------------------------------------------------

Console:DbgPrint( "Loading firmware" );
-- sio 初期化・ファームロード
if( NoSio ) then
	-- ログファイル リオープン
	LogFile = "vsd.log"
	fpLog = io.open( os.date( LogFile ), "wb" )
	bSIOActive = true
elseif not Controls.read():l() then
	if( not LoadFirmware()) then
		-- return	-- Fail しても通常画面に戻って，config できるようにする
	end
end

-- DebugRefresh = 0
CtrlPrev = Controls.read()
PrevMin = 99

Console:DbgPrint( "Init GPS" );
UsbGps.open()
UsbGps.set_init_loc( 0 )

while true do
	DoIntervalProc()
	
	if( RefreshFlag or RedrawLap > 0 ) then
		-- 通常の画面処理
		if( PrevMin ~= os.date( "*t" ).min ) then
			-- 時間更新
			PrevMin = os.date( "*t" ).min
			RedrawLap = 2
		end
		
		RefreshFlag = nil
		-- DebugRefresh = DebugRefresh - 1
		
		if( RedrawLap > 0 ) then
			DrawLap()
			RedrawLap = RedrawLap - 1
		end
		DrawMeters()
		screen:flip()
	else
		Ctrl:Read()
		if Ctrl:Pushed( "r" ) then
			-- リスタート
			SetVSDMode( VSDMode )
		elseif Ctrl:Pushed( "right" ) then
			VSDMode = SetVSDMode( VSDMode + 1 )
		elseif Ctrl:Pushed( "left" ) then
			VSDMode = SetVSDMode( VSDMode - 1 )
		elseif Ctrl:Pushed( "down" ) then
			-- ラップチャート
			DrawLapChart()
		elseif Ctrl:Pushed( "circle" ) then
			DoMenu( MainMenu )
			RedrawLap = 2
		elseif Ctrl:Pushed( "triangle" ) then
			-- calibration
			if( bSIOActive ) then System.sioWrite( "c" ) end
		elseif Ctrl:Pushed( "start" ) then
			break
		end
	end
end

if( fpLog ) then
	fpLog:close()
	fpLog = nil
end
UsbGps.close()
