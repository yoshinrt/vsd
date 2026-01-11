package jp.dds.vsdroid

import android.bluetooth.BluetoothAdapter
import android.bluetooth.BluetoothDevice
import android.bluetooth.BluetoothManager
import android.bluetooth.BluetoothSocket
import android.content.Context
import android.content.SharedPreferences
import android.os.Handler
import android.util.Log
import java.io.BufferedReader
import java.io.IOException
import java.io.InputStreamReader
import java.io.OutputStream
import java.net.SocketTimeoutException
import java.time.LocalDateTime
import java.util.UUID
import kotlin.concurrent.Volatile
import kotlin.math.floor

//*** VSD アクセス *******************************************************
class GpsInterface(context: Context?, var MsgHandler: Handler?, var Pref: SharedPreferences?) : Runnable {
	var GpsThread: Thread? = null

	// Bluetooth アダプタ
	var device: BluetoothDevice? = null
	var BTSock: BluetoothSocket? = null
	var mBluetoothAdapter: BluetoothAdapter? = null

	var brInput: BufferedReader? = null
	var OutStream: OutputStream? = null

	@Volatile
	var bKillThread: Boolean = false
	var iPrevSec: Int = 0
	var iLogCntPerSec: Int = 0
	var fUpdateRate: Float = 0.0F

	private var UbxInitData: ByteArray = byteArrayOf(
		/*  0 */	0xB5.toByte(), 0x62.toByte(),	// UBX protocol
		/*  2 */	0x06.toByte(), 0x8A.toByte(),	// CFG-VALSET
		/*  4 */	0x13.toByte(), 0x00.toByte(), 	// Len
		/*  6 */	0x00.toByte(),					// Ver.
		/*  7 */	0x01.toByte(),					// Layer = SRAM
		/*  8 */	0x00.toByte(), 0x00.toByte(),	// Reserved
		/* 10 */	0xBB.toByte(), 0x00.toByte(), 0x91.toByte(), 0x20.toByte(), 0x00.toByte(),	// GGA
		/* 15 */	0xC0.toByte(), 0x00.toByte(), 0x91.toByte(), 0x20.toByte(), 0x00.toByte(),	// GSA
		/* 20 */	0xC5.toByte(), 0x00.toByte(), 0x91.toByte(), 0x20.toByte(), 0x00.toByte(),	// GSV
		/* 25 */	0x22.toByte(), 0x00.toByte(), 0x31.toByte(), 0x10.toByte(), 0x01.toByte(),	// BeiDou
		/* 30 */	0x00.toByte(), 0x00.toByte()	// checksum
	)

	//////////////////////////////////////////////////////////////////////////
	// GPS データ

	class CGpsData (var iNmeaTime: Int) {
		var GpsTime: LocalDateTime? = null
		var dLong: Double = Double.NaN
		var dLati: Double = 0.0
		var fAlt: Float = 0.0F
		var fSpeed: Float = 0.0F
		var fUpdateRate: Float = 0.0F
	}

	protected var GpsData: CGpsData? = null

	//*** リトライしないerror ************************************************

	class UnrecoverableException(message: String?) : Exception(message) {
		companion object {
			private const val serialVersionUID = 1L
		}
	}

	//************************************************************************
	// コンストラクタ - 変数の初期化だけやってる
	init {
		if (bDebug) Log.d("VSDroid", "GpsInterface:GpsInterface::constructor")

		val bluetoothManager = context?.getSystemService(Context.BLUETOOTH_SERVICE) as BluetoothManager
		mBluetoothAdapter = bluetoothManager.getAdapter()
	}

	@Throws(IOException::class, UnrecoverableException::class)
	fun Open() {
		if (bDebug) Log.d("VSDroid", "GpsInterface:GpsInterface::Open")
		// If the adapter is null, then Bluetooth is not supported
		if (mBluetoothAdapter == null) {
			//MsgHandler.sendEmptyMessage( R.string.statmsg_bluetooth_na );
			throw UnrecoverableException("Bluetooth error")
		}

		// BT ON でなければエラーで帰る
		if (bDebug) Log.d("VSDroid", "GpsInterface:GpsInterface::Enable")
		if (!mBluetoothAdapter!!.isEnabled) {
			throw UnrecoverableException("Bluetooth error")
		}

		// BT の MAC アドレスを求める
		val s = Pref!!.getString("key_bt_devices_gps", null)
			?: //MsgHandler.sendEmptyMessage( R.string.statmsg_bluetooth_dev_not_selected );
			throw UnrecoverableException("Bluetooth error")
		device = mBluetoothAdapter!!.getRemoteDevice(s.substring(s.length - 17))
		
		try {
			//MsgHandler.sendEmptyMessage( R.string.statmsg_bluetooth_connecting );
			// Get a BluetoothSocket for a connection with the
			// given BluetoothDevice
			if (bDebug) Log.d("VSDroid", "GpsInterface:GpsInterface::createRfcommSocket")
			BTSock = device!!.createInsecureRfcommSocketToServiceRecord(BT_UUID)

			// ソケットの作成 12秒でタイムアウトらしい
			if (bDebug) Log.d("VSDroid", "GpsInterface:GpsInterface::Open:connecting...")
			BTSock!!.connect()
			val `in` = BTSock!!.getInputStream()
			brInput = BufferedReader(InputStreamReader(`in`))
			OutStream = BTSock!!.getOutputStream()

			if (bDebug) Log.d("VSDroid", "GpsInterface:GpsInterface::Open:connected")
			//MsgHandler.sendEmptyMessage( R.string.statmsg_gps_connected );
		} catch (e: SocketTimeoutException) {
			if (bDebug) Log.d("VSDroid", "GpsInterface:GpsInterface::Open:timeout")
			Close()
			throw IOException("Bluetooth error")
		} catch (e: SecurityException) {
			if (bDebug) Log.d("VSDroid", "GpsInterface:GpsInterface::Open:SeculityException")
			Close()
			throw e
		} catch (e: IOException) {
			if (bDebug) Log.d("VSDroid", "GpsInterface:GpsInterface::Open:IOException")
			Close()
			throw e
		}
	}

	fun Close(): Int {
		if (bDebug) Log.d("VSDroid", "GpsInterface:GpsInterface::Close")

		try {
			if (brInput != null) {
				brInput!!.close()
				brInput = null
			}
		} catch (_: IOException) {}

		try {
			if (OutStream != null) {
				OutStream!!.close()
				OutStream = null
			}
		} catch (_: IOException) {}

		try {
			if (BTSock != null) {
				BTSock!!.close()
				BTSock = null
			}
		} catch (_: IOException) {}

		GpsData = null

		return 0
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
		try {
			return (
					ParseInt(str.substring(0, 2), 0) * 3600 * 1000 +
							ParseInt(str.substring(2, 4), 0) * 60 * 1000 +
							(ParseDouble(str.substring(4)) * 1000).toInt()
					)
		} catch (_: Exception){}
		return 0
	}

	private fun IsNewGpsRecord(GpsData: CGpsData?, strTime: String): CGpsData {
		val iTime = ParseTime(strTime)

		// GpsData が null の場合は new
		if (GpsData == null){
			return CGpsData(iTime)
		}

		// 直前の iNmeeTime と異なる場合，直前のレコードを出力
		if(GpsData.iNmeaTime != iTime) {
			GpsData.fUpdateRate = fUpdateRate
			if(GpsData.GpsTime != null) MsgHandler!!.sendMessage(
				MsgHandler!!.obtainMessage(R.string.statmsg_gps_updated, GpsData)
			)
			return CGpsData(iTime)
		}

		return GpsData
	}

	@Throws(IOException::class)
	fun Read() {
		val line = brInput!!.readLine()
		//Log.d("VSDroid", "GPS:" + line)

		try{
			if (line.startsWith("RMC", 3)) {
				val str = line.split(",", limit = 11)
				
				//      時間           lat           long           knot  方位   日付
				// 0      1          2 3           4 5            6 7     8      9
				// $GPRMC,043431.200,A,3439.997825,N,13523.377978,E,0.602,178.29,240612,,,A*59
	
				GpsData = IsNewGpsRecord(GpsData, str[1])
	
				// Lat
				GpsData!!.dLati = ParseDouble(str[3])
				GpsData!!.dLati = floor(GpsData!!.dLati / 100) + (GpsData!!.dLati % 100.0) / 60.0
				if (str[4].startsWith("S")) GpsData!!.dLati = -GpsData!!.dLati
	
				// Long
				GpsData!!.dLong = ParseDouble(str[5])
				GpsData!!.dLong = floor(GpsData!!.dLong / 100) + (GpsData!!.dLong % 100.0) / 60.0
				if (str[6].startsWith("W")) GpsData!!.dLong = -GpsData!!.dLong
	
				// Speed
				GpsData!!.fSpeed = (ParseDouble(str[7]) * 1.85200).toFloat()
	
				// Calender
				if(!str.isEmpty()) {
					GpsData!!.GpsTime = LocalDateTime.of(
						ParseInt(str[9].substring(4, 6), 0) + 2000,
						ParseInt(str[9].substring(2, 4), 1),
						ParseInt(str[9].substring(0, 2), 1),
						GpsData!!.iNmeaTime / (3600 * 1000),
						GpsData!!.iNmeaTime / (60 * 1000) % 60,
						GpsData!!.iNmeaTime / (1000) % 60,
						(GpsData!!.iNmeaTime % 1000) * 1000000
					)
				}
	
				// Update rate
				val iSec = GpsData!!.iNmeaTime / (1000) % 60
				if(iPrevSec != iSec){
					if(fUpdateRate > iLogCntPerSec.toFloat()) {
						fUpdateRate = iLogCntPerSec.toFloat()
					}else {
						fUpdateRate = (fUpdateRate + iLogCntPerSec.toFloat()) / 2
					}
					iLogCntPerSec = 1
					iPrevSec = iSec
				}else{
					++iLogCntPerSec
				}
	
			} else if (line.startsWith("GGA", 3)) {
				val str = line.split(",", limit = 11)
				
				//        時間       lat           long               高度
				// 0      1          2           3 4            5 6 789
				// $GPGGA,233132.000,3439.997825,N,13523.377978,E,1,,,293.425,M,,,,*21
	
				GpsData = IsNewGpsRecord(GpsData, str[1])
				GpsData!!.fAlt = ParseDouble(str[9]).toFloat()
			}
		}catch(_: Exception){
			GpsData = null
		}
	}

	//*** データ処理スレッド *********************************************
	fun Start() {
		if (bDebug) Log.d("VSDroid", "Start()")
		// VSD スレッド起動
		GpsThread = Thread(this)
		GpsThread!!.start()
	}

	override fun run() {
		bKillThread = false

		if (bDebug) Log.d("VSDroid", "GpsInterface:run_loop()")
		GpsData = null

		MainLoop@ while (!bKillThread) {
			// 開く，失敗したらやり直し

			while (!bKillThread) {
				try {
					if (bDebug) Log.d("VSDroid", "GpsInterface:opening...")
					Close() // 開いていたら一旦 close
					Open()
					break
				} catch (e: UnrecoverableException) {
					// BT off 等，リカバリ不能な問題
					if (bDebug) Log.d("VSDroid", "GpsInterface:Unrecoverable exception")
					break@MainLoop
				} catch (e: IOException) {
					// 一般的な IOException
					if (bDebug) Log.d("VSDroid", "GpsInterface:retrying open...")
					try {
						Thread.sleep((3 * 1000).toLong())
					} catch (_: InterruptedException) {}
				}
			}

			if (bDebug) Log.d("VSDroid", "GpsInterface:open done.")
			
			// GGA センテンス使用
			UbxInitData[14] = (if(Pref!!.getBoolean("key_use_ubx_gga", true)) 1 else 0).toByte()
			
			// BeiDou
			UbxInitData[29] = (if(Pref!!.getBoolean("key_use_ubx_beidou", true)) 1 else 0).toByte()
			
			// 初期化コード送信
			OutStream?.write(AddUbxChecksum(UbxInitData))
			OutStream?.flush()

			iPrevSec = -1
			iLogCntPerSec = 0
			fUpdateRate = 10.0F
			
			// 1行読む
			while (!bKillThread) {
				try {
					Read()
				} catch (e: IOException) {
					break
				}
			}
			
			// ここに来たということは，何らかの理由で GPS 切断
			MsgHandler?.sendEmptyMessage( R.string.statmsg_gps_disconnected );
		}

		if (bDebug) Log.d("VSDroid", "GpsInterface:run() loop extting.")

		Close()

		bKillThread = false
		if (bDebug) Log.d("VSDroid", "GpsInterface:exit_run()")
	}

	fun KillThread() {
		bKillThread = true

		if (bDebug) Log.d("VSDroid", "GpsInterface:KillThread: killing...")
		try {
			while (bKillThread && GpsThread != null && GpsThread!!.isAlive) {
				Thread.sleep(100)
				Close()
			}
		} catch (_: InterruptedException) {}

		GpsThread = null
		if (bDebug) Log.d("VSDroid", "GpsInterface:KillThread: done")

		GpsData = null
	}

	fun AddUbxChecksum(data: ByteArray): ByteArray{
		var a: UInt = 0u
		var b: UInt = 0u
		
		data[5] = ((data.size - 6 - 2) shl 8   ).toByte()
		data[4] = ((data.size - 6 - 2) and 0xFF).toByte()
		
		for(i in 2 .. data.size - 3){
			a += data[i].toUInt()
			b += a
		}
		data[data.size - 2] = a.toByte()
		data[data.size - 1] = b.toByte()
		return data
	}

	companion object {
		val bDebug: Boolean = BuildConfig.DEBUG

		private val BT_UUID: UUID = UUID.fromString("00001101-0000-1000-8000-00805F9B34FB")
	}
}
