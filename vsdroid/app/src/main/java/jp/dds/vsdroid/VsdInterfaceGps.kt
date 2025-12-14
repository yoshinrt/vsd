package jp.dds.vsdroid

import androidx.activity.ComponentActivity
import java.io.IOException

//*** VSD アクセス *******************************************************
open class VsdInterfaceGps(activity: ComponentActivity) : VsdInterface(activity) {

	override fun OpenVsdIf() {}
	override fun CloseVsdIf() {}

	// 1 <= :受信したレコード数  0:新データなし
	@Throws(IOException::class, UnrecoverableException::class)
	override fun Read(): Int {
		try {
			if (GpsData != null) {
				iSpeedRaw = (GpsData!!.fSpeed * 100).toInt()
				iTacho = GpsData!!.fSpeed.toInt()
				iTSCRaw = GpsData!!.iNmeaTime
			}
		}catch(e: Exception){}
		
		// ここで描画要求
		MsgHandler!!.sendEmptyMessage(R.string.statmsg_update)
		Sleep(100)
		return 0
	}

	//*** FW ロード ******************************************************
	override open fun LoadFirmware() {}
}
