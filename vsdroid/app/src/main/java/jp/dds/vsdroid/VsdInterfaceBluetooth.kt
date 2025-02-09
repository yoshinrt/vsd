package jp.dds.vsdroid

import android.bluetooth.BluetoothAdapter
import android.bluetooth.BluetoothDevice
import android.bluetooth.BluetoothManager
import android.bluetooth.BluetoothSocket
import android.content.Context
import android.util.Log
import androidx.activity.ComponentActivity
import java.io.IOException
import java.net.SocketTimeoutException
import java.util.UUID

internal class VsdInterfaceBluetooth(activity: ComponentActivity) : VsdInterface(activity) {
	var device: BluetoothDevice? = null
	var BTSock: BluetoothSocket? = null
	var mBluetoothAdapter: BluetoothAdapter? = null

	init {
		if (bDebug) Log.d("VSDroid", "VsdInterfaceBluetooth::new")
		// Get local Bluetooth adapter
		val bluetoothManager = activity.getSystemService(Context.BLUETOOTH_SERVICE) as BluetoothManager
		mBluetoothAdapter = bluetoothManager.getAdapter()
	}

	@Throws(IOException::class, UnrecoverableException::class)
	override fun OpenVsdIf() {
		if (bDebug) Log.d("VSDroid", "VsdInterfaceBluetooth::OpenVsdIf")
		// If the adapter is null, then Bluetooth is not supported
		if (mBluetoothAdapter == null) {
			MsgHandler!!.sendEmptyMessage(R.string.statmsg_bluetooth_na)
			throw UnrecoverableException("Bluetooth error")
		}

		// BT ON でなければエラーで帰る
		if (bDebug) Log.d("VSDroid", "VsdInterfaceBluetooth::Enable")
		if (!mBluetoothAdapter!!.isEnabled) {
			throw UnrecoverableException( "Bluetooth error" )
		}

		// BT の MAC アドレスを求める
		val s = Pref!!.getString("key_bt_devices_vsd", null)
		if (s == null) {
			MsgHandler!!.sendEmptyMessage(R.string.statmsg_bluetooth_dev_not_selected)
			throw UnrecoverableException("Bluetooth error")
		}
		device = mBluetoothAdapter!!.getRemoteDevice(s.substring(s.length - 17))

		try {
			MsgHandler!!.sendEmptyMessage(R.string.statmsg_bluetooth_connecting)
			// Get a BluetoothSocket for a connection with the
			// given BluetoothDevice
			if (bDebug) Log.d(
				"VSDroid",
				"VsdInterfaceBluetooth::createRfcommSocket"
			)
			BTSock = device!!.createInsecureRfcommSocketToServiceRecord(BT_UUID)

			// ソケットの作成 12秒でタイムアウトらしい
			if (bDebug) Log.d(
				"VSDroid",
				"VsdInterfaceBluetooth::OpenVsdIf:connecting..."
			)
			BTSock!!.connect()
			InStream = BTSock!!.getInputStream()
			OutStream = BTSock!!.getOutputStream()

			if (bDebug) Log.d(
				"VSDroid",
				"VsdInterfaceBluetooth::OpenVsdIf:connected"
			)
			MsgHandler!!.sendEmptyMessage(R.string.statmsg_bluetooth_connected)
		} catch (e: SocketTimeoutException) {
			if (bDebug) Log.d(
				"VSDroid",
				"VsdInterfaceBluetooth::OpenVsdIf:timeout"
			)
			CloseVsdIf()
			throw IOException("Bluetooth error")
		} catch (e: SecurityException) {
			if (bDebug) Log.d(
				"VSDroid",
				"VsdInterfaceBluetooth::OpenVsdIf:SecurityException"
			)
			CloseVsdIf()
			throw e
		} catch (e: IOException) {
			if (bDebug) Log.d(
				"VSDroid",
				"VsdInterfaceBluetooth::OpenVsdIf:IOException"
			)
			CloseVsdIf()
			throw e
		}
	}

	override fun CloseVsdIf() {
		if (bDebug) Log.d("VSDroid", "VsdInterfaceBluetooth::CloseVsdIf")


		// BT 切断すると AT コマンドモードになるので，シリアル出力を止める
		try {
			SendCmd("z")
		} catch (_: IOException) {}

		try {
			if (BTSock != null) {
				BTSock!!.close()
				BTSock = null
			}
		} catch (_: IOException) {}
	}

	companion object {
		private val BT_UUID: UUID = UUID.fromString("00001101-0000-1000-8000-00805F9B34FB")
	}
}
