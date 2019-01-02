package jp.dds.vsdroid;

import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothDevice;
import android.bluetooth.BluetoothSocket;
import android.content.SharedPreferences;
import android.os.Handler;
import android.util.Log;
import java.io.*;
import java.net.SocketTimeoutException;
import java.util.*;
import java.util.Calendar;
import java.util.UUID;

//*** VSD アクセス *******************************************************

class GpsInterface implements Runnable {

	static final boolean bDebug = BuildConfig.DEBUG;

	SharedPreferences Pref;
	Thread GpsThread = null;

	// Bluetooth アダプタ
	BluetoothDevice device;
	BluetoothSocket BTSock = null;
	BluetoothAdapter mBluetoothAdapter = null;
	
	boolean			bDebugInfo	= false;

	BufferedReader	brInput		= null;
	OutputStream	OutStream	= null;

	volatile boolean bKillThread = false;

	Handler	MsgHandler	= null;

    private static final UUID BT_UUID = UUID.fromString( "00001101-0000-1000-8000-00805F9B34FB" );
	
	// GPS データ
	int		iCurrentYear;
	int			iNmeaFlag	= 0;
	int			iNmeaTime	= -1;
	Calendar	GpsTime;
	double		dLong		= Double.NaN;
	double		dLati;
	double		dAlt;
	double		dSpeed;
	
	int			iPrevNmeaTime	= -1;
	double		dPrevLong		= Double.NaN;
	double		dPrevLati;
	
	//*** リトライしないerror ************************************************
	
	public class UnrecoverableException extends Exception {
		private static final long serialVersionUID = 1L;
		public UnrecoverableException( String message ){
			super( message );
		}
	}
	
	//************************************************************************

	// コンストラクタ - 変数の初期化だけやってる
	public GpsInterface( Handler _MsgHandler, SharedPreferences _Pref ){
		if( bDebug ) Log.d( "VSDroid", "GpsInterface:GpsInterface::constructor" );

		MsgHandler		= _MsgHandler;
		Pref			= _Pref;
		
		mBluetoothAdapter = BluetoothAdapter.getDefaultAdapter();
		
		// 現在の年取得
		GpsTime = Calendar.getInstance( TimeZone.getTimeZone( "GMT+0" ));
		iCurrentYear = GpsTime.get( Calendar.YEAR );
	}

	public void Open() throws IOException, UnrecoverableException {

		if( bDebug ) Log.d( "VSDroid", "GpsInterface:GpsInterface::Open" );
		// If the adapter is null, then Bluetooth is not supported
		if( mBluetoothAdapter == null ){
			//MsgHandler.sendEmptyMessage( R.string.statmsg_bluetooth_na );
			throw new UnrecoverableException( "Bluetooth error" );
		}

		// BT ON でなければエラーで帰る
		if( bDebug ) Log.d( "VSDroid", "GpsInterface:GpsInterface::Enable" );
		if( !mBluetoothAdapter.isEnabled()){
			throw new UnrecoverableException( "Bluetooth error" );
		}

		// BT の MAC アドレスを求める
		String s = Pref.getString( "key_bt_devices_gps", null );
		if( s == null ){
			//MsgHandler.sendEmptyMessage( R.string.statmsg_bluetooth_dev_not_selected );
			throw new UnrecoverableException( "Bluetooth error" );
		}
		device = mBluetoothAdapter.getRemoteDevice( s.substring( s.length() - 17 ));

		try{
			//MsgHandler.sendEmptyMessage( R.string.statmsg_bluetooth_connecting );
			// Get a BluetoothSocket for a connection with the
			// given BluetoothDevice
			if( bDebug ) Log.d( "VSDroid", "GpsInterface:GpsInterface::createRfcommSocket" );
			BTSock = device.createInsecureRfcommSocketToServiceRecord( BT_UUID );
			
			// ソケットの作成 12秒でタイムアウトらしい
			if( bDebug ) Log.d( "VSDroid", "GpsInterface:GpsInterface::Open:connecting..." );
			BTSock.connect();
			InputStream in = BTSock.getInputStream();
			brInput		= new BufferedReader( new InputStreamReader( in ));
			OutStream	= BTSock.getOutputStream();
			
			if( bDebug ) Log.d( "VSDroid", "GpsInterface:GpsInterface::Open:connected" );
			//MsgHandler.sendEmptyMessage( R.string.statmsg_gps_connected );
		}catch( SocketTimeoutException e ){
			if( bDebug ) Log.d( "VSDroid", "GpsInterface:GpsInterface::Open:timeout" );
			Close();
			throw new IOException( "Bluetooth error" );
		}catch( IOException e ){
			if( bDebug ) Log.d( "VSDroid", "GpsInterface:GpsInterface::Open:IOException" );
			Close();
			throw e;
		}
	}

	public int Close(){
		if( bDebug ) Log.d( "VSDroid", "GpsInterface:GpsInterface::Close" );
		
		try{
			if( brInput != null ){
				brInput.close();
				brInput = null;
			}
		}catch( IOException e ){}
		
		try{
			if( OutStream != null ){
				OutStream.close();
				OutStream = null;
			}
		}catch( IOException e ){}
		
		try{
			if( BTSock != null ){
				BTSock.close();
				BTSock = null;
			}
		}catch( IOException e ){}
		
		dLong = dPrevLong = Double.NaN;
		
		return 0;
	}

	//*** 1行処理 ********************************************************
	
	private int ParseInt( String str, int iDefault ){
		try{
			return Integer.parseInt( str );
		}catch( Exception e ){}
		return iDefault;
	}
	
	private double ParseDouble( String str ){
		try{
			return Double.parseDouble( str );
		}catch( Exception e ){}
		return Double.NaN;
	}
	
	private int ParseTime( String str ){
		return
			ParseInt( str.substring( 0, 2 ), 0 ) * 3600 * 1000 +
			ParseInt( str.substring( 2, 4 ), 0 ) *   60 * 1000 +
			( int )( ParseDouble( str.substring( 4 ))   * 1000 );
	}
	
	void Read() throws IOException {
		String[] str = brInput.readLine().split( "," );
		
		if( str[ 0 ].equals( "$GPRMC" )){
			dPrevLong		= dLong;
			dPrevLati		= dLati;
			iPrevNmeaTime	= iNmeaTime;
			
			//        時間         lat           long           knot  方位   日付
			// 0      1          2 3           4 5            6 7     8      9
			// $GPRMC,043431.200,A,3439.997825,N,13523.377978,E,0.602,178.29,240612,,,A*59
			
			int iTime = ParseTime( str[ 1 ]);
			
			if( iNmeaFlag == 0 ){
				iNmeaTime	= iTime;
				iNmeaFlag	= 1;
			}else{
				iNmeaFlag	|= 1;
			}
			
			// Lat
			dLati = ParseDouble( str[ 3 ]);
			dLati = Math.floor( dLati / 100 ) + ( dLati % 100.0 ) / 60.0;
			if( str[ 4 ].equals( "S" )) dLati = -dLati;
			
			// Long
			dLong = ParseDouble( str[ 5 ]);
			dLong = Math.floor( dLong / 100 ) + ( dLong % 100.0 ) / 60.0;
			if( str[ 6 ].equals( "W" )) dLong = -dLong;
			
			// Speed
			dSpeed = ParseDouble( str[ 7 ]) * 1.85200;
			
			// 日付
			int iYear	= ParseInt( str[ 9 ].substring( 4, 6 ), 0) +
				( iCurrentYear / 100 * 100 );
			
			if(( iCurrentYear % 100 ) >= 50 && iYear < 50 ) iYear += 100;
			
			GpsTime.set(
				iYear,
				ParseInt( str[ 9 ].substring( 2, 4 ), 1 ) - 1,
				ParseInt( str[ 9 ].substring( 0, 2 ), 1 ),
				iTime / ( 3600 * 1000 ),
				iTime / (   60 * 1000 ) % 60,
				iTime / (        1000 ) % 60
			);
			GpsTime.set( Calendar.MILLISECOND, iTime % 1000 );
		}
		
		else if( str[ 0 ].equals( "$GPGGA" )){
			//        時間       lat           long               高度
			// 0      1          2           3 4            5 6 789
			// $GPGGA,233132.000,3439.997825,N,13523.377978,E,1,,,293.425,M,,,,*21
			
			int iTime = ParseTime( str[ 1 ]);
			
			if( iNmeaFlag == 0 ){
				iNmeaTime	= iTime;
				iNmeaFlag	= 2;
			}else{
				iNmeaFlag	|= 2;
			}
			
			dAlt = ParseDouble( str[ 9 ]);
		}
		
		if( iNmeaFlag == 3 ){
			// NMEA データが 1組分揃った
			
			iNmeaFlag	= 0;
			
			if( !Double.isNaN( dLong )){
				MsgHandler.sendEmptyMessage( R.string.statmsg_gps_updated );
			}
		}
	}
	
	//*** データ処理スレッド *********************************************

	public void Start(){
		// VSD スレッド起動
		GpsThread = new Thread( this );
		GpsThread.start();
	}

	public void run(){
		bKillThread = false;
		
		if( bDebug ) Log.d( "VSDroid", "GpsInterface:run_loop()" );
		
		MainLoop: while( !bKillThread ){
			
			// 開く，失敗したらやり直し
			while( !bKillThread ){
				try{
					if( bDebug ) Log.d( "VSDroid", "GpsInterface:opening..." );
					Close(); // 開いていたら一旦 close
					Open();
					break;
				}catch( UnrecoverableException e ){
					// BT off 等，リカバリ不能な問題
					if( bDebug ) Log.d( "VSDroid", "GpsInterface:Unrecoverable exception" );
					break MainLoop;
				}catch( IOException e ){
					// 一般的な IOException
					if( bDebug ) Log.d( "VSDroid", "GpsInterface:retrying open..." );
					try{ Thread.sleep( 3 * 1000 );
                    }catch( InterruptedException e1 ){}
				}
			}
			
			if( bDebug ) Log.d( "VSDroid", "GpsInterface:open done." );
			
			// 1行読む
			while( !bKillThread ){
				try{
					Read();
				}catch( IOException e ){ break; }
			}
			
			// ここに来たということは，何らかの理由で GPS 切断
			//MsgHandler.sendEmptyMessage( R.string.statmsg_gps_disconnected );
		}
		
		if( bDebug ) Log.d( "VSDroid", "GpsInterface:run() loop extting." );

		Close();

		bKillThread	= false;
		if( bDebug ) Log.d( "VSDroid", "GpsInterface:exit_run()" );
	}

	public void KillThread(){
		bKillThread = true;

		if( bDebug ) Log.d( "VSDroid", "GpsInterface:KillThread: killing..." );
		try{
			while( bKillThread && GpsThread != null && GpsThread.isAlive()){
				Thread.sleep( 100 );
				Close();
			}
		}catch( InterruptedException e ){}

		GpsThread = null;
		if( bDebug ) Log.d( "VSDroid", "GpsInterface:KillThread: done" );
		
		dLong = dPrevLong = Double.NaN;
	}
}
