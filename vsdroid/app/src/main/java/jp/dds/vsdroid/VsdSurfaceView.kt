package jp.dds.vsdroid

import android.content.Context
import android.graphics.Color
import android.graphics.Paint
import android.graphics.PorterDuff
import android.graphics.RectF
import android.graphics.Typeface
import android.util.AttributeSet
import android.util.Log
import android.view.SurfaceHolder
import android.view.SurfaceView
import java.util.Calendar
import java.util.Locale
import kotlin.math.cos
import kotlin.math.sin

//************************************************************************
class VsdSurfaceView (context: Context?, attrs: AttributeSet) : SurfaceView(context, attrs),
	SurfaceHolder.Callback {
	//*** 生成 / 破棄 ****************************************************
	var paint: Paint

	var fScale: Float = 0f
	var fOffsX: Float = 0f
	var fOffsY: Float = 0f
	
	// メッセージログ
	private val iMsgLog = IntArray(MAX_MSG_LOG)
	private var iMsgLogNum = 0
	private var iMsgLogPtr = 0
	private var bMsgLogShow = false
	private var bEnableUpdate = false

	private var fLapTimeX = 0f

	override fun surfaceChanged(holder: SurfaceHolder, format: Int, width: Int, height: Int) {
		if (bDebug) Log.d("VSDroid", "surfaceChanged")
		bEnableUpdate = true
	}

	override fun surfaceCreated(holder: SurfaceHolder) {
		if (bDebug) Log.d("VSDroid", "surfaceCreated")

		val fScaleX = width.toFloat() / BASE_WIDTH
		val fScaleY = height.toFloat() / BASE_HEIGHT
		if (fScaleX < fScaleY) {
			// X 側が幅いっぱい
			fScale = fScaleX
			fOffsY = (height - BASE_HEIGHT * fScale) / 2 * fScale
		} else {
			// Y 側が高さいっぱい
			fScale = fScaleY
			fOffsX = (width - BASE_WIDTH * fScale) / 2 * fScale
		}

		try {
			Draw(R.string.statmsg_update, null, null)
		} catch (e: Exception) {
			if (bDebug) Log.d("VSDroid", "surfaceCreated::LockCanvas failed")
		}
		bEnableUpdate = true
	}

	override fun surfaceDestroyed(holder: SurfaceHolder) {
		if (bDebug) Log.d("VSDroid", "surfaceDestroyed")
		bEnableUpdate = false
	}

	var bBlink: Boolean = false
	var iTacho: Int = 0
	var iTachoBar: Int = 0

	init {
		if (bDebug) Log.d("VSDroid", "VsdSurfaceView::constructor")

		holder.addCallback(this)
		isFocusable = true // キーイベントを使うために必須
		requestFocus() // フォーカスを当てないとキーイベントを拾わない

		// Paint 取得・初期設定
		paint = Paint()
		//paint.setAntiAlias( true );
	}

	fun Draw(Msg: Int, Vsd: VsdInterface?, Setting: ViewSetting?) {
		var rectf: RectF
		var s: String
		var iGear = 1

		if(!bEnableUpdate) return

		val canvas = holder.lockCanvas()
		if(canvas == null) return
		canvas.translate(fOffsX / fScale, fOffsY / fScale)
		canvas.scale(fScale, fScale)

		// メッセージ push
		if (Msg == R.string.statmsg_update) {
			bMsgLogShow = false
		} else {
			bMsgLogShow = true
			iMsgLog[iMsgLogPtr] = Msg
			if (iMsgLogNum < MAX_MSG_LOG) ++iMsgLogNum
			iMsgLogPtr = (iMsgLogPtr + 1) % MAX_MSG_LOG
		}

		if (Vsd != null) {
			iTacho = if (Setting?.bEcoMode ?: false) Vsd.iTacho * 2 else Vsd.iTacho


			// ギアを求める
			val dGearRatio = Vsd.iSpeedRaw.toDouble() / Vsd.iTacho
			if (Vsd.iTacho == 0) iGear = 1
			else {
				iGear = 1
				while (iGear <= VsdInterface.GEAR_RATIO.size) {
					if (dGearRatio < VsdInterface.GEAR_RATIO[iGear - 1]) break
					++iGear
				}
			}

			if ((Vsd.iSpeedRaw == 0xFFFF) && (Vsd.iTacho == 0)) {
				// キャリブレーション表示
				iTachoBar = iSweepAngle / 2
			} else {
				// LED の表示 LV を求める
				iTachoBar =
					(iTacho - iRevLimit + iTachoBarList[iGear - 1]) * (iSweepAngle / 2) / iTachoBarList[iGear - 1]

				if (iTachoBar < 0) {
					iTachoBar = 0
				} else if (iTachoBar > iSweepAngle / 2) {
					iTachoBar = if (bBlink) iSweepAngle / 2 else 0
					bBlink = !bBlink
				}
			}
		}

		// メーターパネル
		paint.setTypeface(Typeface.DEFAULT_BOLD)
		canvas.drawColor(0, PorterDuff.Mode.CLEAR)

		// メーター目盛り円弧
		paint.style = Paint.Style.STROKE
		paint.strokeWidth = fMeterScaleW
		paint.color = COLOR_PURPLE

		rectf = RectF(
			(iMeterCx - iMeterR).toFloat(),
			(iMeterCy - iMeterR).toFloat(),
			(iMeterCx + iMeterR).toFloat(),
			(iMeterCy + iMeterR).toFloat()
		)
		canvas.drawArc(
			rectf,
			iStartAngle.toFloat(),
			iSweepAngle.toFloat(),
			false,
			paint
		)

		// タコバー描画
		if (iTachoBar > 0) {
			paint.color = Color.CYAN
			rectf = RectF(
				(iMeterCx - iMeterR).toFloat(),
				(iMeterCy - iMeterR).toFloat(),
				(iMeterCx + iMeterR).toFloat(),
				(iMeterCy + iMeterR).toFloat()
			)
			canvas.drawArc(
				rectf,
				iStartAngle.toFloat(),
				iTachoBar.toFloat(),
				false,
				paint
			)
			canvas.drawArc(
				rectf,
				(iStartAngle + iSweepAngle - iTachoBar).toFloat(),
				iTachoBar.toFloat(),
				false,
				paint
			)
		}

		// レッドゾーン円弧
		paint.strokeWidth = fMeterRedZoneW
		paint.color = COLOR_ORANGE
		rectf = RectF(
			iMeterCx - fMeterRedZoneR,
			iMeterCy - fMeterRedZoneR,
			iMeterCx + fMeterRedZoneR,
			iMeterCy + fMeterRedZoneR
		)
		canvas.drawArc(
			rectf,
			iRedZoneStartAngle.toFloat(),
			iRedZoneSweepAngle.toFloat(),
			false,
			paint
		)

		// 目盛線・数値
		paint.strokeWidth = 6f
		paint.color = Color.WHITE
		paint.textSize = iMeterFontSize.toFloat()
		for (i in 0..(iMeterMaxRev * 2)) {
			paint.style = Paint.Style.STROKE
			val dAngle =
				Math.toRadians(iStartAngle + i * (iSweepAngle / (iMeterMaxRev * 2).toDouble()))
			// 目盛り点
			canvas.drawLine(
				(iMeterCx + (cos(dAngle) * fMeterLineR1)).toFloat(),
				(iMeterCy + (sin(dAngle) * fMeterLineR1)).toFloat(),
				(iMeterCx + (cos(dAngle) * fMeterLineR2)).toFloat(),
				(iMeterCy + (sin(dAngle) * fMeterLineR2)).toFloat(),
				paint
			)

			if ((i and 1) != 0) {
				// 目盛線
				canvas.drawLine(
					(iMeterCx + (cos(dAngle) * fMeterLineR3)).toFloat(),
					(iMeterCy + (sin(dAngle) * fMeterLineR3)).toFloat(),
					(iMeterCx + (cos(dAngle) * fMeterLineR4)).toFloat(),
					(iMeterCy + (sin(dAngle) * fMeterLineR4)).toFloat(),
					paint
				)
			} else {
				// 目盛り数値
				s = (i / 2).toString()
				paint.style = Paint.Style.FILL
				canvas.drawText(
					s,
					(iMeterCx + (cos(dAngle) * iMeterR) - paint.measureText(
						s
					) / 2).toFloat(),
					(iMeterCy + (sin(dAngle) * iMeterR) + iMeterFontSize / 2).toFloat(),
					paint
				)
			}
		}

		canvas.drawLine(
			(iMeterCx - iSpeedUnderlineW / 2).toFloat(), 292f,
			(iMeterCx + iSpeedUnderlineW / 2).toFloat(), 292f, paint)

		// ギア箱
		paint.color = COLOR_ORANGE
		paint.style = Paint.Style.FILL
		rectf = RectF(
			iGearLeft.toFloat(),
			iGearTop.toFloat(),
			iGearRight.toFloat(),
			iGearBottom.toFloat()
		)
		canvas.drawRoundRect(rectf, 28f, 28f, paint)

		if (Vsd != null) {
			// Vsd.iTacho 針
			paint.color = Color.RED
			paint.strokeWidth = 10f

			val dTachoAngle =
				Math.toRadians(iStartAngle + (iSweepAngle * iTacho / (iMeterMaxRev * 1000.0)))
			canvas.drawLine(
				iMeterCx.toFloat(), iMeterCy.toFloat(),
				(iMeterCx + (cos(dTachoAngle) * fMeterNeedleR).toInt()).toFloat(),
				(iMeterCy + (sin(dTachoAngle) * fMeterNeedleR).toInt()).toFloat(),
				paint
			)

			// スピード
			paint.color = Color.WHITE
			s = (Vsd.iSpeedRaw / 100).toString()
			paint.textSize = 180f
			canvas.drawText(s, (BASE_WIDTH - paint.measureText(s)) / 2, 270f, paint)

			// ギア
			paint.color = Color.BLACK
			paint.textSize = iGearFontSize.toFloat()
			s = iGear.toString()
			canvas.drawText(
				s,
				(iGearRight + iGearLeft - paint.measureText(s)) / 2,
				((iGearTop + iGearBottom + (iGearFontSize * 0.7).toInt()) / 2).toFloat(),
				paint
			)

			paint.setTypeface(Typeface.DEFAULT)
			// Lap計測モード
			when (Vsd.iMainMode) {
				VsdInterface.MODE_LAPTIME -> s = "Lap"
				VsdInterface.MODE_GYMKHANA -> s = "Gymkhana"
				VsdInterface.MODE_ZERO_FOUR -> s = "0-400m"
				VsdInterface.MODE_ZERO_ONE -> s = "0-100km/h"
			}

			if (Vsd.iRtcPrevRaw == -1) {
				// まだスタートしてない
				s += " ready"
			}

			// Lap 数
			paint.color = Color.GRAY
			paint.textSize = 32f
			canvas.drawText(s, fLapX, 355f, paint)
			paint.textSize = 64f
			canvas.drawText(String.format(Locale.US, "%02d", Vsd.GetLapNum()), fLapX, 410f, paint)

			// Lap タイム
			if(fLapTimeX == 0f){
				fLapTimeX = BASE_WIDTH - fLapX - paint.measureText("0'00.000")
			}
			paint.color = if (Vsd.GetLastLapTime() == 0) Color.GRAY else if (Vsd.GetLastLapTime() <= Vsd.iTimeBestRaw) Color.CYAN else Color.RED
			canvas.drawText(VsdInterface.FormatTime(Vsd.GetLastLapTime()), fLapTimeX, 410f, paint)
			paint.color = Color.GRAY
			canvas.drawText(VsdInterface.FormatTime(Vsd.iTimeBestRaw), fLapTimeX, 470f, paint)

			// GPS ステータス
			if (Vsd.GpsData != null) {
				paint.color = Color.WHITE
				paint.textSize = 32f
				canvas.drawText("🛰", 0f, 468f, paint)
			}

			if (Setting?.bDebugInfo ?: false) {
				// デバッグ用
				var y = 0
				paint.color = Color.CYAN
				paint.textSize = 30f
				canvas.drawText(
					String.format(
						Locale.US,
						"Throttle: %d (%d - %d)",
						Vsd.iThrottleRaw + Vsd.iThrottle0,
						Vsd.iThrottle0,
						Vsd.iThrottleMax + Vsd.iThrottle0
					),
					0f, 30.let { y += it; y }.toFloat(), paint
				)
				canvas.drawText(
					String.format(Locale.US, "Throttle(%%): %.1f", Vsd.iThrottle / 10.0),
					0f,
					30.let { y += it; y }.toFloat(),
					paint
				)
				canvas.drawText(
					String.format(
						Locale.US,
						"Gx: %+.2f Gy: %+.2f",
						Vsd.iGx / 4096.0,
						Vsd.iGy / 4096.0
					), 0f, 30.let { y += it; y }.toFloat(), paint
				)
				canvas.drawText(
					String.format(Locale.US, "Batt: %d", Setting?.iBattery ?: 0),
					0f,
					30.let { y += it; y }.toFloat(),
					paint
				)
				if (Vsd.GpsData != null) {
					canvas.drawText(
						String.format(
							Locale.US,
							"GPS: %.8f, %8f",
							Vsd.GpsData!!.dLong,
							Vsd.GpsData!!.dLati
						), 0f, 30.let { y += it; y }.toFloat(), paint
					)
				}
			}
		}

		paint.setTypeface(Typeface.DEFAULT)

		// バッテリー
		if ((Setting?.iBattery ?: 0) <= 30) {
			paint.color = Color.RED
			canvas.drawLine(0f, (480 - 480 * (Setting?.iBattery ?: 0) / 30).toFloat(), 0f, 480f, paint)
		}

		// 時計
		val cal = Calendar.getInstance()
		s = String.format(
			Locale.US,
			"%02d:%02d", cal[Calendar.HOUR_OF_DAY],
			cal[Calendar.MINUTE]
		)
		paint.color = Color.GRAY
		paint.textSize = 64f
		canvas.drawText(s, 0f, paint.textSize, paint)

		// メッセージログ
		if (bMsgLogShow) {
			canvas.drawColor(-0x80000000) // 暗くする
			paint.textSize = 30f

			for (i in 0 until iMsgLogNum) {
				val msg =
					resources.getString(iMsgLog[(iMsgLogPtr + i - iMsgLogNum + MAX_MSG_LOG) % MAX_MSG_LOG])
				paint.color =
					if (msg.startsWith("[E")) Color.RED else if (msg.startsWith(
							"[W"
						)
					) Color.YELLOW else Color.WHITE
				canvas.drawText(msg, 0f, ((i + 1) * 30).toFloat(), paint)
			}
		}

		holder.unlockCanvasAndPost(canvas)
	}

	//////////////////////////////////////////////////////////////////////////
	
	companion object {
		private val bDebug: Boolean = BuildConfig.DEBUG

		// シフトインジケータの表示
		private const val dTachoBarScale = 0.66
		private val iTachoBarList = intArrayOf(
			(1336 * dTachoBarScale).toInt(),
			(800 * dTachoBarScale).toInt(),
			(600 * dTachoBarScale).toInt(),
			(472 * dTachoBarScale).toInt(),
			(388 * dTachoBarScale).toInt()
		)
		private const val iRevLimit = 6500

		//*** 描画 ***********************************************************
		
		private const val BASE_WIDTH			= 960
		private const val BASE_HEIGHT			= 480

		private const val iMeterCx				= BASE_WIDTH / 2
		private const val iMeterCy				= 310
		private const val iMeterR				= 265 // メータ目盛り円弧半径
		private const val fMeterScaleW			= (iMeterR * 0.256).toInt().toFloat() // メータ目盛り円弧幅
		private const val fMeterNeedleR			= (iMeterR * 1.1).toFloat() // 針
		private const val fMeterLineR1			= (iMeterR * (1 + 0.025) + fMeterScaleW * 0.5).toFloat() // 目盛り線
		private const val fMeterLineR2			= (iMeterR + fMeterScaleW * 0.5).toFloat() // 目盛り線
		private const val fMeterLineR3			= (iMeterR * (1 - 0.025) + fMeterScaleW * 0.5).toFloat() // 目盛り線
		private const val fMeterLineR4			= (iMeterR - fMeterScaleW * 0.1).toFloat() // 目盛り線
		private const val fMeterRedZoneW		= (iMeterR * 0.06).toInt().toFloat() // レッドゾーン円弧幅
		private const val fMeterRedZoneR		= (fMeterLineR2 - fMeterRedZoneW * 0.5).toFloat() // レッドゾーン円弧幅
		private const val iMeterFontSize		= 44 // メータ目盛り数値サイズ
		private const val iStartAngle			= 150 // 開始角
		private const val iSweepAngle			= 240 // 範囲角
		private const val iMeterMaxRev			= 7 // タコメータ最大値
		private const val iRedZoneSweepAngle	= ((1 - iRevLimit / (iMeterMaxRev * 1000.0)) * iSweepAngle).toInt()
		private const val iRedZoneStartAngle	= iStartAngle + iSweepAngle - iRedZoneSweepAngle
		private const val iGearLeft				= BASE_WIDTH - 130
		private const val iGearTop				= 13
		private const val iGearRight			= BASE_WIDTH - 12
		private const val iGearBottom			= 155
		private const val iGearFontSize			= 140
		private const val iSpeedUnderlineW		= 372
		private const val fLapX					= iMeterCx - 190f

		private const val COLOR_ORANGE = -0xa000
		private const val COLOR_PURPLE = -0xbbff58
		
		private const val MAX_MSG_LOG = 15
	}

	//////////////////////////////////////////////////////////////////////////
	// Pref class

	class ViewSetting {
		var bEcoMode: Boolean = false
		var iBattery: Int = 0
		var bDebugInfo: Boolean = false
	}
}
