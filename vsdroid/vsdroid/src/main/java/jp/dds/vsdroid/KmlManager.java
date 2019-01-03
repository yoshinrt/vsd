package jp.dds.vsdroid;

import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.IOException;
import java.io.InputStream;
import java.util.ArrayList;
import java.util.Enumeration;
import java.util.List;
import java.util.zip.ZipEntry;
import java.util.zip.ZipFile;

import org.xmlpull.v1.XmlPullParser;

import android.util.Log;
import android.util.Xml;

public class KmlManager{
	static final boolean bDebug	= BuildConfig.DEBUG;
	
	public ArrayList<Double> m_Coordinates = new ArrayList<Double>();
	public int m_iErrorCode	= OK;
	
	static final int OK					= 0;
	static final int FILE_NOT_FOUND		= 1;
	static final int INVALID_KML_FORMAT	= 2;
	
	KmlManager(){}

	/*** Load KML ***********************************************************/

	private static final int	KML_NONE		= 0;
	private static final int	KML_COORDINATES	= 1 << 0;
	
	public int Load( String strKmlFile ){
		int		iState;
		
		if( strKmlFile == null ){
			return FILE_NOT_FOUND;
		}

		ZipFile zfIn = null;
		InputStream fsIn = null;
		
		try {
			// KMZ を開いてみる
			zfIn = new ZipFile( strKmlFile );
			
			for( Enumeration<? extends ZipEntry> enumulation = zfIn.entries(); enumulation.hasMoreElements();){
				ZipEntry entry = enumulation.nextElement();
				// System.out.println(entry.getName());
				if( entry.isDirectory()) continue;
				
				if( bDebug ) Log.d( "WpNavi", "LoadKML:ZipEntry:" + entry.getName());
				if( entry.getName().endsWith( ".kml" )){
					fsIn = zfIn.getInputStream( entry );
					break;
				}
			}
			// kmz 中に kml がなかった
			if( fsIn == null ){
				zfIn.close();
				return FILE_NOT_FOUND;
			}
		}catch( IOException e ){
			// KMZ で失敗したので，KML を開く
			if( zfIn != null ) try{ zfIn.close(); }catch( IOException e2 ){}
			
			try{
				fsIn = new FileInputStream( strKmlFile );
			}catch( FileNotFoundException e1 ){
				return FILE_NOT_FOUND;
			}
		}

		XmlPullParser xpp = Xml.newPullParser();

		iState = KML_NONE;

		double[] Point = new double[ 2 ];
		ArrayList<Integer> TmpPoints	= new ArrayList<Integer>();
		
		try{
			xpp.setInput( fsIn, "UTF-8" );

			// パース
			String str	= "";

			for( int iType = xpp.getEventType(); iType != XmlPullParser.END_DOCUMENT;
				iType = xpp.next()){
				switch( iType ){
				case XmlPullParser.START_TAG: // 開始タグ
					str = xpp.getName();

					if( str.equals( "coordinates" )){
						iState |= KML_COORDINATES;
					}

					//Log.d( "WpNavi", "Tag:" + str );
					break;

				case XmlPullParser.TEXT: // タグの内容
					if(( iState & KML_COORDINATES ) != 0 ){
						str = xpp.getText();

						// ルート
						int c1 = 0, c2;
						do{
							// 空白のサーチ
							for( c2 = c1; c2 < str.length(); ++c2 ){
								if( str.charAt( c2 ) <= ' ' ) break;
							}

							if( c1 != c2 ){
								ParseCoordinate( str.substring( c1, c2 ));
							}
							c1 = c2 + 1;
						}while( c1 < str.length());

						//Log.d( "WpNavi", "Val:" + str );
						// 空白で取得したものは全て処理対象外とする
					}
					break;

				case XmlPullParser.END_TAG: // 終了タグ
					str = xpp.getName();
					//Log.d( "WpNavi", "Tag/:" + str );
					if( str.equals( "coordinates" )){
						iState &= ~KML_COORDINATES;
					}
					break;
				}
			}
		}catch( Exception e ){
			//e.printStackTrace();
			try{ fsIn.close(); }catch( IOException e2 ){}
			
			m_Coordinates = null;
			return INVALID_KML_FORMAT;
		}

		// close
		try{ fsIn.close(); }catch( IOException e ){}

		// 一応数チェック
		if(
			m_Coordinates.size() == 0
		){
			m_Coordinates = null;
			return INVALID_KML_FORMAT;
		}

		return OK;
	}
	
	private final void ParseCoordinate( String str ){
		int c1, c2;
		if(
			( c1 = str.indexOf( ',' )) >= 0 &&
			( c2 = str.indexOf( ',', c1 + 1 )) >= 0
		){
			m_Coordinates.add( Double.parseDouble( str.substring( 0, c1 )));
			m_Coordinates.add( Double.parseDouble( str.substring( c1 + 1, c2 )));
		}
	}
}
