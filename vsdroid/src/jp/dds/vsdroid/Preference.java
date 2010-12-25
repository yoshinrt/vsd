package jp.dds.vsdroid;

import android.content.SharedPreferences;
import android.content.SharedPreferences.OnSharedPreferenceChangeListener;
import android.os.Bundle;
import android.preference.*;

public class Preference extends PreferenceActivity implements OnSharedPreferenceChangeListener {

	private ListPreference		ListMode;
	private ListPreference		ListSectors;
	private EditTextPreference	EditGymkhaStart;
	private EditTextPreference	EditIPAddr;

	// create
	@Override
	public void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		addPreferencesFromResource( R.xml.preference );

		ListMode		= ( ListPreference     )getPreferenceScreen().findPreference( "key_vsd_mode" );
		ListSectors		= ( ListPreference     )getPreferenceScreen().findPreference( "key_sectors" );
		EditGymkhaStart	= ( EditTextPreference )getPreferenceScreen().findPreference( "key_gymkha_start" );
		EditIPAddr		= ( EditTextPreference )getPreferenceScreen().findPreference( "key_ip_addr" );

		Bundle extras = getIntent().getExtras();
		if( extras != null ){
			getPreferenceScreen().findPreference( "key_status" ).
				setTitle( extras.getCharSequence( "Message" ));
		}
	}

	// callback ìoò^ÅEâèú
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

	// ê›íËïœçXéû
	private void SetupSummery( SharedPreferences sharedPreferences, String key ){
		if( key == null || key.equals( "key_vsd_mode" )){
			String s = sharedPreferences.getString( "key_vsd_mode", "LAPTIME" );

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

		if( key == null || key.equals( "key_ip_addr" )){
			EditIPAddr.setSummary( sharedPreferences.getString( "key_ip_addr", "192.168.0.1" ));
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
