//			enum name
//				|							変数名
//				|							|						初期値
//				|							|						|				config 名
//				|							|						|				|
DEF_STR_PARAM(	STRPARAM_LOGFILE,			m_szLogFile,			nullptr,		"log_file"	)			// VSD ログ
DEF_STR_PARAM(	STRPARAM_LOGFILE_READER,	m_szLogFileReader,		nullptr,		"log_file_reader"	)	// VSD ログ
DEF_STR_PARAM(	STRPARAM_GPSFILE,			m_szGPSLogFile,			nullptr,		"gps_file"	)			// GPS ログ
DEF_STR_PARAM(	STRPARAM_GPSFILE_READER,	m_szGPSLogFileReader,	nullptr,		"gps_file_reader"	)	// GPS ログ
DEF_STR_PARAM(	STRPARAM_LAPCHART,			m_szLapChart,			nullptr,		"lap_chart"	)			// LAP チャート
DEF_STR_PARAM(	STRPARAM_SKINFILE,			m_szSkinFile,			DEFAULT_SKIN,	"skin_file"	)			// スキン名

#undef DEF_STR_PARAM
