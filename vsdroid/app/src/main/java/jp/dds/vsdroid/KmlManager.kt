package jp.dds.vsdroid

import android.util.Log
import android.util.Xml
import org.xmlpull.v1.XmlPullParser
import java.io.FileInputStream
import java.io.FileNotFoundException
import java.io.IOException
import java.io.InputStream
import java.util.zip.ZipFile

class KmlManager internal constructor() {
	var m_Coordinates: ArrayList<Double>? = ArrayList()

	fun Load(strKmlFile: String?): Int {
		var iState: Int

		if (strKmlFile == null) {
			return FILE_NOT_FOUND
		}

		var zfIn: ZipFile? = null
		var fsIn: InputStream? = null

		try {
			// KMZ を開いてみる
			zfIn = ZipFile(strKmlFile)

			val enumulation = zfIn.entries()
			while (enumulation.hasMoreElements()) {
				val entry = enumulation.nextElement()
				// System.out.println(entry.getName());
				if (entry.isDirectory) {
					continue
				}

				if (bDebug) Log.d("WpNavi", "LoadKML:ZipEntry:" + entry.name)
				if (entry.name.endsWith(".kml")) {
					fsIn = zfIn.getInputStream(entry)
					break
				}
			}
			// kmz 中に kml がなかった
			if (fsIn == null) {
				zfIn.close()
				return FILE_NOT_FOUND
			}
		} catch (e: IOException) {
			// KMZ で失敗したので，KML を開く
			if (zfIn != null) try {
				zfIn.close()
			} catch (_: IOException) {}

			try {
				fsIn = FileInputStream(strKmlFile)
			} catch (e1: FileNotFoundException) {
				return FILE_NOT_FOUND
			}
		}

		val xpp = Xml.newPullParser()

		iState = KML_NONE

		try {
			xpp.setInput(fsIn, "UTF-8")

			// パース
			var str = ""

			var iType = xpp.eventType
			while (iType != XmlPullParser.END_DOCUMENT
			) {
				when (iType) {
					XmlPullParser.START_TAG -> {
						str = xpp.name

						if (str == "coordinates") {
							iState = iState or KML_COORDINATES
						}
					}

					XmlPullParser.TEXT -> if ((iState and KML_COORDINATES) != 0) {
						str = xpp.text

						// ルート
						var c1 = 0
						var c2: Int
						do {
							// 空白のサーチ
							c2 = c1
							while (c2 < str.length) {
								if (str[c2] <= ' ') break
								++c2
							}

							if (c1 != c2) {
								ParseCoordinate(str.substring(c1, c2))
							}
							c1 = c2 + 1
						} while (c1 < str.length)

						//Log.d( "WpNavi", "Val:" + str );
						// 空白で取得したものは全て処理対象外とする
					}

					XmlPullParser.END_TAG -> {
						str = xpp.name
						//Log.d( "WpNavi", "Tag/:" + str );
						if (str == "coordinates") {
							iState = iState and KML_COORDINATES.inv()
						}
					}
				}
				iType = xpp.next()
			}
		} catch (e: Exception) {
			//e.printStackTrace();
			try {
				fsIn!!.close()
			} catch (_: IOException) {}

			m_Coordinates = null
			return INVALID_KML_FORMAT
		}

		// close
		try {
			fsIn!!.close()
		} catch (_: IOException) {}

		// 一応数チェック
		if (m_Coordinates!!.size == 0
		) {
			m_Coordinates = null
			return INVALID_KML_FORMAT
		}

		return OK
	}

	private fun ParseCoordinate(str: String) {
		var c1: Int
		var c2: Int
		if ((str.indexOf(',').also { c1 = it }) >= 0){
			if((str.indexOf(',', c1 + 1).also { c2 = it }) >= 0) {
				m_Coordinates!!.add(str.substring(0, c1).toDouble())
				m_Coordinates!!.add(str.substring(c1 + 1, c2).toDouble())
			}
		}
	}

	companion object {
		val bDebug: Boolean = BuildConfig.DEBUG

		const val OK: Int = 0
		const val FILE_NOT_FOUND: Int = 1
		const val INVALID_KML_FORMAT: Int = 2

		/*** Load KML  */
		private const val KML_NONE = 0
		private const val KML_COORDINATES = 1 shl 0
	}
}
