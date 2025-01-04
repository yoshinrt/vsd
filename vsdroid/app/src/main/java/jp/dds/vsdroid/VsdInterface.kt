package jp.dds.vsdroid

import android.content.Context
import android.content.SharedPreferences
import android.os.Handler
import android.os.Looper
import android.os.Message
import android.util.Log
import java.io.BufferedOutputStream
import java.io.File
import java.io.FileInputStream
import java.io.FileNotFoundException
import java.io.FileOutputStream
import java.io.IOException
import java.io.InputStream
import java.io.OutputStream
import java.io.UnsupportedEncodingException
import java.net.InetSocketAddress
import java.net.Socket
import java.net.SocketAddress
import java.net.SocketTimeoutException
import java.util.Calendar
import java.util.Locale
import java.util.TimeZone
import java.util.zip.GZIPOutputStream
import kotlin.concurrent.Volatile
import kotlin.math.atan2
import kotlin.math.cos

//*** VSD アクセス *******************************************************
open class VsdInterface(context: Context) : Runnable {
	// モード・config
	var iMainMode: Int = MODE_LAPTIME
	private var dPulsePer1Km: Double = PULSE_PER_1KM_CE28N

	var Pref: SharedPreferences? = null
	var VsdThread: Thread? = null

	// VSD リンクのリードデータ
	var iTacho: Int = 0
	var iSpeedRaw: Int = 0
	private var iRtcRaw: Int = 0
	var iRtcPrevRaw: Int = -1
	private var iMileageRaw: Int = 0
	private var iTSCRaw: Int = 0
	private var iSectorCnt: Int = 0
	private var iSectorCntMax: Int = 1
	private var Cal: Calendar
	private var CalDev: Calendar

	var iGx: Int = 0
	var iGy: Int = 0
	private var iGx0: Int
	private var iGy0: Int
	var iThrottle: Int = 0
	var iThrottle0: Int
	var iThrottleMax: Int
	var iThrottleRaw: Int = 0
	private var dGymkhaStart: Double = 1.0

	var iConnMode: Int = CONN_MODE_ETHER
	private var iGCaribCnt: Int = iGCaribCntMax
	private var LapState: LAP_STATE = LAP_STATE.NONE

	var Buf: ByteArray = ByteArray(iBufSize)
	private var iBufLen: Int = 0
	private var iBufPtr: Int = 0

	private var fsLog: GZIPOutputStream? = null
	private var fsBinLog: BufferedOutputStream? = null

	private var bDebugInfo: Boolean = false
	private var bLogStart: Boolean = false
	private var Sock: Socket? = null

	var InStream: InputStream? = null
	var OutStream: OutputStream? = null

	@Volatile
	var bKillThread: Boolean = false
	var MsgHandler: Handler? = null
	var Gps: GpsInterface? = null
	var AppContext: Context? = null

	internal enum class LAP_STATE {
		NONE,
		START,
		UPDATE,
		SECTOR,
	}

	// コントロールライン
	var dCtrlLineX0: Double = Double.NaN
	var dCtrlLineY0: Double = Double.NaN
	var dCtrlLineX1: Double = Double.NaN
	var dCtrlLineY1: Double = Double.NaN
	var dCtrlLineAngle: Double = 0.0

	//*** リトライしないerror ************************************************
	class UnrecoverableException(message: String?) : Exception(message) {
		companion object {
			private const val serialVersionUID = 1L
		}
	}

	//*** utils **************************************************************
	fun Sleep(ms: Int) {
		try {
			Thread.sleep(ms.toLong())
		} catch (_: InterruptedException) {}
	}

	// 緯度・経度から線分の角度を算出
	private fun ComputeAngle(x0: Double, y0: Double, x1: Double, y1: Double): Double {
		return atan2(
			y1 - y0,
			(x1 - x0) * cos(y0 * (Math.PI / 180))
		)
	}

	// 2byte を 16bit int にアンパックする
	private fun Unpack(): Int {
		var iRet: Int

		if (Buf[iBufPtr] == 0xFE.toByte()) {
			++iBufPtr
			iRet = Buf[iBufPtr++] + 0xFE
		} else {
			iRet = Buf[iBufPtr++].toInt() and 0xFF
		}
		if (Buf[iBufPtr] == 0xFE.toByte()) {
			++iBufPtr
			iRet += (Buf[iBufPtr++] + 0xFE) shl 8
		} else {
			iRet += Buf[iBufPtr++].toInt() shl 8
		}
		return iRet and 0xFFFF
	}

	open fun OpenLog(): Int {
		// 非デバッグモードかつ Log リプレイモードなら，ログは作らない

		if (!bDebug && iConnMode == CONN_MODE_LOGREPLAY) return 0

		// log dir 作成
		val dir = File(Pref!!.getString("key_system_dir", null) + "/log")
		dir.mkdirs()

		// 日付
		val Now = Calendar.getInstance()
		val s = String.format(
			Locale.US,
			"%s/log/vsd%04d%02d%02d_%02d%02d%02d",
			Pref!!.getString("key_system_dir", null),
			Now[Calendar.YEAR],
			Now[Calendar.MONTH] + 1,
			Now[Calendar.DAY_OF_MONTH],
			Now[Calendar.HOUR_OF_DAY],
			Now[Calendar.MINUTE],
			Now[Calendar.SECOND]
		)

		// ログファイルオープン
		try {
			fsLog = GZIPOutputStream(
				BufferedOutputStream(
					FileOutputStream(
						"$s.log.gz"
					)
				)
			)
			if (bDebugInfo) {
				fsBinLog = BufferedOutputStream(FileOutputStream("$s.bin"))
			}
		} catch (e: Exception) {
			MsgHandler!!.sendEmptyMessage(R.string.statmsg_mklog_failed)
			return FATAL_ERROR
		}

		// ヘッダ
		try {
			if (fsLog != null) fsLog!!.write(
				("Date/Time\tDev Date Time\tTacho\tSpeed\tDistance\t" +
						"Gy\tGx\tThrottle\tThrottle(raw)\t" +
						"AuxInfo\tLapTime\tSectorTime\r\n"
						).toByteArray()
			)
		} catch (_: IOException) {}

		return 0
	}

	@Throws(IOException::class, UnrecoverableException::class)
	protected open fun OpenVsdIf() {
		if (bDebug) Log.d("VSDroid", "VsdInterface::OpenVsdIf")

		MsgHandler!!.sendEmptyMessage(R.string.statmsg_tcpip_connecting)
		try {
			// ソケットの作成
			Sock = Socket()
			val adr: SocketAddress = InetSocketAddress(Pref!!.getString("key_ip_addr", null), 12345)
			Sock!!.connect(adr, 1000)
			if (bDebug) Log.d("VSDroid", "VsdInterface::OpenVsdIf:connected")
			InStream = Sock!!.getInputStream()
			OutStream = Sock!!.getOutputStream()

			MsgHandler!!.sendEmptyMessage(R.string.statmsg_tcpip_connected)
		} catch (e: SocketTimeoutException) {
			if (bDebug) Log.d("VSDroid", "VsdInterface::OpenVsdIf:timeout")
			CloseVsdIf()
			throw IOException("socket timeout")
		} catch (e: IOException) {
			if (bDebug) Log.d("VSDroid", "VsdInterface::OpenVsdIf:IOException")
			CloseVsdIf()
			throw e
		}
	}

	@Throws(IOException::class, UnrecoverableException::class)
	protected open fun RawRead(iStart: Int, iLen: Int): Int {
		var iRetryCnt: Int = 0

		// 3秒でタイムアウト
		while(InStream!!.available() == 0){
			Sleep(50)
			if(++iRetryCnt >= 60){
				throw IOException("read timeout")
			}
		}

		val iReadSize = InStream!!.read(Buf, iStart, iLen)
		if (iReadSize <= 0) {
			throw IOException("read size < 1")
		}
		return iReadSize
	}

	// 1 <= :受信したレコード数  0:新データなし
	@Throws(IOException::class, UnrecoverableException::class)
	private fun Read(): Int {
		var iMileage16: Int
		var iRet = 0
		var iEOLPos = 0
		var i: Int

		//if( bDebug ) Log.d( "VSDroid", "VsdInterface::Read" );
		val iReadSize = RawRead(iBufLen, iBufSize - iBufLen)
		if (iReadSize == 0) return 0

		try {
			if (fsBinLog != null) fsBinLog!!.write(Buf, iBufLen, iReadSize)
		} catch (_: IOException) {}

		iBufLen += iReadSize

		while (!bKillThread) {
			// 0xFF をサーチ
			iBufPtr = iEOLPos
			while (iEOLPos < iBufLen) {
				if (Buf[iEOLPos] == 0xFF.toByte()) break
				++iEOLPos
			}
			if (iEOLPos == iBufLen) break

			++iRet
			// 0xFF が見つかったので，データ変換
			iTacho = Unpack()
			iSpeedRaw = Unpack()

			// ここで描画要求
			MsgHandler!!.sendEmptyMessage(R.string.statmsg_update)

			iMileage16 = Unpack()
			iTSCRaw = Unpack()
			iGy = Unpack()
			iGx = Unpack()

			iThrottleRaw = -(0x7FFFFFFF / Unpack()) - iThrottle0

			if (iGCaribCnt-- > 0) {
				// センサーキャリブレーション中
				iGx0 += iGx
				iGy0 += iGy

				if (iGCaribCnt == 0) {
					iGx0 /= iGCaribCntMax
					iGy0 /= iGCaribCntMax
				}

				iGx = 0
				iGy = 0
			} else {
				iGx -= iGx0
				iGy -= iGy0

				// スロットル 0, Full 値補正
				// Raw 値が 0～Full 値を外れたら 1/16 だけ補正する
				if (iThrottleRaw < 0) {
					iThrottle0 += iThrottleRaw shr 4
				} else if (iThrottleRaw > iThrottleMax) {
					iThrottleMax += (iThrottleRaw - iThrottleMax) shr 4
				}

				// 0, 100% 付近の 1/32 ≒ 3% 分は不感帯にする
				iThrottle = ((iThrottleRaw - (iThrottleMax shr 5))
						* (1000 / (1 - 1.0 / 16)).toInt() / iThrottleMax)

				if (iThrottle < 0) iThrottle = 0
				else if (iThrottle > 1000) iThrottle = 1000
			}

			// mileage 補正
			if ((iMileageRaw and 0xFFFF) > iMileage16) iMileageRaw += 0x10000
			iMileageRaw = iMileageRaw and -0x10000
			iMileageRaw = iMileageRaw or iMileage16

			// LapTime が見つかった
			LapState = LAP_STATE.NONE

			if (iBufPtr < iEOLPos) {
				iRtcRaw = Unpack() + (Unpack() shl 16)

				if (++iSectorCnt >= iSectorCntMax) {
					// 規定のセクタを通過したので，NewLap
					if (iRtcPrevRaw != -1) {
						// 1周回った
						LapState = LAP_STATE.UPDATE

						val iLapTimeRaw = iRtcRaw - iRtcPrevRaw
						if (iTimeBestRaw == 0 || iLapTimeRaw < iTimeBestRaw) iTimeBestRaw = iLapTimeRaw
						iLapTimeList.add(iLapTimeRaw)

						if (iMainMode != MODE_LAPTIME) {
							// Laptime モード以外でゴールしたら，Ready 状態に戻す
							iRtcPrevRaw = -1
						}
					} else {
						// ラップスタート
						LapState = LAP_STATE.START
					}
					iRtcPrevRaw = iRtcRaw
					iSectorCnt = 0
				} else if (iRtcPrevRaw != -1) {
					// 中間セクタ通過
					LapState = LAP_STATE.SECTOR
				}
			}

			WriteLog() // テキストログライト
			++iEOLPos
		}

		// 使用したデータを Buf から削除
		if (iBufPtr != 0) {
			i = 0
			while (iBufPtr < iBufLen) {
				Buf[i++] = Buf[iBufPtr++]
			}
			iBufLen = i
		}

		return iRet
	}

	private var iLogTimeMilli: Long = 0
	private var iTSC: Int = 0 // 200KHz >> 8

	private fun WriteLog() {
		if (!bLogStart) {
			// まだ動いていないので，Log 保留
			if (!bDebugInfo && iSpeedRaw == 0) return


			// 動いたので Log 開始
			bLogStart = true
			iTSC = iTSCRaw
			iLogTimeMilli = System.currentTimeMillis() - Tsc2Milli(iTSC)

			if (fsLog == null) OpenLog()
		}

		if (fsLog == null) return


		// iTSCRaw から時刻算出
		if ((iTSC and 0xFFFF) > iTSCRaw) iTSC += 0x10000
		iTSC = (iTSC and -0x10000) or iTSCRaw
		Cal.timeInMillis = iLogTimeMilli + Tsc2Milli(iTSC)
		CalDev.timeInMillis = System.currentTimeMillis()

		// 基本データ
		val s = StringBuilder(
			String.format(
				Locale.US,
				"%04d-%02d-%02dT%02d:%02d:%02d.%03dZ\t" +
						"%04d-%02d-%02dT%02d:%02d:%02d.%03dZ\t" +
						"%d\t%.2f\t%.2f\t" +
						"%.3f\t%.3f\t" +
						"%.1f\t%d\t%d",
				Cal[Calendar.YEAR], Cal[Calendar.MONTH] + 1,
				Cal[Calendar.DATE],
				Cal[Calendar.HOUR_OF_DAY],
				Cal[Calendar.MINUTE],
				Cal[Calendar.SECOND],
				Cal[Calendar.MILLISECOND],
				CalDev[Calendar.YEAR], CalDev[Calendar.MONTH] + 1,
				CalDev[Calendar.DATE],
				CalDev[Calendar.HOUR_OF_DAY],
				CalDev[Calendar.MINUTE], CalDev[Calendar.SECOND],
				CalDev[Calendar.MILLISECOND],
				iTacho, iSpeedRaw / 100.0,
				iMileageRaw * (1000 / dPulsePer1Km),
				iGy / 4096.0, iGx / 4096.0,
				iThrottle / 10.0, iThrottleRaw,
				iTSCRaw
			)
		)

		// ラップタイム
		when (LapState) {
			LAP_STATE.UPDATE -> s.append("\t").append(FormatTime2(GetLastLapTime())).append("\t")
			LAP_STATE.START  -> s.append("\t0:00.000\t")
			LAP_STATE.SECTOR -> s.append("\t\t").append(FormatTime2(iRtcRaw - iRtcPrevRaw))
			else -> {}
		}

		s.append("\r\n")
		try {
			if (fsLog != null) fsLog!!.write(s.toString().toByteArray())
		} catch (_: IOException) {}
	}

	fun CloseLog(): Int {
		try {
			if (fsLog != null) {
				fsLog!!.close()
				fsLog = null
			}
			if (fsBinLog != null) {
				fsBinLog!!.close()
				fsBinLog = null
			}
		} catch (_: IOException) {}
		return 0
	}

	protected open fun CloseVsdIf(): Int {
		if (bDebug) Log.d("VSDroid", "VsdInterface::CloseVsdIf")


		// BT 切断すると AT コマンドモードになるので，シリアル出力を止める
		try {
			SendCmd("z")
		} catch (_: IOException) {}

		try {
			if (Sock != null) {
				Sock!!.close()
				Sock = null
			}
		} catch (_: IOException) {}
		return 0
	}

	//*** GPS message handler ********************************************
	var GpsMsgHandler: Handler = object : Handler(Looper.getMainLooper()) {
		override fun handleMessage(Msg: Message) {
			if (Msg.what == R.string.statmsg_gps_updated) {
				UpdateGps()
			} else {
				MsgHandler!!.sendEmptyMessage(Msg.what)
			}
		}
	}

	protected fun UpdateGps() {
		// ログ出力
		try {
			if (fsLog != null) {
				fsLog!!.write(
					String.format(
						Locale.US,
						"GPS\t%04d-%02d-%02dT%02d:%02d:%02d.%03dZ\t" +
								"%.8f\t%.8f\t%.3f\t%.3f\n",
						Gps!!.GpsTime[Calendar.YEAR], Gps!!.GpsTime[Calendar.MONTH] + 1,
						Gps!!.GpsTime[Calendar.DATE],
						Gps!!.GpsTime[Calendar.HOUR_OF_DAY],
						Gps!!.GpsTime[Calendar.MINUTE],
						Gps!!.GpsTime[Calendar.SECOND],
						Gps!!.GpsTime[Calendar.MILLISECOND],
						Gps!!.dLong, Gps!!.dLati, Gps!!.dAlt, Gps!!.dSpeed
					).toByteArray()
				)
			}
		} catch (_: IOException) {}

		if (java.lang.Double.isNaN(Gps!!.dPrevLong) || java.lang.Double.isNaN(dCtrlLineX0)) return

		// コントロールライン通過から 10秒経過するまでスキップ
		var iDiffTime = Gps!!.iNmeaTime - iRtcPrevRaw
		if (iDiffTime < 0) iDiffTime += 3600 * 24 * 1000
		if (iDiffTime < 10 * 1000) return

		// コントロールライン通過判定
		val x1 = dCtrlLineX0
		val y1 = dCtrlLineY0
		val x2 = dCtrlLineX1
		val y2 = dCtrlLineY1
		val x3 = Gps!!.dPrevLong
		val y3 = Gps!!.dPrevLati
		val x4 = Gps!!.dLong
		val y4 = Gps!!.dLati

		// 交点が iLogNum ～ +1 線分上かの判定
		val s1 = (x1 - x2) * (y3 - y1) + (y1 - y2) * (x1 - x3)
		val s2 = (x1 - x2) * (y4 - y1) + (y1 - y2) * (x1 - x4)
		if (s1 * s2 > 0) return

		// 交点がスタートライン線分上かの判定
		val s3 = (x3 - x4) * (y1 - y3) + (y3 - y4) * (x3 - x1)
		val s4 = (x3 - x4) * (y2 - y3) + (y3 - y4) * (x3 - x2)
		if (s3 * s4 > 0) return

		// 進行方向の判定，dAngle ±45度
		var dAngle = ComputeAngle(x3, y3, x4, y4) - dCtrlLineAngle
		if (dAngle < -Math.PI) dAngle += Math.PI * 2
		else if (dAngle > Math.PI) dAngle -= Math.PI * 2
		if (dAngle < -Math.PI / 2 || dAngle > Math.PI / 2) return

		/*** ここまで来たらコントロールライン通過  */

		// 直前座標の時間との差分, 5Hz なら 200[ms]
		var iTime = Gps!!.iNmeaTime - Gps!!.iPrevNmeaTime
		if (iTime < 0) iTime += 3600 * 24 * 1000


		// 200ms 以内の通過時間を等比分割で求める
		iTime = if (s1 == s2) 0
		else (iTime * s1 / (s1 - s2)).toInt()

		iTime += Gps!!.iPrevNmeaTime // 通過時間確定

		if (iRtcPrevRaw != -1) {
			// 1周回った
			var iLapTimeRaw = iTime - iRtcPrevRaw
			if (iLapTimeRaw < 0) iLapTimeRaw += 24 * 3600 * 1000
			iLapTimeRaw = iLapTimeRaw * (TIMER_HZ / 1000) // TIMER_HZ が 1000 で割り切れないときは★要修正
			if (iTimeBestRaw == 0 || iLapTimeRaw < iTimeBestRaw) iTimeBestRaw = iLapTimeRaw
			
			iLapTimeList.add(iLapTimeRaw)
		}
		iRtcPrevRaw = iTime
	}

	//*** コントロールライン設定 *********************************************
	private fun LoadCtrlLine() {
		val Kml = KmlManager()

		if (Kml.Load(Pref!!.getString("key_circuit", null)) == KmlManager.OK &&
			Kml.m_Coordinates!!.size >= 4
		) {
			dCtrlLineX0 = Kml.m_Coordinates!![0]
			dCtrlLineY0 = Kml.m_Coordinates!![1]
			dCtrlLineX1 = Kml.m_Coordinates!![2]
			dCtrlLineY1 = Kml.m_Coordinates!![3]

			dCtrlLineAngle = ComputeAngle(
				dCtrlLineX0, dCtrlLineY0,
				dCtrlLineX1, dCtrlLineY1
			) + Math.PI / 2
			if (dCtrlLineAngle > Math.PI) dCtrlLineAngle -= 2 * Math.PI

			if (bDebug) Log.d(
				"VSDroid", String.format(
					"(%f,%f)-(%f,%f),%d",
					dCtrlLineX0, dCtrlLineY0,
					dCtrlLineX1, dCtrlLineY1,
					(dCtrlLineAngle * (180 / Math.PI)).toInt()
				)
			)
		}
	}

	//*** VSD + GPS open / close *********************************************
	
	@Throws(UnrecoverableException::class, IOException::class)
	protected open fun Open() {
		OpenVsdIf()
		LoadFirmware()
		SetToReadyState()

		if (Pref!!.getBoolean("key_use_btgps", true)) {
			Gps = GpsInterface(AppContext, GpsMsgHandler, Pref)
			Gps!!.Start()
		}
	}

	protected open fun Close() {
		if (Gps != null) {
			Gps!!.KillThread()
			Gps!!.Close()
		}

		CloseVsdIf()
	}

	//*** sio コマンド関係 ***************************************************
	@Throws(IOException::class)
	open fun SendCmd(s: String) {
		if (OutStream == null) return

		try {
			OutStream!!.write(s.toByteArray())
		} catch (_: UnsupportedEncodingException) {}
	}

	@Throws(IOException::class)
	private fun WaitChar(ch: Char, iWait: Int = 30): Int {
		var wait = iWait
		var iReadSize: Int
		var i: Int

		while (wait-- != 0 && !bKillThread) {
			if (InStream!!.available() > 0) {
				iReadSize = InStream!!.read(Buf, 0, iBufSize)
				i = 0
				while (i < iReadSize) {
					if (Buf[i] == ch.code.toByte()) return i
					++i
				}
			} else {
				Sleep(100)
			}
		}
		throw IOException("WaitChar() time out")
	}

	//*** FW ロード ******************************************************
	@Throws(IOException::class)
	protected open fun LoadFirmware() {
		var fsFirm: InputStream? = null

		var iReadSize = 0
		var i = 0

		var iFwSendWait = 100
		try {
			iFwSendWait = Pref!!.getString("key_fw_send_wait", "100")!!.toInt()
		} catch (_: NumberFormatException) {}

		MsgHandler!!.sendEmptyMessage(R.string.statmsg_loadfw_loading)

		try {
			// .mot オープン
			fsFirm = FileInputStream(Pref!!.getString("key_roms", "vsd2.mot"))

			if (bDebug) Log.d("VSDroid", "LoadFirm::sending magic code")
			MsgHandler!!.sendEmptyMessage(R.string.statmsg_loadfw_magic)
			SendCmd("F15EF117*z")

			if (bDebug) Log.d("VSDroid", "LoadFirm::sending firmware")

			MsgHandler!!.sendEmptyMessage(R.string.statmsg_loadfw_wait)
			WaitChar(':')


			// FW 送信
			MsgHandler!!.sendEmptyMessage(R.string.statmsg_loadfw_sending)
			while ((fsFirm.read(Buf, 0, iBufSize).also { iReadSize = it }) > 0) {
				OutStream!!.write(Buf, 0, iReadSize)
				if (iFwSendWait > 0) Sleep(iFwSendWait)
			}

			MsgHandler!!.sendEmptyMessage(R.string.statmsg_loadfw_wait)
			if (bDebug) Log.d("VSDroid", "LoadFirm::go")
			WaitChar(':')
		} catch (e: FileNotFoundException) {
			// FW がない場合ここに飛ぶ
			if (bDebug) Log.d("VSDroid", "LoadFirm::sending firmware canceled")
			MsgHandler!!.sendEmptyMessage(R.string.statmsg_loadfw_none)
		}

		try {
			fsFirm?.close()
		} catch (_: IOException) {}

		if (bDebug) Log.d("VSDroid", "LoadFirm::sending log output request")
		Sleep(500)

		for (iRetry in 2 downTo 1) {
			MsgHandler!!.sendEmptyMessage(R.string.statmsg_loadfw_magic)
			SendCmd("F15EF117*")

			// 最初の 0xFF までスキップ
			MsgHandler!!.sendEmptyMessage(R.string.statmsg_loadfw_wait)
			if (bDebug) Log.d("VSDroid", "LoadFirm::waiting first log record")

			try {
				i = WaitChar(0xFF.toChar(), 10) + 1
			} catch (e1: IOException) {
				if (iRetry == 1) throw e1
				continue
			}
			break
		}

		iBufLen = 0
		while (i < iReadSize) {
			Buf[iBufLen] = Buf[i]
			++i
			++iBufLen
		}

		MsgHandler!!.sendEmptyMessage(R.string.statmsg_loadfw_loaded)
		if (bDebug) Log.d("VSDroid", "LoadFirm::completed.")
	}

	//*** config に従って設定 ********************************************
	@Throws(IOException::class)
	fun SetToReadyState() {
		var i: Int

		// セクタ数
		iSectorCntMax = try {
			Pref!!.getString("key_sectors", "1")!!.toInt()
		} catch (e: NumberFormatException) {
			1
		}

		// ジムカスタート距離
		dGymkhaStart = try {
			Pref!!.getString("key_gymkha_start", "1")!!.toDouble()
		} catch (e: NumberFormatException) {
			1.0
		}

		// ホイール設定
		dPulsePer1Km = if (Pref!!.getString("key_wheel_select", "CE28N") == "CE28N")
			PULSE_PER_1KM_CE28N
		else
			PULSE_PER_1KM_NORMAL

		// メインモード
		try {
			iMainMode = Pref!!.getString("key_vsd_mode", "0")!!.toInt()
			if (iMainMode < 0 || MODE_NUM <= iMainMode) iMainMode = MODE_LAPTIME
		} catch (_: NumberFormatException) {}

		bDebugInfo = Pref!!.getBoolean("key_debug_info", false)

		when (iMainMode) {
			MODE_LAPTIME -> SendCmd("l")
			MODE_GYMKHANA -> {
				// * は g_lParam をいったんクリアする意図
				i = (dGymkhaStart * dPulsePer1Km / 1000 + 0.5).toInt()
				if (i < 1) i = 1
				SendCmd(String.format("*%Xg", i))
			}

			MODE_ZERO_FOUR -> SendCmd(String.format("*%Xf", iStartGThrethold))
			MODE_ZERO_ONE -> SendCmd(String.format("*%Xo", iStartGThrethold))
		}

		// ホイール・Hz 設定

		// 再び sio on と wheel 設定
		var iLogHz = 16
		try {
			iLogHz = Pref!!.getString("key_log_hz", "16")!!.toInt()
		} catch (_: NumberFormatException) {}

		SendCmd(
			String.format(
				"*%Xw%Xh",
				(dPulsePer1Km * (1 shl 18) - (0x40000000 * 2.0)).toInt() xor -0x80000000,
				iLogHz
			)
		)

		iRtcPrevRaw = -1
		iSectorCnt = 0
		bLogStart = false // TSC と実時間の同期を取り直す
	}

	//*** データ処理スレッド *********************************************
	fun Start() {
		// VSD スレッド起動
		VsdThread = Thread(this)
		VsdThread!!.start()

		LoadCtrlLine()
	}

	@Volatile
	var uReadWdt: Int = 0
	
	//////////////////////////////////////////////////////////////////////////
	// レコード

	var iLapTimeList: MutableList<Int> = mutableListOf()
	var iTimeBestRaw: Int = 0

	fun GetLapNum(): Int {return iLapTimeList.size}
	fun GetLastLapTime(): Int {return if(iLapTimeList.size == 0){0} else iLapTimeList.last()}

	// (不正な) 最速ラップ削除
	fun DeleteFastestLap(){
		iLapTimeList.remove(iTimeBestRaw)

		iTimeBestRaw = 0
		for(iTime in iLapTimeList){
			if(iTimeBestRaw == 0 || iTimeBestRaw > iTime) iTimeBestRaw = iTime
		}
	}

	//////////////////////////////////////////////////////////////////////////

	fun Calibration(){
		try {
			SendCmd("c")
		} catch (_: IOException) {}
	}
	
	//************************************************************************
	// コンストラクタ - 変数の初期化だけやってる
	init {
		if (bDebug) Log.d("VSDroid", "VsdInterface::constructor")

		iTacho = 0
		iSpeedRaw = 0
		iRtcRaw = 0
		iRtcPrevRaw = -1
		iMileageRaw = 0
		iTSCRaw = 0
		iThrottle0 = -48650 // スロットルの 0地点	(実測値 49650-39277)
		iThrottleMax = 8000 // スロットル 0～100% 幅
		iSectorCnt = 0
		iSectorCntMax = 1
		dGymkhaStart = 1.0

		Cal = Calendar.getInstance(TimeZone.getTimeZone("GMT+0"))
		CalDev = Calendar.getInstance(TimeZone.getTimeZone("GMT+0"))

		LapState = LAP_STATE.NONE

		// レコード
		iTimeBestRaw = 0

		iBufLen = 0
		iBufPtr = 0

		bKillThread = false

		fsLog = null
		fsBinLog = null
		bLogStart = false
		Sock = null

		iGy0 = 0
		iGx0 = iGy0
		iGCaribCnt = iGCaribCntMax

		InStream = null
		OutStream = null
		AppContext = context
	}

	override fun run() {
		bKillThread = false

		if (bDebug) Log.d("VSDroid", "run_loop()")

		while (!bKillThread) {
			try {
				Close() // 開いていたら一旦 close
				Open()

				while (!bKillThread && Read() >= 0) {
					uReadWdt = 0
					SendCmd("*") // ビーコン送信
				}
			} catch (e: UnrecoverableException) {
				// リトライしない中止要求
				// メッセージは各個設定しておくこと
				break
			} catch (e: IOException) {
				MsgHandler!!.sendEmptyMessage(R.string.statmsg_disconnected)
				if (bDebug) e.printStackTrace()
				Sleep(1000)
			}
		}

		if (bDebug) Log.d("VSDroid", String.format("run() loop extting."))

		Close()
		CloseLog()

		bKillThread = false
		if (bDebug) Log.d("VSDroid", "exit_run()")
	}

	fun KillThread() {
		bKillThread = true

		if (bDebug) Log.d("VSDroid", "KillThread: killing...")
		try {
			while (bKillThread && VsdThread != null && VsdThread!!.isAlive) {
				Thread.sleep(100)
				Close()
			}
		} catch (_: InterruptedException) {}

		VsdThread = null
		if (bDebug) Log.d("VSDroid", "KillThread: done")
	}

	companion object {
		val bDebug: Boolean = BuildConfig.DEBUG

		const val MODE_LAPTIME:		Int = 0
		const val MODE_GYMKHANA:	Int = 1
		const val MODE_ZERO_FOUR:	Int = 2
		const val MODE_ZERO_ONE:	Int = 3
		const val MODE_NUM:			Int = 4

		const val TIMER_HZ: Int = 200000 // 1000 で割り切れないときは★を要修正
		const val TSC_SHIFT: Int = 5

		// スピード * 100/Taco 比
		// ELISE
		// ギア比 * マージンじゃなくて，ave( ギアn, ギアn+1 ) に変更
		val GEAR_RATIO: DoubleArray = doubleArrayOf(
			1.2381712993947,
			1.82350889069989,
			2.37581451065366,
			2.95059529470571,
		)

		// たぶん，ホイル一周が30パルス
		const val PULSE_PER_1KM_CE28N: Double = 15473.76689 // CE28N
		const val PULSE_PER_1KM_NORMAL: Double = 14958.80127 // ノーマル
		private const val iBufSize = 256

		private const val iStartGThrethold = 1024 // 0.25G
		private const val iGCaribCntMax = 30

		const val CONN_MODE_ETHER: Int = 0
		const val CONN_MODE_BLUETOOTH: Int = 1
		const val CONN_MODE_LOGREPLAY: Int = 2

		const val ERROR: Int = -1
		const val FATAL_ERROR: Int = -2

		fun FormatTime(iTime: Int): String {
			return String.format(
				Locale.US,
				"%d'%02d.%03d",
				iTime / (TIMER_HZ * 60),
				(iTime / TIMER_HZ) % 60,
				1000 * (iTime % TIMER_HZ) / TIMER_HZ
			)
		}

		fun FormatTime2(iTime: Int): String {
			return String.format(
				Locale.US,
				"%d:%02d.%03d",
				iTime / (TIMER_HZ * 60),
				(iTime / TIMER_HZ) % 60,
				1000 * (iTime % TIMER_HZ) / TIMER_HZ
			)
		}

		fun Tsc2Milli(iTSC: Int): Int {
			return (iTSC * ((1 shl TSC_SHIFT) * 1000.0 / TIMER_HZ)).toInt()
		}
	}
}
