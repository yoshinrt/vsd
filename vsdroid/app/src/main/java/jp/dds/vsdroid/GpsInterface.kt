package jp.dds.vsdroid

import android.content.SharedPreferences
import android.os.Handler
import android.util.Log
import androidx.activity.ComponentActivity
import androidx.lifecycle.lifecycleScope
import jp.dds.Bluetooth.BluetoothCom
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.delay
import kotlinx.coroutines.launch
import java.io.BufferedReader
import java.io.IOException
import java.io.InputStreamReader
import java.io.OutputStream
import java.util.Calendar
import java.util.TimeZone
import java.util.UUID
import kotlin.concurrent.Volatile
import kotlin.math.floor

//*** VSD アクセス *******************************************************
class GpsInterface(_activity: ComponentActivity?, _MsgHandler: Handler?, _Pref: SharedPreferences?) {
	var Pref: SharedPreferences?
	var GpsThread: Thread? = null
	var BtCom: BluetoothCom? = null

	@Volatile
	var bKillThread: Boolean = false
	var MsgHandler: Handler? = null
	var activity: ComponentActivity? = null

	//////////////////////////////////////////////////////////////////////////
	// GPS データ
	private var iNmeaRcvState: Int = 0
	var iCurrentYear: Int

	class CGpsData {
		var iNmeaTime: Int = -1
		var GpsTime: Calendar = Calendar.getInstance()
		var dLong: Double = Double.NaN
		var dLati: Double = 0.0
		var dAlt: Double = 0.0
		var dSpeed: Double = 0.0
	}

	protected var GpsData: CGpsData? = null

	//************************************************************************
	// コンストラクタ - 変数の初期化だけやってる
	init {
		if (bDebug) Log.d("VSDroid:GPS", "GpsInterface:GpsInterface::constructor")

		MsgHandler = _MsgHandler
		Pref = _Pref
		activity = _activity

		// 現在の年取得
		iCurrentYear = Calendar.getInstance(TimeZone.getTimeZone("GMT+0"))[Calendar.YEAR]

		BtCom = BluetoothCom(_activity!!)
	}

	//*** リトライしないerror ************************************************

	class UnrecoverableException(message: String?) : Exception(message) {
		companion object {
			private const val serialVersionUID = 1L
		}
	}

	@Throws(IOException::class, UnrecoverableException::class)
	fun Open() {
		activity!!.lifecycleScope.launch(Dispatchers.IO) {
			if (bDebug) Log.d("VSDroid:GPS", "GpsInterface:GpsInterface::Open")

			// Enable Bluetooth adapter
			val result = BtCom?.EnableBluetooth()
			if (result != BluetoothCom.BtResult.OK) {
				throw UnrecoverableException("Bluetooth error")
			}

			// Open Bluetooth
			val BtName = Pref!!.getString("key_bt_devices_gps", null)
			while (
				!bKillThread &&
				BtCom?.Open(BtName) != BluetoothCom.BtResult.OK
			) {
				delay(1000)
			}
			
			// Open buffered reader
			BtCom?.OpenBufferedReader()
		}
	}

	fun Close() {
		if (bDebug) Log.d("VSDroid:GPS", "GpsInterface:GpsInterface::Close")
		BtCom?.Close()
		GpsData = null
	}

	//*** 1行処理 ************************************************************
	private fun ParseInt(str: String, iDefault: Int): Int {
		try {
			return str.toInt()
		} catch (_: Exception) {}
		return iDefault
	}

	private fun ParseDouble(str: String): Double {
		try {
			return str.toDouble()
		} catch (_: Exception) {}
		return Double.NaN
	}

	private fun ParseTime(str: String): Int {
		return ParseInt(str.substring(0, 2), 0) * 3600 * 1000 + ParseInt(
			str.substring(2, 4),
			0
		) * 60 * 1000 + (ParseDouble(str.substring(4)) * 1000).toInt()
	}

	@Throws(IOException::class)
	fun Read() {
		val str = BtCom!!.bfReader!!.readLine().split(",".toRegex()).dropLastWhile { it.isEmpty() }.toTypedArray()

		if (str[0] == "\$GPRMC") {
			if(GpsData == null) GpsData = CGpsData()
			
			//		時間		 lat		   long		   knot  方位   日付
			// 0	  1		  2 3		   4 5			6 7	 8	  9
			// $GPRMC,043431.200,A,3439.997825,N,13523.377978,E,0.602,178.29,240612,,,A*59
			val iTime = ParseTime(str[1])

			GpsData!!.iNmeaTime = iTime
			iNmeaRcvState = iNmeaRcvState or 1

			// Lat
			GpsData!!.dLati = ParseDouble(str[3])
			GpsData!!.dLati = floor(GpsData!!.dLati / 100) + (GpsData!!.dLati % 100.0) / 60.0
			if (str[4] == "S") GpsData!!.dLati = -GpsData!!.dLati

			// Long
			GpsData!!.dLong = ParseDouble(str[5])
			GpsData!!.dLong = floor(GpsData!!.dLong / 100) + (GpsData!!.dLong % 100.0) / 60.0
			if (str[6] == "W") GpsData!!.dLong = -GpsData!!.dLong

			// Speed
			GpsData!!.dSpeed = ParseDouble(str[7]) * 1.85200

			// 日付
			var iYear = ParseInt(str[9].substring(4, 6), 0) +
					(iCurrentYear / 100 * 100)

			if ((iCurrentYear % 100) >= 50 && iYear < 50) iYear += 100

			GpsData!!.GpsTime[iYear, ParseInt(str[9].substring(2, 4), 1) - 1, ParseInt(
				str[9].substring(0, 2),
				1
			), iTime / (3600 * 1000), iTime / (60 * 1000) % 60] =
				iTime / (1000) % 60
			GpsData!!.GpsTime[Calendar.MILLISECOND] = iTime % 1000
		} else if (str[0] == "\$GPGGA") {
			if(GpsData == null) GpsData = CGpsData()
			
			//		時間	   lat		   long			   高度
			// 0	  1		  2		   3 4			5 6 789
			// $GPGGA,233132.000,3439.997825,N,13523.377978,E,1,,,293.425,M,,,,*21

			val iTime = ParseTime(str[1])

			if (iNmeaRcvState == 0) {
				GpsData!!.iNmeaTime = iTime
				iNmeaRcvState = 2
			} else {
				iNmeaRcvState = iNmeaRcvState or 2
			}

			GpsData!!.dAlt = ParseDouble(str[9])
		}

		if (iNmeaRcvState == 3) {
			// NMEA データが 1組分揃った
			if (!java.lang.Double.isNaN(GpsData!!.dLong)) {
				MsgHandler!!.sendMessage(
					MsgHandler!!.obtainMessage(R.string.statmsg_gps_updated, GpsData)
				)
			}
			
			GpsData = null
			iNmeaRcvState = 0
		}
	}

	//*** データ処理スレッド *********************************************
	
	fun Start() {
		if (bDebug) Log.d("VSDroid:GPS", "Start()")
		// VSD スレッド起動
		GpsThread = Thread {
			bKillThread = false
	
			if (bDebug) Log.d("VSDroid:GPS", "GpsInterface:run_loop()")
			GpsData = null
	
			MainLoop@ while (!bKillThread) {
				// 開く，失敗したらやり直し
	
				while (!bKillThread) {
					try {
						if (bDebug) Log.d("VSDroid:GPS", "GpsInterface:opening...")
						Close() // 開いていたら一旦 close
						Open()
						break
					} catch (e: UnrecoverableException) {
						// BT off 等，リカバリ不能な問題
						if (bDebug) Log.d("VSDroid:GPS", "GpsInterface:Unrecoverable exception")
						break@MainLoop
					} catch (e: IOException) {
						// 一般的な IOException
						if (bDebug) Log.d("VSDroid:GPS", "GpsInterface:retrying open...")
						try {
							Thread.sleep((3 * 1000).toLong())
						} catch (_: InterruptedException) {}
					}
				}
	
				if (bDebug) Log.d("VSDroid:GPS", "GpsInterface:open done.")
	
				// 1行読む
				while (!bKillThread) {
					try {
						Read()
					} catch (e: IOException) {
						break
					}
				}
				
				// ここに来たということは，何らかの理由で GPS 切断
			}
			
			if (bDebug) Log.d("VSDroid:GPS", "GpsInterface:run() loop extting.")
			
			Close()
			
			bKillThread = false
			if (bDebug) Log.d("VSDroid:GPS", "GpsInterface:exit_run()")
		}
		GpsThread!!.start()
	}

	fun KillThread() {
		bKillThread = true

		if (bDebug) Log.d("VSDroid:GPS", "GpsInterface:KillThread: killing...")
		try {
			while (bKillThread && GpsThread != null && GpsThread!!.isAlive) {
				Thread.sleep(100)
				Close()
			}
		} catch (_: InterruptedException) {}

		GpsThread = null
		if (bDebug) Log.d("VSDroid:GPS", "GpsInterface:KillThread: done")

		GpsData = null
	}

	companion object {
		val bDebug: Boolean = BuildConfig.DEBUG
	}
}
