package jp.dds.vsdroid;

import android.content.SharedPreferences;
import android.os.Handler;
import android.util.Log;
import java.net.Socket;
import java.net.SocketTimeoutException;
import java.io.*;
import java.util.UUID;

import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothDevice;
import android.bluetooth.BluetoothSocket;

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
	}

	public void Open() throws IOException, UnrecoverableException {

		if( bDebug ) Log.d( "VSDroid", "GpsInterface:GpsInterface::Open" );
		// If the adapter is null, then Bluetooth is not supported
		if( mBluetoothAdapter == null ){
			MsgHandler.sendEmptyMessage( R.string.statmsg_bluetooth_na );
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
			MsgHandler.sendEmptyMessage( R.string.statmsg_bluetooth_dev_not_selected );
			throw new UnrecoverableException( "Bluetooth error" );
		}
		device = mBluetoothAdapter.getRemoteDevice( s.substring( s.length() - 17 ));

		try{
			MsgHandler.sendEmptyMessage( R.string.statmsg_bluetooth_connecting );
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
			MsgHandler.sendEmptyMessage( R.string.statmsg_bluetooth_connected );
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
		
		return 0;
	}

	//*** データ処理スレッド *********************************************

	public void Start(){
		// VSD スレッド起動
		GpsThread = new Thread( this );
		GpsThread.start();
	}

	volatile int uReadWdt;
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
					String str = brInput.readLine();
					if( bDebug ) Log.d( "VSDroid", str );
				}catch( IOException e ){ break; }
			}
			
			// ここに来たということは，何らかの理由で GPS 切断
			MsgHandler.sendEmptyMessage( R.string.statmsg_disconnected );
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
	}
}
