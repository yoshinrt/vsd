package jp.dds.vsdroid;

import java.util.Calendar;

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

import java.net.InetSocketAddress;
import java.net.Socket;
import java.net.SocketAddress;
import java.net.SocketTimeoutException;
import java.util.UUID;

import java.io.*;
import java.util.*;

public class Vsdroid extends Activity {

	private static final boolean	bDebug		= false;

	// 定数
	private static final int MODE_LAPTIME	= 0;
	private static final int MODE_GYMKHANA	= 1;
	private static final int MODE_ZERO_FOUR	= 2;
	private static final int MODE_ZERO_ONE	= 3;
	private static final int MODE_NUM		= 4;

	private static final int CONN_MODE_ETHER		= 0;
	private static final int CONN_MODE_BLUETOOTH	= 1;
	private static final int CONN_MODE_LOGREPLAY	= 2;

	//private static final double H8HZ			= 16030000;
	//private static final double SERIAL_DIVCNT	= 16;		// シリアル出力を行う周期
	//private static final double LOG_FREQ		= 16;

	// スピード * 100/Taco 比
	// ELISE
	// ギア比 * マージンじゃなくて，ave( ギアn, ギアn+1 ) に変更
	private static final double GEAR_RATIO1 = 1.2381712993947;
	private static final double GEAR_RATIO2 = 1.82350889069989;
	private static final double GEAR_RATIO3 = 2.37581451065366;
	private static final double GEAR_RATIO4 = 2.95059529470571;

	// たぶん，ホイル一周が30パルス
	private static final double PULSE_PER_1KM	= 15473.76689;	// ELISE(CE28N)

	//private static final double ACC_1G_X	= 6762.594337;
	private static final double ACC_1G_Y	= 6667.738702;
	private static final double ACC_1G_Z	= 6842.591839;

	// シフトインジケータの表示
	private static final int iTachoBar[] = { 334, 200, 150, 118, 97 };
	private static final int iRevLimit = 6500;

	// モード・config
	int		iMainMode	= MODE_LAPTIME;
	int		iConnMode	= CONN_MODE_ETHER;

	static final String VSD_ROOT = "/sdcard/vsd";
	private static final String VSD_LOG  = VSD_ROOT + "/log";

	private static final UUID BT_UUID = UUID.fromString( "00001101-0000-1000-8000-00805F9B34FB" );

	private static final int SHOW_CONFIG		= 0;
	private static final int REQUEST_ENABLE_BT	= 1;

	enum LAP_STATE {
		NONE,
		START,
		UPDATE,
		SECTOR,
	};

	// VSD コミュニケーション
	VsdInterface Vsd = null;
	Thread VsdThread = null;

	VsdSurfaceView	VsdScreen = null;

	// ステータスメッセージ
	int iMessage = R.string.statmsg_normal;

	// config
	SharedPreferences Pref;

	//*** utils **************************************************************

	String FormatTime( int iTime ){
		return String.format( "%d'%02d.%03d",
			iTime / ( 256 * 60 ),
			( iTime >> 8 ) % 60,
			1000 * ( iTime & 0xFF ) / 256
		);
	}

	String FormatTime2( int iTime ){
		return String.format( "%d:%02d.%03d",
			iTime / ( 256 * 60 ),
			( iTime >> 8 ) % 60,
			1000 * ( iTime & 0xFF ) / 256
		);
	}

	void Sleep( int ms ){
		try{ Thread.sleep( ms ); }catch( InterruptedException e ){}
	}

	//*** VSD アクセス *******************************************************

	class VsdInterface implements Runnable {

		// VSD リンクのリードデータ
		int		iTacho			= 0;
		int		iSpeedRaw		= 0;
		int		iRtcRaw			= 0;
		int		iRtcPrevRaw		= 0;
		int		iMileageRaw		= 0;
		int		iTSCRaw			= 0;
		int 	iSectorCnt		= 0;
		int 	iSectorCntMax	= 1;
		double	dGx, dGy;
		double	dGcx, dGcy;
		double	dGymkhaStart	= 1;
		boolean	bEcoMode		= false;

		LAP_STATE	LapState	= LAP_STATE.NONE;
		int		iGCaribCnt		= iGCaribCntMax;

		private static final int	iStartGThrethold	= 500;
		private static final int	iGCaribCntMax		= 30;

		// レコード
		int 	iLapNum			= 0;
		int 	iTimeLastRaw	= 0;
		int 	iTimeBestRaw	= 0;

		private static final int iBufSize = 256;

		byte	Buf[] = new byte[ iBufSize ];
		int		iBufLen = 0;
		int		iBufPtr = 0;

		BufferedWriter			fsLog		= null;
		BufferedOutputStream	fsBinLog	= null;

		boolean			bLogStart	= false;
		Socket			Sock		= null;

		InputStream		InStream	= null;
		OutputStream	OutStream	= null;

		volatile boolean bKillThread = false;

		Handler	DrawHandler;

		// コンストラクタ - 変数の初期化だけやってる
		public VsdInterface(){
			if( bDebug ) Log.d( "VSDroid", "VsdInterface::constructor" );
			iTacho			= 0;
			iSpeedRaw		= 0;
			iRtcRaw			= 0;
			iRtcPrevRaw		= 0;
			iMileageRaw		= 0;
			iTSCRaw			= 0;
			iSectorCnt		= 0;
			iSectorCntMax	= 1;
			dGymkhaStart	= 1;

			LapState		= LAP_STATE.NONE;

			// レコード
			iLapNum			= 0;
			iTimeLastRaw	= 0;
			iTimeBestRaw	= 0;

			iBufLen 		= 0;
			iBufPtr 		= 0;

			bKillThread		= false;

			fsLog			= null;
			fsBinLog		= null;
			bLogStart		= false;
			Sock			= null;

			dGcx = dGcy		= 0;
			iGCaribCnt		= iGCaribCntMax;

			InStream		= null;
			OutStream		= null;

			DrawHandler = new Handler(){
				public void handleMessage( Message Msg ){
					if( VsdScreen != null ) VsdScreen.Draw();
				}
			};
		}

		// 2byte を 16bit int にアンパックする
		private int Unpack(){
			int iRet;

			if( Buf[ iBufPtr ] == ( byte )0xFE ){
				++iBufPtr;
				iRet = ( Buf[ iBufPtr++ ] + 0xFE ) & 0xFF;
			}else{
				iRet = Buf[ iBufPtr++ ] & 0xFF;
			}
			if( Buf[ iBufPtr ] == ( byte )0xFE ){
				++iBufPtr;
				iRet += ( Buf[ iBufPtr++ ] + 0xFE ) << 8;
			}else{
				iRet += Buf[ iBufPtr++ ] << 8;
			}
			return iRet & 0xFFFF;
		}

		public int OpenLog(){

			// 非デバッグモードかつ Log リプレイモードなら，ログは作らない
			if( !bDebug && iConnMode == CONN_MODE_LOGREPLAY ) return 0;

			// 日付
			Calendar Now = Calendar.getInstance();
			String s = String.format(
				"/vsd%04d%02d%02d_%02d%02d%02d",
				Now.get( Calendar.YEAR ),
				Now.get( Calendar.MONTH ) + 1,
				Now.get( Calendar.DAY_OF_MONTH ),
				Now.get( Calendar.HOUR_OF_DAY ),
				Now.get( Calendar.MINUTE ),
				Now.get( Calendar.SECOND )
			);

			// ログファイルオープン
			try{
				fsLog    = new BufferedWriter( new FileWriter( VSD_LOG + s + ".log" ));
				//fsBinLog = new BufferedOutputStream( new FileOutputStream( VSD_LOG + s + "bin.log" ));
			}catch( Exception e ){
				iMessage = R.string.statmsg_log_open_failed;
				return -1;
			}

			return 0;
		}

		public int Open(){
			if( bDebug ) Log.d( "VSDroid", "VsdInterface::Open" );

			while( !bKillThread ){
				try{
					// ソケットの作成
					Sock = new Socket();
					SocketAddress adr = new InetSocketAddress( Pref.getString( "key_ip_addr", "192.168.0.1" ), 12345 );
					Sock.connect( adr, 1000 );
					if( bDebug ) Log.d( "VSDroid", "VsdInterface::Open:connected" );
					InStream	= Sock.getInputStream();
					OutStream	= Sock.getOutputStream();
					return 0;
				}catch( SocketTimeoutException e ){
					if( bDebug ) Log.d( "VSDroid", "VsdInterface::Open:timeout" );
				}catch( IOException e ){
					if( bDebug ) Log.d( "VSDroid", "VsdInterface::Open:IOException" );
				}

				if( Sock != null ) try{
					Sock.close();
					Sock = null;
				}catch( IOException e ){}

				Sleep( 1000 );
			}
			iMessage = R.string.statmsg_socket_open_failed;
			return -1;
		}

		public int RawRead( int iStart, int iLen ){
			//if( bDebug ) Log.d( "VSDroid", "VsdInterface::RawRead" );
			int iReadSize = 0;
			try{
				iReadSize = InStream.read( Buf, iStart, iLen );
			}catch( IOException e ){
				iMessage = R.string.statmsg_socket_rw_failed;
				return -1;
			}
			return iReadSize;
		}

		// 1 <= :受信したレコード数  0:新データなし -1:エラー
		public int Read(){
			int	iReadSize;
			int	iMileage16;
			int	iRet	= 0;
			int	iEOLPos	= 0;
			int i;

			//if( bDebug ) Log.d( "VSDroid", "VsdInterface::Read" );

			iReadSize = RawRead( iBufLen, iBufSize - iBufLen );
			if( iReadSize < 0 ) return -1;

			try{
				if( fsBinLog != null ) fsBinLog.write( Buf, iBufLen, iReadSize );
			}catch( IOException e ){}

			iBufLen += iReadSize;

			while( true ){
				// 0xFF をサーチ
				for( iBufPtr = iEOLPos; iEOLPos < iBufLen; ++iEOLPos ){
					if( Buf[ iEOLPos ] == ( byte )0xFF ) break;
				}
				if( iEOLPos == iBufLen ) break;

				++iRet;
				// 0xFF が見つかったので，データ変換
				iTacho		= Unpack();
				iSpeedRaw	= Unpack();

				// ここで描画要求
				DrawHandler.sendEmptyMessage( 0 );

				if( iBufPtr < iEOLPos ){
					iMileage16	= Unpack();
					iTSCRaw		= Unpack();

					dGx			= Unpack();
					dGy			= Unpack();
					if( iGCaribCnt-- > 0 ){
						// G センサーキャリブレーション中
						dGcx += dGx;
						dGcy += dGy;

						if( iGCaribCnt == 0 ){
							dGcx /= iGCaribCntMax;
							dGcy /= iGCaribCntMax;
						}

						dGx	= 0;
						dGy	= 0;
					}else{
						dGx	= -( dGx - dGcx ) / ACC_1G_Y;
						dGy	=  ( dGy - dGcy ) / ACC_1G_Z;
					}

					// mileage 補正
					if(( iMileageRaw & 0xFFFF ) > iMileage16 ) iMileageRaw += 0x10000;
					iMileageRaw &= 0xFFFF0000;
					iMileageRaw |= iMileage16;

					// LapTime が見つかった
					LapState = LAP_STATE.NONE;

					if( iBufPtr < iEOLPos ){
						iRtcRaw = Unpack() + ( Unpack() << 16 );

						if( ++iSectorCnt >= iSectorCntMax ){
							// 規定のセクタを通過したので，NewLap
							if( iRtcPrevRaw != 0 ){
								// 1周回った
								LapState = LAP_STATE.UPDATE;

								++iLapNum;
								if( iTimeLastRaw < iTimeBestRaw ){
									iTimeBestRaw = iTimeLastRaw;
								}
								iTimeLastRaw = iRtcRaw - iRtcPrevRaw;
								if( iTimeBestRaw == 0 ) iTimeBestRaw = iTimeLastRaw;

								if( iMainMode != MODE_LAPTIME ){
									// Laptime モード以外でゴールしたら，Ready 状態に戻す
									Vsd.iRtcPrevRaw = 0;
								}
							}else{
								// ラップスタート
								LapState = LAP_STATE.START;
							}
							iRtcPrevRaw	= iRtcRaw;
							iSectorCnt	= 0;
						}else{
							// 中間セクタ通過
							LapState = LAP_STATE.SECTOR;
						}
					}

					WriteLog();		// テキストログライト
				}

				// デバッグ用: 複数の 0xFF を破棄する
				/*
				if( bDebug ){
					for( i = iBufLen - 1; i >= 0; --i ) if( Buf[ i ] == ( byte )0xFF ) break;
					if( i >= 0 ) iEOLPos = i;
				}
				*/
				++iEOLPos;
			}

			// 使用したデータを Buf から削除
			if( iBufPtr != 0 ){
				for( i = 0; iBufPtr < iBufLen; ){
					Buf[ i++ ] = Buf[ iBufPtr++ ];
				}
				iBufLen = i;
			}

			return iRet;
		}

		public void WriteLog(){
			String s;

			if( iSpeedRaw > 0 ) bLogStart = true;
			if( !bLogStart ) return;

			// 基本データ
			s = String.format(
				"%d\t%.2f\t%.2f\t%.4f\t%.4f\t%d",
				iTacho, ( double )iSpeedRaw / 100,
				( double )iMileageRaw / PULSE_PER_1KM * 1000,
				dGy, dGx, iTSCRaw
			);

			// ラップタイム
			switch( LapState ){
			  case UPDATE:
				s += String.format( "\tLAP%d\t", iLapNum ) + FormatTime2( iTimeLastRaw );
				break;

			  case START:
				s += String.format( "\tLAP%d start", iLapNum + 1 );
				break;

			  case SECTOR:
				s += String.format( "\tSector%d\t", iSectorCnt ) + FormatTime2( iRtcPrevRaw - iRtcRaw );
			}

			try{
				if( fsLog != null ) fsLog.write( s + "\r\n" );
			}catch( IOException e ){}
		}

		public int CloseLog(){
			try{
				if( fsLog	 != null ){
					fsLog.close();
					fsLog = null;
				}
				if( fsBinLog != null ){
					fsBinLog.close();
					fsBinLog = null;
				}
			}catch( IOException e ){}
			return 0;
		}

		public int Close(){
			if( bDebug ) Log.d( "VSDroid", "VsdInterface::Close" );
			try{
				if( Sock != null ){
					Sock.close();
					Sock = null;
				}
			}catch( IOException e ){}
			return 0;
		}

		//*** sio コマンド関係 ***********************************************

		public void SendCmd( String s ) throws IOException {
			byte[] Buf;

			if( OutStream == null ) return;

			try{
				Buf = s.getBytes( "US-ASCII" );
				OutStream.write( Buf, 0, Buf.length );
			}catch( UnsupportedEncodingException e ){}
		}

		public int WaitChar( char ch ) throws IOException {
			int iReadSize;
			int iRetryCnt = 100;
			int i;

			while( iRetryCnt-- != 0 && !bKillThread ){
				if( InStream.available() > 0 ){
					iReadSize = InStream.read( Buf, 0, iBufSize );
					for( i = 0; i < iReadSize; ++i ){
						if( Buf[ i ] == ( byte )ch ){
							iRetryCnt = 0;
							return i;
						}
					}
				}else{
					Sleep( 30 );
				}
			}
			return -1;
		}

		//*** FW ロード ******************************************************

		public int LoadFirmWare(){
			InputStream	fsFirm = null;

			int iReadSize = 0;
			int iPtr, i = 0;

			try{
				if( bDebug ) Log.d( "VSDroid", "LoadFirm::sending magic code" );
				SendCmd( "F15EF117*\rz\r" );

				if( bDebug ) Log.d( "VSDroid", "LoadFirm::sending firmware" );
				try{
					fsFirm = new FileInputStream( VSD_ROOT + "/" + Pref.getString( "key_roms", "vsd.mot" ));

					WaitChar( ':' ); SendCmd( "l\r" );	// FW があったときだけ l コマンド

					// FW の \n を削除しつつ送信
					while(( iReadSize = fsFirm.read( Buf, 0, iBufSize )) > 0 ){
						for( iPtr = i = 0; i < iReadSize; ++i ){
							if( Buf[ i ] != ( byte )0x0A ) Buf[ iPtr++ ] = Buf[ i ];
						}
						OutStream.write( Buf, 0, iPtr );
					}
				}catch( FileNotFoundException e ){
					if( bDebug ) Log.d( "VSDroid", "LoadFirm::sending firmware canceled" );
				}

				if( fsFirm != null ) fsFirm.close();

				if( bDebug ) Log.d( "VSDroid", "LoadFirm::go" );
				WaitChar( ':' ); SendCmd( "g\r" );

				if( bDebug ) Log.d( "VSDroid", "LoadFirm::sending log output request" );
				Sleep( 100 );
				// 1S: Serial output ON  s: speed  1a: auto mode
				SendCmd( "F15EF117*1Ss1a" );

				// 最初の 0xFF までスキップ
				if( bDebug ) Log.d( "VSDroid", "LoadFirm::waiting first log record" );

				int iRetryCnt = 100;
				while( iRetryCnt-- != 0 && !bKillThread ){
					if( InStream.available() > 0 ){
						iReadSize = InStream.read( Buf, 0, iBufSize );
						for( i = 0; i < iReadSize; ++i ){
							if( Buf[ i ] == ( byte )0xFF ){
								iRetryCnt = 0;
								break;
							}
						}
					}else{
						Sleep( 30 );
					}
				}

				// 0xFF を見つけた
				if( i < iReadSize ){
					++i;
					for( iBufLen = 0; i < iReadSize; ++i, ++iBufLen ){
						Buf[ iBufLen ] = Buf[ i ];
					}
				}else{
					// タイムアウト
					iMessage = R.string.statmsg_vsd_initialize_failed;
					return -1;
				}
			}catch( IOException e ){
				iMessage = R.string.statmsg_socket_rw_failed;
				return -1;
			}

			if( bDebug ) Log.d( "VSDroid", "LoadFirm::completed." );
			return 0;
		}

		//*** config に従って設定 ********************************************

		public void SetupMode(){
			int i;

			// セクタ数
			try{
				iSectorCntMax = Integer.parseInt( Pref.getString( "key_sectors", "1" ));
			}catch( NumberFormatException e ){
				iSectorCntMax = 1;
			}

			// ジムカスタート距離
			try{
				dGymkhaStart = Double.parseDouble( Pref.getString( "key_gymkha_start", "1" ));
			}catch( NumberFormatException e ){
				dGymkhaStart = 1.0;
			}

			// エコモードw
			bEcoMode = Pref.getBoolean( "key_eco_mode", false );

			// メインモード
			try{
				iMainMode = Integer.parseInt( Pref.getString( "key_vsd_mode", "0" ));
				if( iMainMode < 0 || MODE_NUM <= iMainMode ) iMainMode = MODE_LAPTIME;
			}catch( NumberFormatException e ){}

			try{
				switch( iMainMode ){
				  case MODE_LAPTIME:
					SendCmd( "l" );
					break;

				  case MODE_GYMKHANA:
					// * は g_lParam をいったんクリアする意図
					i = ( int )( dGymkhaStart * PULSE_PER_1KM / 1000 + 0.5 );
					if( i < 1 ) i = 1;
					SendCmd( String.format( "*%Xg", i ));
					break;

				  case MODE_ZERO_FOUR:
					SendCmd( String.format( "*%Xf", iStartGThrethold ));
					break;

				  case MODE_ZERO_ONE:
					SendCmd( String.format( "*%Xo", iStartGThrethold ));
				}
			}catch( IOException e ){}

			iRtcPrevRaw		= 0;
			iSectorCnt		= 0;
		}

		//*** データ処理スレッド *********************************************

		public void run(){
			bKillThread = false;

			if( bDebug ) Log.d( "VSDroid", "run_loop()" );

			if(
				Open()			>= 0 &&
				LoadFirmWare()	>= 0 &&
				OpenLog()		>= 0
			){
				SetupMode();
				while( !bKillThread && Read() >= 0 );
			}

			Close();
			CloseLog();
			if( !bKillThread ) Config();

			bKillThread	= false;
			if( bDebug ) Log.d( "VSDroid", "exit_run()" );
		}

		public void KillThread(){
			bKillThread = true;
			try{
				while( bKillThread && VsdThread != null && VsdThread.isAlive()){
					Close();
					Thread.sleep( 100 );
				}
			}catch( InterruptedException e ){}

			VsdThread = null;
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
				BTSock = device.createRfcommSocketToServiceRecord( BT_UUID );
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
			if( bDebug ) Log.d( "VSDroid", "VsdInterfaceEmu::Open" );

			try{
				brEmuLog = new BufferedReader( new FileReader( VSD_ROOT + "/vsd.log" ));
			}catch( FileNotFoundException e ){
				iMessage = R.string.statmsg_emulog_open_failed;
				return -1;
			}
			return 0;
		}

		@Override
		public int RawRead( int iStart, int iLen ){

			String strBuf;
			String[] strToken = new String[ 8 ];
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

				for( iTokCnt = 0; iTokCnt < 8 && Token.hasMoreTokens(); ++iTokCnt ){
					strToken[ iTokCnt ] = Token.nextToken();
				}

				// Tacho
				iIdx = Pack( iIdx, Integer.parseInt( strToken[ 0 ] ));

				// Speed
				iIdx = Pack( iIdx, ( int )( Double.parseDouble( strToken[ 1 ] ) * 100 ));

				// Mileage
				iIdx = Pack( iIdx, ( int )( Double.parseDouble( strToken[ 2 ] ) / 1000 * PULSE_PER_1KM ));

				// TSC
				iIdx = Pack( iIdx, 0 );

				// G
				iIdx = Pack( iIdx, ( int )( -Double.parseDouble( strToken[ 4 ] ) * ACC_1G_Y ) + 32000 );
				iIdx = Pack( iIdx, ( int )(  Double.parseDouble( strToken[ 3 ] ) * ACC_1G_Z ) + 32000 );

				// ラップタイム
				if( iTokCnt >= 8 && strToken[ 6 ].startsWith( "LAP" )){
					if( strToken[ 7 ].equals( "start" )){
						// LAP start なので，とりあえず RTC をクリア
						i = 1;
						iIdx = Pack( iIdx, i );
						iIdx = Pack( iIdx, i >> 16 );
					}else{
						// ラップタイム記録発見
						i = strToken[ 7 ].indexOf( ':' );
						j = iRtcPrevRaw + ( int )(
							(
								Double.parseDouble( strToken[ 7 ].substring( 0, i )) * 60 +
								Double.parseDouble( strToken[ 7 ].substring( i + 1 ))
							) * 256
						);

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
				dOutputWaitTime += 1000.0 / 16;
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

	class VsdSurfaceView extends SurfaceView implements SurfaceHolder.Callback {

		//*** 生成・破棄 ****************************************************

		Bitmap[] bitmap;
		Paint paint;

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

			Canvas canvas = getHolder().lockCanvas();
			canvas.drawBitmap( bitmap[ 0 ], 0, 0, null );
			getHolder().unlockCanvasAndPost( canvas );
		}

		@Override
		public void surfaceDestroyed( SurfaceHolder holder ){
			if( bDebug ) Log.d( "VSDroid", "surfaceDestroyed" );
			VsdScreen = null;
		}

		//*** 描画 ***********************************************************

		private static final int iScreenWidth = 800;
		private static final int iMeterCx = 399;
		private static final int iMeterCy = 310;
		private static final int iMeterR  = 292;

		boolean bBlink = false;

		private void Draw(){

			if( Vsd == null ) return;

			Canvas canvas = getHolder().lockCanvas();

			int iTacho = Vsd.bEcoMode ? Vsd.iTacho * 2 : Vsd.iTacho;

			// ギアを求める
			double dGearRatio = ( double )Vsd.iSpeedRaw / Vsd.iTacho;
			int	iGear;
			if( Vsd.iTacho == 0 )				iGear = 1;
			else if( dGearRatio < GEAR_RATIO1 ) iGear = 1;
			else if( dGearRatio < GEAR_RATIO2 ) iGear = 2;
			else if( dGearRatio < GEAR_RATIO3 ) iGear = 3;
			else if( dGearRatio < GEAR_RATIO4 ) iGear = 4;
			else								iGear = 5;

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
			canvas.drawText( s, ( iScreenWidth - paint.measureText( s )) / 2, 270, paint );

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
			switch( iMainMode ){
				case MODE_LAPTIME:	s = "Lap";			break;
				case MODE_GYMKHANA:	s = "Gymkhana"; 	break;
				case MODE_ZERO_FOUR:s = "0-400m";		break;
				case MODE_ZERO_ONE:	s = "0-100km/h";
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
		intent.putExtra( "Message", getString( iMessage ));

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
		}else if( VsdThread.isAlive()){
			Vsd.SetupMode();

			if( Pref.getBoolean( "key_reopen_log", false )){
				Vsd.CloseLog();
				Vsd.OpenLog();
			}
		}else{
			// スレッドが死んでいるなら，再スタートすることで
			// 上の処理は実行される．
			VsdThread = new Thread( Vsd );
			VsdThread.start();
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
			Vsd.SetupMode();
			break;

		  case KeyEvent.KEYCODE_MENU:
			iMessage = R.string.statmsg_normal;
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

		// log dir 作成
		File dir;
		dir = new File( VSD_ROOT ); dir.mkdir();
		dir = new File( VSD_LOG  ); dir.mkdir();

		// preference 参照
		Pref = PreferenceManager.getDefaultSharedPreferences( this );

		iConnMode = Integer.parseInt( Pref.getString( "key_connection_mode", "0" ));
		CreateVsdInterface( iConnMode );
	}

	void CreateVsdInterface( int iNewMode ){
		// VSD コネクション
		Vsd =	iNewMode == CONN_MODE_BLUETOOTH	? new VsdInterfaceBluetooth() :
				iNewMode == CONN_MODE_LOGREPLAY	? new VsdInterfaceEmulation() :
												  new VsdInterface();

		// VSD スレッド起動
		VsdThread = new Thread( Vsd );
		VsdThread.start();
		iConnMode = iNewMode;
	}

	@Override
	protected void onDestroy(){
		super.onDestroy();
		Vsd.KillThread();
		if( bDebug ) Log.d( "VSDroid", "Activity::onDestroy finished" );
	}
}
