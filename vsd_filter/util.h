#pragma once

static char *StringNewA(char *&szDst, const char *szSrc){
	if(szDst == szSrc) return(szDst);
	if(szDst) delete [] szDst;
	
	if(szSrc == nullptr || *szSrc == '\0'){
		return szDst = nullptr;
	}
	szDst = new char[strlen(szSrc) + 1];
	return strcpy(szDst, szSrc);
}

inline std::wstring MultiByteToWide(const char* text, UINT codePage) {
	if (text == nullptr || *text == '\0') {
		return {};
	}

	const int size = MultiByteToWideChar(
		codePage, 0,
		text, -1,
		nullptr, 0
	);

	if (size <= 0) {
		return {};
	}

	std::wstring result(static_cast<size_t>(size), L'\0');

	MultiByteToWideChar(
		codePage, 0,
		text, -1,
		result.data(), size
	);

	result.resize(static_cast<size_t>(size - 1));
	return result;
}

inline std::string WideToMultiByte(const wchar_t* text, UINT codePage) {
	if (text == nullptr || *text == L'\0') {
		return {};
	}

	const int size = WideCharToMultiByte(
		codePage, 0,
		text, -1,
		nullptr, 0,
		nullptr, nullptr
	);

	if (size <= 0) {
		return {};
	}

	std::string result(static_cast<size_t>(size), '\0');

	WideCharToMultiByte(
		codePage, 0,
		text, -1,
		result.data(), size,
		nullptr, nullptr
	);

	result.resize(static_cast<size_t>(size - 1));
	return result;
}

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
