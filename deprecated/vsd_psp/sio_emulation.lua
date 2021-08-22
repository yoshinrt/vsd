------------------------------------------------------------------------------
--		VSD -- vehicle data logger system  Copyright(C) by DDS
--		$Id$
------------------------------------------------------------------------------
-- .tab=4
--- ダミーシリアル入力 -------------------------------------------------------

-- シリアルポートなし (debug)
NoSio = "vsd20090523_145505.log"
-- NoSio = true

DummySioTimer = Timer.new()
DummySioTimer:start()
NextSioTime = 0

TSC = Timer.new() TSC:start()
--DebugPrevKey = 0

function SerialPack( num )
	num = math.fmod( math.floor( num ), 65536 )
	return string.char( math.fmod( num, 256 ), math.floor( num / 256 ))
end

if( type( NoSio ) == "string" ) then
	fpIn = io.open( NoSio, "r" )
	assert( fpIn, "Can't open file:" .. NoSio )
end

-- ログ再生
System.sioRead = function ()
	local Ret = ""
	local Line = nil
	local Params = {}
	
	if( DummySioTimer:time() >= NextSioTime ) then
		NextSioTime = NextSioTime + 1000 / LOG_FREQ
		
		if( fpIn ) then
			-- ログファイルから数値を取得
			repeat
				-- 行頭が数値でなければ，有効な行ではない
				repeat
					Line = fpIn:read()
				until( 0x30 <= Line:byte() and Line:byte() <= 0x39 )
				
				-- 
				Params = {}
				for w in Line:gmatch( "[^%s]+" ) do
					Params[ #Params + 1 ] = tonumber( w )
				end
			until( Params[ 2 ] ~= 0 )
			
			Params[ 6 ] = nil
			
			if( Params[ 4 ] < 10 ) then
				Params[ 4 ] =  Params[ 4 ] * ACC_1G_Z + 32000;
				Params[ 5 ] = -Params[ 5 ] * ACC_1G_Y + 32000;
			end
			
			-- ラップタイムあり?
			local result, tmp, min, sec = Line:find( "LAP.*(%d+):([%d%.]+)" )
			if( result ) then
				LapTimePrev = 0
				Params[ 6 ] = ( min * 60 + sec ) * 256
			elseif( Line:find( "LAP" )) then
				Params[ 6 ] = 0
			end
		else
			-- ログファイルはないので，自動ジェネレート
			Params[ 1 ] = math.fmod( TSC:time(), 6800 ) / 2 + 3400
			Params[ 2 ] = math.fmod( TSC:time() / 100, 200 )
			Params[ 3 ] = math.fmod( TSC:time(), 65536 )
			Params[ 4 ] = 32000 + 6000 * math.sin( TSC:time() / 150 * 17 )
			Params[ 5 ] = 32000 + 6000 * math.cos( TSC:time() / 150 * 23 )
			
			if( not DebugPrevKey and Controls.read():l()) then
				Params[ 6 ] = TSC:time() / 1000 * 256
			end
		end
		
		Ret =
			SerialPack( Params[ 1 ] ) ..
			SerialPack( Params[ 2 ] * 100 ) ..
			SerialPack( math.fmod( Params[ 3 ] / 1000 * PULSE_PER_1KM, 65536 )) ..
			SerialPack( 0 ) ..
			SerialPack( Params[ 5 ] ) ..
			SerialPack( Params[ 4 ] )
		
		-- ラップタイムあり?
		
		if( Params[ 6 ] ) then
			Ret = Ret .. SerialPack( Params[ 6 ] ) .. "\0\0"
		end
		
		-- 0xFE, 0xFF を 0xFE, 0x00/01 に置換
		Ret = Ret:gsub( "\254", "\254\0" )
		Ret = Ret:gsub( "\255", "\254\1" )
		
		return Ret .. "\255"
	end
	
	return ""
end

System.sioWrite = function ( str )
	fpLog:write( "<<SIO output:" .. str .. "\r\n" )
end
