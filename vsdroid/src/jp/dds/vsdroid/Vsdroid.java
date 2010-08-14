package jp.dds.vsdroid;

import java.util.Calendar;

import android.app.Activity;
import android.content.Context;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.graphics.Canvas;
import android.graphics.Color;
import android.graphics.Paint;
import android.os.Bundle;
import android.util.Log;
import android.view.SurfaceHolder;
import android.view.SurfaceView;
import android.view.KeyEvent;
import android.content.Intent;
import android.view.WindowManager;

import java.net.InetSocketAddress;
import java.net.Socket;
import java.net.SocketAddress;
import java.net.SocketTimeoutException;

import java.io.*;
import java.util.*;

public class Vsdroid extends Activity {

	final int		iScreenWidth = 800;
	final boolean	bDebug	= true;

	// 定数
	final int MODE_LAPTIME	= 0;
	final int MODE_GYMKHANA	= 1;
	final int MODE_ZERO_FOUR= 2;
	final int MODE_ZERO_ONE	= 3;
	final int MODE_NUM		= 4;

	final double H8HZ			= 16030000;
	final double SERIAL_DIVCNT	= 16;		// シリアル出力を行う周期
	final double LOG_FREQ		= 16;

	// スピード * 100/Taco 比
	// ELISE
	// ギア比 * マージンじゃなくて，ave( ギアn, ギアn+1 ) に変更
	final double GEAR_RATIO1 = 1.2381712993947;
	final double GEAR_RATIO2 = 1.82350889069989;
	final double GEAR_RATIO3 = 2.37581451065366;
	final double GEAR_RATIO4 = 2.95059529470571;

	// たぶん，ホイル一周が30パルス
	final double PULSE_PER_1KM	= 15473.76689;	// ELISE(CE28N)

	final double ACC_1G_X	= 6762.594337;
	final double ACC_1G_Y	= 6667.738702;
	final double ACC_1G_Z	= 6842.591839;

	// シフトインジケータの表示
	final int iTachoBar[] = { 334, 200, 150, 118, 97 };
	final int iRevLimit = 6500;

	// モード・config
	int iMainMode		= MODE_LAPTIME;

	final String VSD_ROOT = "/sdcard/vsd";

	// VSD コミュニケーション
	VsdInterface/*Emulation*/ Vsd = null;
	//VsdInterface Vsd;
	Thread VsdThread = null;

	VsdSurfaceView	VsdScreen = null;

	// ステータスメッセージ
	String strMessage = "Normal";

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
		boolean	bUpdateLap		= false;
		boolean	bUpdateSector	= false;
		double	dGx, dGy;

		// レコード
		int 	iLapNum			= 0;
		int 	iTimeLastRaw	= 0;
		int 	iTimeBestRaw	= 0;

		public final int iBufSize = 256;
		byte	Buf[] = new byte[ iBufSize ];
		int		iBufLen = 0;
		int		iBufPtr = 0;

		FileWriter		fsLog		= null;
		OutputStream	fsBinLog	= null;
		Socket			Sock		= null;

		boolean bKillThread = false;

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
			iSectorCntMax	= 1;	// ★あやしい
			bUpdateLap		= false;
			bUpdateSector	= false;

			// レコード
			iLapNum			= 0;
			iTimeLastRaw	= 0;
			iTimeBestRaw	= 0;

			iBufLen 		= 0;
			iBufPtr 		= 0;

			bKillThread		= false;

			fsLog			= null;
			fsBinLog		= null;
			Sock			= null;
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
			// 日付
			Calendar Now = Calendar.getInstance();
			String s = String.format( "/vsd%04d%02d%02d_%02d%02d%02d",
				Now.get( Calendar.YEAR ),
				Now.get( Calendar.MONTH ) + 1,
				Now.get( Calendar.DAY_OF_MONTH ),
				Now.get( Calendar.HOUR_OF_DAY ),
				Now.get( Calendar.MINUTE ),
				Now.get( Calendar.SECOND )
			);

			// ログファイルオープン
			try {
				fsLog    = new FileWriter( VSD_ROOT + s + ".log" );
				fsBinLog = new FileOutputStream( VSD_ROOT + s + "bin.log" );
			} catch (FileNotFoundException e) {
				strMessage = "Log file open failed";
				return -1;
			} catch (IOException e) {
				strMessage = "Log file open IOException";
				return -1;
			}

			return 0;
		}

		public int Open(){
			if( bDebug ) Log.d( "VSDroid", "VsdInterface::Open" );
			try {
				// ソケットの作成
				Sock = new Socket();
				SocketAddress adr = new InetSocketAddress( "192.168.0.1", 12345 );
				Sock.connect( adr, 5000 );
				if( bDebug ) Log.d( "VSDroid", "VsdInterface::Open:connected" );
			}catch( SocketTimeoutException e ){
				if( bDebug ) Log.d( "VSDroid", "VsdInterface::Open:timeout" );
				strMessage = "Socket connection timed out";
				if( Sock != null ) try {
					Sock.close();
				} catch (IOException e1) {}
				Sock = null;
				return -1;
			} catch (IOException e) {
				if( bDebug ) Log.d( "VSDroid", "VsdInterface::Open:IOException" );
				strMessage = "Socket IOException";
				if( Sock != null ) try {
					Sock.close();
				} catch (IOException e1) {}
				Sock = null;
				return -1;
			}
			return 0;
		}

		public int RawRead( int iStart, int iLen ){
			//if( bDebug ) Log.d( "VSDroid", "VsdInterface::RawRead" );
			int iReadSize = 0;
			try {
				iReadSize = Sock.getInputStream().read( Buf, iStart, iLen );
			} catch (IOException e) {
				return -1;
			}
			return iReadSize;
		}

		// 2:スピード，タコのみ 1:データ更新  0:新データなし -1:エラー
		public int Read(){
			int	iReadSize;
			int iEOLPos;
			int	iMileage16;
			int i;
			int	iRet = 1;

			//if( bDebug ) Log.d( "VSDroid", "VsdInterface::Read" );

			iReadSize = RawRead( iBufLen, iBufSize - iBufLen );
			if( iReadSize < 0 ) return -1;

			try {
				fsBinLog.write( Buf, iBufLen, iReadSize );
			} catch (IOException e) {}

			iBufLen += iReadSize;

			// 0xFF をサーチ
			for( iEOLPos = 0; iEOLPos < iBufLen; ++iEOLPos ){
				if( Buf[ iEOLPos ] == ( byte )0xFF ) break;
			}
			if( iEOLPos == iBufLen ) return 0;

			// 0xFF が見つかったので，データ変換
			iBufPtr = 0;
			iTacho		= Unpack();
			iSpeedRaw	= Unpack();

			if( iBufPtr < iEOLPos ){
				iMileage16	= Unpack();
				iTSCRaw		= Unpack();
				dGx			= Unpack();
				dGy			= Unpack();

				// mileage 補正
				if(( iMileageRaw & 0xFFFF ) > iMileage16 ) iMileageRaw += 0x10000;
				iMileageRaw &= 0xFFFF0000;
				iMileageRaw |= iMileage16;

				// LapTime が見つかった
				bUpdateSector = bUpdateLap	= false;
				if( iBufPtr < iEOLPos ){
					iRtcRaw = Unpack() + ( Unpack() << 16 );

					if( ++iSectorCnt >= iSectorCntMax ){
						// 規定のセクタを通過したので，NewLap
						if( iRtcPrevRaw != 0 ){
							// 1周回った
							++iLapNum;
							iTimeLastRaw = iRtcRaw - iRtcPrevRaw;
							if( iTimeBestRaw == 0 || iTimeLastRaw < iTimeBestRaw ){
								iTimeBestRaw = iTimeLastRaw;
							}
						}
						iRtcPrevRaw	= iRtcRaw;
						iSectorCnt	= 0;
						bUpdateLap	= true;
					}else{
						// 中間セクタ通過
						bUpdateSector = true;
					}
				}
			}else{
				iRet = 2;
			}

			// デバッグ用: 複数の 0xFF を破棄する
			if( bDebug ){
				for( i = iBufLen - 1; i >= 0; --i ) if( Buf[ i ] == ( byte )0xFF ) break;
				if( i >= 0 ) iEOLPos = i;
			}

			// 使用したデータを Buf から削除
			iBufPtr = ++iEOLPos;
			for( i = 0; iBufPtr < iBufLen; ){
				Buf[ i++ ] = Buf[ iBufPtr++ ];
			}
			iBufLen = i;

			return iRet;
		}

		public void WriteLog(){
			String s;

			// 基本データ
			s = String.format(
				"%d\t%.2f\t%.2f\t%.4f\t%.4f\t%d",
				iTacho, ( double )iSpeedRaw / 100,
				( double )iMileageRaw / PULSE_PER_1KM * 1000,
				dGy, dGx, iTSCRaw
			);

			// ラップタイム
			if( bUpdateSector ){
				s += String.format( "\tSector%d\t", iSectorCnt ) + FormatTime2( iRtcPrevRaw - iRtcRaw );
			}else if( bUpdateLap ){
				if( iRtcPrevRaw != 0 ){
					s += String.format( "\tLAP%d\t", iLapNum ) + FormatTime2( iTimeLastRaw );
				}else{
					s += String.format( "\tLAP%d start", iLapNum + 1 );
				}
			}

			try {
				fsLog.write( s + "\r\n" );
			} catch (IOException e) {}
		}

		public int CloseLog(){
			try {
				if( fsLog	 != null ) fsLog.close();
				if( fsBinLog != null ) fsBinLog.close();
			} catch (IOException e) {}
			return 0;
		}

		public int Close(){
			if( bDebug ) Log.d( "VSDroid", "VsdInterface::Close" );
			try {
				if( Sock != null ) Sock.close();
			} catch (IOException e) {
			}
			return 0;
		}

		public void RestartLap(){
			iRtcPrevRaw		= 0;
			iSectorCnt		= 0;
		}

		public void SendCmd( String s ) throws IOException {
			byte[] Buf;
			try {
				Buf = s.getBytes( "US-ASCII" );
				Sock.getOutputStream().write( Buf, 0, Buf.length );
			} catch (UnsupportedEncodingException e) {}
		}

		public int WaitChar( char ch ) throws IOException {
			int iReadSize;
			int iRetryCnt = 100;
			int i;

			while( iRetryCnt-- != 0 ){
				if( Sock.getInputStream().available() > 0 ){
					iReadSize = Sock.getInputStream().read( Buf, 0, iBufSize );
					for( i = 0; i < iReadSize; ++i ){
						if( Buf[ i ] == ( byte )ch ){
							iRetryCnt = 0;
							return i;
						}
					}
				}else{
					try { Thread.sleep( 30 ); } catch (InterruptedException e) {}
				}
			}
			return -1;
		}

		public int LoadFirmWare(){
			InputStream	fsFirm = null;

			int iReadSize = 0;
			int iPtr, i = 0;

			try {
				SendCmd( "F15EF117*\rz\r" );

				WaitChar( ':' ); SendCmd( "l\r" );

				try {
					fsFirm = new FileInputStream( VSD_ROOT + "/vsd_rom.mot" );

					// FW の \n を削除しつつ送信
					while(( iReadSize = fsFirm.read( Buf, 0, iBufSize )) > 0 ){
						for( iPtr = i = 0; i < iReadSize; ++i ){
							if( Buf[ i ] != ( byte )0x0A ) Buf[ iPtr++ ] = Buf[ i ];
						}
						Sock.getOutputStream().write( Buf, 0, iPtr );
					}
				} catch ( FileNotFoundException e ){}

				if( fsFirm != null ) fsFirm.close();

				WaitChar( ':' ); SendCmd( "g\r" );

				try { Thread.sleep( 100 ); } catch (InterruptedException e) {}
				SendCmd( "F15EF117*1S" );

				// 最初の 0xFF までスキップ
				int iRetryCnt = 100;
				while( iRetryCnt-- != 0 ){
					if( Sock.getInputStream().available() > 0 ){
						iReadSize = Sock.getInputStream().read( Buf, 0, iBufSize );
						for( i = 0; i < iReadSize; ++i ){
							if( Buf[ i ] == ( byte )0xFF ){
								iRetryCnt = 0;
								break;
							}
						}
					}else{
						try { Thread.sleep( 30 ); } catch (InterruptedException e) {}
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
					strMessage = "VSD data waiting timed out";
					return -1;
				}
			} catch (IOException e) {
				strMessage = "LoadFirmWare() IOException";
				return -1;
			}

			return 0;
		}

		//*** データ処理スレッド *********************************************

		public void run(){
			int i;

			if(
				Vsd.Open()			< 0 ||
				Vsd.LoadFirmWare()	< 0 ||
				Vsd.OpenLog()		< 0
			){
				// エラーが起きたので，config を出して thread 終了
				if( !isFinishing()) Config();

			}else while( !bKillThread ){
				if(( i = Read()) > 0 ){
					if( VsdScreen != null ) VsdScreen.Draw();
					if( i == 1 ) WriteLog();		// テキストログライト
				}
			}
			bKillThread	= false;
			VsdThread	= null;
			if( bDebug ) Log.d( "VSDroid", "exit_run()" );
		}

		public void KillThread(){
			bKillThread = true;
			try {
				while( bKillThread && VsdThread != null && VsdThread.isAlive()){
					Thread.sleep( 100 );
				}
			}catch( InterruptedException e ){}

			VsdThread = null;
		}

	}

	//*** エミュレーションモード *********************************************

	class VsdInterfaceEmulation extends VsdInterface {
		BufferedReader brEmuLog;
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

			try {
				brEmuLog = new BufferedReader( new FileReader( VSD_ROOT + "/vsd.log" ));
			} catch ( FileNotFoundException e ){
				strMessage = "Emulation log open failed";
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

			try {
				strBuf = brEmuLog.readLine();

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
				if( i > 0 ){
					try {
						Thread.sleep( i );
					} catch (InterruptedException e) {}
				}

			} catch ( IOException e ){
				return -1;
			}

			// EOL 追加
			Buf[ iIdx++ ] = ( byte )0xFF;

			return iIdx - iStart;
		}

		@Override
		public int Close(){
			if( bDebug ) Log.d( "VSDroid", "VsdInterfaceEmu::Close" );

			try {
				brEmuLog.close();
			} catch ( IOException e ) {}

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

		//*** GUI ***********************************************************

		@Override
		public boolean onKeyDown( int keyCode, KeyEvent event ){
			if( bDebug ) Log.d( "VSDroid", "onKeyDown" );
			switch( keyCode ){
			  case KeyEvent.KEYCODE_DPAD_CENTER:
				Vsd.RestartLap();
				break;
			  case KeyEvent.KEYCODE_MENU:
				strMessage = "Normal";
				Config();
				break;

			  default:
				return false;
			}
			return true;
		}

		//*** 描画 ***********************************************************

		final int iMeterCx = 399;
		final int iMeterCy = 310;
		final int iMeterR  = 292;

		boolean bBlink = false;

		private void Draw(){

			if( Vsd == null ) return;

			Canvas canvas = getHolder().lockCanvas();

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
			}else if( Vsd.iTacho >= iRevLimit ){
				iBarLv = 5;
			}else{
				// LED の表示 LV を求める
				iBarLv = 3 - ( iRevLimit - Vsd.iTacho ) / iTachoBar[ iGear - 1 ];
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
			double dTachoAngle = ( 210 - ( 240 * Vsd.iTacho / 8000.0 )) * Math.PI / 180;
			canvas.drawLine(
				iMeterCx, iMeterCy,
				iMeterCx + ( int )( Math.cos( dTachoAngle ) * iMeterR ),
				iMeterCy - ( int )( Math.sin( dTachoAngle ) * iMeterR ),
				paint
			);

			String s;

			// スピード
			paint.setColor( Color.WHITE );
			s = String.format( "%d", Vsd.iSpeedRaw / 100 );
			paint.setTextSize( 180 );
			canvas.drawText( s, ( iScreenWidth - paint.measureText( s )) / 2, 270, paint );

			// ギア
			paint.setColor( Color.BLACK );
			paint.setTextSize( 140 );
			s = String.format( "%d", iGear );
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
			}else if( Vsd.bUpdateLap && iMainMode != MODE_LAPTIME ){
				// Laptime モード以外でゴールしたら，Ready 状態に戻す
				Vsd.iRtcPrevRaw = 0;
			}

			paint.setTextSize( 32 );
			canvas.drawText( s, 220, 355, paint );
			paint.setTextSize( 64 );
			canvas.drawText( String.format( "%02d", Vsd.iLapNum ), 220, 410, paint );

			paint.setColor(
				Vsd.iTimeBestRaw == 0					? Color.GRAY :
				Vsd.iTimeBestRaw == Vsd.iTimeLastRaw	? Color.CYAN :
														  Color.RED
			);
			canvas.drawText( FormatTime( Vsd.iTimeLastRaw ), 340, 410, paint );
			paint.setColor( Color.GRAY );
			canvas.drawText( FormatTime( Vsd.iTimeBestRaw ), 340, 470, paint );

			getHolder().unlockCanvasAndPost( canvas );
		}
	}

	//*** config メニュー ****************************************************

	// config を開く
	public void Config(){
		if( bDebug ) Log.d( "VSDroid", "Config" );
		Intent intent = new Intent( Vsdroid.this, Config.class );
		intent.putExtra( "Message", strMessage );

		startActivityForResult( intent, 0 /*SHOW_EDITOR*/ );
	}

	@Override
	protected void onActivityResult(int requestCode, int resultCode, Intent data) {
		if( bDebug ) Log.d( "VSDroid", "ConfigResult" );

		if( VsdThread == null ){
			// スレッド再起動
			VsdThread = new Thread( Vsd );
			VsdThread.start();
		}
		/*
		if (requestCode == SHOW_EDITOR) {
			if (resultCode == RESULT_OK) {
				TextView textView = (TextView)findViewById(R.id.TextView01);
			   	textView.setText(data.getCharSequenceExtra("TEXT"));
			}
		}
		*/
	}

	//************************************************************************

	@Override
	protected void onCreate( final Bundle savedInstanceState ){
		super.onCreate( savedInstanceState );

		if( bDebug ) Log.d( "VSDroid", "Activity::onCreate" );
		getWindow().addFlags( WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON );

		setContentView( new VsdSurfaceView( this ));

		// log dir 作成
		File dir = new File( VSD_ROOT );
		dir.mkdir();

		// VSD コネクション
		Vsd = new VsdInterface/*Emulation*/();
		// VSD スレッド起動
		VsdThread = new Thread( Vsd );
		VsdThread.start();
	}

	@Override
	protected void onDestroy(){
		super.onDestroy();

		Vsd.KillThread();
		Vsd.Close();
		Vsd.CloseLog();

		if( bDebug ) Log.d( "VSDroid", "Activity::onDestroy finished" );
	}
}
