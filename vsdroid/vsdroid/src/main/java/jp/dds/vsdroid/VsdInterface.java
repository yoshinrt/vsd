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
import java.util.zip.GZIPOutputStream;

//*** VSD アクセス *******************************************************

class VsdInterface implements Runnable {

	static final boolean bDebug = BuildConfig.DEBUG;

	// モード・config
	int		iMainMode	= MODE_LAPTIME;
	static final int MODE_LAPTIME	= 0;
	static final int MODE_GYMKHANA	= 1;
	static final int MODE_ZERO_FOUR	= 2;
	static final int MODE_ZERO_ONE	= 3;
	static final int MODE_NUM		= 4;

	static final int TIMER_HZ	= 200000;
	static final int TSC_SHIFT	= 5;

	// スピード * 100/Taco 比
	// ELISE
	// ギア比 * マージンじゃなくて，ave( ギアn, ギアn+1 ) に変更
	static final double GEAR_RATIO[] = {
		1.2381712993947,
		1.82350889069989,
		2.37581451065366,
		2.95059529470571,
	};
	
	// たぶん，ホイル一周が30パルス
	static final double PULSE_PER_1KM_CE28N		= 15473.76689;	// CE28N
	static final double PULSE_PER_1KM_NORMAL	= 14958.80127;	// ノーマル
	double dPulsePer1Km	= PULSE_PER_1KM_CE28N;

	SharedPreferences Pref;
	Thread VsdThread = null;

	// VSD リンクのリードデータ
	int		iTacho			= 0;
	int		iSpeedRaw		= 0;
	int		iRtcRaw			= 0;
	int		iRtcPrevRaw		= 0;
	int		iMileageRaw		= 0;
	int		iTSCRaw			= 0;
	int 	iSectorCnt		= 0;
	int 	iSectorCntMax	= 1;
	Calendar	Cal;
	Calendar	CalDev;

	int		iGx, iGy;
	int		iGx0, iGy0;
	int		iThrottle;
	int		iThrottle0;
	int		iThrottleMax;
	int		iThrottleRaw;
	double	dGymkhaStart	= 1;

	int		iConnMode		= CONN_MODE_ETHER;
	int		iGCaribCnt		= iGCaribCntMax;
	LAP_STATE	LapState	= LAP_STATE.NONE;

	// レコード
	int 	iLapNum			= 0;
	int 	iTimeLastRaw	= 0;
	int 	iTimeBestRaw	= 0;

	private static final int iBufSize = 256;

	byte	Buf[] = new byte[ iBufSize ];
	int		iBufLen = 0;
	int		iBufPtr = 0;

	GZIPOutputStream		fsLog		= null;
	BufferedOutputStream	fsBinLog	= null;

	boolean			bDebugInfo	= false;
	boolean			bLogStart	= false;
	Socket			Sock		= null;

	InputStream		InStream	= null;
	OutputStream	OutStream	= null;

	volatile boolean bKillThread = false;

	Handler	MsgHandler	= null;
	
	GpsInterface	Gps	= null;

	private static final int iStartGThrethold		= 1024;	// 0.25G
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
	
	static final int ERROR			= -1;
	static final int FATAL_ERROR	= -2;
	
	//*** リトライしないerror ************************************************
	
	public class UnrecoverableException extends Exception {
		private static final long serialVersionUID = 1L;
		public UnrecoverableException( String message ){
			super( message );
		}
	}
	
	//*** utils **************************************************************

	void Sleep( int ms ){
		try{ Thread.sleep( ms ); }catch( InterruptedException e ){}
	}

	static String FormatTime( int iTime ){
		return String.format( "%d'%02d.%03d",
			iTime / ( TIMER_HZ * 60 ),
			( iTime / TIMER_HZ ) % 60,
			1000 * ( iTime % TIMER_HZ ) / TIMER_HZ
		);
	}

	static String FormatTime2( int iTime ){
		return String.format( "%d:%02d.%03d",
			iTime / ( TIMER_HZ * 60 ),
			( iTime / TIMER_HZ ) % 60,
			1000 * ( iTime % TIMER_HZ ) / TIMER_HZ
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
		iThrottle0		= -48650;	// スロットルの 0地点    (実測値 49650-39277)
		iThrottleMax	= 8000;		// スロットル 0～100% 幅
		iSectorCnt		= 0;
		iSectorCntMax	= 1;
		dGymkhaStart	= 1;

		Cal				= Calendar.getInstance( TimeZone.getTimeZone( "GMT+0" ));
		CalDev			= Calendar.getInstance( TimeZone.getTimeZone( "GMT+0" ));

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

		InStream		= null;
		OutStream		= null;
	}

	// 2byte を 16bit int にアンパックする
	private int Unpack(){
		int iRet;

		if( Buf[ iBufPtr ] == ( byte )0xFE ){
			++iBufPtr;
			iRet = Buf[ iBufPtr++ ] + 0xFE;
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
			"%s/log/vsd%04d%02d%02d_%02d%02d%02d",
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
			fsLog = new GZIPOutputStream( new BufferedOutputStream( new FileOutputStream( s + ".log.gz" )));
			if( bDebugInfo ){
				fsBinLog = new BufferedOutputStream( new FileOutputStream( s + ".bin" ));
			}
		}catch( Exception e ){
			MsgHandler.sendEmptyMessage( R.string.statmsg_mklog_failed );
			return FATAL_ERROR;
		}

		// ヘッダ
		try{
			if( fsLog != null ) fsLog.write(
				(
					"Date/Time\tDev Date Time\tTacho\tSpeed\tDistance\t" +
					"Gy\tGx\tThrottle\tThrottle(raw)\t" +
					"AuxInfo\tLapTime\tSectorTime\r\n"
				).getBytes()
			);
		}catch( IOException e ){}

		return 0;
	}

	public void OpenVsdIf() throws IOException, UnrecoverableException {
		if( bDebug ) Log.d( "VSDroid", "VsdInterface::OpenVsdIf" );

		MsgHandler.sendEmptyMessage( R.string.statmsg_tcpip_connecting );
		try{
			// ソケットの作成
			Sock = new Socket();
			SocketAddress adr = new InetSocketAddress( Pref.getString( "key_ip_addr", null ), 12345 );
			Sock.connect( adr, 1000 );
			if( bDebug ) Log.d( "VSDroid", "VsdInterface::OpenVsdIf:connected" );
			InStream	= Sock.getInputStream();
			OutStream	= Sock.getOutputStream();
			
			MsgHandler.sendEmptyMessage( R.string.statmsg_tcpip_connected );
		}catch( SocketTimeoutException e ){
			if( bDebug ) Log.d( "VSDroid", "VsdInterface::OpenVsdIf:timeout" );
			CloseVsdIf();
			throw new IOException( "socket timeout" );
		}catch( IOException e ){
			if( bDebug ) Log.d( "VSDroid", "VsdInterface::OpenVsdIf:IOException" );
			CloseVsdIf();
			throw e;
		}
	}

	public int RawRead( int iStart, int iLen ) throws IOException, UnrecoverableException {
		int iReadSize = InStream.read( Buf, iStart, iLen );
		if( iReadSize <= 0 ){
			throw new IOException( "read size < 1" );
		}
		return iReadSize;
	}

	// 1 <= :受信したレコード数  0:新データなし
	public int Read() throws IOException, UnrecoverableException {
		int	iReadSize;
		int	iMileage16;
		int	iRet	= 0;
		int	iEOLPos	= 0;
		int i;

		//if( bDebug ) Log.d( "VSDroid", "VsdInterface::Read" );
		
		iReadSize = RawRead( iBufLen, iBufSize - iBufLen );
		if( iReadSize == 0 ) return 0;

		try{
			if( fsBinLog != null ) fsBinLog.write( Buf, iBufLen, iReadSize );
		}catch( IOException e ){}

		iBufLen += iReadSize;

		while( !bKillThread ){
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
			MsgHandler.sendEmptyMessage( R.string.statmsg_update );

			iMileage16	= Unpack();
			iTSCRaw		= Unpack();
			iGy			= Unpack();
			iGx			= Unpack();
			
			iThrottleRaw	= -( 0x7FFFFFFF / Unpack()) - iThrottle0;

			if( iGCaribCnt-- > 0 ){
				// センサーキャリブレーション中
				iGx0 += iGx;
				iGy0 += iGy;
				
				if( iGCaribCnt == 0 ){
					iGx0 /= iGCaribCntMax;
					iGy0 /= iGCaribCntMax;
				}

				iGx = 0;
				iGy = 0;
			}else{
				iGx = iGx - iGx0;
				iGy = iGy - iGy0;

				// スロットル 0, Full 値補正
				// Raw 値が 0～Full 値を外れたら 1/16 だけ補正する
				if( iThrottleRaw < 0 ){
					iThrottle0 += iThrottleRaw >> 4;
				}else if( iThrottleRaw > iThrottleMax ){
					iThrottleMax += ( iThrottleRaw - iThrottleMax ) >> 4;
				}

				// 0, 100% 付近の 1/32 ≒ 3% 分は不感帯にする
				iThrottle = ( iThrottleRaw - ( iThrottleMax >> 5 ))
					* ( int )( 1000 / ( 1 - 1.0 / 16 )) / iThrottleMax;
				
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
	int		iTSC;	// 200KHz >> 8

	static final int Tsc2Milli( int iTSC ){
		return ( int )( iTSC * (( 1 << TSC_SHIFT ) * 1000.0 / TIMER_HZ ));
	}
	
	public void WriteLog(){
		if( !bLogStart ){
			// まだ動いていないので，Log 保留
			if( !bDebugInfo && iSpeedRaw == 0 ) return;
			
			// 動いたので Log 開始
			bLogStart		= true;
			iTSC			= iTSCRaw;
			iLogTimeMilli	= System.currentTimeMillis() - Tsc2Milli( iTSC );
			
			if( fsLog == null ) OpenLog();
		}
		
		if( fsLog == null ) return;
		
		// iTSCRaw から時刻算出
		if(( iTSC & 0xFFFF ) > iTSCRaw ) iTSC += 0x10000;
		iTSC = ( iTSC & 0xFFFF0000 ) | iTSCRaw;
		Cal.setTimeInMillis( iLogTimeMilli + Tsc2Milli( iTSC ));
		CalDev.setTimeInMillis( System.currentTimeMillis());

		// 基本データ
		StringBuilder s = new StringBuilder(
			String.format(
				"%04d-%02d-%02dT%02d:%02d:%02d.%03dZ\t" +
				"%04d-%02d-%02dT%02d:%02d:%02d.%03dZ\t" +
				"%d\t%.2f\t%.2f\t" +
				"%.3f\t%.3f\t" +
				"%.1f\t%d\t%d",
				Cal.get( Calendar.YEAR ), Cal.get( Calendar.MONTH ) + 1, Cal.get( Calendar.DATE ),
				Cal.get( Calendar.HOUR_OF_DAY ), Cal.get( Calendar.MINUTE ), Cal.get( Calendar.SECOND ), Cal.get( Calendar.MILLISECOND ),
				CalDev.get( Calendar.YEAR ), CalDev.get( Calendar.MONTH ) + 1, CalDev.get( Calendar.DATE ),
				CalDev.get( Calendar.HOUR_OF_DAY ), CalDev.get( Calendar.MINUTE ), CalDev.get( Calendar.SECOND ), CalDev.get( Calendar.MILLISECOND ),
				iTacho, iSpeedRaw / 100.0,
				iMileageRaw * ( 1000 / dPulsePer1Km ),
				iGy / 4096.0, iGx / 4096.0,
				iThrottle / 10.0, iThrottleRaw,
				iTSCRaw
			)
		);

		// ラップタイム
		switch( LapState ){
		  case UPDATE:
			s.append( "\t" ).append( FormatTime2( iTimeLastRaw )).append( "\t" );
			break;

		  case START:
			s.append( "\t0:00.000\t" );
			break;

		  case SECTOR:
			s.append( "\t\t" ).append( FormatTime2( iRtcRaw - iRtcPrevRaw ));
		}

		s.append( "\r\n" );
		try{
			if( fsLog != null ) fsLog.write( s.toString().getBytes());
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

	public int CloseVsdIf(){
		if( bDebug ) Log.d( "VSDroid", "VsdInterface::CloseVsdIf" );
		
		// BT 切断すると AT コマンドモードになるので，シリアル出力を止める
		try{
			SendCmd( "z" );
		}catch( IOException e ){}
		
		try{
			if( Sock != null ){
				Sock.close();
				Sock = null;
			}
		}catch( IOException e ){}
		return 0;
	}

	//*** VSD + GPS open / close *****************************************
	
	public void Open() throws UnrecoverableException, IOException {
		OpenVsdIf();
		Gps = new GpsInterface( MsgHandler, Pref );
		Gps.Start();
	}
	
	public void Close(){
		if( Gps != null ){
			Gps.KillThread();
			Gps.Close();
		}
		
		CloseVsdIf();
	}
	
	//*** sio コマンド関係 ***********************************************

	public void SendCmd( String s ) throws IOException {
		if( OutStream == null ) return;

		try{
			OutStream.write( s.getBytes());
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
					if( Buf[ i ] == ( byte )ch ) return i;
				}
			}else{
				Sleep( 30 );
			}
		}
		throw new IOException( "WaitChar() time out" );
	}

	//*** FW ロード ******************************************************

	public void LoadFirmware() throws IOException {
		InputStream	fsFirm = null;

		int iReadSize = 0;
		int i = 0;

		MsgHandler.sendEmptyMessage( R.string.statmsg_loadfw_loading );
		
		try{
			// .mot オープン
			fsFirm = new FileInputStream( Pref.getString( "key_roms", "vsd2.mot" ));
			
			if( bDebug ) Log.d( "VSDroid", "LoadFirm::sending magic code" );
			MsgHandler.sendEmptyMessage( R.string.statmsg_loadfw_magic );
			SendCmd( "F15EF117*z" );
			
			if( bDebug ) Log.d( "VSDroid", "LoadFirm::sending firmware" );
			
			MsgHandler.sendEmptyMessage( R.string.statmsg_loadfw_wait );
			WaitChar( ':' );
			
			// FW 送信
			MsgHandler.sendEmptyMessage( R.string.statmsg_loadfw_sending );
			while(( iReadSize = fsFirm.read( Buf, 0, iBufSize )) > 0 ){
				OutStream.write( Buf, 0, iReadSize );
			}
			
			MsgHandler.sendEmptyMessage( R.string.statmsg_loadfw_wait );
			if( bDebug ) Log.d( "VSDroid", "LoadFirm::go" );
			WaitChar( ':' );
			
		}catch( FileNotFoundException e ){
			// FW がない場合ここに飛ぶ
			if( bDebug ) Log.d( "VSDroid", "LoadFirm::sending firmware canceled" );
			MsgHandler.sendEmptyMessage( R.string.statmsg_loadfw_none );
		}
		
		try {
			if( fsFirm != null ) fsFirm.close();
		}catch( IOException e1 ){}
		
		if( bDebug ) Log.d( "VSDroid", "LoadFirm::sending log output request" );
		Sleep( 100 );
		MsgHandler.sendEmptyMessage( R.string.statmsg_loadfw_magic );
		SendCmd( "F15EF117*" );

		// 最初の 0xFF までスキップ
		MsgHandler.sendEmptyMessage( R.string.statmsg_loadfw_wait );
		if( bDebug ) Log.d( "VSDroid", "LoadFirm::waiting first log record" );

		i = WaitChar(( char )0xFF ) + 1;
		for( iBufLen = 0; i < iReadSize; ++i, ++iBufLen ){
			Buf[ iBufLen ] = Buf[ i ];
		}

		MsgHandler.sendEmptyMessage( R.string.statmsg_loadfw_loaded );
		if( bDebug ) Log.d( "VSDroid", "LoadFirm::completed." );
	}

	//*** config に従って設定 ********************************************

	public void SetupMode() throws IOException {
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
		
		// ホイール設定
		dPulsePer1Km = Pref.getString( "key_wheel_select", "CE28N" ).equals( "CE28N" )
			? PULSE_PER_1KM_CE28N : PULSE_PER_1KM_NORMAL;
		
		// メインモード
		try{
			iMainMode = Integer.parseInt( Pref.getString( "key_vsd_mode", "0" ));
			if( iMainMode < 0 || MODE_NUM <= iMainMode ) iMainMode = MODE_LAPTIME;
		}catch( NumberFormatException e ){}

		bDebugInfo = Pref.getBoolean( "key_debug_info", false );
		
		switch( iMainMode ){
		  case MODE_LAPTIME:
			SendCmd( "l" );
			break;

		  case MODE_GYMKHANA:
			// * は g_lParam をいったんクリアする意図
			i = ( int )( dGymkhaStart * dPulsePer1Km / 1000 + 0.5 );
			if( i < 1 ) i = 1;
			SendCmd( String.format( "*%Xg", i ));
			break;

		  case MODE_ZERO_FOUR:
			SendCmd( String.format( "*%Xf", iStartGThrethold ));
			break;

		  case MODE_ZERO_ONE:
			SendCmd( String.format( "*%Xo", iStartGThrethold ));
		}
		
		// ホイール・Hz 設定
		
		// 再び sio on と wheel 設定
		int iLogHz = 16;
		try{
			iLogHz = Integer.parseInt( Pref.getString( "key_log_hz", "16" ));
		}catch( NumberFormatException e ){}
		
		SendCmd( String.format(
			"*%Xw%Xh",
			( int )( dPulsePer1Km * ( 1 << 18 ) - ( 0x40000000 * 2.0 )) ^ 0x80000000,
			iLogHz
		));
		
		iRtcPrevRaw		= 0;
		iSectorCnt		= 0;
		bLogStart		= false;	// TSC と実時間の同期を取り直す
	}

	//*** データ処理スレッド *********************************************

	public void Start(){
		// VSD スレッド起動
		VsdThread = new Thread( this );
		VsdThread.start();
	}

	volatile int uReadWdt;
	public void run(){
		bKillThread = false;
		Timer ReadWdt = null;
		
		if( bDebug ) Log.d( "VSDroid", "run_loop()" );
		
		while( !bKillThread ){
			try{
				Close(); // 開いていたら一旦 close
				Open();
				LoadFirmware();
				SetupMode();
				
				// Read WDT 発動
				uReadWdt = 0;
				ReadWdt = new Timer();
				ReadWdt.scheduleAtFixedRate(
					new TimerTask(){
						@Override
						public void run(){
							if( bKillThread || ++uReadWdt > 6 ){
								if( bDebug ) Log.d( "VSDroid", String.format( "WDT timeout" ));
								// 0.5 * 6sec Read できなければ
								// ソケット強制クローズで Read() を失敗させる
								Close();
								this.cancel();
							}
						}
					}, 0, 500
				);
				
				while( !bKillThread && Read() >= 0 ){
					uReadWdt = 0;
					SendCmd( "*" );	// ビーコン送信
				}
			}catch( UnrecoverableException e ){
				// リトライしない中止要求
				// メッセージは各個設定しておくこと
				break;
			}catch( IOException e ){
				MsgHandler.sendEmptyMessage( R.string.statmsg_disconnected );
				if( bDebug ) e.printStackTrace();
				Sleep( 1000 );
			}finally{
				if( ReadWdt != null ) ReadWdt.cancel();
			}
		}
		
		if( bDebug ) Log.d( "VSDroid", String.format( "run() loop extting." ));

		Close();
		CloseLog();

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
