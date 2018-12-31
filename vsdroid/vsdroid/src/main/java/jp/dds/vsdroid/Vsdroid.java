package jp.dds.vsdroid;

import java.util.Calendar;
import android.annotation.SuppressLint;
import android.app.Activity;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.IntentFilter;
import android.content.SharedPreferences;
import android.graphics.Bitmap;
import android.graphics.Canvas;
import android.graphics.Color;
import android.graphics.Paint;
import android.graphics.PorterDuff.Mode;
import android.graphics.RectF;
import android.graphics.Typeface;
import android.hardware.camera2.CameraManager;
import android.os.BatteryManager;
import android.os.Build;
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
import java.net.SocketTimeoutException;
import java.util.UUID;
import java.io.*;
import java.util.*;
import jp.dds.vsdroid.VsdInterface;

@SuppressLint("DefaultLocale")
public class Vsdroid extends Activity {

	static final boolean	bDebug		= BuildConfig.DEBUG;

	// シフトインジケータの表示
	private static final double dTachoBarScale = 0.66;
	private static final int iTachoBarList[] = {
		( int )( 1336 * dTachoBarScale ),
		( int )( 800 * dTachoBarScale ),
		( int )( 600 * dTachoBarScale ),
		( int )( 472 * dTachoBarScale ),
		( int )( 388 * dTachoBarScale )
	};
	private static final int iRevLimit = 6500;

	private static final UUID BT_UUID = UUID.fromString( "00001101-0000-1000-8000-00805F9B34FB" );

	private static final int SHOW_CONFIG		= 0;
	private static final int REQUEST_ENABLE_BT	= 1;

	// VSD コミュニケーション
	VsdInterface Vsd = null;

	VsdSurfaceView	VsdScreen = null;

	// config
	SharedPreferences Pref;

	// カメラ
	private Camera Cam = null;
	private Camera.Parameters CamParam;
	private CameraManager CamMgr	= null;
	private String			CamID	= null;
	
	boolean	bRevWarn		= false;
	boolean	bEcoMode		= false;
	boolean	bDebugInfo		= false;
	
	int iBattery = 100;
	
	//*** utils **************************************************************

	void Sleep( int ms ){
		try{ Thread.sleep( ms ); }catch( InterruptedException e ){}
	}

	//*** カメラフラッシュ設定 ***********************************************

	public void SetFlash( boolean sw ){
		
		if( Build.VERSION.SDK_INT >= 23 ){
			if( CamID != null ){
				try{ CamMgr.setTorchMode( CamID, sw ); }catch( android.hardware.camera2.CameraAccessException e ){}
			}
		}else if( Cam != null ){
			// フラッシュモードを設定
			CamParam.setFlashMode(
				sw ?
					Camera.Parameters.FLASH_MODE_TORCH :
					Camera.Parameters.FLASH_MODE_OFF
			);
			
			// パラメータを設定
			Cam.setParameters( CamParam );
		}
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
		public void Open() throws IOException, UnrecoverableException {

			if( bDebug ) Log.d( "VSDroid", "VsdInterfaceBluetooth::Open" );
			// If the adapter is null, then Bluetooth is not supported
			if( mBluetoothAdapter == null ){
				MsgHandler.sendEmptyMessage( R.string.statmsg_bluetooth_na );
				throw new UnrecoverableException( "Bluetooth error" );
			}

			// BT ON でなければエラーで帰る
			if( bDebug ) Log.d( "VSDroid", "VsdInterfaceBluetooth::Enable" );
			if( !mBluetoothAdapter.isEnabled()){
				Intent enableIntent = new Intent( BluetoothAdapter.ACTION_REQUEST_ENABLE );
				startActivityForResult( enableIntent, REQUEST_ENABLE_BT );
				throw new UnrecoverableException( "Bluetooth error" );
			}

			// BT の MAC アドレスを求める
			String s = Pref.getString( "key_bt_devices_vsd", null );
			if( s == null ){
				MsgHandler.sendEmptyMessage( R.string.statmsg_bluetooth_dev_not_selected );
				throw new UnrecoverableException( "Bluetooth error" );
			}
			device = mBluetoothAdapter.getRemoteDevice( s.substring( s.length() - 17 ));

			try{
				MsgHandler.sendEmptyMessage( R.string.statmsg_bluetooth_connecting );
				// Get a BluetoothSocket for a connection with the
				// given BluetoothDevice
				if( bDebug ) Log.d( "VSDroid", "VsdInterfaceBluetooth::createRfcommSocket" );
				BTSock = device.createInsecureRfcommSocketToServiceRecord( BT_UUID );
				
				// ソケットの作成 12秒でタイムアウトらしい
				if( bDebug ) Log.d( "VSDroid", "VsdInterfaceBluetooth::Open:connecting..." );
				BTSock.connect();
				InStream	= BTSock.getInputStream();
				OutStream	= BTSock.getOutputStream();
				
				if( bDebug ) Log.d( "VSDroid", "VsdInterfaceBluetooth::Open:connected" );
				MsgHandler.sendEmptyMessage( R.string.statmsg_bluetooth_connected );
			}catch( SocketTimeoutException e ){
				if( bDebug ) Log.d( "VSDroid", "VsdInterfaceBluetooth::Open:timeout" );
				Close();
				throw new IOException( "Bluetooth error" );
			}catch( IOException e ){
				if( bDebug ) Log.d( "VSDroid", "VsdInterfaceBluetooth::Open:IOException" );
				Close();
				throw e;
			}
		}

		@Override
		public int Close(){
			if( bDebug ) Log.d( "VSDroid", "VsdInterfaceBluetooth::Close" );
			
			// BT 切断すると AT コマンドモードになるので，シリアル出力を止める
			try{
				SendCmd( "z" );
			}catch( IOException e ){}
			
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
		double dTimePrev;

		int	iIdxTime		= 0x7FFFFFFF;
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
		public void Open() throws UnrecoverableException {
			dTimePrev = -1;
			
			String strBuf;
			String strToken;

			if( bDebug ) Log.d( "VSDroid", "VsdInterfaceEmu::Open" );

			try{
				brEmuLog = new BufferedReader( new FileReader( Pref.getString( "key_replay_log", null )));

				// ヘッダ解析
				try{ strBuf = brEmuLog.readLine(); }catch( IOException e ){ strBuf = ""; }
				StringTokenizer Token = new StringTokenizer( strBuf );

				for( int i = 0; Token.hasMoreTokens(); ++i ){
					strToken = Token.nextToken();
					if(      strToken.equals( "Date/Time"		)) iIdxTime		= i;
					else if( strToken.equals( "Tacho"			)) iIdxTacho	= i;
					else if( strToken.equals( "Speed"			)) iIdxSpeed	= i;
					else if( strToken.equals( "Distance"		)) iIdxDistance	= i;
					else if( strToken.equals( "Gx"				)) iIdxGx		= i;
					else if( strToken.equals( "Gy"				)) iIdxGy		= i;
					else if( strToken.equals( "Throttle(raw)"	)) iIdxThrottle	= i;
					else if( strToken.equals( "LapTime"			)) iIdxLapTime	= i;
				}

			}catch( Exception e ){
				MsgHandler.sendEmptyMessage( R.string.statmsg_emulog_open_failed );
				throw new UnrecoverableException( "log can't open" );
			}
		}

		@Override
		public int RawRead( int iStart, int iLen ) throws IOException, UnrecoverableException {

			String strBuf;
			String[] strToken = new String[ 16 ];
			int iIdx = iStart;
			int iTokCnt;
			int i, j;
			double dTime;
			
			//if( bDebug ) Log.d( "VSDroid", "VsdInterfaceEmu::RawRead" );

			try{
				if(( strBuf = brEmuLog.readLine()) == null ){
					MsgHandler.sendEmptyMessage( R.string.statmsg_emulog_finished );
					throw new UnrecoverableException( "EOF" );
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
				iIdx = Pack( iIdx, iIdxDistance < iTokCnt ? ( int )( Double.parseDouble( strToken[ iIdxDistance ] ) / 1000 * PULSE_PER_1KM_CE28N ) : 0 );

				// TSC (Date/Time)
				// 0000-00-00T00:00:00.000Z
				// 012345678901234567890123
				try{
					dTime =
						Integer.parseInt( strToken[ iIdxTime ].substring( 11, 13 )) * 3600 +
						Integer.parseInt( strToken[ iIdxTime ].substring( 14, 16 )) * 60 +
						Double.parseDouble( strToken[ iIdxTime ].substring( 17, 23 ));
				}catch( NumberFormatException e ){
					dTime = dTimePrev + 1.0 / 16;
				}
				if( dTime < dTimePrev ) dTime += 24 * 3600;
				iIdx = Pack( iIdx, ( int )( dTime * ( VsdInterface.TIMER_HZ >> VsdInterface.TSC_SHIFT )));

				// G
				iIdx = Pack( iIdx, iIdxGy < iTokCnt ? ( int )( -Double.parseDouble( strToken[ iIdxGy ] ) * 4096 ) + 0x8000 : 0 );
				iIdx = Pack( iIdx, iIdxGx < iTokCnt ? ( int )(  Double.parseDouble( strToken[ iIdxGx ] ) * 4096 ) + 0x8000 : 0 );

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
						) * VsdInterface.TIMER_HZ
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
				if( dTimePrev >= 0 ) Sleep(( int )(( dTime - dTimePrev ) * 1000 ));
				dTimePrev = dTime;

			}catch( IOException e ){
				MsgHandler.sendEmptyMessage( R.string.statmsg_emulog_failed );
				throw new UnrecoverableException( "error while reading" );
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
		@Override public void LoadFirmware(){}
	}

	//*** 画面更新 or メッセージログ *****************************************
	
	private static final int MAX_MSG_LOG = 15;
	private int iMsgLog[] = new int[ MAX_MSG_LOG ];
	private int iMsgLogNum = 0;
	private int iMsgLogPtr = 0;
	private boolean bMsgLogShow = false;
	
	Handler VsdMsgHandler = new Handler(){
		public void handleMessage( Message Msg ){
			if( Msg.what == R.string.statmsg_update ){
				bMsgLogShow = false;
			}else{
				bMsgLogShow = true;
				iMsgLog[ iMsgLogPtr ] = Msg.what;
				if( iMsgLogNum < MAX_MSG_LOG ) ++iMsgLogNum;
				iMsgLogPtr = ( iMsgLogPtr + 1 ) % MAX_MSG_LOG;
			}
			
			if( VsdScreen != null ) VsdScreen.Draw();
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
			//paint.setAntiAlias( true );
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
			
			try{
				Draw();
			}catch( Exception e ){
				if( bDebug ) Log.d( "VSDroid", "surfaceCreated::LockCanvas failed" );
			}
		}

		@Override
		public void surfaceDestroyed( SurfaceHolder holder ){
			if( bDebug ) Log.d( "VSDroid", "surfaceDestroyed" );
			VsdScreen = null;
		}

		//*** 描画 ***********************************************************

		private static final int iMeterCx			= 400;
		private static final int iMeterCy			= 310;
		private static final int iMeterR 			= 265;	// メータ目盛り円弧半径
		private static final float fMeterScaleW		= ( int )( iMeterR * 0.256 );	// メータ目盛り円弧幅
		private static final float fMeterNeedleR	= ( float )( iMeterR * 1.1 );	// 針
		private static final float fMeterLineR1		= ( float )( iMeterR * ( 1 + 0.025 ) + fMeterScaleW * 0.5 );	// 目盛り線
		private static final float fMeterLineR2		= ( float )( iMeterR                 + fMeterScaleW * 0.5 );	// 目盛り線
		private static final float fMeterLineR3		= ( float )( iMeterR * ( 1 - 0.025 ) + fMeterScaleW * 0.5 );	// 目盛り線
		private static final float fMeterLineR4		= ( float )( iMeterR                 - fMeterScaleW * 0.1 );	// 目盛り線
		private static final float fMeterRedZoneW	= ( int )( iMeterR * 0.06 );	// レッドゾーン円弧幅
		private static final float fMeterRedZoneR	= ( float )( fMeterLineR2 - fMeterRedZoneW * 0.5 );	// レッドゾーン円弧幅
		private static final int iMeterFontSize		= 44;	// メータ目盛り数値サイズ
		private static final int iStartAngle		= 150;	// 開始角
		private static final int iSweepAngle		= 240;	// 範囲角
		private static final int iMeterMaxRev		= 7;	// タコメータ最大値
		private static final int iRedZoneSweepAngle = ( int )(( 1 - iRevLimit / ( iMeterMaxRev * 1000.0 )) * iSweepAngle );
		private static final int iRedZoneStartAngle = iStartAngle + iSweepAngle - iRedZoneSweepAngle;
		private static final int iGearLeft			= 669;
		private static final int iGearTop			= 13;
		private static final int iGearRight			= 788;
		private static final int iGearBottom		= 155;
		private static final int iGearFontSize		= 140;
		
		private static final int COLOR_ORANGE		= 0xFFFF6000;
		private static final int COLOR_PURPLE		= 0xFF4400A8;
		
		boolean bBlink	= false;
		int iTacho		= 0;
		int iTachoBar	= 0;
		
		private void Draw(){
			RectF rectf;
			String s;
			int	iGear = 1;
			
			Canvas canvas = getHolder().lockCanvas();
			canvas.translate( fOffsX / fScale, fOffsY / fScale );
			canvas.scale( fScale, fScale );
			
			if( Vsd != null ){
				iTacho = bEcoMode ? Vsd.iTacho * 2 : Vsd.iTacho;
				
				// ギアを求める
				double dGearRatio = ( double )Vsd.iSpeedRaw / Vsd.iTacho;
				if( Vsd.iTacho == 0 )							 iGear = 1;
				else if( dGearRatio < VsdInterface.GEAR_RATIO1 ) iGear = 1;
				else if( dGearRatio < VsdInterface.GEAR_RATIO2 ) iGear = 2;
				else if( dGearRatio < VsdInterface.GEAR_RATIO3 ) iGear = 3;
				else if( dGearRatio < VsdInterface.GEAR_RATIO4 ) iGear = 4;
				else											 iGear = 5;
				
				if(( Vsd.iSpeedRaw == 0xFFFF ) && ( Vsd.iTacho == 0 )){
					// キャリブレーション表示
					iTachoBar = iSweepAngle / 2;
				}else{
					// LED の表示 LV を求める
					iTachoBar = ( iTacho - iRevLimit + iTachoBarList[ iGear - 1 ]) * ( iSweepAngle / 2 ) / iTachoBarList[ iGear - 1 ];
					
					if( iTachoBar < 0 ){
						iTachoBar = 0;
					}else if( iTachoBar > iSweepAngle / 2 ){
						iTachoBar = bBlink ? iSweepAngle / 2 : 0;
						bBlink = !bBlink;
					}
				}
			}
			
			// フラッシュライト
			SetFlash( bRevWarn && iTachoBar > 0 );
			
			// メーターパネル
			paint.setTypeface( Typeface.DEFAULT_BOLD );
			canvas.drawColor( 0, Mode.CLEAR );
			
			// メーター目盛り円弧
			paint.setStyle( Paint.Style.STROKE );
			paint.setStrokeWidth( fMeterScaleW );
			paint.setColor( COLOR_PURPLE );
			
			rectf = new RectF(
				iMeterCx - iMeterR, iMeterCy - iMeterR,
				iMeterCx + iMeterR, iMeterCy + iMeterR
			);
			canvas.drawArc( rectf, iStartAngle, iSweepAngle, false, paint );
			
			// タコバー描画
			if( iTachoBar > 0 ){
				paint.setColor( Color.CYAN );
				rectf = new RectF(
					iMeterCx - iMeterR, iMeterCy - iMeterR,
					iMeterCx + iMeterR, iMeterCy + iMeterR
				);
				canvas.drawArc( rectf, iStartAngle, iTachoBar, false, paint );
				canvas.drawArc( rectf, iStartAngle + iSweepAngle - iTachoBar, iTachoBar, false, paint );
			}
			
			// レッドゾーン円弧
			paint.setStrokeWidth( fMeterRedZoneW );
			paint.setColor( COLOR_ORANGE );
			rectf = new RectF(
				iMeterCx - fMeterRedZoneR, iMeterCy - fMeterRedZoneR,
				iMeterCx + fMeterRedZoneR, iMeterCy + fMeterRedZoneR
			);
			canvas.drawArc( rectf, iRedZoneStartAngle, iRedZoneSweepAngle, false, paint );
			
			// 目盛線・数値
			paint.setStrokeWidth( 6 );
			paint.setColor( Color.WHITE );
			paint.setTextSize( iMeterFontSize );
			for( int i = 0; i <= iMeterMaxRev * 2; ++i ){
				paint.setStyle( Paint.Style.STROKE );
				double dAngle = Math.toRadians( iStartAngle + i * ( iSweepAngle / ( double )( iMeterMaxRev * 2 )));
				// 目盛り点
				canvas.drawLine(
					( float )( iMeterCx + ( Math.cos( dAngle ) * fMeterLineR1 )),
					( float )( iMeterCy + ( Math.sin( dAngle ) * fMeterLineR1 )),
					( float )( iMeterCx + ( Math.cos( dAngle ) * fMeterLineR2 )),
					( float )( iMeterCy + ( Math.sin( dAngle ) * fMeterLineR2 )),
					paint
				);
				
				if(( i & 1 ) != 0 ){
					// 目盛線
					canvas.drawLine(
						( float )( iMeterCx + ( Math.cos( dAngle ) * fMeterLineR3 )),
						( float )( iMeterCy + ( Math.sin( dAngle ) * fMeterLineR3 )),
						( float )( iMeterCx + ( Math.cos( dAngle ) * fMeterLineR4 )),
						( float )( iMeterCy + ( Math.sin( dAngle ) * fMeterLineR4 )),
						paint
					);
				}else{
					// 目盛り数値
					s = Integer.toString( i / 2 );
					paint.setStyle( Paint.Style.FILL );
					canvas.drawText( s,
						( float )( iMeterCx + ( Math.cos( dAngle ) * iMeterR ) - paint.measureText( s ) / 2 ),
						( float )( iMeterCy + ( Math.sin( dAngle ) * iMeterR ) + iMeterFontSize / 2 ),
						paint
					);
				}
			}
			
			canvas.drawLine( 214, 292, 586, 292, paint );
			
			// ギア箱
			paint.setColor( COLOR_ORANGE );
			paint.setStyle( Paint.Style.FILL );
			rectf = new RectF( iGearLeft, iGearTop, iGearRight, iGearBottom );
			canvas.drawRoundRect( rectf, 28, 28, paint );
			
			if( Vsd != null ){
				// Vsd.iTacho 針
				paint.setColor( Color.RED );
				paint.setStrokeWidth( 10 );
				
				double dTachoAngle = Math.toRadians( iStartAngle + ( iSweepAngle * iTacho / ( iMeterMaxRev * 1000.0 )));
				canvas.drawLine(
					iMeterCx, iMeterCy,
					iMeterCx + ( int )( Math.cos( dTachoAngle ) * fMeterNeedleR ),
					iMeterCy + ( int )( Math.sin( dTachoAngle ) * fMeterNeedleR ),
					paint
				);
				
				// スピード
				paint.setColor( Color.WHITE );
				s = Integer.toString( Vsd.iSpeedRaw / 100 );
				paint.setTextSize( 180 );
				canvas.drawText( s, ( BASE_WIDTH - paint.measureText( s )) / 2, 270, paint );
				
				// ギア
				paint.setColor( Color.BLACK );
				paint.setTextSize( iGearFontSize );
				s = Integer.toString( iGear );
				canvas.drawText( s,
					( iGearRight + iGearLeft - paint.measureText( s )) / 2,
					( iGearTop + iGearBottom + ( int )( iGearFontSize * 0.7 )) / 2,
					paint
				);
				
				paint.setTypeface( Typeface.DEFAULT );
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
				
				paint.setColor( Color.GRAY );
				paint.setTextSize( 32 );
				canvas.drawText( s, 220, 355, paint );
				paint.setTextSize( 64 );
				canvas.drawText( String.format( "%02d", Vsd.iLapNum ), 220, 410, paint );
				
				paint.setColor(
					Vsd.iTimeLastRaw == 0					? Color.GRAY :
					Vsd.iTimeLastRaw <= Vsd.iTimeBestRaw	? Color.CYAN :
															  Color.RED
				);
				canvas.drawText( VsdInterface.FormatTime( Vsd.iTimeLastRaw ), 340, 410, paint );
				paint.setColor( Color.GRAY );
				canvas.drawText( VsdInterface.FormatTime( Vsd.iTimeBestRaw ), 340, 470, paint );
				
				if( bDebugInfo ){
					// デバッグ用
					int	y = 0;
					paint.setColor( Color.CYAN );
					paint.setTextSize( 30 );
					canvas.drawText( String.format( "Throttle: %d (%d - %d)",
						Vsd.iThrottleRaw + Vsd.iThrottle0, Vsd.iThrottle0, Vsd.iThrottleMax + Vsd.iThrottle0 ),
						0, y += 30, paint );
					canvas.drawText( String.format( "Throttle(%%): %.1f", Vsd.iThrottle / 10.0 ), 0, y += 30, paint );
					canvas.drawText( String.format( "Gx: %+.2f", Vsd.iGx / 4096.0 ), 0, y += 30, paint );
					canvas.drawText( String.format( "Gy: %+.2f", Vsd.iGy / 4096.0 ), 0, y += 30, paint );
					canvas.drawText( String.format( "Batt: %d", iBattery ), 0, y += 30, paint );
				}
			}
			
			paint.setTypeface( Typeface.DEFAULT );
			
			// バッテリー
			if( iBattery <= 30 ){
				paint.setColor( Color.RED );
				canvas.drawLine( 0, 480 - 480 * iBattery / 30, 0, 480, paint );
			}
			
			// 時計
			Calendar cal = Calendar.getInstance();
			s = String.format( "%02d:%02d", cal.get( Calendar.HOUR_OF_DAY ), cal.get( Calendar.MINUTE ));
			paint.setColor( Color.GRAY );
			paint.setTextSize( 64 );
			canvas.drawText( s, 0, paint.getTextSize(), paint );
			
			// メッセージログ
			if( bMsgLogShow ){
				canvas.drawColor( 0x80000000 );	// 暗くする
				paint.setTextSize( 30 );
				
				for( int i = 0; i < iMsgLogNum; ++i ){
					String msg = getString( iMsgLog[( iMsgLogPtr + i - iMsgLogNum + MAX_MSG_LOG ) % MAX_MSG_LOG ]);
					paint.setColor(
						msg.startsWith( "[E" ) ? Color.RED :
						msg.startsWith( "[W" ) ? Color.YELLOW :
												 Color.WHITE
					);
					canvas.drawText( msg, 0, ( i + 1 ) * 30, paint );
				}
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
		startActivityForResult( intent, SHOW_CONFIG );
	}

	@Override
	protected void onActivityResult( int requestCode, int resultCode, Intent data ){
		if( bDebug ) Log.d( "VSDroid", "ConfigResult" );
		bConfigOpened = false;

		if(
			requestCode == REQUEST_ENABLE_BT ||
			resultCode == Preference.RESULT_RENEW ||
			Vsd == null
		){
			// BT イネーブルダイアログから帰ってきたか，
			// 接続モードが変更されたか，
			// Pref がからの状態での preferenceScreen から帰ってきたので，
			// VsdThread を停止後，VsdInterface を再構築する
			if( Vsd != null ) Vsd.KillThread();
			CreateVsdInterface();
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

	
	/*** バッテリー関係 *****************************************************/
	
	private BroadcastReceiver mBroadcastReceiver = new BroadcastReceiver(){
		@Override
		public void onReceive( Context context, Intent intent ){
			String action = intent.getAction();
			
			if( action.equals( Intent.ACTION_BATTERY_CHANGED )){
				int iStatus = intent.getIntExtra( "status", 0 );
				iBattery = (
					iStatus == BatteryManager.BATTERY_STATUS_DISCHARGING ||
					iStatus == BatteryManager.BATTERY_STATUS_NOT_CHARGING
				) ? intent.getIntExtra( "level", 0 ) * 100 / intent.getIntExtra( "scale", 0 ) : 100;
			}
		}
	};
	
	@Override
	protected void onPause(){
		super.onPause();
		unregisterReceiver( mBroadcastReceiver );
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

		// preference 参照
		Pref = PreferenceManager.getDefaultSharedPreferences( this );

		if( Pref.getString( "key_system_dir", null ) == null ){
			// 初回起動時は config を起動して pref を初期化
			Config();
		}else{
			CreateVsdInterface();
		}
	}

	@Override
	protected void onResume() {
		if( bDebug ) Log.d( "VSDroid", "onResume" );

		super.onResume();

		// カメラ (フラッシュ) オープン
		if( Build.VERSION.SDK_INT >= 23 ){
			if( CamMgr == null ){
				CamMgr = ( CameraManager )getSystemService( Context.CAMERA_SERVICE );
				CamMgr.registerTorchCallback( new CameraManager.TorchCallback(){
					//トーチモードが変更された時の処理
					@Override
					public void onTorchModeChanged( String cameraId, boolean enabled ){
						super.onTorchModeChanged( cameraId, enabled );
						//カメラIDをセット
						CamID = cameraId;
					}
				}, new Handler());
			}
		}else if( Cam == null && ( Cam = Camera.open()) != null ){
			CamParam = Cam.getParameters();	//カメラのパラメータを取得
			Cam.startPreview();				//プレビューをしないと光らない
		}
		
		// バッテリー
		IntentFilter filter = new IntentFilter();
		filter.addAction( Intent.ACTION_BATTERY_CHANGED );
		registerReceiver( mBroadcastReceiver, filter );
	}

	@Override
	protected void onStop() {
		if( bDebug ) Log.d( "VSDroid", "onStop" );
		super.onStop();

		SetFlash( false );
		if( Cam != null ) Cam.release();
		Cam = null;
	}

	void CreateVsdInterface(){
		// VsdInterface が動いている時に呼び出されたらおかしい
		if( bDebug ) assert !Vsd.VsdThread.isAlive();
		
		iMsgLogNum = 0;
		
		// VSD コネクション
		int iMode = Integer.parseInt( Pref.getString( "key_connection_mode", "0" ));
		switch( iMode ){
		  case VsdInterface.CONN_MODE_BLUETOOTH:
			Vsd = new VsdInterfaceBluetooth();
			break;
		  case VsdInterface.CONN_MODE_LOGREPLAY:
			Vsd = new VsdInterfaceEmulation();
			break;
		  default:
			Vsd = new VsdInterface();
		}
		Vsd.iConnMode	= iMode;
		Vsd.Pref		= Pref;
		Vsd.MsgHandler	= VsdMsgHandler;

		SetupMode();

		// VSD スレッド起動
		Vsd.Start();
	}

	void SetupMode(){
		bDebugInfo = Pref.getBoolean( "key_debug_info", false );
		bEcoMode = Pref.getBoolean( "key_eco_mode", false );	// エコモードw
		bRevWarn = Pref.getBoolean( "key_flash", false );		// レブリミット警告
		if( Vsd != null ) try{
			Vsd.SetupMode();
		}catch( IOException e ){}
	}

	@Override
	protected void onDestroy(){
		super.onDestroy();
		Vsd.KillThread();
		if( bDebug ) Log.d( "VSDroid", "Activity::onDestroy finished" );
	}
}
