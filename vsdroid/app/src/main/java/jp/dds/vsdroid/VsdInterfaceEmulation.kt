package jp.dds.vsdroid

import android.content.Context
import android.util.Log
import java.io.BufferedReader
import java.io.FileReader
import java.io.IOException
import java.time.ZonedDateTime
import java.util.Calendar
import java.util.StringTokenizer

//*** エミュレーションモード *********************************************
internal class VsdInterfaceEmulation(context: Context) : VsdInterface(context) {
	var brEmuLog: BufferedReader? = null
	var dTimePrev: Double = 0.0

	var iIdxTime: Int = 0x7FFFFFFF
	var iIdxTacho: Int = 0x7FFFFFFF
	var iIdxSpeed: Int = 0x7FFFFFFF
	var iIdxDistance: Int = 0x7FFFFFFF
	var iIdxGx: Int = 0x7FFFFFFF
	var iIdxGy: Int = 0x7FFFFFFF
	var iIdxThrottle: Int = 0x7FFFFFFF
	var iIdxLapTime: Int = 0x7FFFFFFF

	// 16bit int を 2byte にパックする
	private fun Pack(iIdx: Int, iVal: Int): Int {
		var idx = iIdx
		var lo = iVal and 0xFF
		var hi = (iVal shr 8) and 0xFF

		if (lo >= 0xFE) {
			Buf[idx++] = 0xFE.toByte()
			lo -= 0xFE
		}
		Buf[idx++] = lo.toByte()

		if (hi >= 0xFE) {
			Buf[idx++] = 0xFE.toByte()
			hi -= 0xFE
		}
		Buf[idx++] = hi.toByte()

		return idx
	}

	//////////////////////////////////////////////////////////////////////////
	
	@Throws(UnrecoverableException::class)
	override fun Open() {
		dTimePrev = -1.0

		var strBuf: String
		var strToken: String

		if (bDebug) Log.d("VSDroid", "VsdInterfaceEmu::Open")

		SetToReadyState()
		Gps = GpsInterface(AppContext, GpsMsgHandler, Pref)

		try {
			brEmuLog = BufferedReader(FileReader(Pref!!.getString("key_replay_log", null)))

			// ヘッダ解析
			strBuf = try {
				brEmuLog!!.readLine()
			} catch (e: IOException) {
				""
			}
			val Token = StringTokenizer(strBuf, "\t")

			var i = 0
			while (Token.hasMoreTokens()) {
				strToken = Token.nextToken()
				if (strToken == "Date/Time") iIdxTime = i
				else if (strToken == "Tacho") iIdxTacho = i
				else if (strToken == "Speed") iIdxSpeed = i
				else if (strToken == "Distance") iIdxDistance = i
				else if (strToken == "Gx") iIdxGx = i
				else if (strToken == "Gy") iIdxGy = i
				else if (strToken == "Throttle(raw)") iIdxThrottle = i
				else if (strToken == "LapTime") iIdxLapTime = i
				++i
			}
		} catch (e: Exception) {
			MsgHandler!!.sendEmptyMessage(R.string.statmsg_emulog_open_failed)
			throw UnrecoverableException("log can't open")
		}
	}

	//////////////////////////////////////////////////////////////////////////

	override fun Close() {
		if (bDebug) Log.d("VSDroid", "VsdInterfaceEmu::Close")

		if (brEmuLog != null) {
			try {
				brEmuLog!!.close()
			} catch (_: IOException) {}
			brEmuLog = null
		}
	}

	//////////////////////////////////////////////////////////////////////////
	// Log
	
	override fun OpenLog(): Int {
		return 0
	}

	//////////////////////////////////////////////////////////////////////////
	
	@Throws(IOException::class, UnrecoverableException::class)
	override fun RawRead(iStart: Int, iLen: Int): Int {
		var strBuf: String?
		var strToken = arrayOfNulls<String>(16)
		var iIdx = iStart
		var iTokCnt: Int
		var i: Int
		var j: Int
		var dTime: Double


		//if( bDebug ) Log.d( "VSDroid", "VsdInterfaceEmu::RawRead" );
		try {
			while(true){
				if ((brEmuLog!!.readLine().also { strBuf = it }) == null) {
					MsgHandler!!.sendEmptyMessage(R.string.statmsg_emulog_finished)
					throw UnrecoverableException("EOF")
				}
	
				val Token = StringTokenizer(strBuf)
	
				iTokCnt = 0
				while (iTokCnt < 16 && Token.hasMoreTokens()) {
					strToken[iTokCnt] = Token.nextToken()
					++iTokCnt
				}
	
				if(strToken[0] == "GPS"){
					GpsEmulation(strToken)
					continue
				}
				
				// Tacho
				iIdx = Pack(iIdx, if (iIdxTacho < iTokCnt) strToken[iIdxTacho]!!.toInt() else 0)
	
				// Speed
				iIdx = Pack(
					iIdx,
					if (iIdxSpeed < iTokCnt) (strToken[iIdxSpeed]!!.toDouble() * 100).toInt() else 0
				)
	
				// Mileage
				iIdx = Pack(
					iIdx,
					if (iIdxDistance < iTokCnt) (strToken[iIdxDistance]!!.toDouble() / 1000 * PULSE_PER_1KM_CE28N).toInt() else 0
				)
	
				// TSC (Date/Time)
				// 0000-00-00T00:00:00.000Z
				// 012345678901234567890123
				dTime = try {
					strToken[iIdxTime]!!.substring(11, 13).toInt() * 3600 + strToken[iIdxTime]!!
						.substring(14, 16).toInt() * 60 + strToken[iIdxTime]!!.substring(17, 23)
						.toDouble()
				} catch (e: NumberFormatException) {
					dTimePrev + 1.0 / 16
				}
				if (dTime < dTimePrev) dTime += (24 * 3600).toDouble()
				iIdx = Pack(
					iIdx,
					(dTime * (TIMER_HZ shr TSC_SHIFT)).toInt()
				)
	
				// G
				iIdx = Pack(
					iIdx,
					if (iIdxGy < iTokCnt) (-strToken[iIdxGy]!!.toDouble() * 4096).toInt() + 0x8000 else 0
				)
				iIdx = Pack(
					iIdx,
					if (iIdxGx < iTokCnt) (strToken[iIdxGx]!!.toDouble() * 4096).toInt() + 0x8000 else 0
				)
	
				// Throttle
				iIdx =
					Pack(iIdx, if (iIdxThrottle < iTokCnt) strToken[iIdxThrottle]!!.toInt() else 0x8000)
	
				// ラップタイム
				if (iIdxLapTime < iTokCnt && strToken[iIdxLapTime] != "") {
					// ラップタイム記録発見
					i = strToken[iIdxLapTime]!!.indexOf(':')
					j = ((strToken[iIdxLapTime]!!.substring(0, i).toDouble() * 60 +
							strToken[iIdxLapTime]!!.substring(i + 1).toDouble()
							) * TIMER_HZ
							).toInt()
	
					// LAP モード以外の 0:00.000 をスキップ
					if (!(j == 0 && iRtcPrevRaw != -1)) {
						// 0:00.000 が来た時，Lap Ready を解除する
	
						if(j == 0 && iRtcPrevRaw == -1) j = 1
						j += iRtcPrevRaw
						iIdx = Pack(iIdx, j)
						iIdx = Pack(iIdx, j shr 16)
					}
				}

				// 待つ
				if (dTimePrev >= 0) Sleep(((dTime - dTimePrev) * 1000).toInt())
				//if (dTimePrev >= 0) Sleep(((dTime - dTimePrev) * 100).toInt())
				dTimePrev = dTime
				
				// EOL 追加
				Buf[iIdx++] = 0xFF.toByte()
				break
			}
		} catch (e: IOException) {
			MsgHandler!!.sendEmptyMessage(R.string.statmsg_emulog_failed)
			throw UnrecoverableException("error while reading")
		}
		return iIdx - iStart
	}
	
	//////////////////////////////////////////////////////////////////////////
	
	private fun GpsEmulation(strToken: Array<String?>){
		// GPS	2024-12-31T02:29:26.900Z	136.52792667	34.84315000	43.200	0.093
		// 0	1							2				3			4		5

		val zonedDateTime = ZonedDateTime.parse(strToken[1])
		val calendar = Calendar.getInstance()
		calendar.timeInMillis = zonedDateTime.toInstant().toEpochMilli()
		
		Gps?.dPrevLong		= Gps?.dLong!!
		Gps?.dPrevLati		= Gps?.dLati!!
		
		Gps?.GpsTime		= calendar
		Gps?.dLong			= strToken[2]?.toDouble()!!
		Gps?.dLati			= strToken[3]?.toDouble()!!
		Gps?.dAlt			= strToken[4]?.toDouble()!!
		Gps?.dSpeed			= strToken[5]?.toDouble()!!
		
		Gps?.iPrevNmeaTime = Gps?.iNmeaTime!!
		Gps?.iNmeaTime = (calendar.timeInMillis % (24 * 3600 * 1000)).toInt()

		Gps?.Connected = true
		UpdateGps()
	}
	
	override fun SendCmd(s: String) {}
	override fun LoadFirmware() {}
}
