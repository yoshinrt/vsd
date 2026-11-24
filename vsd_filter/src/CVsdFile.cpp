/*****************************************************************************
	
	VSD -- vehicle data logger system  Copyright(C) by DDS
	
	CVsdFile.cpp - CVsdFile class implementation
	
*****************************************************************************/

#include "StdAfx.h"
#include "CVsdFile.h"
#include "CScript.h"
#include "util.h"

#define JsErrorClosedHandle()		JsError("operation not permitted on closed handle")
#define JsErrorZipNotSupported()	JsError("operation not supported on zip-mode")
#define JsErrorZipNotOpened()		JsError("file in zip not opened, execute ZipNextFile() first.")

/*** ファイルオープン *******************************************************/

int CVsdFile::Open(const std::string& file, const std::string& mode){
	
	// gz open
	if(mode.front() == 'z'){
		m_gzfp = gzopen_w(Utf8ToWide(file).c_str(), mode.substr(1).c_str());
		
		if(m_gzfp) m_uMode = MODE_GZIP;
	}
	
	// zip open
	else if(mode.front() == 'Z'){
		m_unzfp = unzOpen(Utf8ToAnsi(file).c_str());
		if(m_unzfp) m_uMode = MODE_ZIP;
	}
	
	// 普通 fopen
	else{
		m_fp = _wfopen(Utf8ToWide(file).c_str(), Utf8ToWide(mode).c_str());
	}
	
	if(m_fp == nullptr) JsError(ERR_CANT_OPEN_FILE);
	return 0;
}

/*** ファイルクローズ *******************************************************/

void CVsdFile::Close(void){
	if(m_fp) switch(m_uMode){
		case MODE_NORMAL:		fclose(m_fp);
		Case MODE_GZIP:			gzclose(m_gzfp);
		
		Case MODE_ZIP_OPENED:	unzCloseCurrentFile(m_unzfp);
		case MODE_ZIP:			unzClose(m_unzfp);
	}
	m_fp = nullptr;
}

/*** ラインリード ***********************************************************/

std::string CVsdFile::ReadLine(void){
	*m_cBuf = '\0';
	
	if(!m_fp){
		JsErrorClosedHandle();
		return {};
	}
	
	switch(m_uMode){
		case MODE_NORMAL:
			fgets(m_cBuf, FILEBUF_LEN, m_fp);
			return m_cBuf;

		case MODE_GZIP:
			gzgets(m_gzfp, m_cBuf, FILEBUF_LEN);
			return m_cBuf;
		
		case MODE_ZIP: {
			JsErrorZipNotOpened();
			return {};
		}
		
		case MODE_ZIP_OPENED : {

			// \n サーチ
			char* p = (char*)memchr(m_cBuf + m_uBufPtr, '\n', m_uBufSize - m_uBufPtr);

			// 見つからないので，buf read する
			if (!p) {
				ReadZip();
				p = (char*)memchr(m_cBuf + m_uBufPtr, '\n', m_uBufSize - m_uBufPtr);

				// 再び見つからなかったので，p は buf 終端を指す
				if (!p) p = m_cBuf + m_uBufSize - 1;
			}

			// 文字列長付きで String 変換
			std::string ZipBuf(m_cBuf + m_uBufPtr, p - (m_cBuf + m_uBufPtr));
			m_uBufPtr = (UINT)(p - m_cBuf + 1);
			return ZipBuf;
		}
	}
	return {};
}

/*** ラインライト ***********************************************************/

int CVsdFile::WriteLine(const std::string& s){
	
	if(m_fp) switch(m_uMode){
		case MODE_NORMAL:		return fputs(s.c_str(), m_fp);
		case MODE_GZIP:			return gzputs(m_gzfp, s.c_str());
		
		case MODE_ZIP:
		case MODE_ZIP_OPENED:	JsErrorZipNotSupported();
	}
	else JsErrorClosedHandle();
	
	return 0;
}

/*** seek *******************************************************************/

int CVsdFile::Seek(int iOffs, int iOrg){
	
	if(m_fp) switch(m_uMode){
		case MODE_NORMAL:		return fseek(m_fp, iOffs, iOrg);
		case MODE_GZIP:			return gzseek(m_gzfp, iOffs, iOrg) >= 0 ? 0 : -1;
		
		case MODE_ZIP:
		case MODE_ZIP_OPENED:	JsErrorZipNotSupported();
	}
	else JsErrorClosedHandle();
	
	return 0;
}

/*** バイナリリード *********************************************************/

UCHAR *CVsdFile::ReadBin(int iSize){
	static UCHAR cBuf[8];
	
	*(int *)cBuf = *(int *)(cBuf + 4) = 0;
	
	if(m_fp)	switch(m_uMode){
		case MODE_NORMAL:		fread(cBuf, 1, iSize, m_fp);
		Case MODE_GZIP:			gzread(m_gzfp, cBuf, iSize);
		Case MODE_ZIP:			JsErrorZipNotOpened();
		Case MODE_ZIP_OPENED: {
			// buf 残量が足りなかったら，読む
			if(iSize > (int)(m_uBufSize - m_uBufPtr)) ReadZip();
			if(iSize > (int)(m_uBufSize - m_uBufPtr)) iSize = m_uBufSize - m_uBufPtr;
			
			// リードバッファにコピー
			memcpy(cBuf, m_cBuf + m_uBufPtr, iSize);
			m_uBufPtr += iSize;
		}
	}
	return cBuf;
}

/*** バイナリライト *********************************************************/

int CVsdFile::WriteBin(void *pBuf, int iSize){
	if(m_fp) switch(m_uMode){
		case MODE_NORMAL:		return (int)fwrite(pBuf, 1, iSize, m_fp);
		case MODE_GZIP:			return gzread(m_gzfp, pBuf, iSize);
		
		case MODE_ZIP:
		case MODE_ZIP_OPENED:	JsErrorZipNotSupported();
	}
	else JsErrorClosedHandle();
	return 0;
}

/*** eof チェック ***********************************************************/

int CVsdFile::IsEOF(void){
	if(m_fp) switch(m_uMode){
		case MODE_NORMAL:		return feof(m_fp);
		case MODE_GZIP:			return gzeof(m_gzfp);
		
		case MODE_ZIP:			JsErrorZipNotSupported();
		
		case MODE_ZIP_OPENED: return
			!(m_uFlag & FLAG_EOF) ? 0 :
			m_uBufPtr == m_uBufSize ? -1 : 0;
	}
	else JsErrorClosedHandle();
	return 0;
}

/*** zip 次のファイル *******************************************************/

std::string CVsdFile::ZipNextFile(void){
	unz_file_info fileInfo;
	
	// zip じゃなければ ret
	if(!(MODE_ZIP <= m_uMode && m_uMode <= MODE_ZIP_OPENED)){
		JsError("not opened by zip-mode");
		return {};
	}
	
	m_uFlag &= ~FLAG_EOF;	// eof クリア
	
	// ファイルを開いていたら，閉じる
	if(m_uMode == MODE_ZIP_OPENED){
		unzCloseCurrentFile(m_unzfp);
	}
	m_uMode = MODE_ZIP;
	
	while(1){
		if(m_uFlag & FLAG_2ND_FILE){
			// ファイル全部処理した
			if(unzGoToNextFile(m_unzfp) == UNZ_END_OF_LIST_OF_FILE){
				return {};
			}
		}
		m_uFlag |= FLAG_2ND_FILE;
		
		// 書庫内ファイル名取得
		if(
			unzGetCurrentFileInfo(
				m_unzfp, &fileInfo,
				m_cBuf, FILEBUF_LEN - 1,
				nullptr, 0, nullptr, 0
			) != UNZ_OK
		){
			JsError("zlib: unzGetCurrentFileInfo failed (maybe wrong zip format?)");
			return {};
		}
		
		// '/' で終わっていたら dir なのでスキップ
		int iLen = (int)strlen(m_cBuf) - 1;
		if(iLen >= 0 && m_cBuf[iLen] != '/') break;
	}
	
	// 書庫内ファイルオープン
	if(unzOpenCurrentFile(m_unzfp) == UNZ_OK){
		m_uMode = MODE_ZIP_OPENED;
		return m_cBuf;
	}
	
	JsError("zlib: can't open file in zip (maybe wrong zip format?)");
	return {};
}

/*** zip ファイルの body リード *********************************************/

int CVsdFile::ReadZip(void){
	
	// 残っているデータを配列先頭に move
	if(m_uBufPtr != 0 && m_uBufPtr != m_uBufSize){
		memmove(m_cBuf, m_cBuf + m_uBufPtr, m_uBufSize - m_uBufPtr);
	}
	m_uBufSize -= m_uBufPtr;
	m_uBufPtr = 0;
	
	// リード
	int iBufRemained = FILEBUF_LEN - m_uBufSize;
	int iReadSize = unzReadCurrentFile(m_unzfp, m_cBuf + m_uBufSize, iBufRemained);
	m_uBufSize += iReadSize;
	
	if(iReadSize < iBufRemained) m_uFlag |= FLAG_EOF;
	
	return iReadSize;
}
