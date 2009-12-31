//			enum name
//				|					変数名
//				|					|				初期値
//				|					|				|			config 名
//				|					|				|				|
DEF_STR_PARAM(	STRPARAM_LOGFILE,	m_szLogFile,	"",				"log_file"	)	// VSD ログ
DEF_STR_PARAM(	STRPARAM_GPSFILE,	m_szGPSLogFile,	"",				"gps_file"	)	// GPS ログ
DEF_STR_PARAM(	STRPARAM_FONT,		m_szFontName,	DEFAULT_FONT,	"font"		)	// フォント名

#undef DEF_STR_PARAM
