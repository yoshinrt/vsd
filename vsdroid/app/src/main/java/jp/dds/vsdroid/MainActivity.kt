package jp.dds.vsdroid

import android.Manifest
import android.app.Activity
import android.bluetooth.BluetoothAdapter
import android.content.BroadcastReceiver
import android.content.Context
import android.content.Intent
import android.content.IntentFilter
import android.content.SharedPreferences
import android.content.pm.PackageManager
import android.net.Uri
import android.os.BatteryManager
import android.os.Build
import android.os.Bundle
import android.os.Environment
import android.os.Handler
import android.os.Looper
import android.os.Message
import android.provider.Settings
import android.util.Log
import android.view.KeyEvent
import android.view.View
import android.view.WindowInsets
import android.view.WindowManager
import android.widget.Button
import android.widget.Toast
import androidx.activity.enableEdgeToEdge
import androidx.activity.result.ActivityResult
import androidx.activity.result.contract.ActivityResultContracts
import androidx.appcompat.app.AppCompatActivity
import androidx.core.app.ActivityCompat
import androidx.core.content.ContextCompat
import androidx.preference.PreferenceManager
import kotlinx.coroutines.*
import kotlin.concurrent.thread
import kotlin.coroutines.EmptyCoroutineContext

class MainActivity : AppCompatActivity() {
	// VSD コミュニケーション
	private var Vsd: VsdInterface? = null
	private var VsdScreen: VsdSurfaceView? = null
	private var Pref: SharedPreferences? = null
	private var Setting: VsdSurfaceView.ViewSetting = VsdSurfaceView.ViewSetting()

	companion object {
		val bDebug: Boolean = BuildConfig.DEBUG
		const val strVsdHome: String = "/sdcard/OneDrive/vsd/"
		const val PERMISSION_REQUEST_BLUETOOTH_CONNECT = 1
	}

	//////////////////////////////////////////////////////////////////////////

	// VSD メッセージハンドラ
	var VsdMsgHandler: Handler = object : Handler(Looper.getMainLooper()) {
		override fun handleMessage(Msg: Message) {
			VsdScreen?.Draw(Msg.what, Vsd, Setting)
		}
	}

	//*** config メニュー ****************************************************

	val RequestConfig =
		registerForActivityResult(ActivityResultContracts.StartActivityForResult()) { result: ActivityResult? ->

			when (result?.resultCode) {
				SettingsActivity.RESULT_OK -> {
					UpdateSetting()
				}

				SettingsActivity.RESULT_RESTART_VSD -> {
					if (bDebug) Log.d("VSDroid", "ConfigResult")

					// VSD I/F を生成し直す
					CreateVsdInterface()
				}

				SettingsActivity.RESULT_RECREATE_LOG -> {
					Vsd?.CloseLog()
					Vsd?.OpenLog()
				}

				SettingsActivity.RESULT_CALIBRATION -> {
					Vsd?.Calibration()
				}
			}
		}

	// config を開く
	fun OpenMenu() {

		// デフォルト値設定
		if (Pref!!.getString("key_system_dir", null) == null) {
			val ed = Pref!!.edit()
			ed.putString("key_system_dir", strVsdHome)
			ed.putString("key_roms", strVsdHome + "vsd2.mot")
			ed.putString("key_replay_log", strVsdHome + "vsd.log")
			ed.putString("key_circuit", strVsdHome + "circuit/未選択")

			ed.putString("key_bt_devices_gps", "887 GPS / 00:1C:88:31:0A:77")
			ed.putString("key_bt_devices_vsd", "VSD / 00:12:02:10:01:76")
			ed.putString("key_ip_addr", "192.168.0.13")
			ed.putString("key_connection_mode", "1")
			ed.putString("key_fw_send_wait", "0")
			ed.putBoolean("key_use_btgps", true)
			ed.commit()
		}

		RequestConfig.launch(Intent(this, SettingsActivity::class.java))
	}

	//////////////////////////////////////////////////////////////////////////

	/*** バッテリー関係  */
	private val mBroadcastReceiver: BroadcastReceiver = object : BroadcastReceiver() {
		override fun onReceive(context: Context, intent: Intent) {
			val action = intent.action

			if (action == Intent.ACTION_BATTERY_CHANGED) {
				val iStatus = intent.getIntExtra("status", 0)
				Setting.run {
					this.iBattery = if ((
								iStatus == BatteryManager.BATTERY_STATUS_DISCHARGING ||
										iStatus == BatteryManager.BATTERY_STATUS_NOT_CHARGING)
					) intent.getIntExtra("level", 0) * 100 / intent.getIntExtra("scale", 0) else 100
				}
			}
		}
	}

	//////////////////////////////////////////////////////////////////////////
	// 接続モードに合わせて VSD I/F を再生成

	fun CreateVsdInterface() {

		// VsdThread が死んでいたら，一旦 kill する
		Vsd?.KillThread()

		// VSD コネクション
		val iConnMode = Pref!!.getString("key_connection_mode", "0")!!.toInt()
		when (iConnMode) {
			VsdInterface.CONN_MODE_BLUETOOTH -> Vsd = VsdInterfaceBluetooth(this)
			VsdInterface.CONN_MODE_LOGREPLAY -> Vsd = VsdInterfaceEmulation(this)
			else -> Vsd = VsdInterface(this)
		}

		UpdateSetting()
		Vsd?.Start()        // VSD スレッド起動
	}

	fun UpdateSetting() {
		Setting.bDebugInfo = Pref!!.getBoolean("key_debug_info", false)
		Setting.bEcoMode = Pref!!.getBoolean("key_eco_mode", false) // エコモードw

		Vsd?.iConnMode = Pref!!.getString("key_connection_mode", "0")!!.toInt()
		Vsd?.Pref = Pref
		Vsd?.MsgHandler = VsdMsgHandler
	}

	//************************************************************************

	override fun onCreate(savedInstanceState: Bundle?) {
		super.onCreate(savedInstanceState)

		enableEdgeToEdge()

		setContentView(R.layout.activity_main)
		window.addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON)

		HideSystemBar()

		VsdScreen = findViewById<VsdSurfaceView>(R.id.MainSurfaceView)
		VsdScreen?.setFocusable(true); // キーイベントを使うために必須
		VsdScreen?.setFocusableInTouchMode(true);
		VsdScreen?.requestFocus(); // フォーカスを当てないとキーイベントを拾わない

		// メニューボタン
		val myButton = findViewById<Button>(R.id.MenuButton)
		myButton.setOnClickListener(object : View.OnClickListener {
			override fun onClick(v: View?) {
				OpenMenu()
			}
		})

		// preference 参照
		Pref = PreferenceManager.getDefaultSharedPreferences(this)

		if (Pref?.getString("key_system_dir", null) == null) {
			// 初回起動時は config を起動して pref を初期化
			OpenMenu()
		} else {
			CreateVsdInterface()
		}


		RequestBluetoothConnectPermission()				// Bluetooth 権限
		RequestFileAccessPermission()					// File アクセス権
		VsdScreen?.setOnKeyListener(KeyListener())		// Vol キー
	}

	//////////////////////////////////////////////////////////////////////////
	// File access permittion

	fun RequestFileAccessPermission() {
		if (!Environment.isExternalStorageManager()) {
			// ユーザーに権限をリクエスト
			//val uri = Uri.parse("package:$packageName")
			val intent = Intent(Settings.ACTION_MANAGE_ALL_FILES_ACCESS_PERMISSION)//, uri)
			startActivity(intent)
			Toast.makeText(this, "MANAGE_EXTERNAL_STORAGE権限がありません", Toast.LENGTH_LONG).show()
		}
	}

	//////////////////////////////////////////////////////////////////////////
	// Key

	inner class KeyListener : View.OnKeyListener {
		private var ButtonPushCnt = 0
		private val ButtonPushInterval = 500L

		val scope = CoroutineScope(EmptyCoroutineContext)

		override fun onKey(v: View?, keyCode: Int, event: KeyEvent?): Boolean {
			if (event!!.action == KeyEvent.ACTION_DOWN){
				scope.coroutineContext.cancelChildren()

				when (keyCode) {
					KeyEvent.KEYCODE_VOLUME_UP -> {
						++ButtonPushCnt
						scope.launch(Dispatchers.Default) {
							// 500ms 以内に push で連続扱い
							delay(ButtonPushInterval)
							Log.d(
								"VSDroid",
								"Push button(delay) count = " + ButtonPushCnt.toString()
							)

							withContext(Dispatchers.Main){
								when (ButtonPushCnt) {
									1 -> {
										Toast.makeText(this@MainActivity, "ラップ計測初期化", Toast.LENGTH_LONG).show()
										thread{this@MainActivity.Vsd?.SetToReadyState()}
									}
									2 -> {
										Toast.makeText(this@MainActivity, "最速ラップ削除", Toast.LENGTH_LONG).show()
										this@MainActivity.Vsd?.DeleteFastestLap()
									}
									3 -> this@MainActivity.Vsd?.Calibration()
									else -> {}
								}
								ButtonPushCnt = 0
							}
						}
					}

					KeyEvent.KEYCODE_BACK -> this@MainActivity.finish()
				}
				return true
			}
			return false
		}
	}

	//////////////////////////////////////////////////////////////////////////
	// onPause / onResume

	override fun onPause() {
		if (bDebug) Log.d("VSDroid", "onPause")
		super.onPause()
		unregisterReceiver(mBroadcastReceiver)
	}

	override fun onResume() {
		if (bDebug) Log.d("VSDroid", "onResume")

		super.onResume()
		VsdScreen?.requestFocus(); // フォーカスを当てないとキーイベントを拾わない
		HideSystemBar()

		// バッテリー
		val filter = IntentFilter()
		filter.addAction(Intent.ACTION_BATTERY_CHANGED)
		registerReceiver(mBroadcastReceiver, filter)
	}

	//////////////////////////////////////////////////////////////////////////
	// SystemBar を隠す

	fun HideSystemBar(){
		// ステータスバーを隠す
		@Suppress("DEPRECATION")
		if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.R) {
			window.insetsController?.hide(WindowInsets.Type.systemBars())
		} else {
			window.setFlags(
				WindowManager.LayoutParams.FLAG_FULLSCREEN,
				WindowManager.LayoutParams.FLAG_FULLSCREEN
			)
		}
	}

	//////////////////////////////////////////////////////////////////////////
	// Bluetooth permittion 取得

	private fun RequestBluetoothConnectPermission() {
		if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.S) {
			if (ContextCompat.checkSelfPermission(
					this,
					Manifest.permission.BLUETOOTH_CONNECT
				) != PackageManager.PERMISSION_GRANTED
			) {
				ActivityCompat.requestPermissions(
					this,
					arrayOf(Manifest.permission.BLUETOOTH_CONNECT),
					PERMISSION_REQUEST_BLUETOOTH_CONNECT
				)
				return
			}
		}
		RequestBluetooth.launch(Intent(BluetoothAdapter.ACTION_REQUEST_ENABLE))
	}

	override fun onRequestPermissionsResult(
		requestCode: Int,
		permissions: Array<String>,
		grantResults: IntArray
	) {
		super.onRequestPermissionsResult(requestCode, permissions, grantResults)

		if (requestCode == PERMISSION_REQUEST_BLUETOOTH_CONNECT) {
			if (grantResults.size > 0 && grantResults[0] == PackageManager.PERMISSION_GRANTED) {
				// パーミッションが許可された場合
				RequestBluetooth.launch(Intent(BluetoothAdapter.ACTION_REQUEST_ENABLE))
			} else {
				// パーミッションが拒否された場合
				Toast.makeText(
					this,
					"Bluetooth権限がありません",
					Toast.LENGTH_LONG
				).show()
			}
		}
	}

	// Bluetooth On
	val RequestBluetooth =
		registerForActivityResult(ActivityResultContracts.StartActivityForResult()) { result: ActivityResult? ->
			if (result?.resultCode == Activity.RESULT_OK) {
			}else{
				Toast.makeText(
					this,
					"BluetoothがOFF",
					Toast.LENGTH_LONG
				).show()
			}
		}

	//////////////////////////////////////////////////////////////////////////

	override fun onDestroy() {
		super.onDestroy()
		Vsd?.KillThread()
		if (bDebug) Log.d("VSDroid", "Activity::onDestroy finished")
	}
}
