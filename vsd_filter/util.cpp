#include "StdAfx.h"
#include "util.h"

//////////////////////////////////////////////////////////////////////////////
// 文字コード変換

std::wstring MultiByteToWide(const char* text, UINT codePage) {
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

std::string WideToMultiByte(const wchar_t* text, UINT codePage) {
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

//////////////////////////////////////////////////////////////////////////////
// ファイル名操作

std::string GetModuleFileNameDirectString(HMODULE hModule) {
	std::string path(MAX_PATH, '\0');

	while (true) {
		DWORD copied = ::GetModuleFileNameA(
			hModule,
			path.data(), // C++17以降。C++11/14なら &path[0]
			static_cast<DWORD>(path.size())
		);

		if (copied == 0) {
			throw std::runtime_error("GetModuleFileNameA failed. Error: " + std::to_string(::GetLastError()));
		}

		// バッファ内に収まった（copied は終端 '\0' を含まない文字数）
		if (copied < path.size()) {
			path.resize(copied);
			return path;
		}

		// copied == path.size() の場合はバッファ不足の可能性が高いため拡張して再試行
		if (::GetLastError() == ERROR_INSUFFICIENT_BUFFER || copied == path.size()) {
			path.resize(path.size() * 2);
			continue;
		}

		throw std::runtime_error("Unexpected error in GetModuleFileNameA. Error: " + std::to_string(::GetLastError()));
	}
}

namespace {

// GetFullPathNameW をバッファ拡張対応で呼び出すヘルパー
std::wstring ResolveFullPathW(const std::wstring& target) {
	if (target.empty()) {
		return L"";
	}

	DWORD needed = ::GetFullPathNameW(target.c_str(), 0, nullptr, nullptr);
	if (needed == 0) {
		return target; // 失敗時は元の文字列をフォールバック
	}

	std::wstring buf(needed, L'\0');
	DWORD written = ::GetFullPathNameW(target.c_str(), needed, buf.data(), nullptr);
	if (written == 0) {
		return target;
	}

	// written は終端 L'\0' を含まない文字数
	if (written < buf.size()) {
		buf.resize(written);
	} else {
		// 途中でパス長が変わった場合などの念のため再試行
		while (written >= buf.size()) {
			buf.resize(buf.size() * 2);
			written = ::GetFullPathNameW(
				target.c_str(),
				static_cast<DWORD>(buf.size()),
				buf.data(),
				nullptr
			);
			if (written == 0) {
				return target;
			}
		}
		buf.resize(written);
	}

	return buf;
}

// filename を cdir 基準で解決する（カレントディレクトリを汚さないスレッド安全な方式）
std::wstring MakeFullPathWithBase(const std::wstring& filename, const std::wstring& cdir) {
	if (cdir.empty()) {
		return ResolveFullPathW(filename);
	}

	// filename がすでにドライブ指定やルート/UNCパスなら cdir を基準にしない
	bool hasDrive = (filename.size() >= 2 && filename[1] == L':');
	bool isRootedOrUnc = (!filename.empty() && (filename[0] == L'\\' || filename[0] == L'/'));

	if (hasDrive || isRootedOrUnc) {
		return ResolveFullPathW(filename);
	}

	std::wstring combined = cdir;
	if (!combined.empty() && combined.back() != L'\\' && combined.back() != L'/') {
		combined.push_back(L'\\');
	}
	combined += filename;

	return ResolveFullPathW(combined);
}

} // namespace

std::wstring StrTokFile(const std::wstring& filename, UINT mode, const std::wstring& cdir) {
	std::wstring source = filename;

	if (mode & STF_FULL) {
		source = MakeFullPathWithBase(source, cdir);
	}

	if (source.empty()) {
		return L"";
	}

	// 1. ドライブ部分（例: "C:"）
	size_t drvEnd = 0;
	if (source.size() >= 2 && source[1] == L':') {
		drvEnd = 2;
	}

	// 2. パス部分（ドライブの後ろから最後の '\\' の直後まで）
	size_t lastSlash = source.find_last_of(L'\\');
	size_t pathStart = drvEnd;
	size_t pathEnd = (lastSlash != std::wstring::npos && lastSlash >= drvEnd)
		? (lastSlash + 1)
		: drvEnd;

	// 3. ノード名（パス部分の後ろから最後の '.' の直前まで）
	size_t nodeStart = pathEnd;
	size_t lastDot = source.find_last_of(L'.');
	size_t extStart = (lastDot != std::wstring::npos && lastDot >= nodeStart)
		? lastDot
		: source.size();

	const UINT partMask = STF_DRV | STF_PATH | STF_NODE | STF_EXT;

	// パーツ指定フラグがひとつもない場合は全体を返す（元コードの _tcscpy(pDst, pSce) 相当）
	if ((mode & partMask) == 0) {
		return source;
	}

	std::wstring result;
	result.reserve(source.size());

	if (mode & STF_DRV) {
		result.append(source, 0, drvEnd);
	}
	if (mode & STF_PATH) {
		result.append(source, pathStart, pathEnd - pathStart);
	}
	if (mode & STF_NODE) {
		result.append(source, nodeStart, extStart - nodeStart);
	}
	if (mode & STF_EXT) {
		result.append(source, extStart, source.size() - extStart);
	}

	return result;
}

//////////////////////////////////////////////////////////////////////////////
// ファイルリスト取得

// szBasePath: 探索開始ディレクトリ（関数内で末尾に '\' を付与して正規化します）
// szPattern : 検索パターン（例: "*", "*.txt"）
// CallbackFunc(現在のディレクトリパス(末尾 '\' 付き), ファイル名, pParam)
//   戻り値が false の場合は探索を中断します

BOOL ListTree(
	std::string szBasePath,
	const std::string& szPattern,
	BOOL (*CallbackFunc)(const std::string&, const std::string&, void*),
	void* pParam)
{
	// szBasePath が '\' で終わっていない場合の対処
	if (!szBasePath.empty() && szBasePath.back() != '\\') {
		szBasePath.push_back('\\');
	}

	const std::string searchPath = szBasePath + szPattern;

	WIN32_FIND_DATAA FindData{};
	const HANDLE hFindFile = FindFirstFileA(searchPath.c_str(), &FindData);

	BOOL bSuccess = TRUE;

	if (hFindFile != INVALID_HANDLE_VALUE) {
		do {
			const char* fileName = FindData.cFileName;

			// '_' で始まる名前、"."、".." を除外
			if (fileName[0] != '_' &&
				strcmp(fileName, ".") != 0 &&
				strcmp(fileName, "..") != 0) {

				if (FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
					std::string subDir = szBasePath + fileName;
					if (!ListTree(std::move(subDir), szPattern, CallbackFunc, pParam)) {
						bSuccess = FALSE;
						break;
					}
				} else {
					if (CallbackFunc && !CallbackFunc(szBasePath, fileName, pParam)) {
						bSuccess = FALSE;
						break;
					}
				}
			}
		} while (FindNextFileA(hFindFile, &FindData));

		FindClose(hFindFile);
	}

	return bSuccess;
}

