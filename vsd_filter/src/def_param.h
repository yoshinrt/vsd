#ifndef DEF_PARAM_STR
	#define DEF_PARAM_STR(...)
#endif
#ifndef DEF_PARAM_TRK
	#define DEF_PARAM_TRK(...)
#endif
#ifndef DEF_PARAM_CHK
	#define DEF_PARAM_CHK(...)
#endif
#ifndef DEF_PARAM_SDW
	#define DEF_PARAM_SDW(...)
#endif


//				config 名			type			変数名					初期値	Avu表示名			スライダmin,max
DEF_PARAM_STR(	log_file,			std::string,	m_strLogFile,			""											)	// VSD ログ
DEF_PARAM_STR(	log_file_reader,	std::string,	m_strLogFileReader,		""											)	// VSD ログ
DEF_PARAM_STR(	gps_file,			std::string,	m_strGPSLogFile,		""											)	// GPS ログ
DEF_PARAM_STR(	gps_file_reader,	std::string,	m_strGPSLogFileReader,	""											)	// GPS ログ
DEF_PARAM_STR(	lap_chart,			std::string,	m_strLapChart,			""											)	// LAP チャート
DEF_PARAM_STR(	skin_file,			std::string,	m_strSkinFile,			""											)	// スキン名

DEF_PARAM_TRK(	video_start,		int,			m_iVideoStart,			0,		"Video先頭",		0,		9999	)
DEF_PARAM_TRK(	video_end,			int,			m_iVideoEnd,			0,		"Video最後",		0,		9999	)
DEF_PARAM_TRK(	log_start,			int,			m_iVsdStart,			0,		"ログ先頭",			0,		9999	)
DEF_PARAM_TRK(	log_end,			int,			m_iVsdEnd,				0,		"ログ最後",			0,		9999	)
DEF_PARAM_TRK(	gps_start,			int,			m_iGpsStart,			0,		"GPS先頭",			0,		9999	)
DEF_PARAM_TRK(	gps_end,			int,			m_iGpsEnd,				0,		"GPS最後",			0,		9999	)
DEF_PARAM_TRK(	map_length,			int,			m_iMapLength,			240,	"走行軌跡長さ",		0,		1000	)
DEF_PARAM_TRK(	map_angle,			int,			m_iMapAngle,			0,		"走行軌跡回転",		0,		3600	)
DEF_PARAM_TRK(	start_width,		int,			m_iStartWidth,			200,	"計測地点幅",		-1,		1000	)

DEF_PARAM_CHK(	lap_time,			bool,			m_bShowLapTime,			1,		"ラップタイム表示"					)
DEF_PARAM_CHK(	graph,				bool,			m_bShowGraph,			0,		"グラフ表示"						)
DEF_PARAM_CHK(	prefer_gps,			bool,			m_bPreferGps,			0,		"GPSログ優先"						)

DEF_PARAM_SDW(	lap_chart_start,	int,			m_iLapChartStart,		0											)
DEF_PARAM_SDW(	lap_chart_end,		int,			m_iLapChartEnd,			0											)
DEF_PARAM_SDW(	lap_start,			int,			m_iLapStart,			0											)	// ラップ開始
DEF_PARAM_SDW(	lap_end,			int,			m_iLapEnd,				INT_MAX										)	// ラップ終了
DEF_PARAM_SDW(	saving,				bool,			m_bSaving,				0											)	// avs 用 IsSaving 値

#undef DEF_PARAM_STR
#undef DEF_PARAM_TRK
#undef DEF_PARAM_CHK
#undef DEF_PARAM_SDW
