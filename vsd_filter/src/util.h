#pragma once

std::wstring MultiByteToWide(const char* text, UINT codePage) ;
std::string WideToMultiByte(const wchar_t* text, UINT codePage) ;

// ANSI -> Wide
inline std::wstring AnsiToWide(const char* text) { return MultiByteToWide(text, CP_ACP); }
inline std::wstring AnsiToWide(const std::string& text) { return AnsiToWide(text.c_str()); }

// Wide -> ANSI
inline std::string WideToAnsi(const wchar_t* text) { return WideToMultiByte(text, CP_ACP); }
inline std::string WideToAnsi(const std::wstring& text) { return WideToAnsi(text.c_str()); }

// UTF-8 -> Wide
inline std::wstring Utf8ToWide(const char* text) { return MultiByteToWide(text, CP_UTF8); }
inline std::wstring Utf8ToWide(const std::string& text) { return Utf8ToWide(text.c_str()); }

// Wide -> UTF-8
inline std::string WideToUtf8(const wchar_t* text) { return WideToMultiByte(text, CP_UTF8); }
inline std::string WideToUtf8(const std::wstring& text) { return WideToUtf8(text.c_str()); }

// ANSI -> UTF-8
inline std::string AnsiToUtf8(const char* text) { return WideToUtf8(AnsiToWide(text)); }
inline std::string AnsiToUtf8(const std::string& text) { return AnsiToUtf8(text.c_str()); }

// UTF-8 -> ANSI
inline std::string Utf8ToAnsi(const char* text) { return WideToAnsi(Utf8ToWide(text)); }
inline std::string Utf8ToAnsi(const std::string& text) { return Utf8ToAnsi(text.c_str()); }

std::string GetModuleFileNameDirectString(HMODULE hModule = nullptr);

enum{
	STF_DRV		= 1 << 0,
	STF_PATH	= 1 << 1,
	STF_NODE	= 1 << 2,
	STF_EXT		= 1 << 3,
	STF_SHORT	= 1 << 4,
	STF_LONG	= 1 << 5,
	STF_FULL	= 1 << 6,
	
	STF_NAME	= STF_NODE | STF_EXT,
	STF_PATH2	= STF_DRV  | STF_PATH
};

std::wstring StrTokFile(const std::wstring& filename, UINT mode, const std::wstring& cdir = L"");

inline std::string StrTokFile(const std::string& filename, UINT mode, const std::string& cdir = ""){
	return WideToAnsi(
		StrTokFile(
			AnsiToWide(filename),
			mode,
			AnsiToWide(cdir)
		)
	);
}

bool IsExt(const std::string& file, const std::string& ext);

BOOL ListTree(
	std::string szBasePath,
	const std::string& szPattern,
	BOOL (*CallbackFunc)(const std::string&, const std::string&, void*),
	void* pParam
);

class CPushDir {
public:
	CPushDir(const char* szPath) {
		int iBufSize = GetCurrentDirectoryA(0, nullptr);
		m_szCurDir.resize(iBufSize, '\0');
		GetCurrentDirectoryA(iBufSize, &m_szCurDir[0]);
		SetCurrentDirectoryA(szPath);
	}

	~CPushDir() {
		SetCurrentDirectoryA(m_szCurDir.c_str());
	}

private:
	std::string m_szCurDir;
};
