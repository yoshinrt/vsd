package jp.dds.vsdroid

import android.bluetooth.BluetoothManager
import android.content.Context
import android.content.SharedPreferences
import android.net.Uri
import android.os.Bundle
import android.os.Environment
import androidx.activity.result.contract.ActivityResultContracts
import androidx.appcompat.app.AppCompatActivity
import androidx.preference.ListPreference
import androidx.preference.Preference
import androidx.preference.PreferenceFragmentCompat
import androidx.preference.PreferenceManager
import java.net.URLDecoder
import java.util.Arrays

class SettingsActivity : AppCompatActivity() {

	override fun onCreate(savedInstanceState: Bundle?) {
		super.onCreate(savedInstanceState)
		setContentView(R.layout.settings_activity)
		if (savedInstanceState == null) {
			supportFragmentManager
				.beginTransaction()
				.replace(R.id.settings, SettingsFragment())
				.commit()
		}
		supportActionBar?.setDisplayHomeAsUpEnabled(true)
		
		Pref = PreferenceManager.getDefaultSharedPreferences(this)
	}

	class SettingsFragment : PreferenceFragmentCompat(),
		SharedPreferences.OnSharedPreferenceChangeListener {
		private var ListBTDevicesVsd: ListPreference? = null
		private var ListBTDevicesGps: ListPreference? = null
		
		private var PrefGpsCtrlLine: Preference? = null
		private var PrefVsdDir: Preference? = null
		private var PrefReplayLog: Preference? = null
		private var PrefFirmware: Preference? = null
		private var PrefReOpenLog: Preference? = null
		private var PrefCalibration: Preference? = null
		
		private var PrefContext: Context? = null

		override fun onCreatePreferences(savedInstanceState: Bundle?, rootKey: String?) {
			setPreferencesFromResource(R.xml.root_preferences, rootKey)
		}
		
		// create
		override fun onCreate(savedInstanceState: Bundle?) {
			super.onCreate(savedInstanceState)
			
			ListBTDevicesVsd = findPreference("key_bt_devices_vsd")
			ListBTDevicesGps = findPreference("key_bt_devices_gps")
			PrefGpsCtrlLine = findPreference("key_circuit")
			PrefVsdDir = findPreference("key_system_dir")
			PrefReplayLog = findPreference("key_replay_log")
			PrefFirmware = findPreference("key_roms")
			PrefReOpenLog = findPreference("key_reopen_log")
			PrefCalibration = findPreference("key_caribration")

		//#	fopReplayLog =
		//#		findPreference("key_replay_log") as jp.dds.dds_lib.FileOpenPreference
		//#	fopFirmware =
		//#		findPreference("key_roms") as jp.dds.dds_lib.FileOpenPreference
		//#	fopCircuit =
		//#		findPreference("key_circuit") as jp.dds.dds_lib.FileOpenPreference
	
		//#	val extras = intent.extras
			
			(PrefContext as SettingsActivity).setResult(RESULT_OK)

			//////////////////////////////////////////////////////////////////////
			// BT デバイスリストの作成
			// http://web.dimension-maker.info/archives/2010/11/22163814.html
			//////////////////////////////////////////////////////////////////////

			// 項目の取得。 ArrayList と Arrayの変換
			val entriesList = ArrayList<CharSequence>()

			// Get the local Bluetooth adapter
			val bluetoothManager = PrefContext?.getSystemService(Context.BLUETOOTH_SERVICE) as BluetoothManager
			val mBtAdapter = bluetoothManager.getAdapter()

			// Get a set of currently paired devices
			try {
				val pairedDevices = mBtAdapter.bondedDevices

				// If there are paired devices, add each one to the ArrayAdapter
				var s: String
				for (device in pairedDevices) {
					s = device.name + " / " + device.address
					entriesList.add(s)
				}
			} catch (_: SecurityException) {}

			if(entriesList.size == 0){
				var BtDevice = Pref!!.getString("key_bt_devices", "")
				entriesList.add(BtDevice!!)
			}

			// 各配列を再度当てはめる。
			val entries = entriesList.toArray(arrayOf<CharSequence>())
			Arrays.sort(entries)

			ListBTDevicesVsd!!.entries = entries
			ListBTDevicesVsd!!.entryValues = entries
			ListBTDevicesGps!!.entries = entries
			ListBTDevicesGps!!.entryValues = entries
			
			// ファイル選択ランチャ登録
			PrefGpsCtrlLine?.setOnPreferenceClickListener {
				GpsCtrlLineSelectorLauncher.launch(arrayOf("*/*"))
				true
			}
			
			PrefVsdDir?.setOnPreferenceClickListener {
				VsdDirSelectorLauncher.launch(null)
				true
			}
			
			PrefReplayLog?.setOnPreferenceClickListener {
				ReplayLogSelectorLauncher.launch(arrayOf("*/*"))
				true
			}
			
			PrefFirmware?.setOnPreferenceClickListener {
				FirmwareSelectorLauncher.launch(arrayOf("*/*"))
				true
			}

			// ログ再作成・キャリブレーションボタン登録
			PrefReOpenLog?.setOnPreferenceClickListener {
				(PrefContext as SettingsActivity).setResult(RESULT_RECREATE_LOG)
				(PrefContext as SettingsActivity).finish()
				true // デフォルトの処理を抑制
			}

			PrefCalibration?.setOnPreferenceClickListener {
				(PrefContext as SettingsActivity).setResult(RESULT_CALIBRATION)
				(PrefContext as SettingsActivity).finish()
				true // デフォルトの処理を抑制
			}

		}
		
		//////////////////////////////////////////////////////////////////////
		// Dir 選択ランチャ
		
		private fun Uri2FilePathAndSavePref(uri: Uri, Key: String){
			// DocumentFile作成
			var FileName = URLDecoder.decode(uri.toString(), "UTF-8")
			var Pos = FileName.lastIndexOf(':')
			if(Pos > 0) FileName = FileName.substring(Pos + 1)
			FileName = Environment.getExternalStorageDirectory().getPath() + "/" + FileName

			val editor = Pref!!.edit()
			editor.putString(Key, FileName)
			editor.apply()
		}
		
		// GPS コントロールラインファイル
		val GpsCtrlLineSelectorLauncher = registerForActivityResult(ActivityResultContracts.OpenDocument()) { uri ->
			uri ?: return@registerForActivityResult
			Uri2FilePathAndSavePref(uri, "key_circuit")
		}
		
		// VSD ディレクトリ
		val VsdDirSelectorLauncher = registerForActivityResult(ActivityResultContracts.OpenDocumentTree()) { uri ->
			uri ?: return@registerForActivityResult
			Uri2FilePathAndSavePref(uri, "key_system_dir")
		}
		
		// リプレイログ
		val ReplayLogSelectorLauncher = registerForActivityResult(ActivityResultContracts.OpenDocument()) { uri ->
			uri ?: return@registerForActivityResult
			Uri2FilePathAndSavePref(uri, "key_replay_log")
		}
		
		// ファームウェア
		val FirmwareSelectorLauncher = registerForActivityResult(ActivityResultContracts.OpenDocument()) { uri ->
			uri ?: return@registerForActivityResult
			Uri2FilePathAndSavePref(uri, "key_roms")
		}
		
		//////////////////////////////////////////////////////////////////////
		// callback 登録・解除
		override fun onResume() {
			super.onResume()
			val Pref = PreferenceManager.getDefaultSharedPreferences(PrefContext!!)
			Pref!!.registerOnSharedPreferenceChangeListener(this)
			SetupSummery(Pref!!, null)
		}
	
		override fun onPause() {
			super.onPause()
			val Pref = PreferenceManager.getDefaultSharedPreferences(PrefContext!!)
			Pref!!.unregisterOnSharedPreferenceChangeListener(this)
		}
	
		// 設定変更時
		private fun SetupSummery(Pref: SharedPreferences?, key: String?) {
			if (PrefGpsCtrlLine != null && (key == null || key == "key_circuit")) {
				PrefGpsCtrlLine?.summary = Pref?.getString("key_circuit", "-")
			}
			if (PrefVsdDir != null && (key == null || key == "key_system_dir")) {
				PrefVsdDir?.summary = Pref?.getString("key_system_dir", "-")
			}
			if (PrefReplayLog != null && (key == null || key == "key_replay_log")) {
				PrefReplayLog?.summary = Pref?.getString("key_replay_log", "-")
			}
			if (PrefFirmware != null && (key == null || key == "key_roms")) {
				PrefFirmware?.summary = Pref?.getString("key_roms", "-")
			}
		}
	
		override fun onSharedPreferenceChanged(Pref: SharedPreferences?, key: String?) {
			if (key == "key_reopen_log") {
				(PrefContext as SettingsActivity).finish()
				return
			}
			if (key == "key_caribration") {
				(PrefContext as SettingsActivity).finish()
				return
			}
			SetupSummery(Pref, key)
	
			if (key == "key_connection_mode" || key == "key_roms" ||
				key == "key_circuit" || key == "key_bt_devices_gps"
			) {
				(PrefContext as SettingsActivity).setResult(RESULT_RESTART_VSD)
			}
		}

		override fun onAttach(context: Context) {
			super.onAttach(context)
			PrefContext = context
		}
	}
	
	companion object {
		const val RESULT_OK: Int = 0
		const val RESULT_RESTART_VSD: Int = 1
		const val RESULT_RECREATE_LOG = 2
		const val RESULT_CALIBRATION = 3
		var Pref: SharedPreferences? = null
	}
}
