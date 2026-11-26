#include "StdAfx.h"
#include "CVsdParam.h"

namespace {

bool IsSpace(char c) {
	return c == ' ' || c == '\t' || c == '\r' || c == '\n';
}

// ソース全体から VSDFilter( ... ) の中身を抽出する
bool ExtractVsdFilterBody(const std::string& src, std::string& outBody) {
	const std::string target = "VSDFilter";
	size_t pos = 0;

	while ((pos = src.find(target, pos)) != std::string::npos) {
		if (pos > 0) {
			unsigned char prev = static_cast<unsigned char>(src[pos - 1]);
			if (std::isalnum(prev) || prev == '_') {
				pos += target.size();
				continue;
			}
		}

		size_t i = pos + target.size();
		if (i < src.size()) {
			unsigned char next = static_cast<unsigned char>(src[i]);
			if (std::isalnum(next) || next == '_') {
				pos += target.size();
				continue;
			}
		}

		while (i < src.size() && IsSpace(src[i])) {
			++i;
		}

		if (i >= src.size() || src[i] != '(') {
			pos += target.size();
			continue;
		}

		++i;
		size_t start = i;
		bool inString = false;
		int depth = 1;

		for (; i < src.size(); ++i) {
			char c = src[i];
			if (c == '"') {
				inString = !inString;
			} else if (!inString) {
				if (c == '(') {
					++depth;
				} else if (c == ')') {
					--depth;
					if (depth == 0) {
						outBody = src.substr(start, i - start);
						return true;
					}
				}
			}
		}

		return false;
	}

	return false;
}

// "..." 文字列リテラルを読み込む（'\' はエスケープ文字ではない）
bool ParseStringLiteral(const std::string& s, size_t& pos, std::string& outStr) {
	if (pos >= s.size() || s[pos] != '"') {
		return false;
	}
	++pos;
	size_t start = pos;
	while (pos < s.size() && s[pos] != '"') {
		++pos;
	}
	if (pos >= s.size() || s[pos] != '"') {
		return false;
	}
	outStr = s.substr(start, pos - start);
	++pos;
	return true;
}

// 非文字列トークン（識別子や数値）を読み込む
bool ParsePlainToken(const std::string& s, size_t& pos, std::string& outTok) {
	size_t start = pos;
	while (pos < s.size()) {
		char c = s[pos];
		if (IsSpace(c) || c == '=' || c == ',' || c == '"') {
			break;
		}
		++pos;
	}
	if (pos == start) {
		return false;
	}
	outTok = s.substr(start, pos - start);
	return true;
}

bool ParseInt(const std::string& text, int& outVal) {
	if (text.empty()) return false;
	char* endPtr = nullptr;
	long v = std::strtol(text.c_str(), &endPtr, 10);
	if (endPtr == text.c_str() || *endPtr != '\0') {
		return false;
	}
	outVal = static_cast<int>(v);
	return true;
}

bool ParseBool(const std::string& text, bool& outVal) {
	if (text == "0") {
		outVal = false;
		return true;
	}
	if (text == "1") {
		outVal = true;
		return true;
	}
	return false;
}

void ParseMarkList(const std::string& markStr, std::vector<int>& outMark) {
	outMark.clear();
	if (markStr.empty()) {
		return;
	}
	std::stringstream ss(markStr);
	std::string item;
	while (std::getline(ss, item, ',')) {
		size_t b = 0;
		while (b < item.size() && IsSpace(item[b])) ++b;
		size_t e = item.size();
		while (e > b && IsSpace(item[e - 1])) --e;
		if (b < e) {
			int v = 0;
			if (ParseInt(item.substr(b, e - b), v)) {
				outMark.push_back(v);
			}
		}
	}
}

template <typename T>
bool AssignValue(T& dst, const std::string& rawVal, bool isQuoted);

template <>
bool AssignValue<std::string>(std::string& dst, const std::string& rawVal, bool isQuoted) {
	if (!isQuoted) {
		return false;
	}
	dst = rawVal;
	return true;
}

template <>
bool AssignValue<int>(int& dst, const std::string& rawVal, bool) {
	int iv = 0;
	if (!ParseInt(rawVal, iv)) {
		return false;
	}
	dst = iv;
	return true;
}

template <>
bool AssignValue<bool>(bool& dst, const std::string& rawVal, bool) {
	bool bv = false;
	if (!ParseBool(rawVal, bv)) {
		return false;
	}
	dst = bv;
	return true;
}

template <typename T>
std::string FormatValue(const T& val);

template <>
std::string FormatValue<std::string>(const std::string& val) {
	return "\"" + val + "\"";
}

template <>
std::string FormatValue<int>(const int& val) {
	return std::to_string(val);
}

template <>
std::string FormatValue<bool>(const bool& val) {
	return val ? "1" : "0";
}

bool AssignByConfigName(
	CVsdParam& self,
	const std::string& name,
	const std::string& rawVal,
	bool isQuoted)
{
	#define DEF_PARAM_STR(cfg_name, type, var_name, init, ...) \
		if (name == (#cfg_name)) { \
			AssignValue(self.var_name, rawVal, isQuoted); \
			return true; \
		}
	#define DEF_PARAM_TRK DEF_PARAM_STR
	#define DEF_PARAM_CHK DEF_PARAM_STR
	#define DEF_PARAM_SDW DEF_PARAM_STR
	#include "def_param.h"

	return false;
}

void AppendDiffLines(const CVsdParam& self, std::vector<std::string>& lines) {
	#define DEF_PARAM_STR(cfg_name, type, var_name, init, ...) \
		if (self.var_name != static_cast<type>(init)) { \
			lines.push_back(std::string("\t") + (#cfg_name) + "=" + FormatValue(self.var_name)); \
		}
	#define DEF_PARAM_TRK DEF_PARAM_STR
	#define DEF_PARAM_CHK DEF_PARAM_STR
	#define DEF_PARAM_SDW DEF_PARAM_STR
	#include "def_param.h"
}

} // namespace

void CVsdParam::Initialize(){
	#define DEF_PARAM_STR(name, type, var, init, ...)	var = init;
	#define DEF_PARAM_TRK	DEF_PARAM_STR
	#define DEF_PARAM_CHK	DEF_PARAM_STR
	#define DEF_PARAM_SDW	DEF_PARAM_STR
	#include "def_param.h"
	
	m_Mark.clear();
}

bool CVsdParam::Load(const char* szCfgFile) {
	if (!szCfgFile) {
		return false;
	}

	std::ifstream ifs(szCfgFile, std::ios::in | std::ios::binary);
	if (!ifs.is_open()) {
		return false;
	}

	std::ostringstream buf;
	buf << ifs.rdbuf();
	std::string content = buf.str();

	std::string body;
	if (!ExtractVsdFilterBody(content, body)) {
		return false;
	}

	size_t pos = 0;
	const size_t n = body.size();
	
	Initialize();
	
	while (pos < n) {
		while (pos < n && (IsSpace(body[pos]) || body[pos] == ',')) {
			++pos;
		}
		if (pos >= n) {
			break;
		}

		std::string name;
		if (!ParsePlainToken(body, pos, name)) {
			++pos;
			continue;
		}

		while (pos < n && IsSpace(body[pos])) {
			++pos;
		}
		if (pos >= n || body[pos] != '=') {
			continue;
		}
		++pos;

		while (pos < n && IsSpace(body[pos])) {
			++pos;
		}
		if (pos >= n) {
			break;
		}

		bool isQuoted = (body[pos] == '"');
		std::string rawVal;
		if (isQuoted) {
			if (!ParseStringLiteral(body, pos, rawVal)) {
				return false;
			}
		} else {
			if (!ParsePlainToken(body, pos, rawVal)) {
				return false;
			}
		}

		if (name == "mark") {
			ParseMarkList(rawVal, m_Mark);
			continue;
		}

		AssignByConfigName(*this, name, rawVal, isQuoted);
	}

	return true;
}

bool CVsdParam::Save(const char* szCfgFile, const char* szMovieFile, int fpsDivident, int fpsDivisor) const {
	if (!szCfgFile || !szMovieFile || fpsDivisor == 0) {
		return false;
	}

	std::ofstream ofs(szCfgFile, std::ios::out | std::ios::binary | std::ios::trunc);
	if (!ofs.is_open()) {
		return false;
	}

	std::vector<std::string> lines;
	AppendDiffLines(*this, lines);

	if (!m_Mark.empty()) {
		std::ostringstream markBuf;
		markBuf << "\tmark=\"";
		for (size_t i = 0; i < m_Mark.size(); ++i) {
			if (i > 0) {
				markBuf << ",";
			}
			markBuf << m_Mark[i];
		}
		markBuf << "\"";
		lines.push_back(markBuf.str());
	}

	ofs << std::format(
		"DirectShowSource( \"{}\", pixel_type=\"YUY2\", fps={}.0/{} )\nVSDFilter( \\\n",
		szMovieFile,
		fpsDivident,
		fpsDivisor
	);
	for (size_t i = 0; i < lines.size(); ++i) {
		ofs << lines[i];
		if (i + 1 < lines.size()) {
			ofs << ",";
		}
		ofs << " \\\n";
	}
	ofs << ")\n";

	return ofs.good();
}
