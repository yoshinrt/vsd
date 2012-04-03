package jp.dds.vsdroid;

import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothDevice;
import android.content.SharedPreferences;
import android.content.SharedPreferences.OnSharedPreferenceChangeListener;
import android.os.Bundle;
import android.preference.*;
import java.lang.CharSequence;
import java.util.ArrayList;
import java.util.Set;

public class Preference extends PreferenceActivity implements OnSharedPreferenceChangeListener {

	private ListPreference		ListMode;
	private ListPreference		ListSectors;
	private ListPreference		ListConnMode;
	private ListPreference		ListBTDevices;
	private EditTextPreference	EditGymkhaStart;
	private EditTextPreference	EditIPAddr;

	// create
	@Override
	public void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		addPreferencesFromResource( R.xml.preference );

		ListMode		= ( ListPreference	 )getPreferenceScreen().findPreference( "key_vsd_mode" );
		ListSectors		= ( ListPreference	 )getPreferenceScreen().findPreference( "key_sectors" );
		ListConnMode	= ( ListPreference	 )getPreferenceScreen().findPreference( "key_connection_mode" );
		ListBTDevices	= ( ListPreference	 )getPreferenceScreen().findPreference( "key_bt_devices" );
		EditGymkhaStart	= ( EditTextPreference )getPreferenceScreen().findPreference( "key_gymkha_start" );
		EditIPAddr		= ( EditTextPreference )getPreferenceScreen().findPreference( "key_ip_addr" );

		Bundle extras = getIntent().getExtras();
		if( extras != null ){
			getPreferenceScreen().findPreference( "key_status" ).
				setTitle( extras.getCharSequence( "Message" ));
		}

		//////////////////////////////////////////////////////////////////////
		// BT デバイスリストの作成
		// http://web.dimension-maker.info/archives/2010/11/22163814.html
		//////////////////////////////////////////////////////////////////////

		// 項目の取得。 ArrayList と Arrayの変換
		ArrayList<CharSequence> entriesList = new ArrayList<CharSequence> ();
		ArrayList<CharSequence> entryValuesList = new ArrayList<CharSequence> ();

		// Get the local Bluetooth adapter
		BluetoothAdapter mBtAdapter = BluetoothAdapter.getDefaultAdapter();

		// Get a set of currently paired devices
		Set<BluetoothDevice> pairedDevices = mBtAdapter.getBondedDevices();

		// If there are paired devices, add each one to the ArrayAdapter
		String s;
		for (BluetoothDevice device : pairedDevices) {
			s = device.getName() + " / " + device.getAddress();
			entriesList.add( s );
			entryValuesList.add( s );
		}

		// 各配列を再度当てはめる。
		CharSequence entries[]		= entriesList.toArray( new CharSequence[]{} );
		CharSequence entryValues[]	= entryValuesList.toArray( new CharSequence[]{} );

		ListBTDevices.setEntries( entries );
		ListBTDevices.setEntryValues( entryValues );
	}

	// callback 登録・解除
	@Override
	protected void onResume() {
		super.onResume();
		SharedPreferences sharedPreferences = getPreferenceScreen().getSharedPreferences();

		SetupSummery( sharedPreferences, null );

		sharedPreferences.registerOnSharedPreferenceChangeListener( this );
	}

	@Override
	protected void onPause() {
		super.onPause();
		getPreferenceScreen().getSharedPreferences().unregisterOnSharedPreferenceChangeListener( this );
	}

	// 設定変更時
	private void SetupSummery( SharedPreferences sharedPreferences, String key ){
		if( key == null || key.equals( "key_vsd_mode" )){
			String s = sharedPreferences.getString( "key_vsd_mode", "0" );

			ListMode.setSummary(
				getString(
					s.equals( "0" ) ? R.string.mode_laptime	:
					s.equals( "1" ) ? R.string.mode_gymkhana:
					s.equals( "2" ) ? R.string.mode_0_400	:
									  R.string.mode_0_100
				)
			);
		}

		if( key == null || key.equals( "key_sectors" )){
			String s = sharedPreferences.getString( "key_sectors", "1" );
			try{
				Integer.parseInt( s );
			}catch( NumberFormatException e ){
				s = "1";
			}
			ListSectors.setSummary( s );
		}

		if( key == null || key.equals( "key_gymkha_start" )){
			String s = sharedPreferences.getString( "key_gymkha_start", "1.0" );
			try{
				Double.parseDouble( s );
			}catch( NumberFormatException e ){
				s = "1.0";
			}
			EditGymkhaStart.setSummary( s );
		}

		if( key == null || key.equals( "key_connection_mode" )){
			String s = sharedPreferences.getString( "key_connection_mode", "0" );

			ListConnMode.setSummary(
				getString(
					s.equals( "0" ) ? R.string.conn_mode_ether		:
					s.equals( "1" ) ? R.string.conn_mode_bluetooth	:
									  R.string.conn_mode_logreplay
				)
			);
		}

		if( key == null || key.equals( "key_ip_addr" )){
			EditIPAddr.setSummary( sharedPreferences.getString( "key_ip_addr", "192.168.0.1" ));
		}

		if( key == null || key.equals( "key_bt_devices" )){
			ListBTDevices.setSummary( sharedPreferences.getString( "key_bt_devices", "Not selected" ));
		}
	}

	public void onSharedPreferenceChanged( SharedPreferences sharedPreferences, String key ){
		if( key.equals( "key_reopen_log" )){
			finish();
			return;
		}
		if( key.equals( "key_caribration" )){
			finish();
			return;
		}
		SetupSummery( sharedPreferences, key );
	}
}
