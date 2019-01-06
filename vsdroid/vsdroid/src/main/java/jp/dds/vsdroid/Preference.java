package jp.dds.vsdroid;

import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothDevice;
import android.content.SharedPreferences;
import android.content.SharedPreferences.OnSharedPreferenceChangeListener;
import android.os.Bundle;
import android.preference.*;

import java.io.File;
import java.io.FilenameFilter;
import java.lang.CharSequence;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Set;

public class Preference extends PreferenceActivity implements OnSharedPreferenceChangeListener {

	private ListPreference		ListMode;
	private ListPreference		ListSectors;
	private ListPreference		ListConnMode;
	private ListPreference		ListBTDevicesVsd;
	private ListPreference		ListBTDevicesGps;
	private ListPreference		ListLogHz;
	private ListPreference		ListWheelSelect;
	private EditTextPreference	EditGymkhaStart;
	private EditTextPreference	EditIPAddr;
	private jp.dds.dds_lib.FileOpenPreference   fopReplayLog;
	private jp.dds.dds_lib.FileOpenPreference   fopFirmware;
	private jp.dds.dds_lib.FileOpenPreference   fopCircuit;

	static final int	RESULT_OK		= 0;
	static final int	RESULT_RENEW	= 1;

	// create
	@Override
	public void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		addPreferencesFromResource( R.xml.preference );

		ListMode			= ( ListPreference	 )getPreferenceScreen().findPreference( "key_vsd_mode" );
		ListSectors			= ( ListPreference	 )getPreferenceScreen().findPreference( "key_sectors" );
		ListConnMode		= ( ListPreference	 )getPreferenceScreen().findPreference( "key_connection_mode" );
		ListBTDevicesVsd	= ( ListPreference	 )getPreferenceScreen().findPreference( "key_bt_devices_vsd" );
		ListBTDevicesGps	= ( ListPreference	 )getPreferenceScreen().findPreference( "key_bt_devices_gps" );
		ListLogHz			= ( ListPreference	 )getPreferenceScreen().findPreference( "key_log_hz" );
		ListWheelSelect		= ( ListPreference	 )getPreferenceScreen().findPreference( "key_wheel_select" );
		EditGymkhaStart		= ( EditTextPreference )getPreferenceScreen().findPreference( "key_gymkha_start" );
		EditIPAddr			= ( EditTextPreference )getPreferenceScreen().findPreference( "key_ip_addr" );
		
		fopReplayLog		= ( jp.dds.dds_lib.FileOpenPreference )getPreferenceScreen().findPreference( "key_replay_log" );
		fopFirmware			= ( jp.dds.dds_lib.FileOpenPreference )getPreferenceScreen().findPreference( "key_roms" );
		fopCircuit			= ( jp.dds.dds_lib.FileOpenPreference )getPreferenceScreen().findPreference( "key_circuit" );

		Bundle extras = getIntent().getExtras();
		setResult( RESULT_OK );

		//////////////////////////////////////////////////////////////////////
		// BT デバイスリストの作成
		// http://web.dimension-maker.info/archives/2010/11/22163814.html
		//////////////////////////////////////////////////////////////////////

		// 項目の取得。 ArrayList と Arrayの変換
		ArrayList<CharSequence> entriesList = new ArrayList<CharSequence> ();

		// Get a set of currently paired devices
		Set<BluetoothDevice> pairedDevices = BluetoothAdapter.getDefaultAdapter().getBondedDevices();

		// If there are paired devices, add each one to the ArrayAdapter
		String s;
		for( BluetoothDevice device : pairedDevices ){
			s = device.getName() + " / " + device.getAddress();
			entriesList.add( s );
		}

		// 各配列を再度当てはめる。
		CharSequence entries[]		= entriesList.toArray( new CharSequence[]{} );
		Arrays.sort( entries );
		ListBTDevicesVsd.setEntries( entries );
		ListBTDevicesVsd.setEntryValues( entries );
		ListBTDevicesGps.setEntries( entries );
		ListBTDevicesGps.setEntryValues( entries );
	}

	static FilenameFilter getFileExtensionFilter( String extension ){
		final String _extension = extension;
		return new FilenameFilter(){
			@Override
			public boolean accept( File file, String name ){
				return name.endsWith( _extension );
			}
		};
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
			String s = sharedPreferences.getString( "key_gymkha_start", null );
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
			EditIPAddr.setSummary( sharedPreferences.getString( "key_ip_addr", null ));
		}

		if( key == null || key.equals( "key_bt_devices_vsd" )){
			ListBTDevicesVsd.setSummary( sharedPreferences.getString( "key_bt_devices_vsd", "Not selected" ));
		}
		
		if( key == null || key.equals( "key_bt_devices_gps" )){
			ListBTDevicesGps.setSummary( sharedPreferences.getString( "key_bt_devices_gps", "Not selected" ));
		}
		
		if( key == null || key.equals( "key_log_hz" )){
			String s = sharedPreferences.getString( "key_log_hz", "16" );
			try{
				Integer.parseInt( s );
			}catch( NumberFormatException e ){
				s = "16";
			}
			ListLogHz.setSummary( s );
		}

		if( key == null || key.equals( "key_wheel_select" )){
			ListWheelSelect.setSummary(
				sharedPreferences.getString( "key_wheel_select", "CE28N" )
			);
		}
		
		if( key == null || key.equals( "key_replay_log" )){
			fopReplayLog.setSummary(
				sharedPreferences.getString( "key_replay_log", null )
					.replaceAll( ".*/", "" )
			);
		}
		
		if( key == null || key.equals( "key_roms" )){
			fopFirmware.setSummary(
				sharedPreferences.getString( "key_roms", null )
					.replaceAll( ".*/", "" )
			);
		}
		
		if( key == null || key.equals( "key_circuit" )){
			fopCircuit.setSummary(
				sharedPreferences.getString( "key_circuit", null )
					.replaceAll( ".*/", "" )
                    .replaceAll( "\\.[^\\.]*$", "" )
			);
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

		if(
			key.equals( "key_connection_mode" ) || key.equals( "key_roms" ) ||
			key.equals( "key_circuit" ) || key.equals( "key_bt_devices_gps" )
		){
			setResult( RESULT_RENEW );
		}
	}
}
