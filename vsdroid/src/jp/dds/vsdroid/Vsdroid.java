package jp.dds.vsdroid;

import java.util.Calendar;
import android.annotation.SuppressLint;
import android.app.Activity;
import android.content.Context;
import android.content.SharedPreferences;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.graphics.Canvas;
import android.graphics.Color;
import android.graphics.Paint;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.preference.PreferenceManager;
import android.util.Log;
import android.view.SurfaceHolder;
import android.view.SurfaceView;
import android.view.KeyEvent;
import android.content.Intent;
import android.content.SharedPreferences.Editor;
import android.view.WindowManager;
import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothDevice;
import android.bluetooth.BluetoothSocket;
import android.hardware.Camera;
import android.hardware.Sensor;
import android.hardware.SensorEvent;
import android.hardware.SensorEventListener;
import android.hardware.SensorManager;
import java.net.SocketTimeoutException;
import java.util.UUID;
import java.io.*;
import java.util.*;
import jp.dds.vsdroid.VsdInterface;

@SuppressLint("DefaultLocale")
public class Vsdroid extends Activity implements SensorEventListener {

	static final boolean	bDebug		= true;

	// シフトインジケータの表示
	private static final int iTachoBar[] = { 334, 200, 150, 118, 97 };
	private static final int iRevLimit = 6500;

	private static final UUID BT_UUID = UUID.fromString( "00001101-0000-1000-8000-00805F9B34FB" );

	private static final int SHOW_CONFIG		= 0;
	private static final int REQUEST_ENABLE_BT	= 1;

	private static final double GRAVITY = 9.80665;

	// VSD コミュニケーション
	VsdInterface Vsd = null;

	VsdSurfaceView	VsdScreen = null;

	// config
	SharedPreferences Pref;

	// G センサー用
	private SensorManager manager;

	// カメラ
	Camera Cam = null;
	Camera.Parameters CamParam;
	boolean	bRevWarn		= false;
	boolean	bEcoMode		= false;

	enum FLASH_STATE {
		OFF,
		ON,
		BLINK,
	};

	//*** utils **************************************************************

	String FormatTime( int iTime ){
		return String.format( "%d'%02d.%03d",
			iTime / ( 256 * 60 ),
			( iTime >> 8 ) % 60,
			1000 * ( iTime & 0xFF ) / 256
		);
	}

	void Sleep( int ms ){
		try{ Thread.sleep( ms ); }catch( InterruptedException e ){}
	}

	//*** カメラフラッシュ設定 ***********************************************

	boolean bToggle = false;
	public void SetFlash( FLASH_STATE Switch ){

		// フラッシュモードを設定
		CamParam.setFlashMode(
			bRevWarn && Switch == FLASH_STATE.ON ?
				Camera.Parameters.FLASH_MODE_TORCH :
				Camera.Parameters.FLASH_MODE_OFF
		);
		bToggle = !bToggle;

		// パラメータを設定
		Cam.setParameters( CamParam );
	}

	//*** Bluetooth 接続 *****************************************************

	class VsdInterfaceBluetooth extends VsdInterface {

		BluetoothDevice device;
		BluetoothSocket BTSock = null;
		BluetoothAdapter mBluetoothAdapter = null;

		public VsdInterfaceBluetooth(){
			if( bDebug ) Log.d( "VSDroid", "VsdInterfaceBluetooth::new" );
			// Get local Bluetooth adapter
			mBluetoothAdapter = BluetoothAdapter.getDefaultAdapter();
		}

		@Override
		public int Open(){

			if( bDebug ) Log.d( "VSDroid", "VsdInterfaceBluetooth::Open" );
			// If the adapter is null, then Bluetooth is not supported
			if( mBluetoothAdapter == null ){
				iMessage = R.string.statmsg_bluetooth_not_available;
				return -1;
			}

			// BT ON でなければエラーで帰るが，
			// bKillThread = true にすることで，config を出さずにスレッドを抜ける
			if( bDebug ) Log.d( "VSDroid", "VsdInterfaceBluetooth::Enable" );
			if( !mBluetoothAdapter.isEnabled()){
				Intent enableIntent = new Intent( BluetoothAdapter.ACTION_REQUEST_ENABLE );
				startActivityForResult( enableIntent, REQUEST_ENABLE_BT );
				bKillThread = true;
				return -1;
			}

			// BT の MAC アドレスを求める
			String s = Pref.getString( "key_bt_devices", null );
			if( s == null ){
				iMessage = R.string.statmsg_bluetooth_device_not_selected;
				return -1;
			}
			device = mBluetoothAdapter.getRemoteDevice( s.substring( s.length() - 17 ));

			// Get a BluetoothSocket for a connection with the
			// given BluetoothDevice
			if( bDebug ) Log.d( "VSDroid", "VsdInterfaceBluetooth::createRfcommSocket" );
			try{
				BTSock = device.createInsecureRfcommSocketToServiceRecord( BT_UUID );
			}catch( IOException e ){
				if( BTSock != null ) try{
					BTSock.close();
					BTSock = null;
				}catch( IOException e1 ){}

				iMessage = R.string.statmsg_bluetooth_server_error;
				if( bDebug ) Log.d( "VSDroid", "VsdInterfaceBluetooth::createRfcommSocket:Failed" );
				return -1;
			}

			try{
				// ソケットの作成 12秒でタイムアウトらしい
				if( bDebug ) Log.d( "VSDroid", "VsdInterfaceBluetooth::Open:connecting..." );
				BTSock.connect();
				InStream	= BTSock.getInputStream();
				OutStream	= BTSock.getOutputStream();
				if( bDebug ) Log.d( "VSDroid", "VsdInterfaceBluetooth::Open:connected" );
				return 0;
			}catch( SocketTimeoutException e ){
				if( bDebug ) Log.d( "VSDroid", "VsdInterfaceBluetooth::Open:timeout" );
			}catch( IOException e ){
				if( bDebug ) Log.d( "VSDroid", "VsdInterfaceBluetooth::Open:IOException" );
			}

			if( BTSock != null ) try{
				BTSock.close();
				BTSock = null;
			}catch( IOException e ){}

			iMessage = R.string.statmsg_bluetooth_connection_failed;
			return -1;
		}

		@Override
		public int Close(){
			if( bDebug ) Log.d( "VSDroid", "VsdInterfaceBluetooth::Close" );

			// 念のためログ出力停止，* は g_lParam をいったんクリアする意図
			try{ SendCmd( "*0S" ); }catch( IOException e ){}

			try{
				if( BTSock != null ){
					BTSock.close();
					BTSock = null;
				}
			}catch( IOException e ){}
			return 0;
		}
	}

	//*** エミュレーションモード *********************************************

	class VsdInterfaceEmulation extends VsdInterface {
		BufferedReader brEmuLog = null;
		double	dOutputWaitTime = 0;

		int	iIdxTacho		= 0x7FFFFFFF;
		int	iIdxSpeed		= 0x7FFFFFFF;
		int	iIdxDistance	= 0x7FFFFFFF;
		int	iIdxGx			= 0x7FFFFFFF;
		int	iIdxGy			= 0x7FFFFFFF;
		int	iIdxThrottle	= 0x7FFFFFFF;
		int	iIdxLapTime		= 0x7FFFFFFF;

		// 16bit int を 2byte にパックする
		private int Pack( int iIdx, int iVal ){
			int lo = iVal & 0xFF;
			int hi = ( iVal >> 8 ) & 0xFF;

			if( lo >= 0xFE ){
				Buf[ iIdx++ ] = ( byte )0xFE;
				lo -= 0xFE;
			}
			Buf[ iIdx++ ] = ( byte )lo;

			if( hi >= 0xFE ){
				Buf[ iIdx++ ] = ( byte )0xFE;
				hi -= 0xFE;
			}
			Buf[ iIdx++ ] = ( byte )hi;

			return iIdx;
		}

		@Override
		public int Open(){
			String strBuf;
			String strToken;

			if( bDebug ) Log.d( "VSDroid", "VsdInterfaceEmu::Open" );

			try{
				brEmuLog = new BufferedReader( new FileReader( Pref.getString( "key_system_dir", null ) + "/vsd.log" ));

				// ヘッダ解析
				try{ strBuf = brEmuLog.readLine(); }catch( IOException e ){ strBuf = ""; }
				StringTokenizer Token = new StringTokenizer( strBuf );

				for( int i = 0; Token.hasMoreTokens(); ++i ){
					strToken = Token.nextToken();
					if(      strToken.equals( "Tacho"			)) iIdxTacho		= i;
					else if( strToken.equals( "Speed"			)) iIdxSpeed		= i;
					else if( strToken.equals( "Distance"		)) iIdxDistance	= i;
					else if( strToken.equals( "Gx"				)) iIdxGx		= i;
					else if( strToken.equals( "Gy"				)) iIdxGy		= i;
					else if( strToken.equals( "Throttle(raw)"	)) iIdxThrottle	= i;
					else if( strToken.equals( "LapTime"			)) iIdxLapTime	= i;
				}

			}catch( FileNotFoundException e ){
				iMessage = R.string.statmsg_emulog_open_failed;
				return -1;
			}
			return 0;
		}

		@Override
		public int RawRead( int iStart, int iLen ){

			String strBuf;
			String[] strToken = new String[ 16 ];
			int iIdx = iStart;
			int iTokCnt;
			int i, j;

			//if( bDebug ) Log.d( "VSDroid", "VsdInterfaceEmu::RawRead" );

			try{
				if(( strBuf = brEmuLog.readLine()) == null ){
					iMessage = R.string.statmsg_log_replay_finished;
					return -1;
				}

				StringTokenizer Token = new StringTokenizer( strBuf );

				for( iTokCnt = 0; iTokCnt < 16 && Token.hasMoreTokens(); ++iTokCnt ){
					strToken[ iTokCnt ] = Token.nextToken();
				}

				// Tacho
				iIdx = Pack( iIdx, iIdxTacho < iTokCnt ? Integer.parseInt( strToken[ iIdxTacho ] ) : 0 );

				// Speed
				iIdx = Pack( iIdx, iIdxSpeed < iTokCnt ? ( int )( Double.parseDouble( strToken[ iIdxSpeed ] ) * 100 ) : 0 );

				// Mileage
				iIdx = Pack( iIdx, iIdxDistance < iTokCnt ? ( int )( Double.parseDouble( strToken[ iIdxDistance ] ) / 1000 * PULSE_PER_1KM ) : 0 );

				// TSC
				iIdx = Pack( iIdx, 0 );

				// G
				iIdx = Pack( iIdx, iIdxGy < iTokCnt ? ( int )( -Double.parseDouble( strToken[ iIdxGy ] ) * ACC_1G_Y ) + 32000 : 0 );
				iIdx = Pack( iIdx, iIdxGx < iTokCnt ? ( int )(  Double.parseDouble( strToken[ iIdxGx ] ) * ACC_1G_Z ) + 32000 : 0 );

				// Throttle
				iIdx = Pack( iIdx, iIdxThrottle < iTokCnt ? Integer.parseInt( strToken[ iIdxThrottle ] ) : 0x8000 );

				// ラップタイム
				if( iIdxLapTime < iTokCnt && !strToken[ iIdxLapTime ].equals( "" )){
					// ラップタイム記録発見
					i = strToken[ iIdxLapTime ].indexOf( ':' );
					j = ( int )(
						(
							Double.parseDouble( strToken[ iIdxLapTime ].substring( 0, i )) * 60 +
							Double.parseDouble( strToken[ iIdxLapTime ].substring( i + 1 ))
						) * 256
					);

					// LAP モード以外の 0:00.000 をスキップ
					if( !( j == 0 && iRtcPrevRaw != 0 )){

						// 0:00.000 が来た時，Lap Ready を解除する
						if( j == 0 && iRtcPrevRaw == 0 ) j = 1;
						j += iRtcPrevRaw;
						iIdx = Pack( iIdx, j );
						iIdx = Pack( iIdx, j >> 16 );
					}
				}

				// 待つ
				Calendar Now = Calendar.getInstance();
				int NowMs =
					Now.get( Calendar.HOUR_OF_DAY ) * 3600 * 1000 +
					Now.get( Calendar.MINUTE ) * 60 * 1000 +
					Now.get( Calendar.SECOND ) * 1000 +
					Now.get( Calendar.MILLISECOND );

				if( dOutputWaitTime == 0 ) dOutputWaitTime = NowMs;
				dOutputWaitTime += 1000.0 / LOG_FREQ;
				i = ( int )( dOutputWaitTime - NowMs );
				if( i > 0 ) Sleep( i );

			}catch( IOException e ){
				iMessage = R.string.statmsg_log_replay_failed;
				return -1;
			}

			// EOL 追加
			Buf[ iIdx++ ] = ( byte )0xFF;

			return iIdx - iStart;
		}

		@Override
		public int Close(){
			if( bDebug ) Log.d( "VSDroid", "VsdInterfaceEmu::Close" );

			if( brEmuLog != null ){
				try{
					brEmuLog.close();
				}catch( IOException e ){}
				brEmuLog = null;
			}

			return 0;
		}

		@Override public void SendCmd( String s ){}
		@Override public int LoadFirmWare(){ return 0; }
	}

	//************************************************************************

	Handler VsdMsgHandler = new Handler(){
		public void handleMessage( Message Msg ){
			switch( Msg.what ){
			  case VsdInterface.MSG_LOG_UPDATE:
				if( VsdScreen != null ) VsdScreen.Draw();
				break;

			  case VsdInterface.MSG_OPEN_FAILED:
				Config();
			}
		}
	};

	//************************************************************************

	class VsdSurfaceView extends SurfaceView implements SurfaceHolder.Callback {

		//*** 生成 / 破棄 ****************************************************

		Bitmap[] bitmap;
		Paint paint;

		private static final int BASE_WIDTH		= 800;
		private static final int BASE_HEIGHT	= 480;
		float	fScale;
		float	fOffsX = 0, fOffsY = 0;

		public VsdSurfaceView( Context context ){
			super( context );
			if( bDebug ) Log.d( "VSDroid", "VsdSurfaceView::constructor" );

			getHolder().addCallback( this );
			setFocusable( true ); // キーイベントを使うために必須
			requestFocus(); // フォーカスを当てないとキーイベントを拾わない

			// Paint 取得・初期設定
			paint = new Paint();
			paint.setStrokeWidth( 10 );
			//paint.setAntiAlias( true );

			bitmap = new Bitmap[ 5 ];
			bitmap[ 0 ] = BitmapFactory.decodeResource( getContext().getResources(), R.drawable.meter0 );
			bitmap[ 1 ] = BitmapFactory.decodeResource( getContext().getResources(), R.drawable.meter1 );
			bitmap[ 2 ] = BitmapFactory.decodeResource( getContext().getResources(), R.drawable.meter2 );
			bitmap[ 3 ] = BitmapFactory.decodeResource( getContext().getResources(), R.drawable.meter3 );
			bitmap[ 4 ] = BitmapFactory.decodeResource( getContext().getResources(), R.drawable.meter4 );
		}

		@Override
		public void surfaceChanged( SurfaceHolder holder, int format, int width, int height ){
			if( bDebug ) Log.d( "VSDroid", "surfaceChanged" );
		}

		@Override
		public void surfaceCreated( final SurfaceHolder holder ){
			if( bDebug ) Log.d( "VSDroid", "surfaceCreated" );
			VsdScreen = this;

			float fScaleX = ( float )getWidth()  / BASE_WIDTH;
			float fScaleY = ( float )getHeight() / BASE_HEIGHT;
			if( fScaleX < fScaleY ){
				// X 側が幅いっぱい
				fScale = fScaleX;
				fOffsY = ( getHeight() - BASE_HEIGHT * fScale ) / 2 * fScale;
			}else{
				// Y 側が高さいっぱい
				fScale = fScaleY;
				fOffsX = ( getWidth()  - BASE_WIDTH  * fScale ) / 2 * fScale;
			}

			Canvas canvas = getHolder().lockCanvas();
			canvas.translate( fOffsX / fScale, fOffsY / fScale );
			canvas.scale( fScale, fScale );

			canvas.drawBitmap( bitmap[ 0 ], 0, 0, null );
			getHolder().unlockCanvasAndPost( canvas );
		}

		@Override
		public void surfaceDestroyed( SurfaceHolder holder ){
			if( bDebug ) Log.d( "VSDroid", "surfaceDestroyed" );
			VsdScreen = null;
		}

		//*** 描画 ***********************************************************

		private static final int iMeterCx = 399;
		private static final int iMeterCy = 310;
		private static final int iMeterR  = 292;

		boolean bBlink = false;

		private void Draw(){

			if( Vsd == null ) return;

			Canvas canvas = getHolder().lockCanvas();
			canvas.translate( fOffsX / fScale, fOffsY / fScale );
			canvas.scale( fScale, fScale );

			int iTacho = bEcoMode ? Vsd.iTacho * 2 : Vsd.iTacho;

			// ギアを求める
			double dGearRatio = ( double )Vsd.iSpeedRaw / Vsd.iTacho;
			int	iGear;
			if( Vsd.iTacho == 0 )							 iGear = 1;
			else if( dGearRatio < VsdInterface.GEAR_RATIO1 ) iGear = 1;
			else if( dGearRatio < VsdInterface.GEAR_RATIO2 ) iGear = 2;
			else if( dGearRatio < VsdInterface.GEAR_RATIO3 ) iGear = 3;
			else if( dGearRatio < VsdInterface.GEAR_RATIO4 ) iGear = 4;
			else											 iGear = 5;

			int iBarLv;

			if(( Vsd.iSpeedRaw >= 30000 ) && ( Vsd.iTacho == 0 )){
				// キャリブレーション表示
				iBarLv = 4;
			}else if( iTacho >= iRevLimit ){
				iBarLv = 5;
			}else{
				// LED の表示 LV を求める
				iBarLv = 3 - ( iRevLimit - iTacho ) / iTachoBar[ iGear - 1 ];
				if( iBarLv < 0 ) iBarLv = 0;
			}

			if( iBarLv >= 5 ){
				iBarLv = bBlink ? 4 : 0;
				bBlink = !bBlink;
			}else{
				bBlink = true;
			}

			// メーターパネル
			canvas.drawBitmap( bitmap[ iBarLv ], 0, 0, null );
			SetFlash( iBarLv >= 2 ? FLASH_STATE.ON : FLASH_STATE.OFF );

			// Vsd.iTacho 針
			paint.setColor( Color.RED );
			double dTachoAngle = ( 210 - ( 240 * iTacho / 8000.0 )) * Math.PI / 180;
			canvas.drawLine(
				iMeterCx, iMeterCy,
				iMeterCx + ( int )( Math.cos( dTachoAngle ) * iMeterR ),
				iMeterCy - ( int )( Math.sin( dTachoAngle ) * iMeterR ),
				paint
			);

			String s;

			// スピード
			paint.setColor( Color.WHITE );
			s = Integer.toString( Vsd.iSpeedRaw / 100 );
			paint.setTextSize( 180 );
			canvas.drawText( s, ( BASE_WIDTH - paint.measureText( s )) / 2, 270, paint );

			// ギア
			paint.setColor( Color.BLACK );
			paint.setTextSize( 140 );
			s = Integer.toString( iGear );
			canvas.drawText( s, 692, 132, paint );

			// 時計
			Calendar cal = Calendar.getInstance();
			s = String.format( "%02d:%02d", cal.get( Calendar.HOUR_OF_DAY ), cal.get( Calendar.MINUTE ));
			paint.setColor( Color.GRAY );
			paint.setTextSize( 64 );
			canvas.drawText( s, 0, paint.getTextSize(), paint );

			// タイム
			switch( Vsd.iMainMode ){
				case VsdInterface.MODE_LAPTIME:		s = "Lap";			break;
				case VsdInterface.MODE_GYMKHANA:	s = "Gymkhana"; 	break;
				case VsdInterface.MODE_ZERO_FOUR:	s = "0-400m";		break;
				case VsdInterface.MODE_ZERO_ONE:	s = "0-100km/h";
			}

			if( Vsd.iRtcPrevRaw == 0 ){
				// まだスタートしてない
				s += " ready";
			}

			paint.setTextSize( 32 );
			canvas.drawText( s, 220, 355, paint );
			paint.setTextSize( 64 );
			canvas.drawText( String.format( "%02d", Vsd.iLapNum ), 220, 410, paint );

			paint.setColor(
				Vsd.iTimeLastRaw == 0					? Color.GRAY :
				Vsd.iTimeLastRaw <= Vsd.iTimeBestRaw	? Color.CYAN :
														  Color.RED
			);
			canvas.drawText( FormatTime( Vsd.iTimeLastRaw ), 340, 410, paint );
			paint.setColor( Color.GRAY );
			canvas.drawText( FormatTime( Vsd.iTimeBestRaw ), 340, 470, paint );

			if( bDebug ){
				// デバッグ用
				int	y = 0;
				paint.setColor( Color.CYAN );
				paint.setTextSize( 30 );
				canvas.drawText( String.format( "Throttle(raw): %d", Vsd.iThrottleRaw ), 0, y += 30, paint );
				canvas.drawText( String.format( "Throttle(full): %d", Vsd.iThrottleFull ), 0, y += 30, paint );
				canvas.drawText( String.format( "Throttle(cm): %.2f", ( Vsd.iThrottleRaw / ( double )0x7FFFFFFF - 7.5395E-06 ) / 2.5928E-06 ), 0, y += 30, paint );
				canvas.drawText( String.format( "Throttle(%%): %.1f", Vsd.iThrottle / 10.0 ), 0, y += 30, paint );
				canvas.drawText( String.format( "Gx: %.2f", Vsd.iGx / 1000.0 ), 0, y += 30, paint );
				canvas.drawText( String.format( "Gy: %.2f", Vsd.iGy / 1000.0 ), 0, y += 30, paint );
				canvas.drawText( String.format( "Gx(dev): %.2f", Vsd.iPhoneGx / 1000.0 ), 0, y += 30, paint );
				canvas.drawText( String.format( "Gy(dev): %.2f", Vsd.iPhoneGy / 1000.0 ), 0, y += 30, paint );
			}

			getHolder().unlockCanvasAndPost( canvas );
		}
	}

	//*** config メニュー ****************************************************

	boolean bConfigOpened = false;

	// config を開く
	public void Config(){
		if( bDebug ) Log.d( "VSDroid", "Config" );

		if( bConfigOpened ) return;
		bConfigOpened = true;

		Editor ed = Pref.edit();
		ed.putBoolean( "key_reopen_log",  false );
		ed.putBoolean( "key_caribration", false );
		ed.commit();

		Intent intent = new Intent( Vsdroid.this, Preference.class );
		intent.putExtra( "Message", getString( Vsd == null ? R.string.statmsg_normal : Vsd.iMessage ));

		startActivityForResult( intent, SHOW_CONFIG );
	}

	@Override
	protected void onActivityResult( int requestCode, int resultCode, Intent data ){
		if( bDebug ) Log.d( "VSDroid", "ConfigResult" );
		bConfigOpened = false;

		if(
			requestCode == REQUEST_ENABLE_BT ||
			resultCode == Preference.RESULT_RENEW
		){
			// BT イネーブルダイアログから帰ってきたか，
			// 接続モードが変更されたので，
			// VsdThread を停止後，VsdInterface を再構築する
			Vsd.KillThread();
			CreateVsdInterface( Integer.parseInt( Pref.getString( "key_connection_mode", "0" )));
		}else if( Vsd.VsdThread.isAlive()){
			// 設定メニューから帰ってきた (?)
			SetupMode();
			if( Pref.getBoolean( "key_reopen_log", false )){
				Vsd.CloseLog();
				Vsd.OpenLog();
			}
		}else{
			// スレッドが死んでいるなら，再スタートすることで
			// 上の処理は実行される．
			Vsd.Start();
		}

		if( Pref.getBoolean( "key_caribration", false )){
			try{
				Vsd.SendCmd( "c" );
			}catch( IOException e ){}
		}
	}

	//************************************************************************

	@Override
	public boolean onKeyDown( int keyCode, KeyEvent event ){
		if( bDebug ) Log.d( "VSDroid", "onKeyDown" );
		switch( keyCode ){
		  case KeyEvent.KEYCODE_DPAD_CENTER:
			SetupMode();
			break;

		  case KeyEvent.KEYCODE_MENU:
			// Vsd.iMessage = R.string.statmsg_normal;
			Config();
			break;

		  case KeyEvent.KEYCODE_BACK:
			finish();
			break;

		  default:
			return false;
		}
		return true;
	}

	@Override
	protected void onCreate( final Bundle savedInstanceState ){
		super.onCreate( savedInstanceState );

		if( bDebug ) Log.d( "VSDroid", "Activity::onCreate" );
		getWindow().addFlags( WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON );

		setContentView( new VsdSurfaceView( this ));
		manager = ( SensorManager )getSystemService( SENSOR_SERVICE );

		// preference 参照
		Pref = PreferenceManager.getDefaultSharedPreferences( this );

		if( Pref.getString( "key_system_dir", null ) == null ){
			// 初回起動時は config を起動して pref を初期化
			Config();
		}else{
			CreateVsdInterface( Integer.parseInt( Pref.getString( "key_connection_mode", "0" )));
		}
	}

	@Override
	protected void onResume() {
		if( bDebug ) Log.d( "VSDroid", "onResume" );

		super.onResume();
		// Listenerの登録
		List<Sensor> sensors = manager.getSensorList( Sensor.TYPE_ACCELEROMETER );
		if( sensors.size() > 0 ){
			Sensor s = sensors.get( 0 );
			manager.registerListener( this, s, SensorManager.SENSOR_DELAY_UI );
		}

		// カメラ (フラッシュ) オープン
		if( Cam == null ){
			Cam = Camera.open();			//カメラを取得
			CamParam = Cam.getParameters();	//カメラのパラメータを取得
			Cam.startPreview();				//プレビューをしないと光らない
		}
	}

	@Override
	protected void onStop() {
		super.onStop();
		// Listenerの登録解除
		manager.unregisterListener( this );

		SetFlash( FLASH_STATE.OFF );
		Cam.release();
		Cam = null;
	}

	@Override
	public void onSensorChanged( SensorEvent event ){
		if( Vsd != null && event.sensor.getType() == Sensor.TYPE_ACCELEROMETER ){
			Vsd.iPhoneGxRaw = ( int )( -event.values[ SensorManager.DATA_Y ] * ( 1000 / GRAVITY ));
			Vsd.iPhoneGyRaw = ( int )( -event.values[ SensorManager.DATA_Z ] * ( 1000 / GRAVITY ));
		}
	}

	void CreateVsdInterface( int iNewMode ){
		// VSD コネクション
		switch( iNewMode ){
		  case VsdInterface.CONN_MODE_BLUETOOTH:
			Vsd = new VsdInterfaceBluetooth();
			break;
		  case VsdInterface.CONN_MODE_LOGREPLAY:
			Vsd = new VsdInterfaceEmulation();
			break;
		  default:
			Vsd = new VsdInterface();
		}
		Vsd.iConnMode	= iNewMode;
		Vsd.Pref		= Pref;
		Vsd.MsgHandler	= VsdMsgHandler;

		Vsd.SetupMode();

		// VSD スレッド起動
		Vsd.Start();
	}

	void SetupMode(){
		bEcoMode = Pref.getBoolean( "key_eco_mode", false );	// エコモードw
		bRevWarn = Pref.getBoolean( "key_flash", false );		// レブリミット警告
		if( Vsd != null ) Vsd.SetupMode();
	}

	@Override
	protected void onDestroy(){
		super.onDestroy();
		Vsd.KillThread();
		if( bDebug ) Log.d( "VSDroid", "Activity::onDestroy finished" );
	}

	@Override
	public void onAccuracyChanged(Sensor arg0, int arg1) {}
}
