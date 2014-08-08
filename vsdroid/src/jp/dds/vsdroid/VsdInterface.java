package jp.dds.vsdroid;

import java.util.Calendar;
import android.content.SharedPreferences;
import android.os.Handler;
import android.util.Log;
import java.net.InetSocketAddress;
import java.net.Socket;
import java.net.SocketAddress;
import java.net.SocketTimeoutException;
import java.io.*;
import java.util.*;

//*** VSD アクセス *******************************************************

class VsdInterface implements Runnable {

	static final boolean bDebug = Vsdroid.bDebug;

	// モード・config
	int		iMainMode	= MODE_LAPTIME;
	static final int MODE_LAPTIME	= 0;
	static final int MODE_GYMKHANA	= 1;
	static final int MODE_ZERO_FOUR	= 2;
	static final int MODE_ZERO_ONE	= 3;
	static final int MODE_NUM		= 4;

	//static final double H8HZ			= 16030000;
	//static final double SERIAL_DIVCNT	= 16;		// シリアル出力を行う周期
	static final double LOG_FREQ		= 16;

	// スピード * 100/Taco 比
	// ELISE
	// ギア比 * マージンじゃなくて，ave( ギアn, ギアn+1 ) に変更
	static final double GEAR_RATIO1 = 1.2381712993947;
	static final double GEAR_RATIO2 = 1.82350889069989;
	static final double GEAR_RATIO3 = 2.37581451065366;
	static final double GEAR_RATIO4 = 2.95059529470571;

	// たぶん，ホイル一周が30パルス
	static final double PULSE_PER_1KM	= 15473.76689;	// ELISE(CE28N)

	//static final double ACC_1G_X	= 6762.594337;
	static final double ACC_1G_Y	= 6667.738702;
	static final double ACC_1G_Z	= 6842.591839;

	// VSD ドライバからのメッセージ
	static final int	MSG_LOG_UPDATE	= 0;
	static final int	MSG_OPEN_FAILED	= 1;

	SharedPreferences Pref;
	Thread VsdThread = null;

	// VSD リンクのリードデータ
	int		iTacho			= 0;
	int		iSpeedRaw		= 0;
	int		iRtcRaw			= 0;
	int		iRtcPrevRaw		= 0;
	int		iMileageRaw		= 0;
	int		iTSCRaw			= 0;
	int		iIRCnt			= 0;
	int 	iSectorCnt		= 0;
	int 	iSectorCntMax	= 1;
	Calendar	Cal;

	int		iGx, iGy;
	int		iGx0, iGy0;
	int		iPhoneGx;
	int		iPhoneGy;
	int		iPhoneGx0;
	int		iPhoneGy0;
	int		iThrottle;
	int		iThrottleRaw;
	int		iThrottle0;
	int		iThrottleFull;
	double	dGymkhaStart	= 1;

	int		iConnMode		= CONN_MODE_ETHER;
	int		iGCaribCnt		= iGCaribCntMax;
	LAP_STATE	LapState	= LAP_STATE.NONE;

	// レコード
	int 	iLapNum			= 0;
	int 	iTimeLastRaw	= 0;
	int 	iTimeBestRaw	= 0;

	// ステータスメッセージ
	int iMessage = R.string.statmsg_normal;

	// G センサー用
	int	iPhoneGxRaw = 0;
	int	iPhoneGyRaw = 0;

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

	Handler	MsgHandler	= null;

	private static final int iStartGThrethold		= 500;
	private static final int iGCaribCntMax			= 30;

	static final int CONN_MODE_ETHER		= 0;
	static final int CONN_MODE_BLUETOOTH	= 1;
	static final int CONN_MODE_LOGREPLAY	= 2;

	enum LAP_STATE {
		NONE,
		START,
		UPDATE,
		SECTOR,
	};

	//*** utils **************************************************************

	void Sleep( int ms ){
		try{ Thread.sleep( ms ); }catch( InterruptedException e ){}
	}

	String FormatTime2( int iTime ){
		return String.format( "%d:%02d.%03d",
			iTime / ( 256 * 60 ),
			( iTime >> 8 ) % 60,
			1000 * ( iTime & 0xFF ) / 256
		);
	}

	//************************************************************************

	// コンストラクタ - 変数の初期化だけやってる
	public VsdInterface(){
		if( bDebug ) Log.d( "VSDroid", "VsdInterface::constructor" );

		iTacho			= 0;
		iSpeedRaw		= 0;
		iRtcRaw			= 0;
		iRtcPrevRaw		= 0;
		iMileageRaw		= 0;
		iTSCRaw			= 0;
		iIRCnt			= 0;
		iThrottle0		= 0;
		iSectorCnt		= 0;
		iSectorCntMax	= 1;
		dGymkhaStart	= 1;

		Cal				= Calendar.getInstance( TimeZone.getTimeZone( "GMT+0" ));

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

		iGx0 = iGy0		= 0;
		iGCaribCnt		= iGCaribCntMax;
		iPhoneGx0		= 0;
		iPhoneGy0		= 0;

		InStream		= null;
		OutStream		= null;
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

		// log dir 作成
		File dir = new File( Pref.getString( "key_system_dir", null ) + "/log" );
		dir.mkdirs();

		// 日付
		Calendar Now = Calendar.getInstance();
		String s = String.format(
			"%s/log/vsd%04d%02d%02d_%02d%02d%02d.log",
			Pref.getString( "key_system_dir", null ),
			Now.get( Calendar.YEAR ),
			Now.get( Calendar.MONTH ) + 1,
			Now.get( Calendar.DAY_OF_MONTH ),
			Now.get( Calendar.HOUR_OF_DAY ),
			Now.get( Calendar.MINUTE ),
			Now.get( Calendar.SECOND )
		);

		// ログファイルオープン
		try{
			fsLog    = new BufferedWriter( new FileWriter( s ));
			fsBinLog = new BufferedOutputStream( new FileOutputStream( s + ".bin" ));
		}catch( Exception e ){
			iMessage = R.string.statmsg_log_open_failed;
			return -1;
		}

		// ヘッダ
		try{
			if( fsLog != null ) fsLog.write(
				"Date/Time\tTacho\tSpeed\tDistance\t" +
				"Gy\tGx\tGx(Device)\tGy(Device)\tThrottle\tThrottle(raw)\t" +
				"AuxInfo\tLapTime\tSectorTime\r\n"
			);
		}catch( IOException e ){}

		return 0;
	}

	public int Open(){
		if( bDebug ) Log.d( "VSDroid", "VsdInterface::Open" );

		while( !bKillThread ){
			try{
				// ソケットの作成
				Sock = new Socket();
				SocketAddress adr = new InetSocketAddress( Pref.getString( "key_ip_addr", null ), 12345 );
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
		if( iReadSize < 0 ){
			iMessage = R.string.statmsg_socket_rw_failed;
			return -1;
		}

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
			MsgHandler.sendEmptyMessage( MSG_LOG_UPDATE );

			if( iBufPtr < iEOLPos ){
				iMileage16	= Unpack();
				iTSCRaw		= Unpack();
				iIRCnt		= iTSCRaw;
				iTSCRaw		>>= 8;

				iGy			= ( int )( Unpack() * ( 1000 / ACC_1G_Y / 0.86054332 ));	// 斜めに設置しているので補正
				iGx			= ( int )( Unpack() * ( 1000 / ACC_1G_Z ));
				iThrottleRaw	= 0x7FFFFFFF / Unpack();

				if( iGCaribCnt-- > 0 ){
					// センサーキャリブレーション中
					iGx0		+= iGx;
					iGy0		+= iGy;
					iPhoneGx0	+= iPhoneGxRaw;
					iPhoneGy0	+= iPhoneGyRaw;
					iThrottle0	+= iThrottleRaw;

					if( iGCaribCnt == 0 ){
						iGx0		/= iGCaribCntMax;
						iGy0		/= iGCaribCntMax;
						iPhoneGx0	/= iGCaribCntMax;
						iPhoneGy0	/= iGCaribCntMax;
						iThrottle0	/= iGCaribCntMax;

						iThrottleFull = iThrottle0 - 10000;	// スロットル全開の暫定値
					}

					iGx			= 0;
					iGy			= 0;
					iPhoneGx	= 0;
					iPhoneGy	= 0;
					iThrottle	= 0;
				}else{
					iGx			= iGx - iGx0;
					iGy			= iGy - iGy0;
					iPhoneGx	= iPhoneGxRaw - iPhoneGx0;
					iPhoneGy	= iPhoneGyRaw - iPhoneGy0;

					// スロットル全開値補正
					if( iThrottleRaw < iThrottleFull ){
						iThrottleFull -= ( iThrottleFull - iThrottleRaw ) >> 4;	// 1/16 だけ補正する
					}

					iThrottle	= ( iThrottleRaw - iThrottle0 ) * 1000 / ( iThrottleFull - iThrottle0 );
					if( iThrottle < 0 ) iThrottle = 0;
					else if( iThrottle > 1000 ) iThrottle = 1000;
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
							iTimeLastRaw = iRtcRaw - iRtcPrevRaw;
							if( iTimeBestRaw == 0 || iTimeLastRaw < iTimeBestRaw ) iTimeBestRaw = iTimeLastRaw;

							if( iMainMode != MODE_LAPTIME ){
								// Laptime モード以外でゴールしたら，Ready 状態に戻す
								iRtcPrevRaw = 0;
							}
						}else{
							// ラップスタート
							LapState = LAP_STATE.START;
						}
						iRtcPrevRaw	= iRtcRaw;
						iSectorCnt	= 0;
					}else if( iRtcPrevRaw != 0 ){
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

	long	iLogTimeMilli;
	int		iLogTSC;

	public void WriteLog(){
		String s;

		if( !bLogStart ){
			if( iSpeedRaw > 0 ){
				// 動いたので Log 開始
				bLogStart = true;

				//iLogTSC			= iTSCRaw;
				iLogTimeMilli	= System.currentTimeMillis()
					- iTSCRaw * 1000 / ( int )LOG_FREQ;
			}else{
				// まだ動いていないので，Log 保留
				return;
			}
		}

		// iTSCRaw から時刻算出
		// ★ iLogTSC は ( 2 << 31 ) / 1000 / 16 / 3600 = 37時間，が max
		//if( iTSCRaw < ( iLogTSC & 0xFF )) iLogTSC += 0x100;
		//iLogTSC = ( iLogTSC & ~0xFF ) | iTSCRaw;
		// ★よくわからないが iTSCRaw が信用出来ない
		++iLogTSC;
		Cal.setTimeInMillis( iLogTimeMilli + iLogTSC * 1000 / ( int )LOG_FREQ );

		// 基本データ
		s = String.format(
			"%04d-%02d-%02d" +
			"T%02d:%02d:%02d.%03dZ\t" +
			"%d\t%.2f\t%.2f\t" +
			"%.3f\t%.3f\t%.3f\t%.3f\t" +
			"%.1f\t%d\t%d",
			Cal.get( Calendar.YEAR ),
			Cal.get( Calendar.MONTH ) + 1,
			Cal.get( Calendar.DATE ),
			Cal.get( Calendar.HOUR_OF_DAY ),
			Cal.get( Calendar.MINUTE ),
			Cal.get( Calendar.SECOND ),
			Cal.get( Calendar.MILLISECOND ),
			iTacho, ( double )iSpeedRaw / 100,
			( double )iMileageRaw / PULSE_PER_1KM * 1000,
			iGy / 1000.0, iGx / 1000.0, iPhoneGx / 1000.0, iPhoneGy / 1000.0,
			iThrottle / 10.0, iThrottleRaw,
			iIRCnt
		);

		// ラップタイム
		switch( LapState ){
		  case UPDATE:
			s += "\t" + FormatTime2( iTimeLastRaw ) + "\t";
			break;

		  case START:
			s += "\t0:00.000\t";
			break;

		  case SECTOR:
			s += "\t\t" + FormatTime2( iRtcRaw - iRtcPrevRaw );
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
				fsFirm = new FileInputStream( Pref.getString( "key_roms", null ));

				WaitChar( ':' ); SendCmd( "l\r" );	// FW があったときだけ l コマンド

				// FW の \n を削除しつつ送信
				while(( iReadSize = fsFirm.read( Buf, 0, iBufSize )) > 0 ){
					for( iPtr = i = 0; i < iReadSize; ++i ){
						if( Buf[ i ] != ( byte )0x0A ) Buf[ iPtr++ ] = Buf[ i ];
					}
					OutStream.write( Buf, 0, iPtr );
				}
			}catch( Exception e ){
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

	public void Start(){
		// VSD スレッド起動
		VsdThread = new Thread( this );
		VsdThread.start();
	}

	public void run(){
		bKillThread = false;

		if( bDebug ) Log.d( "VSDroid", "run_loop()" );

		if(
			Open()			>= 0 &&
			LoadFirmWare()	>= 0 &&
			OpenLog()		>= 0
		){
			while( !bKillThread && Read() >= 0 );
		}

		if( bDebug ) Log.d( "VSDroid", String.format( "run() loop extting. reason = %X", iMessage ));

		Close();
		CloseLog();
		if( !bKillThread ) MsgHandler.sendEmptyMessage( MSG_OPEN_FAILED );;

		bKillThread	= false;
		if( bDebug ) Log.d( "VSDroid", "exit_run()" );
	}

	public void KillThread(){
		bKillThread = true;

		if( bDebug ) Log.d( "VSDroid", "KillThread: killing..." );
		try{
			while( bKillThread && VsdThread != null && VsdThread.isAlive()){
				Thread.sleep( 100 );
				Close();
			}
		}catch( InterruptedException e ){}

		VsdThread = null;
		if( bDebug ) Log.d( "VSDroid", "KillThread: done" );
	}
}
