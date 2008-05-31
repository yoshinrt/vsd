------------------------------------------------------------------------------
--		VSD -- vehicle data logger system  Copyright(C) by DDS
--		$Id$
------------------------------------------------------------------------------
-- .tab=4
--- ダミーシリアル入力 -------------------------------------------------------

-- シリアルポートなし (debug)
NoSio = "vsd20080502_102516.log"
-- NoSio = true

DummySioTimer = Timer.new()
DummySioTimer:start()
NextSioTime = 0

TSC = Timer.new() TSC:start()
DebugPrevKey = 0

function ItoA( Hi, Lo )
	local Ret = ""
	local i
	Lo = Hi * 0x10000 + Lo
	
	for i = 1, 5 do
		Ret = string.char( math.fmod( Lo, 0x80 ) + 0x80 ) .. Ret
		Lo  = math.floor( Lo / 0x80 )
	end
	return Ret
end

if( type( NoSio ) == "string" ) then
	fpIn = io.open( NoSio, "r" )
	assert( fpIn, "Can't open file:" .. NoSio )
	
	-- ログ再生
	System.sioRead = function ()
		local Ret = ""
		local Line = nil
		local Params = {}
		
		if( DummySioTimer:time() >= NextSioTime ) then
			NextSioTime = NextSioTime + 1000 / LOG_FREQ
			
			-- 行頭が数値でなければ，有効な行ではない
			repeat
				Line = fpIn:read()
			until( 0x30 <= Line:byte() and Line:byte() <= 0x39 )
			
			-- 
			for w in Line:gmatch( "[^%s]+" ) do
				Params[ #Params + 1 ] = tonumber( w )
			end
			
			if( Params[ 4 ] < 10 ) then
				Params[ 4 ] = math.floor(  Params[ 4 ] * ACC_1G_Z + 32000 );
				Params[ 5 ] = math.floor( -Params[ 5 ] * ACC_1G_Y + 32000 );
			end
			
			Ret =
				ItoA( Params[ 1 ], math.floor( Params[ 2 ] * 100 )) ..
				ItoA( math.floor( math.fmod( Params[ 3 ] / 1000 * PULSE_PAR_1KM, 0x10000 )), 0 ) ..
				ItoA( Params[ 5 ], Params[ 4 ] )
			
			-- ラップタイムあり?
			local result, tmp, min, sec = Line:find( "LAP.*(%d+):([%d%.]+)" )
			if( result ) then
				LapTimePrev = 0
				Ret = Ret .. ItoA( 0, math.floor(( min * 60 + sec ) * H8HZ / 0x10000 + 0.5 ))
			end
			
			return Ret .. "*"
		end
		
		return ""
	end
else
	
	-- 自動ログ生成
	System.sioRead = function ()
		local Ret = ""
		if( DummySioTimer:time() > ( 1000 / 15 )) then
			DummySioTimer:reset()
			DummySioTimer:start()
			Ret = string.format(
				"T%s S%s g%s ",
				ItoA( math.fmod( TSC:time(), 6800 ) / 2 + 3400 ),
				ItoA( math.fmod( TSC:time(), 20000 )),
				ItoA(
					( 32000 + math.floor( 6000 * math.sin( TSC:time() / 150 * 17 ))) * 0x10000 +
					32000 + math.floor( 6000 * math.cos( TSC:time() / 150 * 23 ))
				)
			)
			
			if( not DebugPrevKey and Controls.read():l()) then
				Ret = Ret .. "L" .. ItoA( TSC:time() / 1000 * ( H8HZ / 65536 )) .. " "
			end
			DebugPrevKey = Controls.read():l()
			
			return Ret .. "\r\n"
		end
		
		return ""
	end
end

System.sioWrite = function ( str )
	fpLog:write( "<<SIO output:" .. str .. "\r\n" )
end
